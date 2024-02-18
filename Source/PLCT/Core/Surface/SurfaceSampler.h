#pragma once

#include "Engine/Scripting/ScriptingObject.h"
#include "Engine/Scripting/SerializableScriptingObject.h"
#include "Engine/Scripting/ScriptingType.h"
#include "Engine/Core/Config.h"
#include "Engine/Core/RandomStream.h"
#include "../../Level/PLCTVolume.h"
#include "../PLCTSurface.h"
#include "../PLCTPoint.h"

/// <summary>
/// The settings for the surface sampler.
/// </summary>
API_STRUCT() struct PLCT_API SurfaceSamplerSettings : ISerializable
{
    API_AUTO_SERIALIZATION();
    DECLARE_SCRIPTING_TYPE_NO_SPAWN(SurfaceSamplerSettings);

    /// <summary>
    /// The spacing between each point that gets sampled.
    /// </summary>
    API_FIELD()
    float Spacing = 10;
};

/// <summary>
/// PLCT Surface Sampler. Samples surfaces at points on a grid.
/// </summary>
API_CLASS() class PLCT_API SurfaceSampler : public ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(SurfaceSampler, ScriptingObject);

public:
    ~SurfaceSampler() = default;

    /// <summary>
    /// Sample all XZ coordinates.
    /// </summary>
    /// <param name="surface">The target surface.</param>
    /// <param name="container">The output container.</param>
    /// <returns>True if any points were sampled, otherwise false.</returns>
    API_FUNCTION() virtual bool SampleXZ(PLCTSurface* surface, PLCTPointsContainer* container)
    {
        CHECK_RETURN(surface, false);
        CHECK_RETURN(container, false);

        PLCTVolume* volume = surface->GetVolume();

        CHECK_RETURN(volume, false);
        OrientedBoundingBox box = volume->GetOrientedBox();
        Vector2 start = Vector2(box.GetCenter().X, box.GetCenter().Z) + Vector2(box.Extents.X, box.Extents.Z);
        bool foundAnyPoint = false;

        Vector2 left = Vector2(box.Transformation.GetLeft().X, box.Transformation.GetLeft().Z);
        Vector2 back = Vector2(box.Transformation.GetBackward().X, box.Transformation.GetBackward().Z);
        start += left * 0.01f;
        start += back * 0.01f;

        Vector2 current = start;

        RandomStream stream = RandomStream();
        stream.GenerateNewSeed();

        int iterationCount = 0;
        while (true)
        {
            bool found = surface->SampleXZ(current, container);
            foundAnyPoint = found || foundAnyPoint;

            if (found)
            {
                PLCTPoint* point = container->GetPoints()[container->GetPoints().Count() - 1];

                CHECK_RETURN(point, false);
                point->GetProperties()->SetPropertyValue(TEXT("Random"), Variant(stream.GetFraction()));
                Transform pointTransform = point->GetTransform();
                point->GetProperties()->SetPropertyValue(TEXT("Normal"), Variant(pointTransform.Orientation * Vector3::Up));
            }

            current += left * _settings.Spacing;
            if (!Check(volume, current))
            {
                iterationCount++;

                // Reset back to the right
                current = start + (back * (_settings.Spacing * (float) iterationCount));
                if (!Check(volume, current))
                {
                    break;
                }
            }
        }

        //LOG(Warning, "Done sampling.");
        return foundAnyPoint;
    }

    /// <summary>
    /// Sample all XZ coordinates.
    /// </summary>
    /// <param name="surface">The target surface.</param>
    /// <returns>The points container, or null if no points were sampled.</returns>
    API_FUNCTION() virtual PLCTPointsContainer* SampleXZ(PLCTSurface* surface)
    {
        PLCTPointsContainer* container = New<PLCTPointsContainer>();
        bool foundPoint = SampleXZ(surface, container);

        if (!foundPoint)
        {
            SAFE_DELETE(container);
            return nullptr;
        }

        return container;
    }

    /// <summary>
    /// Sample all XZ coordinates from every surface in this sampler.
    /// </summary>
    /// <returns>The points container, or null if no points were sampled.</returns>
    API_FUNCTION() virtual PLCTPointsContainer* SampleXZ()
    {
        PLCTPointsContainer* container = New<PLCTPointsContainer>();
        bool foundAnyPoints = false;

        for (int surfIdx = 0; surfIdx < _surfaces.GetSurfaces().Count(); surfIdx++)
        {
            foundAnyPoints = SampleXZ(_surfaces.GetSurfaces()[surfIdx], container) || foundAnyPoints;
        }

        return foundAnyPoints ? container : nullptr;
    }

    /// <summary>
    /// Sets the settings for this surface sampler.
    /// </summary>
    /// <param name="settings">The new settings to use.</param>
    API_FUNCTION() void Configure(SurfaceSamplerSettings& settings)
    {
        _settings = settings;
    }

    /// <summary>
    /// Gets/Sets the surface list.
    /// </summary>
    API_PROPERTY() void SetSurfaceList(PLCTSurfaceList* surfaces)
    {
        _surfaces = *surfaces;
    }

    /// <summary>
    /// Gets/Sets the surface list.
    /// </summary>
    API_PROPERTY() PLCTSurfaceList* GetSurfaceList()
    {
        return &_surfaces;
    }

private:
    bool Check(PLCTVolume* volume, Vector2& point)
    {
        CHECK_RETURN(volume, false);
        return volume->GetOrientedBox().Contains(Vector3(point.X, volume->GetPosition().Y, point.Y)) == ContainmentType::Contains;
    }

    PLCTSurfaceList _surfaces;
    SurfaceSamplerSettings _settings;
};
