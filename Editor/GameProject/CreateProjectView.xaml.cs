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
            bool dialogResult = !string.IsNullOrEmpty(projectpath);
            var win = Window.GetWindow(this);
            win.DialogResult = dialogResult;
            win.Close();
        }
    }
}