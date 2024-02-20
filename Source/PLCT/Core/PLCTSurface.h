#pragma once

#include "PLCTProperties.h"
#include "PLCTPoint.h"
#include "Engine/Core/Config.h"
#include "Engine/Core/Collections/Array.h"
#include "Engine/Core/Delegate.h"
#include "Engine/Level/Actor.h"
#include "Engine/Scripting/ScriptingObject.h"
#include "Engine/Scripting/ScriptingType.h"

class PLCTVolume;

/// <summary>
/// PLCT Surface class used for sampling points.
/// </summary>
API_CLASS(Abstract) class PLCT_API PLCTSurface : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTSurface, ScriptingObject);

public:
    ~PLCTSurface() = default;

    /// <summary>
    /// Gets the properties of the surface.
    /// </summary>
    /// <returns>The surface properties.</returns>
    API_PROPERTY() PLCTPropertyStorage* GetProperties()
    {
        return &_properties;
    }

    /// <summary>
    /// Samples the surface along the X and Z axes.
    /// </summary>
    /// <param name="coordinates">(X,Z) coordinates.</param>
    /// <returns>Sampled point, or null if there is no valid point.</returns>
    API_FUNCTION() virtual PLCTPoint* SampleXZ(Vector2 coordinates)
    {
        return nullptr;
    }

    /// <summary>
    /// Samples the surface along the X and Z axes.
    /// </summary>
    /// <param name="coordinates">(X,Z) coordinates.</param>
    /// <param name="targetContainer">The container to add the point to.</param>
    /// <returns>True if there is a valid point, otherwise false.</returns>
    API_FUNCTION() virtual bool SampleXZ(Vector2 coordinates, PLCTPointsContainer* targetContainer)
    {
        return false;
    }
    //

    /// <summary>
    /// Check that the passed actor matches the sample condition for this surface, and then sets it internally if it matches.
    /// </summary>
    /// <param name="actor">The actor to test.</param>
    /// <returns>True if the actor matches, otherwise false.</returns>
    API_FUNCTION() virtual bool CheckActorMatchesAndSet(Actor* actor)
    {
        return false;
    }

    /// <summary>
    /// Check that the surface has the passed tag.
    /// </summary>
    /// <param name="tag">The tag to check for.</param>
    /// <returns>True if tag is present, otherwise false.</returns>
    API_FUNCTION() virtual bool CheckSurfaceTag(Tag tag)
    {
        return false;
    }

    /// <summary>
    /// Gets/Sets the volume associated with this surface.
    /// </summary>
    API_PROPERTY() PLCTVolume* GetVolume()
    {
        return _volume;
    }

    /// <summary>
    /// Gets/Sets the volume associated with this surface.
    /// </summary>
    API_PROPERTY() void SetVolume(PLCTVolume* volume)
    {
        _volume = volume;
    }

private:
    PLCTPropertyStorage _properties;
    PLCTVolume* _volume = nullptr;
};

/// <summary>
/// PLCT surface list, which allows you to reference multiple surfaces to sample.
/// </summary>
API_CLASS() class PLCT_API PLCTSurfaceList : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTSurfaceList, ScriptingObject);

public:
    ~PLCTSurfaceList()
    {
        _surfaces.ClearDelete();
    }

    /// <summary>
    /// Get this array of surfaces.
    /// </summary>
    /// <returns>The surfaces array.</returns>
    API_PROPERTY() Array<PLCTSurface*>& GetSurfaces()
    {
        return _surfaces;
    }

private:
    Array<PLCTSurface*> _surfaces;
};
