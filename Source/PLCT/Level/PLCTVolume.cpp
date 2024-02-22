#include "PLCTVolume.h"
#include "Engine/Core/Log.h"
#include "Engine/Threading/JobSystem.h"
#include "Engine/Core/Collections/Array.h"
#include "Engine/Level/Scene/Scene.h"
#include "Engine/Level/Actor.h"
#include "Engine/Level/Actors/EmptyActor.h"
#include "Engine/Level/Level.h"
#include "../Core/PLCTSurface.h"
#include "../Core/PLCTGraph.h"
#include "../Core/PLCTProperties.h"
#include <typeinfo>

PLCTVolume::PLCTVolume(const SpawnParams& params)
    : BoxVolume(params)
{
    LOG(Warning, "Created new PLCT volume.");
}

void GetAllActors(Array<Actor*>& actors, Actor* actor)
{
    for (Actor* child : actor->Children)
    {
        actors.Add(child);
        GetAllActors(actors, child);
    }
}

bool PLCTVolume::FindSurfaceAtIndex(PLCTSurface* surface, int index)
{
    CHECK_RETURN(surface, false);
    int foundCounter = 0;
    for (int sceneIdx = -1; sceneIdx < Level::Scenes.Count(); sceneIdx++)
    {
        Scene* readingScene = sceneIdx == -1 ? this->GetScene() : Level::Scenes[sceneIdx];
        if (readingScene == this->GetScene() && sceneIdx != -1)
            continue;

        Array<Actor*> sceneActors;
        GetAllActors(sceneActors, readingScene);
        for (int actorIdx = 0; actorIdx < sceneActors.Count(); actorIdx++)
        {
            Actor* actor = sceneActors[actorIdx];
            ContainmentType contains = this->GetOrientedBox().Contains(actor->GetSphere());
            if (contains == ContainmentType::Disjoint)
            {
                continue;
            }

            if (surface->CheckActorMatchesAndSet(actor))
            {
                if (foundCounter != index)
                {
                    foundCounter++;
                    continue;
                }

                surface->SetVolume(this);
                return true;
            }
        }
    }

    return false;
}

void PLCTVolume::GenerateThread(int32 id)
{
    LOG(Info, "Started Generation...");
    PLCTGraph* graph = Graph.Get();

    if (!graph)
    {
        if (RuntimeCache)
        {
            Delete(RuntimeCache);
            RuntimeCache = nullptr;
        }

        LOG(Warning, "Volume missing graph! Cancelling...");
        Platform::AtomicStore(&_generateThreadID, -1);
        return;
    }

    graph->RunGeneration(this);
    LOG(Info, "Freeing cache ({0})...", property_tracker::property_counter);
    if (RuntimeCache)
    {
        Delete(RuntimeCache);
        RuntimeCache = nullptr;
    }

    Platform::AtomicStore(&_generateThreadID, -1);
    LOG(Info, "Generation Completed.");
    return;
}

void PLCTVolume::CleanupThread(int32 id)
{
    this->GenerationContainer.Get()->DestroyChildren();
    Platform::AtomicStore(&_cleanupThreadID, -1);
}

int property_tracker::property_counter;

bool PLCTVolume::Generate()
{
    if (!(Graph && !Graph->WaitForLoaded()))
    {
        LOG(Warning, "Graph failed to load on PLCT volume!");
        return false;
    }

    if (Platform::AtomicRead(&_generateThreadID) != -1)
    {
        return false;
    }

    RuntimeCache = New<PLCTPropertyStorage>();
    CHECK_RETURN(RuntimeCache, false);

    Function<void (int32)> action;
    action.Bind<PLCTVolume, &PLCTVolume::GenerateThread>(this);
    Platform::AtomicStore(&_generateThreadID, JobSystem::Dispatch(action));

    return true;
}

void PLCTVolume::Cleanup()
{
    CleanupThread(0);
}

bool PLCTVolume::FindFirstSurface(PLCTSurface* surface)
{
    return FindSurfaceAtIndex(surface, 0);
}

PLCTSurfaceList* PLCTVolume::FindAllSurfaces(PLCTSurface* baseInstance)
{
    bool foundAnySurface = false;
    CHECK_RETURN(baseInstance, nullptr);
    PLCTSurfaceList* surfaces = New<PLCTSurfaceList>();    

    int index = 0;
    while (true)
    {
        PLCTSurface* surface = (PLCTSurface*) ScriptingObject::NewObject(baseInstance->GetTypeHandle());
        if (!FindSurfaceAtIndex(surface, index++))
        {
            SAFE_DELETE(surface);
            surface = nullptr;
            break;
        }

        foundAnySurface = true;
        surfaces->GetSurfaces().Add(surface);
    }

    if (!foundAnySurface)
    {
        SAFE_DELETE(surfaces);
        return nullptr;
    }

    return surfaces;
}
