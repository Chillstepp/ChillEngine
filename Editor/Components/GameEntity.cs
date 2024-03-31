using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Runtime.Serialization;
using System.Windows.Input;
using Editor.Common;
using Editor.DLLWrapper;
using Editor.GameProject;
using Editor.Utilities;

namespace Editor.Components
{
    [DataContract]
    [KnownType(typeof(Transform))]
    class GameEntity : ViewModeBase
    {
        private int _entityId = ID.INVALID_ID;
        public int EntityId
        {
            get => _entityId;
            set
            {
                if (_entityId != value)
                {
                    _entityId = value;
                    OnPropertyChanged(nameof(EntityId));
                }
            }
        }

        private bool _isActive;

        public bool IsActive
        {
            get => _isActive;
            set
            {
                if (_isActive != value)
                {
                    _isActive = value;
                    if (_isActive)
                    {
                        EntityId = EngineAPI.CreateGameEntity(this);
                        Debug.Assert(ID.IsValid(_entityId));
                    }
                    else if(ID.IsValid(EntityId))
                    {
                        EngineAPI.RemoveGameEntity(this);
                        EntityId = ID.INVALID_ID;
                    }
                    OnPropertyChanged(nameof(IsActive));
                }
            }
        }
        
        public string _name;

        [DataMember]
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }
        
        // public ICommand RenameCommand { get; private set; }
        // public ICommand IsEnableCommand { get; private set; }

        private bool _isEnabled = true;

        public bool IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }
        
        [DataMember]
        public Scene ParentScene { get; private set; }

        [DataMember(Name = nameof(Components))]
        private ObservableCollection<Component> _components = new ObservableCollection<Component>();
        public ReadOnlyObservableCollection<Component> Components { get; private set; }

        public Component GetComponent(Type type) => Components.FirstOrDefault(c => c.GetType() == type);
        public T GetComponent<T>() where T : Component => GetComponent(typeof(T)) as T;
        [OnDeserialized]
        void OnDeserialized(StreamingContext context)
        {
            if (_components != null)
            {
                Components = new ReadOnlyObservableCollection<Component>(_components);
                OnPropertyChanged(nameof(Components));
            }

            // RenameCommand = new RelayCommand<string>(x =>
            // {
            //     var oldName = _name;
            //     Name = x;
            //     Project.UndoRedo.Add(new UndoRedoAction(nameof(Name), this, oldName, x,
            //         $"Rename entity '{oldName}' to '{x}"));
            // }, x => x != _name);
            //
            //
            // IsEnableCommand = new RelayCommand<bool>(x =>
            // {
            //     var oldValue = _isEnabled;
            //     IsEnabled = x;
            //     Project.UndoRedo.Add(new UndoRedoAction(nameof(IsEnabled), this, oldValue, x,
            //         x?$"Enable entity '{Name}'":$"Disable entity '{Name}'"));
            // });
        }
        public GameEntity(Scene scene)
        {
            Debug.Assert(scene != null);
            ParentScene = scene;
            _components.Add(new Transform(this));
            OnDeserialized(new StreamingContext());
        }
        
        
    }

    abstract class MSEntity : ViewModeBase
    {
        private bool _enableUpdates = true;
        
        private bool? _isEnabled;

        public bool? IsEnabled
        {
            get => _isEnabled;
            set
            {
                if (_isEnabled != value)
                {
                    _isEnabled = value;
                    OnPropertyChanged(nameof(IsEnabled));
                }
            }
        }

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                if (_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        private readonly ObservableCollection<IMSComponent> _components = new ObservableCollection<IMSComponent>();
        public ReadOnlyObservableCollection<IMSComponent> Components { get; }

        public T GetMSComponent<T>() where T : IMSComponent
        {
            return (T)Components.FirstOrDefault(x => x.GetType() == typeof(T));
        }
        public List<GameEntity> SelectedEnties { get; }
        private void MakeComponentList()
        {
            _components.Clear();
            var firstEntity = SelectedEnties.First();
            if (firstEntity == null) return;
            foreach (var component in firstEntity.Components)
            {
                var type = component.GetType();
                if (!SelectedEnties.Skip(1).Any(entity => entity.GetComponent(type) == null))
                {
                    Debug.Assert(Components.FirstOrDefault(x => x.GetType() == type) == null);
                    _components.Add(component.GetMultiSelectionComponent(this));
                }
            }
        }
        

        public MSEntity(List<GameEntity>gameEntities)
        {
            Debug.Assert(gameEntities?.Any() == true);
            Components = new ReadOnlyObservableCollection<IMSComponent>(_components);
            SelectedEnties = gameEntities;
            PropertyChanged += (s, e) => { if(_enableUpdates) UpdateGameEntities(e.PropertyName); };
            
        }

        
        public static float? GetMixedValue<T>(List<T> objs, Func<T, float> getProperty)
        {
            var value = getProperty(objs.First());
            return objs.Skip(1).Any(x => !getProperty(x).IsTheSameAs(value)) ? (float?) null : value;
        }
        
        public static bool? GetMixedValue(List<GameEntity> objs, Func<GameEntity, bool> getProperty)
        {
            var value = getProperty(objs.First());
            return objs.Skip(1).Any(x => getProperty(x) != value) ? (bool?) null : value;
        }
        
        public static string GetMixedValue(List<GameEntity> objs, Func<GameEntity, string> getProperty)
        {
            var value = getProperty(objs.First());
            return objs.Skip(1).Any(x => getProperty(x) != value) ? null : value;
        }

        protected virtual bool UpdateMSGameEntity()
        {
            IsEnabled = GetMixedValue(SelectedEnties, new Func<GameEntity, bool>(x => x.IsEnabled));
            Name = GetMixedValue(SelectedEnties, new Func<GameEntity, string>(x => x.Name));
            return true;
        }

        public void Refresh()
        {
            _enableUpdates = false;
            UpdateMSGameEntity();
            MakeComponentList();
            _enableUpdates = true;
        }
        

        protected virtual bool UpdateGameEntities(string propertyName)
        {
            switch (propertyName)
            {
                case nameof(IsEnabled): SelectedEnties.ForEach(x => x.IsEnabled = IsEnabled.Value);
                    return true;
                case nameof(Name): SelectedEnties.ForEach(x => x.Name = Name);
                    return true;
            }
            
            return false;
        }
    }
    
    class MSGameEntity : MSEntity
    {
        public MSGameEntity(List<GameEntity>entities) : base(entities)
        {
            Refresh();
        }
    }
}