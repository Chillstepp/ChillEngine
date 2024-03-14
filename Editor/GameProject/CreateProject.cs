using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Editor.Common;
using System.Runtime.Serialization;

namespace Editor.GameProject
{
    [DataContract]
    public class ProjectTemplate
    {
        [DataMember]
        public string ProjectType { get; set; }

        [DataMember]
        public string ProjectFile { get; set; }

        [DataMember]
        public List<string> Folders { get; set; }

    }
    
    public class CreateProject : ViewModeBase
    {
        //@todo: get the path from the installation location
        private readonly string _templatePath = @"..\..\ProjectTemplates";
        private string _name = "NewProject";
        public string Name
        {
            get => _name;
            set
            {
                if(_name != value)
                {
                    _name = value;
                    OnPropertyChanged(nameof(Name));
                }
            }
        }

        private string _path = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\HYProject\";
        public string Path
        {
            get => _path;
            set
            {
                if (_path != value)
                {
                    _path = value;
                    OnPropertyChanged(nameof(Path));
                }
            }
        }
        
        public CreateProject()
        {
            try
            {
                var templateFiles = Directory.GetFiles(_templatePath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templateFiles.Any());
                foreach (var file in templateFiles)
                {
                    var template = new ProjectTemplate()
                    {
                        ProjectType = "Empty Project",
                        ProjectFile = "project.primal",
                        Folders = new List<string>() { ".Primal", "Content", "GameCode"}
                    };
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                //TODO: LOG ERROR
            }
            
        }
    }
}