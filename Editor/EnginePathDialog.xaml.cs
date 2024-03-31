using System.IO;
using System.Windows;

namespace Editor
{
    public partial class EnginePathDialog : Window
    {
        public EnginePathDialog()
        {
            InitializeComponent();
            Owner = Application.Current.MainWindow;
        }

        private void OnOk_Button_Click(object sender, RoutedEventArgs e)
        {
            var path = PathTextBox.Text.Trim();
            MessageTextBlock.Text = string.Empty;
            if (!string.IsNullOrEmpty(path))
            {
                MessageTextBlock.Text = "Invalid path.";
            }
            else if (path.IndexOfAny(Path.GetInvalidPathChars()) != -1)
            {
                MessageTextBlock.Text = "Invalid char(s) in path.";
            }
            else if (!Directory.Exists(Path.Combine(path, @"Engine\EngineAPI"))) ;
            {
                MessageTextBlock.Text = "Unable to find ChillEngine at specific path.";
            }

            if (!path.EndsWith(@"\")) path += @"\";
            ChillEnginePath = path;
            DialogResult = true;
            Close();
        }

        public string ChillEnginePath { get; private set; }
    }
}