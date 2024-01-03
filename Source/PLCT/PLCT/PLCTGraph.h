#pragma once

#include "Engine/Content/BinaryAsset.h"
#include "Engine/Visject/VisjectGraph.h"
#include "Engine/Scripting/Script.h"

class PLCTNode;
class PLCTVolume;

/// <summary>
/// PLCT Graph Node.
/// </summary>
class PLCTGraphNode : public VisjectGraphNode<>
{
public:
    // Instance of the graph node.
    PLCTNode* Instance = nullptr;

    ~PLCTGraphNode();
};

/// <summary>
/// PLCT (Procedural Level Creation Tools) graph
/// </summary>
class VisjectPLCTGraph :
    public VisjectGraph<PLCTGraphNode>
{

public:
    // [VisjectGraph]
    void Clear() override;
    bool onNodeLoaded(Node* n) override;
};

/// <summary>
/// PLCT Graph
/// </summary>
API_CLASS(NoSpawn, Sealed) class FLAXENGINE_API PLCTGraph : public BinaryAsset {
    DECLARE_BINARY_ASSET_HEADER(PLCTGraph, 1);

public:
    VisjectPLCTGraph Graph;

    /// <summary>
    /// Execute this graph.
    /// </summary>
    API_FUNCTION() bool RunGeneration(PLCTVolume* volume);

    /// <summary>
    /// Tries to load surface graph from the asset.
    /// </summary>
    /// <returns>The surface data or empty if failed to load it.</returns>
    API_FUNCTION() BytesContainer LoadSurface();

#if USE_EDITOR
    /// <summary>
    /// Updates the graph surface (save new one, discard cached data, reload asset).
    /// </summary>
    /// <param name="data">Stream with graph data.</param>
    /// <returns>True if cannot save it, otherwise false.</returns>
    API_FUNCTION() bool SaveSurface(const BytesContainer& data);
#endif

private:
#if USE_EDITOR
    void OnScriptsReloadStart();
    void OnScriptsReloadEnd();
#endif

public:
    // [BinaryAsset]
    void OnScriptingDispose() override;
#if USE_EDITOR
    void GetReferences(Array<Guid>& output) const override;
#endif

protected:
    // [BinaryAsset]
    LoadResult load() override;
    void unload(bool isReloading) override;
    AssetChunksFlag getChunksToPreload() const override;
};
