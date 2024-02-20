using FlaxEditor;
using FlaxEditor.GUI.ContextMenu;
using FlaxEditor.Scripting;
using FlaxEditor.Surface;
using FlaxEditor.Utilities;
using FlaxEngine;
using FlaxEngine.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PLCT
{
    /// <summary>
    /// Contains archetypes for nodes from the PLCT Graph group.
    /// </summary>
    [HideInEditor]
    public static class PLCTArchetype
    {
        /// <summary>
        /// Base class for PLCT Graph nodes wrapped inside <see cref="SurfaceNode" />.
        /// </summary>
        internal class NodeBase : SurfaceNode
        {
            protected const float ConnectionAreaMargin = 12.0f;
            protected const float ConnectionAreaHeight = 12.0f;
            protected const float DecoratorsMarginX = 5.0f;
            protected const float DecoratorsMarginY = 2.0f;

            protected ScriptType _type;
            internal bool _isValueEditing;

            public PLCTNode Instance;

            protected NodeBase(uint id, VisjectSurfaceContext context, NodeArchetype nodeArch, GroupArchetype groupArch)
            : base(id, context, nodeArch, groupArch)
            {
            }

            public static string GetTitle(ScriptType scriptType)
            {
                var title = scriptType.Name;
                if (title.StartsWith("PLCT"))
                    title = title.Substring(4);
                if (title.EndsWith("Node"))
                    title = title.Substring(0, title.Length - 4);
                title = FlaxEditor.Utilities.Utils.GetPropertyNameUI(title);
                return title;
            }

            protected virtual void UpdateTitle()
            {
                string title = null;
                if (Instance != null)
                {
                    title = GetTitle(_type);
                }
                else
                {
                    var typeName = (string)Values[0];
                    title = "Missing Type " + typeName;
                }
                Title = title;
            }

            public override void OnLoaded(SurfaceNodeActions action)
            {
                base.OnLoaded(action);

                // Setup node type and data
                var typeName = (string)Values[0];
                _type = TypeUtils.GetType(typeName);
                if (_type != null)
                {
                    TooltipText = Editor.Instance.CodeDocs.GetTooltip(_type);
                    try
                    {
                        // Load node instance from data
                        Instance = (PLCTNode)_type.CreateInstance();
                        var instanceData = (byte[])Values[1];
                        FlaxEngine.Json.JsonSerializer.LoadFromBytes(Instance, instanceData, Globals.EngineBuildNumber);
                    }
                    catch (Exception ex)
                    {
                        Editor.LogError("Failed to load PLCT Graph node of type " + typeName);
                        Editor.LogWarning(ex);
                    }
                }
                else
                {
                    Instance = null;
                }

                UpdateTitle();
            }

            public override void OnValuesChanged()
            {
                base.OnValuesChanged();

                // Skip updating instance when it's being edited by user via UI
                if (!_isValueEditing)
                {
                    try
                    {
                        if (Instance != null)
                        {
                            // Reload node instance from data
                            var instanceData = (byte[])Values[1];
                            if (instanceData == null || instanceData.Length == 0)
                            {
                                // Recreate instance data to default state if previous state was empty
                                var defaultInstance = (PLCTNode)_type.CreateInstance(); // TODO: use default instance from native ScriptingType
                                instanceData = FlaxEngine.Json.JsonSerializer.SaveToBytes(defaultInstance);
                            }
                            FlaxEngine.Json.JsonSerializer.LoadFromBytes(Instance, instanceData, Globals.EngineBuildNumber);
                        }
                    }
                    catch (Exception ex)
                    {
                        Editor.LogError("Failed to load PLCT Graph node of type " + _type);
                        Editor.LogWarning(ex);
                    }
                }

                UpdateTitle();
            }

            public override void OnSpawned(SurfaceNodeActions action)
            {
                base.OnSpawned(action);

                ResizeAuto();
            }

            public override void Draw()
            {
                base.Draw();
            }

            public override void OnDestroy()
            {
                if (IsDisposing)
                    return;
                _type = ScriptType.Null;
                FlaxEngine.Object.Destroy(ref Instance);

                base.OnDestroy();
            }
        }

        /// <summary>
        /// Customized <see cref="SurfaceNode" /> for the PLCT Graph node.
        /// </summary>
        internal class Node : NodeBase
        {
            internal static SurfaceNode Create(uint id, VisjectSurfaceContext context, NodeArchetype nodeArch, GroupArchetype groupArch)
            {
                return new Node(id, context, nodeArch, groupArch);
            }

            internal Node(uint id, VisjectSurfaceContext context, NodeArchetype nodeArch, GroupArchetype groupArch)
            : base(id, context, nodeArch, groupArch)
            {
            }

            public override void OnShowSecondaryContextMenu(FlaxEditor.GUI.ContextMenu.ContextMenu menu, Float2 location)
            {
                base.OnShowSecondaryContextMenu(menu, location);
            }

            public override void OnValuesChanged()
            {
                base.OnValuesChanged();

                ResizeAuto();
            }

            public override void OnLoaded(SurfaceNodeActions action)
            {
                base.OnLoaded(action);

                ResizeAuto();
            }

            public override unsafe void OnPasted(Dictionary<uint, uint> idsMapping)
            {
                base.OnPasted(idsMapping);
            }

            public override void OnSurfaceLoaded(SurfaceNodeActions action)
            {
                base.OnSurfaceLoaded(action);

                ResizeAuto();
                Surface.NodeDeleted += OnNodeDeleted;
            }

            private void OnNodeDeleted(SurfaceNode node)
            {
            }

            protected override void UpdateRectangles()
            {
                base.UpdateRectangles();
            }

            protected override void OnLocationChanged()
            {
                base.OnLocationChanged();

                // Sync attached elements placement
                UpdateRectangles();
            }
        }

        /// <summary>
        /// The nodes for this group.
        /// </summary>
        public static NodeArchetype[] Nodes =
        {
            new NodeArchetype
            {
                TypeID = 1,
                Create = Node.Create,
                Flags = NodeFlags.NoSpawnViaGUI,
                Title = "PLCT Node Get Surfaces",
                DefaultValues = new object[]
                {
                    string.Empty, // Type Name
                    FlaxEngine.Utils.GetEmptyArray<byte>(), // Instance Data
                },
                Size = new Float2(100, 100),
                Elements = new[]
                {
                    NodeElementArchetype.Factory.Output(0, "Surfaces", new ScriptType(typeof(PLCTSurfaceList)), 0),
                }
            },
            new NodeArchetype
            {
                TypeID = 2,
                Create = Node.Create,
                Flags = NodeFlags.NoSpawnViaGUI,
                Title = "PLCT Node Surface Input",
                DefaultValues = new object[]
                {
                    string.Empty, // Type Name
                    FlaxEngine.Utils.GetEmptyArray<byte>(), // Instance Data
                },
                Size = new Float2(100, 100),
                Elements = new[]
                {
                    NodeElementArchetype.Factory.Input(0, "Surfaces", true, new ScriptType(typeof(PLCTSurfaceList)), 0),
                }
            },
            new NodeArchetype
            {
                TypeID = 3,
                Create = Node.Create,
                Flags = NodeFlags.NoSpawnViaGUI,
                Title = "PLCT SurfaceSample",
                DefaultValues = new object[]
                {
                    string.Empty, // Type Name
                    FlaxEngine.Utils.GetEmptyArray<byte>(), // Instance Data
                },
                Size = new Float2(100, 100),
                Elements = new[]
                {
                    NodeElementArchetype.Factory.Input(0, "Surfaces", true, new ScriptType(typeof(PLCTSurfaceList)), 0),
                    NodeElementArchetype.Factory.Output(0, "Points", new ScriptType(typeof(PLCTPointsContainer)), 1),
                }
            },
            new NodeArchetype
            {
                TypeID = 4,
                Create = Node.Create,
                Flags = NodeFlags.NoSpawnViaGUI,
                Title = "PLCT Points Output",
                DefaultValues = new object[]
                {
                    string.Empty, // Type Name
                    FlaxEngine.Utils.GetEmptyArray<byte>(), // Instance Data
                },
                Size = new Float2(100, 100),
                Elements = new[]
                {
                    NodeElementArchetype.Factory.Input(0, "Points", true, new ScriptType(typeof(PLCTPointsContainer)), 0),
                }
            },
            new NodeArchetype
            {
                TypeID = 5,
                Create = Node.Create,
                Flags = NodeFlags.NoSpawnViaGUI,
                Title = "PLCT Points Input and Output",
                DefaultValues = new object[]
                {
                    string.Empty, // Type Name
                    FlaxEngine.Utils.GetEmptyArray<byte>(), // Instance Data
                },
                Size = new Float2(100, 100),
                Elements = new[]
                {
                    NodeElementArchetype.Factory.Input(0, "Points", true, new ScriptType(typeof(PLCTPointsContainer)), 0),
                    NodeElementArchetype.Factory.Output(0, "Points", new ScriptType(typeof(PLCTPointsContainer)), 1),
                }
            },
            new NodeArchetype
            {
                TypeID = 6,
                Create = Node.Create,
                Flags = NodeFlags.NoSpawnViaGUI,
                Title = "PLCT Node Surface Input Surface Output",
                DefaultValues = new object[]
                {
                    string.Empty, // Type Name
                    FlaxEngine.Utils.GetEmptyArray<byte>(), // Instance Data
                },
                Size = new Float2(100, 100),
                Elements = new[]
                {
                    NodeElementArchetype.Factory.Input(0, "Surface", true, new ScriptType(typeof(PLCTSurfaceList)), 0),
                    NodeElementArchetype.Factory.Output(0, "Surface", new ScriptType(typeof(PLCTSurfaceList)), 1),
                }
            },
        };
    }
}
