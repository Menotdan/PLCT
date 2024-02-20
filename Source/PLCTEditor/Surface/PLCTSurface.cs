using FlaxEditor.Content;
using FlaxEditor.Scripting;
using FlaxEditor.Surface.ContextMenu;
using FlaxEditor.Surface.Elements;
using FlaxEngine;
using FlaxEngine.GUI;
using PLCT;
using System;
using System.Collections.Generic;
using static FlaxEditor.Surface.VisjectSurface;

namespace FlaxEditor.Surface
{
    /// <summary>
    /// The Visject Surface implementation for PLCT graphs.
    /// </summary>
    /// <seealso cref="VisjectSurface" />
    [HideInEditor]
    public class PLCTGraphSurface : VisjectSurface
    {
        private static NodesCache _nodesCache = new NodesCache(IterateNodesCache);
        /// <inheritdoc />
        public PLCTGraphSurface(IVisjectSurfaceOwner owner, Action onSave, FlaxEditor.Undo undo = null, SurfaceStyle style = null, List<GroupArchetype> groups = null)
    : base(owner, onSave, undo, style, new List<GroupArchetype>(1) {
                new GroupArchetype()
                {
                    GroupID = 1,
                    Archetypes = PLCTArchetype.Nodes,
                    Color = Color.Crimson,
                    Name = "PLCT",
                }
            })
        {
        }

        private static SurfaceStyle CreateStyle()
        {
            var editor = Editor.Instance;
            var style = SurfaceStyle.CreateStyleHandler(editor);
            //style.DrawBox = DrawBox;
            style.DrawConnection = DrawConnection;
            return style;
        }

        /*private static void DrawBox(Box box)
        {
            var rect = new Rectangle(Float2.Zero, box.Size);
            const float minBoxSize = 5.0f;
            if (rect.Size.LengthSquared < minBoxSize * minBoxSize)
                return;

            var style = FlaxEngine.GUI.Style.Current;
            var color = style.LightBackground;
            if (box.IsMouseOver)
                color *= 1.2f;
            Render2D.FillRectangle(rect, color);
        }*/

        private static void DrawConnection(Float2 start, Float2 end, Color color, float thickness)
        {
            SurfaceStyle.DrawStraightConnection(start, end, color);
        }

        private void OnActiveContextMenuVisibleChanged(Control activeCM)
        {
            _nodesCache.Wait();
        }

        private static void IterateNodesCache(ScriptType scriptType, Dictionary<KeyValuePair<string, ushort>, GroupArchetype> cache, int version)
        {
            // Filter by PLCT node types only
            if (!new ScriptType(typeof(PLCTNode)).IsAssignableFrom(scriptType))
                return;

            // Skip in-built types
            if (scriptType == typeof(PLCTNode) || scriptType == typeof(PLCTNodeEnd) 
                || scriptType == typeof(PLCTNodeFilter) || scriptType == typeof(PLCTNodeFilterSurface))
                return;

            // Create group archetype
            var groupKey = new KeyValuePair<string, ushort>("PLCT", 1);
            if (!cache.TryGetValue(groupKey, out var group))
            {
                group = new GroupArchetype
                {
                    GroupID = groupKey.Value,
                    Name = groupKey.Key,
                    Color = Color.Crimson,
                    Tag = version,
                    Archetypes = new List<NodeArchetype>(),
                };
                cache.Add(groupKey, group);
            }

            PLCTNode nodeInstance = (PLCTNode) scriptType.CreateInstance();
            int archetypeIndex = nodeInstance.NodeArchetypeIndex();

            // Create node archetype
            var node = (NodeArchetype)PLCT.PLCTArchetype.Nodes[archetypeIndex].Clone();
            node.DefaultValues[0] = scriptType.TypeName;
            node.Flags &= ~NodeFlags.NoSpawnViaGUI;
            node.Title = PLCT.PLCTArchetype.Node.GetTitle(scriptType);
            node.Description = Editor.Instance.CodeDocs.GetTooltip(scriptType);
            ((IList<NodeArchetype>)group.Archetypes).Add(node);
        }

        /// <inheritdoc />
        protected override void OnShowPrimaryMenu(VisjectCM activeCM, Float2 location, Box startBox)
        {
            activeCM.ShowExpanded = true;
            _nodesCache.Get(activeCM);

            base.OnShowPrimaryMenu(activeCM, location, startBox);

            activeCM.VisibleChanged += OnActiveContextMenuVisibleChanged;
        }

        /// <inheritdoc />
        public override string GetTypeName(ScriptType type)
        {
            if (type == ScriptType.Void)
                return string.Empty; // Remove `Void` tooltip from connection areas
            return base.GetTypeName(type);
        }

        /// <inheritdoc />
        public override bool CanUseNodeType(GroupArchetype groupArchetype, NodeArchetype nodeArchetype)
        {
            // Comments
            if (groupArchetype.GroupID == 7 && nodeArchetype.TypeID == 11)
                return true;

            // PLCT Graph nodes only
            return (groupArchetype.GroupID == 1 &&
                   base.CanUseNodeType(groupArchetype, nodeArchetype));
        }

        /*/// <inheritdoc />
        protected override bool ValidateDragItem(AssetItem assetItem)
        {
            if (assetItem.IsOfType<BehaviorTree>())
                return true;
            return base.ValidateDragItem(assetItem);
        }*/

        /*/// <inheritdoc />
        protected override void HandleDragDropAssets(List<AssetItem> objects, DragDropEventArgs args)
        {
            for (int i = 0; i < objects.Count; i++)
            {
                var assetItem = objects[i];
                SurfaceNode node = null;

                if (assetItem.IsOfType<BehaviorTree>())
                {
                    var instance = new BehaviorTreeSubTreeNode();
                    instance.Name = Utilities.Utils.GetPropertyNameUI(assetItem.ShortName);
                    instance.Tree = (BehaviorTree)assetItem.LoadAsync();
                    node = Context.SpawnNode(19, 1, args.SurfaceLocation, new object[]
                    {
                        typeof(BehaviorTreeSubTreeNode).FullName,
                        FlaxEngine.Json.JsonSerializer.SaveToBytes(instance),
                        null,
                    });
                    FlaxEngine.Object.Destroy(instance);
                }

                if (node != null)
                    args.SurfaceLocation.X += node.Width + 10;
            }
            base.HandleDragDropAssets(objects, args);
        }*/

        /// <inheritdoc />
        public override void OnDestroy()
        {
            if (IsDisposing)
                return;
            _nodesCache.Wait();

            base.OnDestroy();
        }
    }
}
