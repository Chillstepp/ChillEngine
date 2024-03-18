using System.Collections.ObjectModel;
using System.Runtime.Serialization;
using Editor.Common;

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
        
        public string FullPath => $"{Path}{Name}{Extension}";
        
        [DataMember(Name = "Scenes")]
        private ObservableCollection<Scene> _scenes = new ObservableCollection<Scene>();
        
        public ReadOnlyObservableCollection<Scene> Scenes { get; }

        public Project(string name, string path)
        {
            Name = name;
            Path = path;
            
            _scenes.Add(new Scene(this, "Default Scene"));
        }
    }
}