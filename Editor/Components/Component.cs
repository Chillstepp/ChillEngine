using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
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
        public abstract IMSComponent GetMultiSelectionComponent(MSEntity msEntity);
        
        [DataMember]
        public GameEntity Owner { get; private set; }

        public Component(GameEntity owner)
        {
            Debug.Assert(owner != null);
            Owner = owner;
        }
    }

    
    //Multiselection component, this is a abstraction of different components
    abstract class MSComponent<T> : ViewModeBase, IMSComponent where T : Component
    {
        //Prevent cyclic trigger
        private bool _enableUpdates = true;
        public List<T> SelectedComponents { get; }

        protected abstract bool UpdateComponents(string propertyName);
        protected abstract bool UpdateMSComponent();
        
        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSComponent();
            _enableUpdates = true;
        }
        public MSComponent(MSEntity msEntity)
        {
            Debug.Assert(msEntity?.SelectedEnties?.Any() == true);
            SelectedComponents = msEntity.SelectedEnties.Select(entity => entity.GetComponent<T>()).ToList();
            PropertyChanged += (s, e) =>
            {
                if (_enableUpdates) UpdateComponents(e.PropertyName);
            };
        }
    }
}