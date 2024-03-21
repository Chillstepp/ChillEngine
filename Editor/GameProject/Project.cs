using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using System.Security.Cryptography.X509Certificates;
using System.Windows;
using System.Windows.Input;
using Editor.Common;
using Editor.Utilities;

namespace Editor.GameProject
{
    [DataContract(Name = "Game")]
    public class Project : ViewModeBase
    {
        public static string Extension { get; } = ".primal";
        [DataMember]
        public string Name { get; private set; }
        
        [DataMember]
        public string Path { get; set; }
        
        public string FullPath => $@"{Path}{Name}\{Name}{Extension}";
        
        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> _scenes = new ObservableCollection<Scene>();
        
        public ReadOnlyObservableCollection<Scene> Scenes { get; private set; }

        private Scene _activeScene;
        
        public Scene ActiveScene
        {
            get => _activeScene;
            set
            {
                if (_activeScene != value)
                {
                    value = _activeScene;
                    OnPropertyChanged(nameof(ActiveScene));
                }
            }
        }

        public ICommand AddSceneCommand { get; private set; }
        public ICommand RemoveSceneCommand { get; private set; }
        
        public ICommand UndoCommand { get; private set; }
        
        public ICommand RedoCommand { get; private set; }
        
        public ICommand SaveCommand { get; private set; }
        
        private void AddSceneInternal(string sceneName)
        {
            Debug.Assert(!String.IsNullOrEmpty(sceneName.Trim()));
            _scenes.Add(new Scene(this, sceneName));
        }
        
        private void RemoveSceneInternal(Scene removeScene)
        {
            Debug.Assert(_scenes.Contains(removeScene));
            _scenes.Remove(removeScene);
        }

        public static Project Current => Application.Current.MainWindow.DataContext as Project;

        public static UndoRedo UndoRedo { get; } = new UndoRedo();

        public static Project Load(string file)
        {
            Debug.Assert(File.Exists(file));
            return Serializer.FromFile<Project>(file);
        }

        public static void Save(Project project)
        {
            Serializer.ToFile(project, project.FullPath);
        }

        public void UnLoad()
        {
            
        }

        [OnDeserialized]
        private void OnDeserialized(StreamingContext context)
        {
            if (_scenes != null)
            {
                Scenes = new ReadOnlyObservableCollection<Scene>(_scenes);
                OnPropertyChanged(nameof(Scenes));
            }
            ActiveScene = Scenes.FirstOrDefault(x => x.IsActive);
            
            AddSceneCommand = new RelayCommand<object>(x =>
            {
                AddSceneInternal($"New Scene {_scenes.Count}");
                var newScene = _scenes.Last();
                var sceneIndex = _scenes.Count - 1;

                UndoRedo.Add(new UndoRedoAction(
                    () => RemoveSceneInternal(newScene),
                    () => _scenes.Insert(sceneIndex, newScene),
                    $"Add {newScene.Name}"));
            });

            RemoveSceneCommand = new RelayCommand<Scene>(x =>
            {
                var sceneIndex = _scenes.IndexOf(x);
                RemoveSceneInternal(x);

                UndoRedo.Add(new UndoRedoAction(
                    () => _scenes.Insert(sceneIndex, x),
                    () => RemoveSceneInternal(x),
                    $"Remove {x.Name}"));
            }, x => !x.IsActive);

            UndoCommand = new RelayCommand<object>(x => UndoRedo.Undo());
            RedoCommand = new RelayCommand<object>(x => UndoRedo.Redo());
            SaveCommand = new RelayCommand<object>(x => Save(this));
        }

        public Project(string name, string path)
        {
            Name = name;
            Path = path;
            
            OnDeserialized(new StreamingContext());
            
        }
    }
}