using System.Windows;
using System.Windows.Controls;

namespace Editor.Utilities
{
    public partial class LoggerView : UserControl
    {
        public LoggerView()
        {
            InitializeComponent();
            //@todo: test delete
            Logger.Log(MessageType.Error, "error !");
            Logger.Log(MessageType.Warning, "warninig !");
            Logger.Log(MessageType.Info, "info !");
        }

        private void OnClear_Button_Click(object sender, RoutedEventArgs e)
        {
            Logger.Clear();
        }

        private void OnMessageFilter_Button_Click(object sender, RoutedEventArgs e)
        {
            var filter = 0x0;
            if (ToggleInfo.IsChecked == true) filter |= (int)MessageType.Info;
            if (ToggleError.IsChecked == true) filter |= (int)MessageType.Error;
            if (ToggleWarning.IsChecked == true) filter |= (int)MessageType.Warning;
            Logger.SetMessageFilter(filter);
        }
    }
}