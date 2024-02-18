using System;
using System.Collections.Generic;
using FlaxEditor;
using FlaxEditor.Content;
using FlaxEngine;
using PLCT;
using static FlaxEditor.GUI.Docking.DockHintWindow;

namespace PLCTEditor;

/// <summary>
/// PLCTEditorPlugin Script.
/// </summary>
public class PLCTEditorPlugin : EditorPlugin
{
    private PLCTGraphProxy _proxy;

    public override void InitializeEditor()
    {
        base.InitializeEditor();
        _proxy = new PLCTGraphProxy();
        Editor.ContentDatabase.AddProxy(_proxy, true);
        CreatePLCTGraph.RegisterCreator();
    }

    public override void DeinitializeEditor()
    {
        Editor.ContentDatabase.RemoveProxy(_proxy, true);
        _proxy = null;
        base.DeinitializeEditor();
    }
}
