#include "BoxColliderSurface.h"
#include "../../Level/Actors/PLCTVolume.h"
#include "Engine/Terrain/TerrainPatch.h"
#include "TerrainSurface.h"

bool PointInsdeTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Vector2 p)
{
    bool b0 = (Vector2::Dot(Vector2(p.X - v1.X, p.Y - v1.Y), Vector2(v1.Y - v2.Y, v2.X - v1.X)) > 0);
    bool b1 = (Vector2::Dot(Vector2(p.X - v2.X, p.Y - v2.Y), Vector2(v2.Y - v3.Y, v3.X - v2.X)) > 0);
    bool b2 = (Vector2::Dot(Vector2(p.X - v3.X, p.Y - v3.Y), Vector2(v3.Y - v1.Y, v1.X - v3.X)) > 0);
    return (b0 == b1 && b1 == b2);
}

PLCTPoint* TerrainSurface::SampleXZ(Vector2 coordinates)
{
    CHECK_RETURN(_actor, nullptr);
    float volumeY = GetVolume()->GetOrientedBox().Extents.Y + GetVolume()->GetOrientedBox().GetCenter().Y;
    Vector3 topPosition = Vector3(coordinates.X, volumeY, coordinates.Y);

    RayCastHit hit;
    if (!_actor->RayCast(topPosition, Vector3::Down, hit, GetVolume()->GetOrientedBox().Extents.Y * 2))
    {
        return nullptr;
    }

    Transform pointTransform = Transform::Identity;
    pointTransform.Translation = hit.Point;

    Vector3 normal = hit.Normal;

    pointTransform.Orientation = Quaternion::FromDirection(normal);
    Transform rotation = Transform::Identity;
    rotation.Orientation = Quaternion::Euler(90, 0, 0);
    Transform result = pointTransform.LocalToWorld(rotation);

    PLCTPoint* point = New<PLCTPoint>();
    point->SetTransform(result);

    return point;
}

bool TerrainSurface::SampleXZ(Vector2 coordinates, PLCTPointsContainer* targetContainer)
{
    CHECK_RETURN(targetContainer, false);
    PLCTPoint* point = SampleXZ(coordinates);
    if (point == nullptr)
    {
        return false;
    }

    targetContainer->GetPoints().Add(point);
    return true;
}

bool TerrainSurface::CheckActorMatchesAndSet(Actor* actor)
{
    CHECK_RETURN(actor, false);
    if (actor->Is<Terrain>())
    {
        _actor = (Terrain*)actor;
        return true;
    }

    return false;
}
