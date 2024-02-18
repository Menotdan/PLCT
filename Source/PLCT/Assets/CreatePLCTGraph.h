#pragma once

#include "Engine/ContentImporters/Types.h"

#include "PLCT/Core/PLCTGraph.h"
#include "Engine/Serialization/MemoryWriteStream.h"

/// <summary>
/// Creating PLCT graph utility
/// </summary>
API_CLASS(Static) class PLCT_API CreatePLCTGraph
{
    DECLARE_SCRIPTING_TYPE_MINIMAL(CreatePLCTGraph);
public:
#if COMPILE_WITH_ASSETS_IMPORTER
    static CreateAssetResult Create(CreateAssetContext& context)
    {
        // Base
        IMPORT_SETUP(PLCTGraph, 1);

        // Chunk 0 - Visject Surface
        if (context.AllocateChunk(0))
            return CreateAssetResult::CannotAllocateChunk;
        {
            const VisjectPLCTGraph graph;
            MemoryWriteStream stream(64);
            graph.Save(&stream, true);
            context.Data.Header.Chunks[0]->Data.Copy(stream.GetHandle(), stream.GetPosition());
        }

        return CreateAssetResult::Ok;
    }
#endif

    API_FUNCTION() static void RegisterCreator();
};
