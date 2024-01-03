using Flax.Build;

public class PLCTEditorTarget : GameProjectEditorTarget
{
    /// <inheritdoc />
    public override void Init()
    {
        base.Init();

        // Reference the modules for editor
        Modules.Add("PLCT");
        Modules.Add("PLCTEditor");
    }
}
