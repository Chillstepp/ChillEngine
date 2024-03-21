using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using Editor.GameProject;

namespace Editor.SubEditor
{
    public partial class WorldEditorView : UserControl
    {
        public WorldEditorView()
        {
            InitializeComponent();
            Loaded += OnWorldEditorViewLoaded;
        }

        private void OnWorldEditorViewLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnWorldEditorViewLoaded;
            Focus();
            //sometimes may lost focus, so focus again when UndoList change
            ((INotifyCollectionChanged)Project.UndoRedo.UndoList).CollectionChanged += (s, e) => Focus();
        }
    }
}