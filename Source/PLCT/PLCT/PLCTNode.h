#pragma once

#include "Engine/Scripting/SerializableScriptingObject.h"
#include "Engine/Visject/VisjectGraph.h"
#include "Engine/Core/Types/Variant.h"
#include "PLCTTypes.h"
#include "PLCTGraph.h"

class PLCTVolume;

/// <summary>
/// Base class for PLCT Graph nodes.
/// </summary>
API_CLASS(Abstract) class FLAXENGINE_API PLCTNode : public SerializableScriptingObject
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTNode, SerializableScriptingObject);
    friend class PLCTGraph;

public:
    ~PLCTNode() = default;

    virtual bool GetOutputBox(PLCTGraphNode& node, PLCTVolume* volume, int id, Variant& output)
    {
        output = Variant(nullptr);
        return false;
    }

    /// <summary>
    /// Gets the index of the node archetype this node uses. This is used to determine the inputs and outputs of the node.
    /// </summary>
    /// <returns>The index.</returns>
    API_FUNCTION() FORCE_INLINE virtual int NodeArchetypeIndex() const
    {
        return 0;
    }

public:
    bool GetInputBox(VisjectGraphBox box, PLCTGraphNode*& outConnectedNode, VisjectGraphBox*& outConnectedBox)
    {
        if (!box.HasConnection())
            return false;

        outConnectedBox = box.FirstConnection();
        CHECK_RETURN(outConnectedBox, false);
        outConnectedNode = outConnectedBox->GetParent<PLCTGraphNode>();
        CHECK_RETURN(outConnectedNode, false);

        return true;
    }

    bool GetObjectFromInputBox(VisjectGraphBox box, PLCTGraphNode*& outConnectedNode, PLCTVolume* volume, ScriptingObject*& objectOut)
    {
        VisjectGraphBox* connectedBox;
        if (!GetInputBox(box, outConnectedNode, connectedBox))
            return false;

        Variant output;
        if (!outConnectedNode->Instance->GetOutputBox(*outConnectedNode, volume, connectedBox->ID, output))
            return false;

        if (!output.AsObject)
            return false;

        objectOut = output.AsObject;
        return true;
    }
};

/// <summary>
/// Base class for PLCT Graph nodes that have no output, this is used to evaluate the graph backwards properly.
/// </summary>
API_CLASS(Abstract) class FLAXENGINE_API PLCTNodeEnd : public PLCTNode
{
    DECLARE_SCRIPTING_TYPE_WITH_CONSTRUCTOR_IMPL(PLCTNodeEnd, PLCTNode);

public:
    virtual bool Execute(PLCTGraphNode& node, PLCTVolume* volume)
    {
        return false;
    }
};
