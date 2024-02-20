#include "PLCTNode.h"
#include "PLCTNodes.h"
#include "../Level/PLCTVolume.h"

bool PLCTNodeFilter::GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
{
    LOG(Warning, "filter");
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

        PLCTPoint* filteredPoint = New<PLCTPoint>();
        Memory::CopyItems<PLCTPoint>(filteredPoint, point, 1);
        filteredPoints->GetPoints().Add(filteredPoint);
    }

    CACHE_WRITE(Arch2RuntimeCache, Points, filteredPoints);
    output = Variant(filteredPoints);
    return true;
}
