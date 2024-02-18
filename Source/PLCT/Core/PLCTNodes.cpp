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

#include "PLCTProperties.h"
#include "Surface/BoxColliderSurface.h"
#include "Surface/TerrainSurface.h"

bool PLCTSampleSurface::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "sample surface");
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch2RuntimeCache* cache = (Arch2RuntimeCache*)Cached.AsPointer;
        output = Variant(cache->Points);
        return true;
    }

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
    Delete(sampler);
    sampler = nullptr;

    Arch2RuntimeCache* cache = New<Arch2RuntimeCache>();
    cache->Points = points;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

    output = Variant(points);
    LOG(Warning, "Sampled {0} points.", points->GetPoints().Count());
    return true;
}

bool PLCTGetBoxColliderSurfaces::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "get collider surfaces");
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch0RuntimeCache* cache = (Arch0RuntimeCache*) Cached.AsPointer;
        output = Variant(cache->SurfaceList);
        return true;
    }

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

    Arch0RuntimeCache* cache = New<Arch0RuntimeCache>();
    cache->SurfaceList = surfaces;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

    output = Variant(surfaces);
    return true;
}

bool PLCTGetTerrainSurfaces::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch0RuntimeCache* cache = (Arch0RuntimeCache*)Cached.AsPointer;
        output = Variant(cache->SurfaceList);
        return true;
    }

    TerrainSurface* baseInstance = New<TerrainSurface>();
    PLCTSurfaceList* surfaces = volume->FindAllSurfaces(baseInstance);
    CHECK_RETURN(surfaces, false);
    SAFE_DELETE(baseInstance);

    Arch0RuntimeCache* cache = New<Arch0RuntimeCache>();
    cache->SurfaceList = surfaces;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

    output = Variant(surfaces);
    return true;
}


bool GetPoints(VisjectGraphBox box, PLCTNode* node, PLCTVolume* volume, PLCTPointsContainer*& outPoints)
{
    PLCTGraphNode* connectedNode;
    ScriptingObject* object;

    CHECK_RETURN(node, false);
    if (!node->GetObjectFromInputBox(box, connectedNode, volume, object))
        return false;

    CHECK_RETURN(object, false);
    if (!object->Is<PLCTPointsContainer>())
        return false;

    outPoints = (PLCTPointsContainer*)object;
    return true;
}

bool PLCTDebugDrawPoints::Execute(PLCTGraphNode& node, PLCTVolume* volume)
{
    LOG(Warning, "debug draw");
    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    CHECK_RETURN(points, false);
    for (int i = 0; i < points->GetPoints().Count(); i++)
    {
#if USE_EDITOR
        DebugDraw::DrawSphere(BoundingSphere(points->GetPoints()[i]->GetTransform().Translation, 2), PointColor, 20.0f);
#endif
    }

    return true;
}

#define PREFAB_SPAWN_JOBS 16
struct PrefabToSpawn
{
    Transform transform;
    Prefab* prefab;
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

            if (!entry->Prefab || entry->Prefab->WaitForLoaded())
            {
                continue;
            }

            Lock->Lock();
            Prefab* prefabPicked = entry->Prefab.Get();
            Transform prefabTransform = points->GetPoints()[pointIdx]->GetTransform();
            Spawn.Add(PrefabToSpawn{ prefabTransform, prefabPicked });
            Lock->Unlock();
        }
    }
};

bool PLCTSpawnPrefabAtPoints::Execute(PLCTGraphNode& node, PLCTVolume* volume)
{
    LOG(Warning, "spawn prefabs");
    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    RandomStream stream = RandomStream();
    stream.GenerateNewSeed();

    CHECK_RETURN(points, false);

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
        Actor* actor = PrefabManager::SpawnPrefab(job->Spawn[i].prefab, (Actor*)volume->GenerationContainer.Get(), job->Spawn[i].transform);
        actor->BreakPrefabLink();
    }

    delete job;

    return true;
}

