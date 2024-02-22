#include "PLCTGraph.h"
#include "Engine/Content/Factories/BinaryAssetFactory.h"
#include "Engine/Content/BinaryAsset.h"
#include "../Level/PLCTVolume.h"
#include "Engine/Serialization/MemoryReadStream.h"

#include "Engine/Content/JsonAsset.h"
#include "Engine/Scripting/Scripting.h"
#include "Engine/Serialization/JsonSerializer.h"
#include "Engine/Threading/Threading.h"
#include "FlaxEngine.Gen.h"

#include "PLCTNode.h"

#if USE_EDITOR
#include "Engine/Level/Level.h"
#endif

#define IS_PLCT_NODE(n) (n.GroupID == 1)

//REGISTER_BINARY_ASSET(PLCTGraph, "PLCT.PLCTGraph", false);


const String PLCTGraph::TypeName = TEXT("PLCT.PLCTGraph");
class FactoryPLCTGraph : public BinaryAssetFactory<PLCTGraph>
{
public:
    FactoryPLCTGraph() 
    { 
        IAssetFactory::Get().Add(PLCTGraph::TypeName, this);
        LOG(Info, "registered factory.");
    }

    ~FactoryPLCTGraph()
    {
        IAssetFactory::Get().Remove(PLCTGraph::TypeName);
    }

    bool SupportsVirtualAssets() const override { return false; }
};
static FactoryPLCTGraph CFactoryPLCTGraph;

PLCTGraphNode::~PLCTGraphNode() {
    SAFE_DELETE(Instance);
}

PLCTGraph::PLCTGraph(const SpawnParams& params, const AssetInfo* info)
    : BinaryAsset(params, info)
{
}

void VisjectPLCTGraph::Clear()
{
    VisjectGraph<PLCTGraphNode>::Clear();
}

bool VisjectPLCTGraph::onNodeLoaded(Node* n)
{
    const Node& node = *n;
    if (IS_PLCT_NODE(node))
    {
        // Create node instance object
        ScriptingTypeHandle type = Scripting::FindScriptingType((StringAnsiView)n->Values[0]);
        if (!type)
            type = Scripting::FindScriptingType(StringAnsi((StringView)n->Values[0]));
        if (type)
        {
            n->Instance = (PLCTNode*)Scripting::NewObject(type);
            const Variant& data = n->Values[1];
            if (data.Type == VariantType::Blob)
                JsonSerializer::LoadFromBytes(n->Instance, Span<byte>((byte*)data.AsBlob.Data, data.AsBlob.Length), FLAXENGINE_VERSION_BUILD);
        }
        else
        {
            const String name = n->Values[0].ToString();
            if (name.HasChars())
                LOG(Error, "Missing type '{0}'", name);
        }
    }

    return VisjectGraph<PLCTGraphNode>::onNodeLoaded(n);
}

bool PLCTGraph::RunGeneration(PLCTVolume* volume)
{
    bool good = true;
    for (int i = 0; i < Graph.Nodes.Count(); i++)
    {
        if (Graph.Nodes[i].Instance && Graph.Nodes[i].Instance->Is<PLCTNodeEnd>())
        {
            PLCTNodeEnd* graphEndNode = (PLCTNodeEnd*)Graph.Nodes[i].Instance;
            if (!graphEndNode->Execute(Graph.Nodes[i], volume))
            {
                good = false;
            }
        }
    }

    return good;
}

BytesContainer PLCTGraph::LoadSurface()
{
    if (WaitForLoaded())
        return BytesContainer();
    ScopeLock lock(Locker);
    if (!LoadChunks(GET_CHUNK_FLAG(0)))
    {
        const auto data = GetChunk(0);
        BytesContainer result;
        result.Copy(data->Data);
        return result;
    }
    LOG(Warning, "\'{0}\' surface data is missing.", ToString());
    return BytesContainer();
}

#if USE_EDITOR

bool PLCTGraph::SaveSurface(const BytesContainer& data)
{
    // Wait for asset to be loaded or don't if last load failed
    if (LastLoadFailed())
    {
        LOG(Warning, "Saving asset that failed to load.");
    }
    else if (WaitForLoaded())
    {
        LOG(Error, "Asset loading failed. Cannot save it.");
        return true;
    }

    ScopeLock lock(Locker);

    // Set Visject Surface data
    GetOrCreateChunk(0)->Data.Copy(data);

    // Save
    AssetInitData assetData;
    assetData.SerializedVersion = 1;
    if (SaveAsset(assetData))
    {
        LOG(Error, "Cannot save \'{0}\'", ToString());
        return true;
    }

    return false;
}

void PLCTGraph::OnScriptsReloadStart()
{
    // Include all node instances in hot-reload
    for (PLCTGraphNode& n : Graph.Nodes)
    {
        Level::ScriptsReloadRegisterObject((ScriptingObject*&)n.Instance);
    }

    //// Clear state
    //Graph.Root = nullptr;
    //Graph.NodesCount = 0;
    //Graph.NodesStatesSize = 0;
}

void PLCTGraph::OnScriptsReloadEnd()
{
    //// Node instances were restored so update the graph cached structure (root, children, decorators, etc.)
    //Graph.Setup(this);
}

void PLCTGraph::GetReferences(Array<Guid>& output) const
{
    // Base
    BinaryAsset::GetReferences(output);

    Graph.GetReferences(output);

    // Extract refs from serialized nodes data
    for (const PLCTGraphNode& n : Graph.Nodes)
    {
        if (n.Instance == nullptr)
            continue;
        const Variant& data = n.Values[1];
        if (data.Type == VariantType::Blob)
            JsonAssetBase::GetReferences(StringAnsiView((char*)data.AsBlob.Data, data.AsBlob.Length), output);
    }
}
#endif

void PLCTGraph::OnScriptingDispose()
{
}

Asset::LoadResult PLCTGraph::load()
{
    // Load graph
    const auto surfaceChunk = GetChunk(0);
    if (surfaceChunk == nullptr)
        return LoadResult::MissingDataChunk;
    MemoryReadStream surfaceStream(surfaceChunk->Get(), surfaceChunk->Size());
    if (Graph.Load(&surfaceStream, true))
    {
        LOG(Warning, "Failed to load graph \'{0}\'", ToString());
        return LoadResult::Failed;
    }
    //Graph.Setup(this);

//#if USE_EDITOR
//    Level::ScriptsReloadStart.Bind<BehaviorTree, &BehaviorTree::OnScriptsReloadStart>(this);
//    Level::ScriptsReloadEnd.Bind<BehaviorTree, &BehaviorTree::OnScriptsReloadEnd>(this);
//#endif

    return LoadResult::Ok;
}

void PLCTGraph::unload(bool isReloading)
{
//#if USE_EDITOR
//    Level::ScriptsReloadStart.Unbind<BehaviorTree, &BehaviorTree::OnScriptsReloadStart>(this);
//    Level::ScriptsReloadEnd.Unbind<BehaviorTree, &BehaviorTree::OnScriptsReloadEnd>(this);
//#endif

    // Clear resources
    Graph.Clear();
}

AssetChunksFlag PLCTGraph::getChunksToPreload() const
{
    return GET_CHUNK_FLAG(0);
}
