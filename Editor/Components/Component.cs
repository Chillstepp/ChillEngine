using System.Diagnostics;
using System.Runtime.Serialization;
using Editor.Common;

namespace Editor.Components
{
    interface IMSComponent
    {
        
    }
    [DataContract]
    abstract class Component : ViewModeBase
    {
        [DataMember]
        public GameEntity Owner { get; private set; }

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }

    abstract class MSComponent<T> : ViewModeBase, IMSComponent where T : Component
    {
        
    }
}