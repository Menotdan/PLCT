#include "BoxColliderSurface.h"
#include "../../Level/PLCTVolume.h"
#include "Engine/Physics/PhysicalMaterial.h"

PLCTPoint* BoxColliderSurface::SampleXZ(Vector2 coordinates)
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
    PhysicalMaterial* mat = hit.Material;
    if (mat)
        point->GetProperties()->SetPropertyValue(TEXT("PhysMat"), Variant(mat->Tag.ToString()));

    return point;
}

bool BoxColliderSurface::SampleXZ(Vector2 coordinates, PLCTPointsContainer* targetContainer)
{
    PLCTPoint* point = SampleXZ(coordinates);
    if (point == nullptr)
    {
        return false;
    }

    CHECK_RETURN(targetContainer, false);
    targetContainer->GetPoints().Add(point);
    return true;
}

bool BoxColliderSurface::CheckActorMatchesAndSet(Actor* actor)
{
    CHECK_RETURN(actor, false);
    if (actor->Is<BoxCollider>())
    {
        _actor = (BoxCollider*) actor;
        return true;
    }

    return false;
}
