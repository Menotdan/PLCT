using Flax.Build;
using Flax.Build.NativeCpp;

public class PLCT : GameModule
{
    /// <inheritdoc />
    public override void Setup(BuildOptions options)
    {
        base.Setup(options);

        options.PublicDependencies.Add("Visject");
        options.PublicDependencies.Add("Threading");
        options.PublicDependencies.Add("Terrain");
        options.PublicDependencies.Add("Level");
        options.PublicDependencies.Add("Debug");
        options.PublicDependencies.Add("Scripting");
        options.PublicDependencies.Add("Serialization");

        options.PublicDefinitions.Add("COMPILE_WITH_ASSETS_IMPORTER");

        // Here you can modify the build options for your game module
        // To reference another module use: options.PublicDependencies.Add("Audio");
        // To add C++ define use: options.PublicDefinitions.Add("COMPILE_WITH_FLAX");
        // To learn more see scripting documentation.
        BuildNativeCode = true;
    }
}