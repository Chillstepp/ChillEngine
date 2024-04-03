using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Media;
using System.Windows.Media.Animation;
using Editor.Utilities;
using EnvDTE;
using Project = Editor.GameProject.Project;
using Window = System.Windows.Window;

namespace Editor.GameDev
{
    public partial class NewScriptDialog : Window
    {
        private static readonly string _cppCode = 
@"#include ""{0}.h""
namespace {1} {{
REGISTER_SCRIPT({0});
void {0}::begin_play()
{{
}}
void {0}::update(float dt)
{{
}}
}} // namespace {1}";

        private static readonly string _hCode = 
@"#pragma once
namespace {1} {{
class {0} : public ChillEngine::script::entity_script
{{
public:
    constexpr explicit {0}(ChillEngine::game_entity::entity entity)
        : ChillEngine::script::entity_script{{entity}} {{}}
    void begin_play() override;
    void update(float dt) override;
private:
}};
}} // namespace {1}";

        private static readonly string _namespace = GetNamespaceFromProjectName();

        private static string GetNamespaceFromProjectName()
        {
            var projectName = Project.Current.Name;
            projectName = projectName.Replace(' ', '_');
            return projectName;
        }
        
        public NewScriptDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
            ScriptPath.Text = @"GameCode\";
        }
        
        private bool Validate()
        {
            bool isValid = false;
            var name = ScriptName.Text.Trim();
            var path = ScriptPath.Text.Trim();
            string errorMsg = string.Empty;
            if (string.IsNullOrEmpty(name))
            {
                errorMsg = "Type in a script name.";
            }
            else if (name.IndexOfAny(Path.GetInvalidFileNameChars()) != -1 || name.Any(x => char.IsWhiteSpace(x)))
            {
                errorMsg = "Invalid character(s) used in script name.";
            }
            else if (string.IsNullOrEmpty(path))
            {
                errorMsg = "Select a valid script folder";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                errorMsg = "Invalid character(s) used in script path.";
            }
            else if (!Path.GetFullPath(Path.Combine(Project.Current.Path, path)).Contains(Path.Combine(Project.Current.Path, @"GameCode\")))
            {
                errorMsg = "Script must be added to (a sub-folder of) GameCode.";
            }
            else if (File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, path), $"{name}.cpp"))) ||
                     File.Exists(Path.GetFullPath(Path.Combine(Path.Combine(Project.Current.Path, path), $"{name}.h"))))
            {
                errorMsg = $"script {name} already exists in this folder.";
            }
            else
            {
                isValid = true;
            }

            if (!isValid)
            {
                MessageTextBlock.Foreground = FindResource("Editor.RedBrush") as Brush;
            }
            else
            {
                MessageTextBlock.Foreground = FindResource("Editor.FontBrush") as Brush;
            }
            MessageTextBlock.Text = errorMsg;
            return isValid;
        }
        
        //create may take a few second, so we use async
        private async void OnOk_Button_Click(object sender, RoutedEventArgs e)
        {
            if(!Validate()) return;
            //等待, 禁用交互
            IsEnabled = false;
            BusyAnimation.Opacity = 0;
            BusyAnimation.Visibility = Visibility.Visible;
            DoubleAnimation fadeIn = new DoubleAnimation(0, 1, new Duration(TimeSpan.FromMilliseconds(500)));
            BusyAnimation.BeginAnimation(OpacityProperty, fadeIn);
            try
            {
                var name = ScriptName.Text.Trim();
                var path = Path.GetFullPath(Path.Combine(Project.Current.Path, ScriptPath.Text.Trim()));
                var solution = Project.Current.Solution;
                var projectName = Project.Current.Name;
                await Task.Run(() =>CreateScript(name, path, solution, projectName));
            }
            catch (Exception exception)
            {
                Debug.WriteLine(exception);
                Logger.Log(MessageType.Error, $"Failed to create script {ScriptName.Text}");
            }
            finally
            {
                DoubleAnimation fadeOut = new DoubleAnimation(1, 0, new Duration(TimeSpan.FromMilliseconds(200)));
                fadeOut.Completed += (s, e) =>
                {
                    BusyAnimation.Opacity = 0;
                    BusyAnimation.Visibility = Visibility.Hidden;
                    Close();
                };
                BusyAnimation.BeginAnimation(OpacityProperty, fadeOut);
            }
        }

        private void CreateScript(string name, string path, string solution, string projectName)
        {
            
            if(!Directory.Exists(path)) Directory.CreateDirectory(path);

            var cpp = Path.GetFullPath(Path.Combine(path, $"{name}.cpp"));
            var h = Path.GetFullPath(Path.Combine(path, $"{name}.h"));
            using (var sw = File.CreateText(cpp))
            {
                sw.Write(string.Format(_cppCode, name, _namespace));
            }

            using (var sw = File.CreateText(h))
            {
                sw.Write(string.Format(_hCode, name, _namespace));
            }

            string[] files = new string[] {cpp, h};
            
            //Here we try three times...
            for (int i = 0; i < 3; i++)
            {
                if (!VisualStudio.AddFilesToSolution(solution, projectName, files))
                {
                    System.Threading.Thread.Sleep(1000);
                }
                else break;
            }


        }

        private void OnScriptName_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if(!Validate()) return;
            var name = ScriptName.Text.Trim();

            MessageTextBlock.Text = $"{name}.h and {name}.cpp will added to {Path.Combine(Path.Combine(Project.Current.Path, Project.Current.Name), @"GameCode\")}";

        }

        private void OnScriptPath_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            Validate();
        }
    }
}