using System;
using FlaxEngine;

namespace PLCT
{
    /// <summary>
    /// The sample game plugin.
    /// </summary>
    /// <seealso cref="FlaxEngine.GamePlugin" />
    public class PLCT : GamePlugin
    {
        /// <inheritdoc />
        public PLCT()
        {
            _description = new PluginDescription
            {
                Name = "PLCT",
                Category = "Other",
                Author = "Cobolt Games",
                AuthorUrl = null,
                HomepageUrl = null,
                RepositoryUrl = "https://github.com/FlaxEngine/PLCT",
                Description = "This is an example plugin project.",
                Version = new Version(1, 0, 0),
                IsAlpha = false,
                IsBeta = false,
            };
        }

        /// <inheritdoc />
        public override void Initialize()
        {
            base.Initialize();

            Debug.Log("Hello from plugin code!");
        }

        /// <inheritdoc />
        public override void Deinitialize()
        {
            // Use it to cleanup data

            base.Deinitialize();
        }
    }
}
