// Copyright (c) 2012-2023 Wojciech Figat. All rights reserved.

#pragma once

#include "Engine/ContentImporters/Types.h"

#if COMPILE_WITH_ASSETS_IMPORTER

#include "../PLCT/PLCTGraph.h"

/// <summary>
/// Creating PLCT graph utility
/// </summary>
class CreatePLCTGraph
{
public:
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
};

#endif
