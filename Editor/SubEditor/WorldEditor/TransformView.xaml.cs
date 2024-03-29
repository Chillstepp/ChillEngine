using System;
using System.Linq;
using System.Windows.Controls;
using System.Windows.Input;
using Editor.Components;

namespace Editor.SubEditor
{
    public partial class TransformView : UserControl
    {
        private Action _undoAction = null;
        public TransformView()
        {
            InitializeComponent();
        }

        private void OnPosition_VectorBox_Mouse_LBD(object sender, MouseButtonEventArgs e)
        {
            if (!(DataContext is MSTransform vm))
            {
                return;
            }

            var selection = vm.SelectedComponents.Select(transform => (transform, transform.Position)).ToList();
            _undoAction = new Action(() =>
            {
                selection.ForEach(item => item.transform.Position = item.Position);
                ((GameEntityView.Instance.DataContext) as MSEntity)?.GetMSComponent<MSTransform>().Refresh();
            });
        }

        private void OnPosition_VectorBox_Mouse_LBU(object sender, MouseButtonEventArgs e)
        {
            throw new System.NotImplementedException();
        }

        private void OnPosition_VectorBox_LostKeyBoardFocus(object sender, KeyboardFocusChangedEventArgs e)
        {
            throw new System.NotImplementedException();
        }
    }
}