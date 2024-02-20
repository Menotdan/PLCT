#include "PLCTNode.h"
#include "PLCTNodes.h"
#include "../Level/PLCTVolume.h"
#include "Engine/Scripting/Scripting.h"

bool PLCTNodeFilter::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "filter points");
    CACHE_READ(Arch2RuntimeCache, Points);

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

        if (!CheckPoint(point))
            continue;

        PLCTPoint* filteredPoint = point->Copy();
        filteredPoints->GetPoints().Add(filteredPoint);
    }

    CACHE_WRITE(Arch2RuntimeCache, Points, filteredPoints);
    output = Variant(filteredPoints);
    return true;
}

bool PLCTNodeFilterSurface::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "filter surface");
    CACHE_READ(Arch0RuntimeCache, SurfaceList);

    PLCTSurfaceList* surfaces;

    VisjectGraphBox box = node.Boxes[0];
    if (!GetSurfaces(box, this, volume, surfaces))
        return false;

    PLCTSurfaceList* filteredSurfaces = New<PLCTSurfaceList>();

    CHECK_RETURN(surfaces, false);
    for (int pointIdx = 0; pointIdx < surfaces->GetSurfaces().Count(); pointIdx++)
    {
        PLCTSurface* surface = surfaces->GetSurfaces()[pointIdx];
        CHECK_RETURN(surface, false);

        if (!CheckSurface(surface))
            continue;

        // Fun memory hacks
        ScriptingType type = surface->GetType();
        PLCTSurface* filteredSurface = (PLCTSurface*) Scripting::NewObject(type.GetHandle());
        Guid id = filteredSurface->GetID();
        Platform::MemoryCopy(filteredSurface, surface, type.Size);
        filteredSurface->ChangeID(id);

        filteredSurfaces->GetSurfaces().Add(filteredSurface);
    }

    CACHE_WRITE(Arch0RuntimeCache, SurfaceList, filteredSurfaces);
    output = Variant(filteredSurfaces);
    return true;
}
