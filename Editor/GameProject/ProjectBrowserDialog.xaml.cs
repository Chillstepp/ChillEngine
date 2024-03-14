using System.Windows;

namespace Editor.GameProject
{
    /// <summary>
    /// Interaction logic for ProjectBrowserDialog.xaml
    /// </summary>
    public partial class ProjectBrowserDialog : Window
    {
        public ProjectBrowserDialog()
        {
            InitializeComponent();
        }

        private void OnToggleButton_Click(object sender, RoutedEventArgs e)
        {
            if(sender == OpenProjectButton)
            {
                if(CreateProjectButton.IsChecked == true)
                {
                    CreateProjectButton.IsChecked = false;
                    BrowserContent.Margin = new Thickness(0);
                }
                OpenProjectButton.IsChecked = true;
            }
            else
            {
                if(OpenProjectButton.IsChecked == true)
                {
                    OpenProjectButton.IsChecked = false;
                    BrowserContent.Margin = new Thickness(-800, 0, 0, 0);
                }
                CreateProjectButton.IsChecked = true;
            }
        }
    }
}