bool PLCTFilterByRandom::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "filter by random");
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch2RuntimeCache* cache = (Arch2RuntimeCache*)Cached.AsPointer;
        output = Variant(cache->Points);
        return true;
    }

    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    PLCTPointsContainer* filteredPoints = New<PLCTPointsContainer>();

    CHECK_RETURN(points, false);
    for (int pointIdx = 0; pointIdx < points->GetPoints().Count(); pointIdx++)
    {
        PLCTPoint* point = points->GetPoints()[pointIdx];
        CHECK_RETURN(point, false);
        Variant randomValue = point->GetProperties()->GetPropertyValue(TEXT("Random"));
        if (randomValue.Type == VariantType::Null)
            continue;

        if (randomValue.AsFloat < MinValue ||
            randomValue.AsFloat > MaxValue)
        {
            continue;
        }

        PLCTPoint* filteredPoint = New<PLCTPoint>();
        Memory::CopyItems<PLCTPoint>(filteredPoint, point, 1);
        filteredPoints->GetPoints().Add(filteredPoint);
    }

    Arch2RuntimeCache* cache = New<Arch2RuntimeCache>();
    cache->Points = filteredPoints;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

    output = Variant(filteredPoints);
    return true;
}

bool PLCTFilterByNormal::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "filter by normal");
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch2RuntimeCache* cache = (Arch2RuntimeCache*)Cached.AsPointer;
        output = Variant(cache->Points);
        return true;
    }

    PLCTPointsContainer* points;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetPoints(box, this, volume, points))
        return false;

    PLCTPointsContainer* filteredPoints = New<PLCTPointsContainer>();

    CHECK_RETURN(points, false);
    for (int pointIdx = 0; pointIdx < points->GetPoints().Count(); pointIdx++)
    {
        PLCTPoint* point = points->GetPoints()[pointIdx];
        CHECK_RETURN(point, false);
        Variant normalVector = point->GetProperties()->GetPropertyValue(TEXT("Normal"));
        if (normalVector.Type == VariantType::Null)
            continue;

        Vector3 normal = normalVector.AsVector3();
        if (normal.X > MaxValue.X || normal.X < MinValue.X
            || normal.Y > MaxValue.Y || normal.Y < MinValue.Y
            || normal.Z > MaxValue.Z || normal.Z < MinValue.Z)
        {
            continue;
        }

        PLCTPoint* filteredPoint = New<PLCTPoint>();
        Memory::CopyItems<PLCTPoint>(filteredPoint, point, 1);
        filteredPoints->GetPoints().Add(filteredPoint);
    }

    Arch2RuntimeCache* cache = New<Arch2RuntimeCache>();
    cache->Points = filteredPoints;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

    output = Variant(filteredPoints);
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
    LOG(Warning, "transform points");
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch2RuntimeCache* cache = (Arch2RuntimeCache*)Cached.AsPointer;
        output = Variant(cache->Points);
        return true;
    }

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

        PLCTPoint* transformedPoint = New<PLCTPoint>();
        Memory::CopyItems<PLCTPoint>(transformedPoint, point, 1);

        Transform transform = transformedPoint->GetTransform();
        TransformPoints(transform);
        transformedPoint->SetTransform(transform);

        transformedPoints->GetPoints().Add(transformedPoint);
    }

    Arch2RuntimeCache* cache = New<Arch2RuntimeCache>();
    cache->Points = transformedPoints;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

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
    LOG(Warning, "set points transform");
    Variant Cached = volume->RuntimeCache->GetPropertyValue(GetID().ToString());
    if (!(Cached.Type == VariantType::Null))
    {
        Arch2RuntimeCache* cache = (Arch2RuntimeCache*)Cached.AsPointer;
        output = Variant(cache->Points);
        return true;
    }

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

        PLCTPoint* transformedPoint = New<PLCTPoint>();
        Memory::CopyItems<PLCTPoint>(transformedPoint, point, 1);

        Transform transform = transformedPoint->GetTransform();
        TransformPoints(transform);
        transformedPoint->SetTransform(transform);

        transformedPoints->GetPoints().Add(transformedPoint);
    }

    Arch2RuntimeCache* cache = New<Arch2RuntimeCache>();
    cache->Points = transformedPoints;
    volume->RuntimeCache->SetPropertyValue(GetID().ToString(), Variant(cache));

    output = Variant(transformedPoints);
    return true;
}
