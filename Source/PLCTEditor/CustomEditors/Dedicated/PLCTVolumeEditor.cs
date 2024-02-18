using FlaxEngine;
using FlaxEditor.CustomEditors.Dedicated;
using FlaxEditor.CustomEditors;
using FlaxEditor.Scripting;
using FlaxEditor;

namespace PLCT
{
    /// <summary>
    /// Dedicated custom editor for PLCTVolume objects.
    /// </summary>
    [CustomEditor(typeof(PLCTVolume)), DefaultEditor]
    public class PLCTVolumeEditor : ActorEditor
    {
        /// <inheritdoc />
        public override void Initialize(LayoutElementsContainer layout)
        {
            base.Initialize(layout);

            layout.Space(20f);
            layout.Button("Generate", Editor.Instance.CodeDocs.GetTooltip(new ScriptMemberInfo(typeof(PLCTVolume).GetMethod("OnGenerateClicked")))).Button.Clicked += OnGenerateClicked;
            layout.Button("Cleanup", "Clean up the generated actors.").Button.Clicked += OnCleanupClicked;
        }

        private void OnGenerateClicked()
        {
            foreach (var value in Values)
            {
                if (value is PLCTVolume volume)
                    volume.Generate();
            }
        }

        private void OnCleanupClicked()
        {
            foreach (var value in Values)
            {
                if (value is PLCTVolume volume)
                {
                    volume.Cleanup();
                }
            }
        }
    }
}
