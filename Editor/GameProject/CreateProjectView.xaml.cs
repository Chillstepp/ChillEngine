using System.Windows;
using System.Windows.Controls;

namespace Editor.GameProject
{
    public partial class CreateProjectView : UserControl
    {
        public CreateProjectView()
        {
            InitializeComponent();
        }

        private void OnCreateButton_Click(object sender, RoutedEventArgs e)
        {
            var vm = DataContext as CreateProject;
            var projectpath = vm.GenerateProject(TemplateListBox.SelectedItem as ProjectTemplate);
            var win = Window.GetWindow(this);
            bool dialogResult = false;
            if(!string.IsNullOrEmpty(projectpath))
            {
                dialogResult = true;
                var project = OpenProject.Open(new ProjectData() { ProjectName = vm.ProjectName, ProjectPath = projectpath });
                win.DataContext = project;
            }
            win.DialogResult = dialogResult;
            win.Close();
        }
    }
}