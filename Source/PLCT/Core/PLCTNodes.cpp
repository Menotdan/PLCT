#include "PLCTNodes.h"
#include "Engine/Debug/DebugDraw.h"
#include "Engine/Core/Random.h"
#include "Engine/Core/RandomStream.h"
#include "Engine/Core/Delegate.h"
#include "Engine/Threading/JobSystem.h"
#include "Engine/Platform/Types.h"
#include "../Level/PLCTVolume.h"
#include "Engine/Level/Prefabs/PrefabManager.h"
#include "Engine/Level/Prefabs/Prefab.h"
#include "Engine/Physics/PhysicalMaterial.h"

#include "PLCTProperties.h"
#include "PLCTTypes.h"
#include "Surface/BoxColliderSurface.h"
#include "Surface/TerrainSurface.h"
#include "Engine/Scripting/Internal/MainThreadManagedInvokeAction.h"

bool PLCTSampleSurface::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    CACHE_READ(Arch2RuntimeCache, Points);

    PLCTSurfaceList* surfaces;

    VisjectGraphBox box = node.Boxes[0];
    PLCTGraphNode* connectedNode;
    ScriptingObject* object;

    if (!GetObjectFromInputBox(box, connectedNode, volume, object))
        return false;

    CHECK_RETURN(object, false);
    if (!object->Is<PLCTSurfaceList>())
        return false;

    surfaces = (PLCTSurfaceList*) object;
    SurfaceSampler* sampler = New<SurfaceSampler>();
    sampler->Configure(Settings);
    sampler->SetSurfaceList(surfaces);

    PLCTPointsContainer* points = sampler->SampleXZ();
    CHECK_RETURN(points, false);
    Delete(sampler);
    sampler = nullptr;

    CACHE_WRITE(Arch2RuntimeCache, Points, points);
    output = Variant(points);
    LOG(Info, "[PLCT] Sampled {0} points.", points->GetPoints().Count());
    return true;
}

bool PLCTGetBoxColliderSurfaces::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    CACHE_READ(Arch0RuntimeCache, SurfaceList);

    BoxColliderSurface* baseInstance = New<BoxColliderSurface>();
    PLCTSurfaceList* surfaces = volume->FindAllSurfaces(baseInstance);
    if (surfaces == nullptr)
    {
        SAFE_DELETE(baseInstance);
        output = Variant(nullptr);
        return false;
    }

    if (surfaces->GetSurfaces().Count() == 0)
    {
        SAFE_DELETE(surfaces);
        SAFE_DELETE(baseInstance);
        output = Variant(nullptr);
        return false;
    }
    SAFE_DELETE(baseInstance);

    CACHE_WRITE(Arch0RuntimeCache, SurfaceList, surfaces);
    output = Variant(surfaces);
    return true;
}

bool PLCTGetTerrainSurfaces::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    CACHE_READ(Arch0RuntimeCache, SurfaceList);

    TerrainSurface* baseInstance = New<TerrainSurface>();
    PLCTSurfaceList* surfaces = volume->FindAllSurfaces(baseInstance);
    CHECK_RETURN(surfaces, false);
    SAFE_DELETE(baseInstance);

    CACHE_WRITE(Arch0RuntimeCache, SurfaceList, surfaces);
    output = Variant(surfaces);
    return true;
}

void DebugDrawer(PLCTPointsContainer* points, Color PointColor)
{
    for (int i = 0; i < points->GetPoints().Count(); i++)
    {
#if USE_EDITOR
        DebugDraw::DrawSphere(BoundingSphere(points->GetPoints()[i]->GetTransform().Translation, 2), PointColor, 20.0f);
#endif
    }
}

bool PLCTDebugDrawPoints::Execute(PLCTGraphNode& node, PLCTVolume* volume)
{
    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    CHECK_RETURN(points, false);
    MainThreadManagedInvokeAction::ParamsBuilder params;
    params.AddParam(points);
    params.AddParam(PointColor);
    MainThreadManagedInvokeAction::Invoke((void*)DebugDrawer, params);

    return true;
}

#define PREFAB_SPAWN_JOBS 1
struct PrefabToSpawn
{
    Transform transform;
    PrefabSpawnEntry* prefab;
};

class SpawnPrefabJob
{
public:
    CriticalSection* Lock;
    RandomStream* stream;
    PLCTPointsContainer* points;
    PLCTVolume* volume;
    Array<PrefabSpawnEntry>* Prefabs;
    Array<PrefabToSpawn> Spawn;

