#pragma once

#include "PLCTNode.h"
#include "Surface/SurfaceSampler.h"
#include "Engine/Core/Collections/Array.h"

/// <summary>
/// Gets all box collider surfaces found in the PLCT volume.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTGetBoxColliderSurfaces : public PLCTNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTGetBoxColliderSurfaces, PLCTNode);
    API_AUTO_SERIALIZATION();

public:
    int NodeArchetypeIndex() const override
    {
        return 0;
    }

public:
    // [PLCTNode]
    bool GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output) override;
};

/// <summary>
/// Gets all terrain surfaces found in the PLCT volume.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTGetTerrainSurfaces : public PLCTNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTGetTerrainSurfaces, PLCTNode);
    API_AUTO_SERIALIZATION();

public:
    int NodeArchetypeIndex() const override
    {
        return 0;
    }

public:
    // [PLCTNode]
    bool GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output) override;
};

/// <summary>
/// Samples a PLCT surface, generating points.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTSampleSurface : public PLCTNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTSampleSurface, PLCTNode);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// The settings to be used during surface sampling.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    SurfaceSamplerSettings Settings = SurfaceSamplerSettings();

    int NodeArchetypeIndex() const override
    {
        return 2;
    }

public:
    // [PLCTNode]
    bool GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output) override;
};

/// <summary>
/// Takes a list of points and debugs them. (Warning: This is very slow.)
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTDebugDrawPoints : public PLCTNodeEnd
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTDebugDrawPoints, PLCTNodeEnd);
    API_AUTO_SERIALIZATION();

public:
    int NodeArchetypeIndex() const override
    {
        return 3;
    }

    /// <summary>
    /// The color of the debug point to be rendered.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    Color PointColor = Color::White;

public:
    // [PLCTNodeEnd]
    bool Execute(PLCTGraphNode& node, PLCTVolume* volume) override;
};

/// <summary>
/// An entry in the list of prefabs that can be spawned.
/// </summary>
API_STRUCT() struct PLCT_API PrefabSpawnEntry : ISerializable
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(PrefabSpawnEntry)
    API_AUTO_SERIALIZATION();

    /// <summary>
    /// The weight of this entry, from 0.0 to 1.0. If the total weight of all entries is greater than 1.0, entries that start past 1.0 will never spawn.
    /// </summary>
    API_FIELD() float Weight = 1;

    /// <summary>
    /// The prefab to spawn. Can be set empty to spawn nothing.
    /// </summary>
    API_FIELD() AssetReference<Prefab> Prefab;

    // Cached actor during spawning to increase performance
    Actor* CachedActor = nullptr;
    Span<byte> CachedActorData;
};

/// <summary>
/// Takes a list of points and spawns a prefab at each point.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTSpawnPrefabAtPoints : public PLCTNodeEnd
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTSpawnPrefabAtPoints, PLCTNodeEnd);
    API_AUTO_SERIALIZATION();

public:
    int NodeArchetypeIndex() const override
    {
        return 3;
    }

    /// <summary>
    /// A list of all prefabs that can be spawned.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    Array<PrefabSpawnEntry> Prefabs;

public:
    // [PLCTNodeEnd]
    bool Execute(PLCTGraphNode& node, PLCTVolume* volume) override;
};

/// <summary>
/// Filters points by the random value assigned to them during creation.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTFilterByRandom : public PLCTNodeFilter
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTFilterByRandom, PLCTNodeFilter);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// The minimum random value that a point can have to not be filtered out.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    float MinValue = 0;

    /// <summary>
    /// The maximum random value that a point can have to not be filtered out.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(20)")
    float MaxValue = 1;

public:
    // [PLCTNodeFilter]
    bool CheckPoint(PLCTPoint* point) override;
};

/// <summary>
/// Filters points by their normal vectors.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTFilterByNormal : public PLCTNodeFilter
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTFilterByNormal , PLCTNodeFilter);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// The minimum normal vector. All components are checked individually.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    Vector3 MinValue = -Vector3::One;

    /// <summary>
    /// The maximum normal vector. All components are checked individually.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(20)")
    Vector3 MaxValue = Vector3::One;

public:
    // [PLCTNodeFilter]
    bool CheckPoint(PLCTPoint* point) override;
};

