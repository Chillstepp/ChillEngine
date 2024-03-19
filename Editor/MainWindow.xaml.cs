using System;
using System.ComponentModel;
using System.Windows;
using Editor.GameProject;

namespace Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        public MainWindow()
        {
            InitializeComponent();
            Loaded += OnMainWindowLoaded;
            Closing += OnMainWindowClosing;
        }
        private void OnMainWindowLoaded(Object sender, RoutedEventArgs e)
        {
            Loaded -= OnMainWindowLoaded;
            OpenProjectBrowserDialog();
        }

        private void OnMainWindowClosing(Object sender, CancelEventArgs e)
        {
            Closing -= OnMainWindowClosing;
            Project.Current?.UnLoad();
        }

        private void OpenProjectBrowserDialog()
        {
            var projectBrowser = new GameProject.ProjectBrowserDialog();
            if(projectBrowser.ShowDialog() == false || projectBrowser.DataContext == null)
            { 
                Application.Current.Shutdown();
            }
            else
            {
                Project.Current?.UnLoad();
                DataContext = projectBrowser.DataContext;
            }
        }
    }
}
