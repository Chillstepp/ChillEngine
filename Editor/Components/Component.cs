using System.Diagnostics;
using System.Runtime.Serialization;
using Editor.Common;

namespace Editor.Components
{
    [DataContract]
    public class Component : ViewModeBase
    {
        [DataMember]
        public GameEntity Owner { get; private set; }

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }
}