/// <summary>
/// Set the transform of all the points.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTSetPointsTransform : public PLCTNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTSetPointsTransform, PLCTNode);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// If the position should be set.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    bool SetPosition = false;

    /// <summary>
    /// If the rotation should be set.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(20)")
    bool SetRotation = false;

    /// <summary>
    /// If the scale should be set.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(30)")
    bool SetScale = false;

    /// <summary>
    /// The target position.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(40)")
    Vector3 Position = Vector3::Zero;

    /// <summary>
    /// The target rotation.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(50)")
    Quaternion Rotation = Quaternion::Identity;

    /// <summary>
    /// The target scale.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(60)")
    Vector3 Scale = Vector3::One;

    int NodeArchetypeIndex() const override
    {
        return 4;
    }

    void TransformPoints(Transform& transform);
public:
    // [PLCTNode]
    bool GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output) override;
};

/// <summary>
/// Transform the points, including an offset, and optional randomized offsets.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTTransformPoints : public PLCTNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTTransformPoints, PLCTNode);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// If the position offset should have a randomized component.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    bool RandomizePosition = false;

    /// <summary>
    /// If the rotation offset should have a randomized component.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(20)")
    bool RandomizeRotation = false;

    /// <summary>
    /// If the scale multiplier should have a randomized component.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(30)")
    bool RandomizeScale = false;

    /// <summary>
    /// The minimum of the randomized position offset.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(40)")
    Vector3 RandomPositionMinimum = Vector3::Zero;

    /// <summary>
    /// The maximum of the randomized position offset.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(50)")
    Vector3 RandomPositionMaximum = Vector3::Zero;

    /// <summary>
    /// The minimum of the randomized rotation offset.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(60)")
    Quaternion RandomRotationMinimum = Quaternion::Identity;

    /// <summary>
    /// The maximum of the randomized rotation offset.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(70)")
    Quaternion RandomRotationMaximum = Quaternion::Identity;

    /// <summary>
    /// The minimum of the randomized scale multipler.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(80)")
    Vector3 RandomScaleMinimum = Vector3::Half;

    /// <summary>
    /// The maximum of the randomized scale multipler.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(90)")
    Vector3 RandomScaleMaximum = Vector3::One;

    /// <summary>
    /// The position offset to be applied.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(100)")
    Vector3 PositionOffset = Vector3::Zero;

    /// <summary>
    /// The rotation offset to be applied.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(110)")
    Quaternion RotationOffset = Quaternion::Identity;

    /// <summary>
    /// The scale multipler to be applied.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(120)")
    Vector3 ScaleMultiplier = Vector3::One;

    int NodeArchetypeIndex() const override
    {
        return 4;
    }

    void TransformPoints(Transform& transform);
public:
    // [PLCTNode]
    bool GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output) override;
};

/// <summary>
/// Filters points by the physical material on the surface they were sampled from.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTFilterByPhysicalMaterial : public PLCTNodeFilter
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTFilterByPhysicalMaterial, PLCTNodeFilter);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// The physical material tag to compare.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    Tag MaterialTag = Tag();

    /// <summary>
    /// Whether to invert the check.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(20)")
    bool Inverted;

public:
    // [PLCTNodeFilter]
    bool CheckPoint(PLCTPoint* point) override;
};

/// <summary>
/// Filters points by the physical material on the surface they were sampled from.
/// </summary>
API_CLASS(Sealed) class PLCT_API PLCTFilterSurfaceByTag : public PLCTNodeFilterSurface
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTFilterSurfaceByTag, PLCTNodeFilterSurface);
    API_AUTO_SERIALIZATION();

public:
    /// <summary>
    /// The tag to compare.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(10)")
    Tag CompareTag = Tag();

    /// <summary>
    /// Whether to invert the check.
    /// </summary>
    API_FIELD(Attributes = "EditorOrder(20)")
    bool Inverted;

public:
    // [PLCTNodeFilterSurface]
    bool CheckSurface(PLCTSurface* surface) override;
};

/* Node Runtime Cache Types */
/* This allows nodes to save their output once it runs. */

/// <summary>
/// Single surface list output cache.
/// </summary>
API_STRUCT() struct PLCT_API Arch0RuntimeCache : ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(Arch0RuntimeCache, ScriptingObject);
    ~Arch0RuntimeCache()
    {
        if (SurfaceList)
            Delete(SurfaceList);
        SurfaceList = nullptr;
    }

    PLCTSurfaceList* SurfaceList;
};

/// <summary>
/// Single points output cache.
/// </summary>
API_STRUCT() struct PLCT_API Arch2RuntimeCache : ScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(Arch2RuntimeCache, ScriptingObject);
    ~Arch2RuntimeCache()
    {
        if (Points)
            Delete(Points);
        Points = nullptr;
    }

    PLCTPointsContainer* Points;
};