    ~SpawnPrefabJob()
    {
        Spawn.Clear();
    }

public:
    void SpawnPrefabThread(int32 id)
    {
        Lock->Lock();
        int32 pointCount = points->GetPoints().Count();
        int32 prefabCount = Prefabs->Count();
        Lock->Unlock();

        for (int pointIdx = id; pointIdx < pointCount; pointIdx += PREFAB_SPAWN_JOBS)
        {
            Lock->Lock();
            float pickedPrefabNum = stream->GetFraction();
            Lock->Unlock();

            float totalRead = 0;
            PrefabSpawnEntry* entry = nullptr;

            for (int entryIdx = 0; entryIdx < prefabCount; entryIdx++)
            {
                totalRead += (*Prefabs)[entryIdx].Weight;
                if (totalRead >= pickedPrefabNum)
                {
                    entry = &((*Prefabs)[entryIdx]);
                    break;
                }
            }
            
            if (entry == nullptr)
            {
                continue;
            }

            Lock->Lock();
            Transform prefabTransform = points->GetPoints()[pointIdx]->GetTransform();
            Spawn.Add(PrefabToSpawn{ prefabTransform, entry });
            Lock->Unlock();
        }
    }
};

static Array<Actor*> FillAllChildrenRecursive(Actor* start, Array<Actor*>& results)
{
    for (auto child : start->Children)
    {
        if (results.Contains(child))
        {
            continue;
        }

        results.Add(child);
        FillAllChildrenRecursive(child, results);
    }

    return results;
}

void CopyData(PrefabSpawnEntry& entry)
{
    Actor* sourceActor = entry.CachedActor;
    Array<Actor*> actors;
    actors.Add(sourceActor);
    FillAllChildrenRecursive(sourceActor, actors);

    auto dataArr = Actor::ToBytes(actors);

    byte* data = (byte*)Platform::Allocate(dataArr.Count(), 1);
    Platform::MemoryCopy(data, dataArr.Get(), dataArr.Count());
    entry.CachedActorData = Span<byte>(data, dataArr.Count());;
}

Actor* Clone(PrefabSpawnEntry* entry)
{
    Span<byte>& dataSpan = entry->CachedActorData;
    
    auto serializedIds = Actor::TryGetSerializedObjectsIds(dataSpan);

    Dictionary<Guid, Guid> idMap;

    for(auto id : serializedIds)
    {
        idMap[id] = Guid::New();
    }

    auto cloned = Actor::FromBytes(dataSpan, idMap);
    return cloned[0];
}

bool PLCTSpawnPrefabAtPoints::Execute(PLCTGraphNode& node, PLCTVolume* volume)
{
    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    CONFIGURE_RAND();
    CHECK_RETURN(points, false);

    for (auto& entry : Prefabs)
    {
        if (!entry.Prefab || entry.Prefab->WaitForLoaded())
            continue;

        Actor* actor = PrefabManager::SpawnPrefab(entry.Prefab, (Actor*)volume->GenerationContainer.Get(), Transform::Identity);
        actor->BreakPrefabLink();

        entry.CachedActor = actor;
        CopyData(entry);
    }

    LOG(Info, "[PLCT] Spawning {0} prefabs.", points->GetPoints().Count());
    CriticalSection prefabSpawnLock;
    prefabSpawnLock.Unlock();
    SpawnPrefabJob* job = new SpawnPrefabJob();
    job->Lock = &prefabSpawnLock;
    job->points = points;
    job->Prefabs = &Prefabs;
    job->stream = &stream;
    job->volume = volume;

    Function<void(int32)> action;
    action.Bind<SpawnPrefabJob, &SpawnPrefabJob::SpawnPrefabThread>(job);
    JobSystem::Wait(JobSystem::Dispatch(action, PREFAB_SPAWN_JOBS));

    for (int i = 0; i < job->Spawn.Count(); i++)
    {
        Actor* actor = Clone(job->Spawn[i].prefab); //PrefabManager::SpawnPrefab(job->Spawn[i].prefab, (Actor*)volume->GenerationContainer.Get(), job->Spawn[i].transform);
        actor->SetTransform(job->Spawn[i].transform);
    }

    delete job;

    return true;
}

bool PLCTFilterByRandom::CheckPoint(PLCTPoint* point)
{
    Variant randomValue = point->GetProperties()->GetPropertyValue(TEXT("Random"));
    if (randomValue.Type == VariantType::Null)
        return false;

    if (randomValue.AsFloat < MinValue ||
        randomValue.AsFloat > MaxValue)
    {
        return false;
    }

    return true;
}

