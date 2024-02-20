#pragma once

#include "Engine/Scripting/ScriptingObject.h"
#include "Engine/Scripting/ScriptingType.h"
#include "Engine/Level/Actor.h"
#include "Engine/Terrain/Terrain.h"
#include "../PLCTSurface.h"

/// <summary>
/// Terrain PLCT surface.
/// </summary>
API_CLASS(Sealed) class PLCT_API TerrainSurface : public PLCTSurface
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(TerrainSurface, PLCTSurface);

public:
    // [PLCTSurface]
    PLCTPoint* SampleXZ(Vector2 coordinates) override;
    bool SampleXZ(Vector2 coordinates, PLCTPointsContainer* targetContainer) override;
    bool CheckActorMatchesAndSet(Actor* actor) override;
    bool CheckSurfaceTag(Tag tag) override;

private:
    Terrain* _actor = nullptr;
};
