using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.IO;
using System.Linq;
using Editor.Common;
using System.Runtime.Serialization;
using Editor.Utilities;

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

        public byte[] Icon { get; set; }
        public byte[] Screenshot { get; set; }
        public string IconFilePath { get; set; }
        public string ScreenshotFilePath { get; set; }
        public string ProjectFilePath { get; set; }
        public string TemplatePath { get; set; }
    }
    
    public class CreateProject : ViewModeBase
    {
        //@todo: get the path from the installation location
        private readonly string _templatePath = @"..\..\Editor\ProjectTemplates";
        private string _projectName = "NewProject";
        public string ProjectName
        {
            get => _projectName;
            set
            {

                if(_projectName != value)
                {
                    _projectName = value;
                    ValidateProjectPath();
                    OnPropertyChanged(nameof(ProjectName));
                }
            }
        }

        private string _projectPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\ChillEngineProjects\";
        public string ProjectPath
        {
            get => _projectPath;
            set
            {
                if (_projectPath != value)
                {
                    _projectPath = value;
                    ValidateProjectPath();
                    OnPropertyChanged(nameof(ProjectPath));
                }
            }
        }

        private bool _isValid;
        public bool IsValid
        {
            get => _isValid;
            set
            {
                if (_isValid != value)
                {
                    _isValid = value;
                    OnPropertyChanged(nameof(IsValid));
                }
            }
        }

        private string _errorMsg;
        public string ErrorMsg
        {
            get => _errorMsg;
            set
            {
                if (_errorMsg != value)
                {
                    _errorMsg = value;
                    OnPropertyChanged(nameof(ErrorMsg));
                }
            }
        }
        
        private ObservableCollection<ProjectTemplate> _projectTemplates = new ObservableCollection<ProjectTemplate>();
        public ReadOnlyObservableCollection<ProjectTemplate> ProjectTemplates { get; }

        private bool ValidateProjectPath()
        {
            var path = ProjectPath;
            if (!path.EndsWith(@"\")) path += @"\";
            path += $@"{ProjectName}\";

            IsValid = false;
            if (string.IsNullOrWhiteSpace(ProjectName.Trim()))
            {
                ErrorMsg = "Type in a project name.";
            }
            else if (ProjectName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
            {
                ErrorMsg = "Invalid char used in project name.";
            }
            else if (string.IsNullOrWhiteSpace(ProjectPath.Trim()))
            {
                ErrorMsg = "Select a valid project folder.";
            }
            else if(ProjectPath.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                ErrorMsg = "Invalid char used in project path.";
            }
            else if (Directory.Exists(path) && Directory.EnumerateFileSystemEntries(path).Any())
            {
                ErrorMsg = "Selected project folder already exists and is not empty";
            }
            else
            {
                ErrorMsg = String.Empty;
                IsValid = true;
            }

            return IsValid;
        }

        public string GenerateProject(ProjectTemplate template)
        {
            ValidateProjectPath();
            if (!IsValid)
            {
                return string.Empty;
            }

            if (!ProjectPath.EndsWith(@"\")) ProjectPath += @"\";
            var path = $@"{ProjectPath}{ProjectName}\";

            try
            {
                if (!Directory.Exists(path)) Directory.CreateDirectory(path);
                foreach (var folder in template.Folders)
                {
                    Directory.CreateDirectory(Path.GetFullPath(Path.Combine(Path.GetDirectoryName(path), folder)));
                }

                var dirInfo = new DirectoryInfo(path + @".Primal");
                dirInfo.Attributes |= FileAttributes.Hidden;
                File.Copy(template.IconFilePath, Path.GetFullPath(Path.Combine(dirInfo.FullName, "Icon.png")));
                File.Copy(template.ScreenshotFilePath, Path.GetFullPath(Path.Combine(dirInfo.FullName, "Screenshot.png")));

                // var project = new Project(ProjectName, path);
                // Serializer.ToFile(project, path + $"{ProjectName}" + Project.Extension);
                var projectXml = File.ReadAllText(template.ProjectFilePath);
                projectXml = string.Format(projectXml, ProjectName, path);
                var projectnamedotprimal_path =
                    Path.GetFullPath(Path.Combine(path, $"{ProjectName}{Project.Extension}"));
                File.WriteAllText(projectnamedotprimal_path, projectXml);
                
                //Create MSVC Solution
                CreateMSVCSolution(template, path);


                return path;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                //TODO: LOG ERROR
                return string.Empty;
            }
        }

        private void CreateMSVCSolution(ProjectTemplate template, string projectPath)
        {
            Debug.Assert(File.Exists(Path.Combine(template.TemplatePath, "MSVCSolution")));
            Debug.Assert(File.Exists(Path.Combine(template.TemplatePath, "MSVCProject")));

            var EngineAPIPath = Path.Combine(MainWindow.ChillEnginePath, @"Engine\EngineAPI\");
            Debug.Assert(Directory.Exists(EngineAPIPath));

            var _0 = ProjectName;
            var _1 = "{" + Guid.NewGuid().ToString().ToUpper() + "}";
            var _2 = EngineAPIPath;
            var _3 = MainWindow.ChillEnginePath;
            
            var solution = File.ReadAllText(Path.Combine(template.TemplatePath, "MSVCSolution"));
            solution = String.Format(solution, _0, _1, "{" + Guid.NewGuid().ToString().ToUpper() + "}");
            File.WriteAllText(Path.GetFullPath(Path.Combine(projectPath, $"{_0}.sln")), solution);
            
            var project = File.ReadAllText(Path.Combine(template.TemplatePath, "MSVCProject"));
            project = String.Format(project, _0, _1, _2, _3);
            File.WriteAllText(Path.GetFullPath(Path.Combine(projectPath, $@"GameCode\{_0}.vcxproj")), project);
            

        }

        public CreateProject()
        {
            ProjectTemplates = new ReadOnlyObservableCollection<ProjectTemplate>(_projectTemplates);
            try
            {
                var templateFiles = Directory.GetFiles(_templatePath, "template.xml", SearchOption.AllDirectories);
                Debug.Assert(templateFiles.Any());
                foreach (var file in templateFiles)
                {
                    var template = Serializer.FromFile<ProjectTemplate>(file);
                    template.TemplatePath = Path.GetDirectoryName(file);
                    
                    template.IconFilePath = Path.GetFullPath(Path.Combine(template.TemplatePath, "Icon.png"));
                    template.Icon = File.ReadAllBytes(template.IconFilePath);
                    template.ScreenshotFilePath = Path.GetFullPath(Path.Combine(template.TemplatePath, "Screenshot.png"));
                    template.Screenshot = File.ReadAllBytes(template.ScreenshotFilePath);
                    template.ProjectFilePath = Path.GetFullPath(Path.Combine(template.TemplatePath, template.ProjectFile));
                    
                    
                    _projectTemplates.Add(template);
                }
                //Validate first time.
                ValidateProjectPath();
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex.Message);
                //TODO: LOG ERROR
            }
            
        }
    }
}