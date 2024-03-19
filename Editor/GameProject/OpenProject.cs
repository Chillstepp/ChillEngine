using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.Serialization;
using Editor.Utilities;


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
        public byte[] Icon { get; set; }
        public byte[] Screenshot { get; set; }
        
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

        //读取路径下ProjectDataList, 更新至_projects
        private static void ReadProjectData()
        {
            if (File.Exists(_projectDataPath))
            {
                var projects = Serializer.FromFile<ProjectDataList>(_projectDataPath).Projects
                    .OrderByDescending(x => x.Date);
                _projects.Clear();
                foreach (var project in projects)
                {
                    if (File.Exists(project.FullPath))
                    {
                        project.Icon = File.ReadAllBytes($@"{project.ProjectPath}\.Primal\Icon.png");
                        project.Screenshot = File.ReadAllBytes($@"{project.ProjectPath}\.Primal\Screenshot.png");
                        _projects.Add(project);
                    }
                }
            }
        }
        
        //序列化_projects至xml
        private static void WriteProjectData()
        {
            var projects = _projects.OrderBy(x => x.Date).ToList();
            Serializer.ToFile(new ProjectDataList() {Projects = projects}, _projectDataPath);
        }

        public static Project Open(ProjectData projectData)
        {
            //反序列化：读取xml数据
            ReadProjectData();
            //找到选择的ProjectData
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
            //序列化：保存至xml
            WriteProjectData();
            
            return null;
        }
        static OpenProject()
        {
            try
            {
                
                if (!Directory.Exists(_applicationDataPath)) Directory.CreateDirectory(_applicationDataPath);
                _projectDataPath = $@"{_applicationDataPath}ProjectData.xml";
                Projects = new ReadOnlyObservableCollection<ProjectData>(_projects);
                ReadProjectData();

            }
            catch (Exception e)
            {
                Debug.WriteLine(e);
                //@todo: log errors
            }
        }
    }
}