#if USE_LARGE_WORLDS
using Real = System.Double;
#else
using Real = System.Single;
#endif

using FlaxEngine;
using FlaxEngine.GUI;
using FlaxEditor.SceneGraph;

namespace PLCT
{
    /// <summary>
    /// Scene tree node for <see cref="PLCTVolume"/> actor type.
    /// </summary>
    /// <seealso cref="ActorNode" />
    [HideInEditor]
    public sealed class PLCTVolumeNode : ActorNode
    {
        /// <inheritdoc />
        public PLCTVolumeNode(Actor actor)
        : base(actor)
        {
        }

        /// <inheritdoc />
        public override void PostSpawn()
        {
            base.PostSpawn();

            if (Actor.HasPrefabLink)
            {
                return;
            }

            PLCTVolume volume = Actor as PLCTVolume;
            if (!Actor.GetChild<EmptyActor>() && !volume.GenerationContainer)
            {
                EmptyActor generationContainer = new EmptyActor();
                generationContainer.Name = "Generation Container";
                generationContainer.Parent = Actor;

                volume.GenerationContainer = generationContainer;
            }
        }
    }
}
