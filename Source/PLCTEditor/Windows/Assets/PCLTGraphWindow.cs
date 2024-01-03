using FlaxEditor.Content;
using FlaxEditor.CustomEditors;
using FlaxEditor.GUI;
using FlaxEditor.SceneGraph;
using FlaxEditor.Scripting;
using FlaxEditor.Surface;
using FlaxEditor.Viewport;
using FlaxEngine;
using FlaxEngine.GUI;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace FlaxEditor.Windows.Assets
{
    /// <summary>
    /// The editor window for PLCT graphs.
    /// </summary>
    public sealed class PLCTGraphWindow : AssetEditorWindowBase<PLCTGraph>, IVisjectSurfaceWindow, IPresenterOwner
    {
        private readonly SplitPanel _split1;
        private readonly SplitPanel _split2;
        private CustomEditorPresenter _nodePropertiesEditor;
        private PLCTGraphSurface _surface;
        private Undo _undo;
        private readonly ToolStripButton _saveButton;
        private readonly ToolStripButton _undoButton;
        private readonly ToolStripButton _redoButton;
        private bool _showWholeGraphOnLoad = true;
        private bool _isWaitingForSurfaceLoad;
        private bool _canEdit = true;

        /// <summary>
        /// Gets the Visject Surface.
        /// </summary>
        public PLCTGraphSurface Surface => _surface;

        /// <summary>
        /// Gets the undo history context for this window.
        /// </summary>
        public Undo Undo => _undo;

        /// <inheritdoc />
        public PLCTGraphWindow(Editor editor, BinaryAssetItem item)
        : base(editor, item)
        {
            var inputOptions = Editor.Options.Options.Input;

            // Undo
            _undo = new Undo();
            _undo.UndoDone += OnUndoRedo;
            _undo.RedoDone += OnUndoRedo;
            _undo.ActionDone += OnUndoAction;

            // Split Panels
            _split1 = new SplitPanel(Orientation.Horizontal, ScrollBars.None, ScrollBars.None)
            {
                AnchorPreset = AnchorPresets.StretchAll,
                Offsets = new Margin(0, 0, _toolstrip.Bottom, 0),
                SplitterValue = 0.7f,
                Parent = this
            };
            _split2 = new SplitPanel(Orientation.Vertical, ScrollBars.Vertical, ScrollBars.Vertical)
            {
                AnchorPreset = AnchorPresets.StretchAll,
                Offsets = Margin.Zero,
                SplitterValue = 0.5f,
                Parent = _split1.Panel2
            };

            // Surface
            _surface = new PLCTGraphSurface(this, Save, _undo)
            {
                Parent = _split1.Panel1,
                Enabled = false
            };
            _surface.SelectionChanged += OnNodeSelectionChanged;

            // Properties editors
            _nodePropertiesEditor = new CustomEditorPresenter(null, null, this); // Surface handles undo for nodes editing
            _nodePropertiesEditor.Features = FeatureFlags.UseDefault;
            _nodePropertiesEditor.Panel.Parent = _split2.Panel1;
            _nodePropertiesEditor.Modified += OnNodePropertyEdited;
            /*_knowledgePropertiesEditor = new CustomEditorPresenter(null, "No blackboard type assigned", this); // No undo for knowledge editing
            _knowledgePropertiesEditor.Features = FeatureFlags.None;
            _knowledgePropertiesEditor.Panel.Parent = _split2.Panel2;*/

            // Toolstrip
            _saveButton = (ToolStripButton)_toolstrip.AddButton(Editor.Icons.Save64, Save).LinkTooltip("Save");
            _toolstrip.AddSeparator();
            _undoButton = (ToolStripButton)_toolstrip.AddButton(Editor.Icons.Undo64, _undo.PerformUndo).LinkTooltip($"Undo ({inputOptions.Undo})");
            _redoButton = (ToolStripButton)_toolstrip.AddButton(Editor.Icons.Redo64, _undo.PerformRedo).LinkTooltip($"Redo ({inputOptions.Redo})");
            _toolstrip.AddSeparator();
            _toolstrip.AddButton(Editor.Icons.Search64, Editor.ContentFinding.ShowSearch).LinkTooltip($"Open content search tool ({inputOptions.Search})");
            _toolstrip.AddButton(editor.Icons.CenterView64, _surface.ShowWholeGraph).LinkTooltip("Show whole graph");
            _toolstrip.AddButton(editor.Icons.Docs64, () => Platform.OpenUrl(Utilities.Constants.DocsUrl + "manual/scripting/plct/plct-graphs/index.html")).LinkTooltip("See documentation to learn more");

            // Setup input actions
            InputActions.Add(options => options.Undo, _undo.PerformUndo);
            InputActions.Add(options => options.Redo, _undo.PerformRedo);
            InputActions.Add(options => options.Search, Editor.ContentFinding.ShowSearch);

            SetCanEdit(!Editor.IsPlayMode);
            //ScriptsBuilder.ScriptsReloadBegin += OnScriptsReloadBegin;
        }

        private void OnUndoRedo(IUndoAction action)
        {
            MarkAsEdited();
            UpdateToolstrip();
            _nodePropertiesEditor.BuildLayoutOnUpdate();
        }

        private void OnUndoAction(IUndoAction action)
        {
            MarkAsEdited();
            UpdateToolstrip();
        }

        private void SetCanEdit(bool canEdit)
        {
            if (_canEdit == canEdit)
                return;
            _canEdit = canEdit;
            _undo.Enabled = canEdit;
            _surface.CanEdit = canEdit;
            _nodePropertiesEditor.ReadOnly = !canEdit;
            UpdateToolstrip();
        }

        private void OnNodeSelectionChanged()
        {
            // Select node instances to view/edit
            var selection = new List<object>();
            var nodes = _surface.Nodes;
            if (nodes != null)
            {
                for (var i = 0; i < nodes.Count; i++)
                {
                    if (nodes[i] is Surface.Archetypes.PLCT.NodeBase node && node.IsSelected && node.Instance)
                        selection.Add(node.Instance);
                }
            }
            _nodePropertiesEditor.Select(selection);
        }

        private void OnNodePropertyEdited()
        {
            _surface.MarkAsEdited();
            var nodes = _surface.Nodes;
            for (var i = 0; i < _nodePropertiesEditor.Selection.Count; i++)
            {
                if (_nodePropertiesEditor.Selection[i] is PLCTNode instance)
                {
                    // Sync instance data with surface node value storage
                    for (var j = 0; j < nodes.Count; j++)
                    {
                        if (nodes[j] is Surface.Archetypes.PLCT.NodeBase node && node.Instance == instance)
                        {
                            node._isValueEditing = true;
                            node.SetValue(1, FlaxEngine.Json.JsonSerializer.SaveToBytes(instance));
                            node._isValueEditing = false;
                            break;
                        }
                    }
                }
            }
        }

        private bool LoadSurface()
        {
            if (_surface.Load())
            {
                Editor.LogError("Failed to load PLCT Graph surface.");
                return true;
            }
            return false;
        }

        private bool SaveSurface()
        {
            _surface.Save();
            return false;
        }

        /// <inheritdoc />
        public override void Save()
        {
            if (!IsEdited || _asset == null || _isWaitingForSurfaceLoad)
                return;

            // Check if surface has been edited
            if (_surface.IsEdited)
            {
                if (SaveSurface())
                    return;
            }

            ClearEditedFlag();
            OnSurfaceEditedChanged();
            _item.RefreshThumbnail();
        }

        /// <inheritdoc />
        protected override void UpdateToolstrip()
        {
            _saveButton.Enabled = _canEdit && IsEdited;
            _undoButton.Enabled = _canEdit && _undo.CanUndo;
            _redoButton.Enabled = _canEdit && _undo.CanRedo;

            base.UpdateToolstrip();
        }

        /// <inheritdoc />
        protected override void OnAssetLinked()
        {
            _isWaitingForSurfaceLoad = true;

            base.OnAssetLinked();
        }

        /// <summary>
        /// Focuses the node.
        /// </summary>
        /// <param name="node">The node.</param>
        public void ShowNode(SurfaceNode node)
        {
            SelectTab();
            RootWindow.Focus();
            Surface.Focus();
            Surface.FocusNode(node);
        }

        /// <inheritdoc />
        public Asset SurfaceAsset => Asset;

        /// <inheritdoc />
        public string SurfaceName => "PLCT Graph";

        /// <inheritdoc />
        public byte[] SurfaceData
        {
            get => _asset.LoadSurface();
            set
            {
                // Save data to the asset
                if (_asset.SaveSurface(value))
                {
                    _surface.MarkAsEdited();
                    Editor.LogError("Failed to save surface data");
                }
                _asset.Reload();
            }
        }

        /// <inheritdoc />
        public VisjectSurfaceContext ParentContext => null;

        /// <inheritdoc />
        public void OnContextCreated(VisjectSurfaceContext context)
        {
        }

        /// <inheritdoc />
        public void OnSurfaceEditedChanged()
        {
            if (_surface.IsEdited)
                MarkAsEdited();
        }

        /// <inheritdoc />
        public void OnSurfaceGraphEdited()
        {
        }

        /// <inheritdoc />
        public void OnSurfaceClose()
        {
            Close();
        }

        /// <inheritdoc />
        protected override void UnlinkItem()
        {
            _isWaitingForSurfaceLoad = false;

            base.UnlinkItem();
        }

        /// <inheritdoc />
        public override void OnPlayBegin()
        {
            base.OnPlayBegin();

            SetCanEdit(false);
        }

        /// <inheritdoc />
        public override void OnPlayEnd()
        {
            SetCanEdit(true);

            base.OnPlayEnd();
        }

        /// <inheritdoc />
        public override void Update(float deltaTime)
        {
            // Wait for asset loaded
            if (_isWaitingForSurfaceLoad && _asset.IsLoaded)
            {
                _isWaitingForSurfaceLoad = false;
                if (LoadSurface())
                {
                    Close();
                    return;
                }

                // Setup
                _undo.Clear();
                _surface.Enabled = true;
                _nodePropertiesEditor.BuildLayout();
                ClearEditedFlag();
                if (_showWholeGraphOnLoad)
                {
                    _showWholeGraphOnLoad = false;
                    _surface.ShowWholeGraph();
                }
                SurfaceLoaded?.Invoke();
            }

            base.Update(deltaTime);
        }

        /// <inheritdoc />
        public override bool UseLayoutData => true;

        /// <inheritdoc />
        public override void OnLayoutSerialize(XmlWriter writer)
        {
            LayoutSerializeSplitter(writer, "Split1", _split1);
            LayoutSerializeSplitter(writer, "Split2", _split1);
        }

        /// <inheritdoc />
        public override void OnLayoutDeserialize(XmlElement node)
        {
            LayoutDeserializeSplitter(node, "Split1", _split1);
            LayoutDeserializeSplitter(node, "Split2", _split2);
        }

        /// <inheritdoc />
        public override void OnLayoutDeserialize()
        {
            _split1.SplitterValue = 0.7f;
            _split2.SplitterValue = 0.5f;
        }

        /*private void OnScriptsReloadBegin()
        {
            // TODO: impl hot-reload for BT to nicely refresh state (save asset, clear undo/properties, reload surface)
            Close();
        }*/

        /// <inheritdoc />
        public override void OnDestroy()
        {
            if (IsDisposing)
                return;
            //ScriptsBuilder.ScriptsReloadBegin -= OnScriptsReloadBegin;
            _undo.Enabled = false;
            _nodePropertiesEditor.Deselect();
            _undo.Clear();

            base.OnDestroy();
        }

        /// <inheritdoc />
        public IEnumerable<ScriptType> NewParameterTypes => Editor.CodeEditing.VisualScriptPropertyTypes.Get();

        /// <inheritdoc />
        public event Action SurfaceLoaded;

        /// <inheritdoc />
        public void OnParamRenameUndo()
        {
        }

        /// <inheritdoc />
        public void OnParamEditAttributesUndo()
        {
        }

        /// <inheritdoc />
        public void OnParamAddUndo()
        {
        }

        /// <inheritdoc />
        public void OnParamRemoveUndo()
        {
        }

        /// <inheritdoc />
        public object GetParameter(int index)
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc />
        public void SetParameter(int index, object value)
        {
            throw new NotSupportedException();
        }

        /// <inheritdoc />
        public Asset VisjectAsset => Asset;

        /// <inheritdoc />
        public VisjectSurface VisjectSurface => _surface;

        /// <inheritdoc />
        public EditorViewport PresenterViewport => null;

        /// <inheritdoc />
        public void Select(List<SceneGraphNode> nodes)
        {
        }
    }
}
