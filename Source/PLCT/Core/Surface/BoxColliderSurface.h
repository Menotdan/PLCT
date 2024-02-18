#pragma once

#include "Engine/Scripting/ScriptingObject.h"
#include "Engine/Scripting/ScriptingType.h"
#include "Engine/Level/Actor.h"
#include "Engine/Physics/Colliders/BoxCollider.h"
#include "Engine/Core/Math/Vector2.h"
#include "Engine/Core/Math/Vector3.h"
#include "../PLCTSurface.h"

struct Face
{
    Vector3 edge1;
    Vector3 edge2;
    Vector3 edge3;
    Vector3 edge4;

    bool IsWithinBounds(Vector2 xz)
    {
        bool tri1 = PointInsdeTriangle(Vector2(edge1.X, edge1.Z), Vector2(edge2.X, edge2.Z), Vector2(edge3.X, edge3.Z), xz);
        bool tri2 = PointInsdeTriangle(Vector2(edge3.X, edge3.Z), Vector2(edge4.X, edge4.Z), Vector2(edge1.X, edge1.Z), xz);

        return tri1 || tri2;
    }

    bool PointInsdeTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Vector2 p)
    {
        bool b0 = (Vector2::Dot(Vector2(p.X - v1.X, p.Y - v1.Y), Vector2(v1.Y - v2.Y, v2.X - v1.X)) > 0);
        bool b1 = (Vector2::Dot(Vector2(p.X - v2.X, p.Y - v2.Y), Vector2(v2.Y - v3.Y, v3.X - v2.X)) > 0);
        bool b2 = (Vector2::Dot(Vector2(p.X - v3.X, p.Y - v3.Y), Vector2(v3.Y - v1.Y, v1.X - v3.X)) > 0);
        return (b0 == b1 && b1 == b2);
    }

    float GetYAt(Vector2 xz)
    {
        Vector3 v1 = edge2 - edge1;
        Vector3 v2 = edge3 - edge1;
        Vector3 n = Vector3::Cross(v1, v2).GetNormalized();

        return (1 / Math::Max((float) n.Y, 0.0001f)) * ((n.X * edge1.X) + (n.Y * edge1.Y) + (n.Z * edge1.Z) - (n.X * xz.X) - (n.Z * xz.Y));
    }
};  

/// <summary>
/// Box collider PLCT surface.
/// </summary>
API_CLASS(Sealed) class PLCT_API BoxColliderSurface : public PLCTSurface
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(BoxColliderSurface, PLCTSurface);

public:
    // [PLCTSurface]
    PLCTPoint* SampleXZ(Vector2 coordinates) override;
    bool SampleXZ(Vector2 coordinates, PLCTPointsContainer* targetContainer) override;
    bool CheckActorMatchesAndSet(Actor* actor) override;

private:
    BoxCollider* _actor = nullptr;
};
