using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
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

        //@todo: in dev.
        public static string ChillEnginePath { get; set; } = @"E:\ChillEngine\ChillEngine";

        private void GetEnginePath()
        {
            var enginePath = Environment.GetEnvironmentVariable("CHILL_ENGINE", EnvironmentVariableTarget.User);
            if (enginePath == null || !Directory.Exists(Path.Combine(enginePath, @"Engine\EngineAPI")))
            {
                //创建一个需要输入engine path的窗口
                var dlg = new EnginePathDialog();
                if (dlg.ShowDialog() == true)
                {
                    ChillEnginePath = dlg.ChillEnginePath;
                    Environment.SetEnvironmentVariable("CHILL_ENGINE", ChillEnginePath.ToUpper(), EnvironmentVariableTarget.User);
                }
                else
                {
                    Application.Current.Shutdown();
                }
            }
            else
            {
                ChillEnginePath = enginePath;
            }
        }

        private void OnMainWindowLoaded(Object sender, RoutedEventArgs e)
        {
            Loaded -= OnMainWindowLoaded;
            GetEnginePath();
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
