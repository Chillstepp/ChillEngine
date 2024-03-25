using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Runtime.Serialization;
using System.Windows.Input;
using Editor.Common;
using Editor.Components;
using Editor.Utilities;

namespace Editor.GameProject
{
    [DataContract]
    class Scene : ViewModeBase
    {
        private string _name;
        
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
        
        [DataMember]
        public Project Project { get; private set; }

        private bool _isActive;

        [DataMember]
        public bool IsActive
        {
            get => _isActive;
            set
            {
                if (_isActive != value)
                {
                    _isActive = value;
                    OnPropertyChanged(nameof(IsActive));
                }
            }
        }
        
        [DataMember(Name = nameof(GameEntities))]
        private ObservableCollection<GameEntity> _gameEntities = new ObservableCollection<GameEntity>();
        public ReadOnlyObservableCollection<GameEntity> GameEntities { get; private set; }
        
        public ICommand AddGameEntityCommand { get; private set; }
        public ICommand RemoveGameEntityCommand { get; private set; }
        private void AddGameEntityInternal(GameEntity entity,int entityIdx)
        {
            _gameEntities.Insert(entityIdx, entity);
        }
        private void AddGameEntityInternal(GameEntity entity)
        {
            _gameEntities.Add(entity);
        }
        
        private void RemoveGameEntityInternal(GameEntity entity)
        {
            Debug.Assert(_gameEntities.Contains(entity));
            _gameEntities.Remove(entity);
        }
        

        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (_gameEntities == null)
            {
                _gameEntities = new ObservableCollection<GameEntity>();
            }
            if (_gameEntities != null)
            {
                GameEntities = new ReadOnlyObservableCollection<GameEntity>(_gameEntities);
                OnPropertyChanged(nameof(GameEntities));
            }
            AddGameEntityCommand = new RelayCommand<GameEntity>(x =>
            {
                AddGameEntityInternal(x);
                var entityIndex = _gameEntities.Count - 1;

                Project.UndoRedo.Add(new UndoRedoAction(
                    () => RemoveGameEntityInternal(x),
                    () => AddGameEntityInternal(x, entityIndex),
                    $"Add {x.Name} to {Name}"));
            });

            RemoveGameEntityCommand = new RelayCommand<GameEntity>(x =>
            {
                var entityIndex = _gameEntities.IndexOf(x);
                RemoveGameEntityInternal(x);

                Project.UndoRedo.Add(new UndoRedoAction(
                    () => AddGameEntityInternal(x, entityIndex),
                    () => RemoveGameEntityInternal(x),
                    $"Remove {x.Name} from {Name}"));
            });
        }
        public Scene(Project project, string name)
        {
            Debug.Assert(project != null);
            Project = project;
            Name = name;
            OnDeserialized(new StreamingContext());
        }
    }
}