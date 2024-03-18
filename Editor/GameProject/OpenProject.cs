using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;


namespace Editor.GameProject
{
    [DataContract]
    public class ProjectData
    {
        [DataMember]
        public string ProjectName { get; set; }
        [DataMember]
        public string ProjectPath { get; set; }
        [DataMember]
        public DateTime Date { get; set; }
        
        public string FullPath
        {
            get => $"{ProjectPath}{ProjectName}{Project.Extension}";
        }
    }

    [DataContract]
    public class ProjectDataList
    {
        [DataMember]
        public List<ProjectData> Projects { get; set; }
    }
    class OpenProject
    {
        private static readonly string _applicationDataPath =
            $@"{Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData)}\Editor\";

        private static readonly string _projectDataPath;
        private static readonly ObservableCollection<ProjectData> _projects = new ObservableCollection<ProjectData>();
        public static ReadOnlyObservableCollection<ProjectData> Projects { get; }

        private static void ReadProjectData()
        {
            
        }

        public static Project Open(ProjectData projectData)
        {
            ReadProjectData();
            var project = _projects.FirstOrDefault(x => x.FullPath == projectData.FullPath);
            if (project != null)
            {
                projectData.Date = DateTime.Now;
            }
            else
            {
                project = projectData;
                project.Date = DateTime.Now;
                _projects.Add(project);
            }
        }
        static OpenProject()
        {
            try
            {
                
                if (!Directory.Exists(_applicationDataPath)) Directory.CreateDirectory(_applicationDataPath);
                _projectDataPath = $@"{_applicationDataPath}ProjectData.xml";
                
                
            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
                //@todo: log errors
            }
        }
    }
}