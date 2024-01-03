#include "BoxColliderSurface.h"
#include "../../Level/Actors/PLCTVolume.h"

PLCTPoint* BoxColliderSurface::SampleXZ(Vector2 xz)
{
    if (!_actor)
    {
        return nullptr;
    }

    Vector3 pointFound;

    OrientedBoundingBox actorBox = _actor->GetOrientedBox();
    Vector3 corners[8];
    actorBox.GetCorners(corners);

    Face faces[6] =
    {
        { corners[0], corners[1], corners[2], corners[3] }, // top
        { corners[5], corners[4], corners[7], corners[6] }, // bottom
        { corners[4], corners[0], corners[3], corners[7] }, // front
        { corners[1], corners[5], corners[6], corners[2] }, // back
        { corners[4], corners[5], corners[1], corners[0] }, // left
        { corners[3], corners[2], corners[6], corners[7] }, // right
    };

    CHECK_RETURN(GetVolume(), nullptr);
    float highestY = 0;
    bool foundAny = false;
    for (int face = 0; face < 6; face++)
    {
        if (!faces[face].IsWithinBounds(xz))
        {
            continue;
        }

        float y = faces[face].GetYAt(xz);
        float maxY = GetVolume()->GetBox().Maximum.Y;
        float minY = GetVolume()->GetBox().Minimum.Y;
        if (y > maxY || y < minY)
        {
            continue;
        }

        if (y > highestY || !foundAny)
        {
            highestY = y;
            pointFound = Vector3(xz.X, y, xz.Y);
        }

        foundAny = true;
    }

    if (!foundAny)
    {
        return nullptr;
    }
    
    Transform pointTransform = Transform::Identity;
    pointTransform.Translation = pointFound;
    pointTransform.Orientation = _actor->GetOrientation();

    PLCTPoint* point = New<PLCTPoint>();
    point->SetTransform(pointTransform);

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
