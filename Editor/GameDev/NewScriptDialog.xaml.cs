using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using Editor.GameProject;
using Editor.Utilities;

namespace Editor.GameDev
{
    public partial class NewScriptDialog : Window
    {
        public NewScriptDialog()
        {
            InitializeComponent();
        }
        
        bool Validate()
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
            IsEnabled = false;

            try
            {
                await Task.Run(() =>CreateScript(, ));
            }
            catch (Exception exception)
            {
                Debug.WriteLine(exception);
                Logger.Log(MessageType.Error, $"Failed to create script {ScriptName.Text}");
            }
        }

        private void OnScriptName_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            if(!Validate()) return;
            var name = ScriptName.Text.Trim();
            var project = Project.Current;
            MessageTextBlock.Text = $"{name}.h and {name}.cpp will added to {Path.Combine(Project.Current.Path, @"GameCode\")}";
        }

        private void OnScriptPath_TextBox_TextChanged(object sender, TextChangedEventArgs e)
        {
            Validate();
        }
    }
}