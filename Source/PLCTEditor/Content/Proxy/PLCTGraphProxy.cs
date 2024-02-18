using System;
using System.IO;
using FlaxEditor;
using FlaxEditor.Content;
using FlaxEditor.Content.Thumbnails;
using FlaxEditor.Windows;
using FlaxEditor.Windows.Assets;
using FlaxEngine;
using FlaxEngine.GUI;

namespace PLCT
{
    /// <summary>
    /// A <see cref="PLCTGraph"/> asset proxy object.
    /// </summary>
    /// <seealso cref="FlaxEditor.Content.BinaryAssetProxy" />
    [ContentContextMenu("New/PLCT/PLCT Graph")]
    public class PLCTGraphProxy : BinaryAssetProxy
    {
        /// <inheritdoc />
        public override string Name => "PLCT Graph";

        /// <inheritdoc />
        public override bool CanReimport(ContentItem item)
        {
            return true;
        }

        /// <inheritdoc />
        public override EditorWindow Open(Editor editor, ContentItem item)
        {
            return new PLCTGraphWindow(editor, item as BinaryAssetItem);
        }

        /// <inheritdoc />
        public override Color AccentColor => Color.FromRGB(0x3256A8);

        /// <inheritdoc />
        public override Type AssetType => typeof(PLCTGraph);

        /// <inheritdoc />
        public override bool CanCreate(ContentFolder targetLocation)
        {
            return targetLocation.CanHaveAssets;
        }

        /// <inheritdoc />
        public override void Create(string outputPath, object arg)
        {
            if (Editor.CreateAsset("PLCTGraph", outputPath))
                throw new Exception("Failed to create new asset.");
        }

        /// <inheritdoc />
        public override void OnThumbnailDrawBegin(ThumbnailRequest request, ContainerControl guiRoot, GPUContext context)
        {
            guiRoot.AddChild(new Label
            {
                Text = Path.GetFileNameWithoutExtension(request.Asset.Path),
                Offsets = Margin.Zero,
                AnchorPreset = AnchorPresets.StretchAll,
                Wrapping = TextWrapping.WrapWords
            });
        }
    }
}