bool PLCTFilterByNormal::CheckPoint(PLCTPoint* point)
{
    Variant normalVector = point->GetProperties()->GetPropertyValue(TEXT("Normal"));
    if (normalVector.Type == VariantType::Null)
        return false;

    Vector3 normal = normalVector.AsVector3();
    if (normal.X > MaxValue.X || normal.X < MinValue.X
        || normal.Y > MaxValue.Y || normal.Y < MinValue.Y
        || normal.Z > MaxValue.Z || normal.Z < MinValue.Z)
    {
        return false;
    }

    return true;
}

void PLCTTransformPoints::TransformPoints(Transform& transform)
{
    transform.Translation += PositionOffset;
    transform.Orientation += RotationOffset;
    transform.Scale *= ScaleMultiplier;

    if (RandomizePosition)
    {
        Vector3 positionOffset = Vector3::Lerp(RandomPositionMinimum, RandomPositionMaximum, Random::Rand());
        transform.Translation += positionOffset;
    }

    if (RandomizeRotation)
    {
        Quaternion rotationOffset = Quaternion::Lerp(RandomRotationMinimum, RandomRotationMaximum, Random::Rand());
        transform.Orientation *= rotationOffset;
    }

    if (RandomizeScale)
    {
        Vector3 scaleMultiplier = Vector3::Lerp(RandomScaleMinimum, RandomScaleMaximum, Random::Rand());
        transform.Scale *= scaleMultiplier;
    }
}

bool PLCTTransformPoints::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    CACHE_READ(Arch2RuntimeCache, Points);

    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    PLCTPointsContainer* transformedPoints = New<PLCTPointsContainer>();

    CHECK_RETURN(points, false);
    for (int pointIdx = 0; pointIdx < points->GetPoints().Count(); pointIdx++)
    {
        PLCTPoint* point = points->GetPoints()[pointIdx];
        CHECK_RETURN(point, false);

        PLCTPoint* transformedPoint = point->Copy();

        Transform transform = transformedPoint->GetTransform();
        TransformPoints(transform);
        transformedPoint->SetTransform(transform);

        transformedPoints->GetPoints().Add(transformedPoint);
    }

    CACHE_WRITE(Arch2RuntimeCache, Points, transformedPoints);
    output = Variant(transformedPoints);
    return true;
}

void PLCTSetPointsTransform::TransformPoints(Transform& transform)
{
    if (SetPosition)
    {
        transform.Translation = Position;
    }

    if (SetRotation)
    {
        transform.Orientation = Rotation;
    }

    if (SetScale)
    {
        transform.Scale = Scale;
    }
}

bool PLCTSetPointsTransform::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    CACHE_READ(Arch2RuntimeCache, Points);

    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    PLCTPointsContainer* transformedPoints = New<PLCTPointsContainer>();

    CHECK_RETURN(points, false);
    for (int pointIdx = 0; pointIdx < points->GetPoints().Count(); pointIdx++)
    {
        PLCTPoint* point = points->GetPoints()[pointIdx];
        CHECK_RETURN(point, false);

        PLCTPoint* transformedPoint = point->Copy();

        Transform transform = transformedPoint->GetTransform();
        TransformPoints(transform);
        transformedPoint->SetTransform(transform);

        transformedPoints->GetPoints().Add(transformedPoint);
    }

    CACHE_WRITE(Arch2RuntimeCache, Points, transformedPoints);
    output = Variant(transformedPoints);
    return true;
}

bool PLCTFilterByPhysicalMaterial::CheckPoint(PLCTPoint* point)
{
    Variant physMat = point->GetProperties()->GetPropertyValue(TEXT("PhysMat"));
    if (physMat.Type == VariantType::Null)
    {
        return false;
    }

    String material = physMat.ToString();
    bool matches = material == MaterialTag.ToString();
    //LOG(Warning, "Read: {0}", material);
    //LOG(Warning, "Required: {0}", MaterialTag.ToString());
    if (Inverted)
        matches = !matches;

    if (!matches)
        return false;

    return true;
}

bool PLCTFilterSurfaceByTag::CheckSurface(PLCTSurface* surface)
{
    bool tagExists = surface->CheckSurfaceTag(CompareTag);
    if (Inverted)
        tagExists = !Inverted;

    return tagExists;
}
