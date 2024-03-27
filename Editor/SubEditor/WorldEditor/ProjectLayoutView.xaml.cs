﻿using System.Linq;
using System.Windows;
using System.Windows.Controls;
using Editor.Components;
using Editor.GameProject;
using Editor.Utilities;

namespace Editor.SubEditor
{
    public partial class ProjectLayoutView : UserControl
    {
        public ProjectLayoutView()
        {
            InitializeComponent();
        }

        private void OnAddGameEntity_Button_Click(object sender, RoutedEventArgs e)
        {
            var btn = sender as Button;
            var vm = btn.DataContext as Scene;
            vm.AddGameEntityCommand.Execute(new GameEntity(vm){ Name = "Empty Game Entity"});
        }

        private void OnGameEntities_ListBox_SelectionCHanged(object sender, SelectionChangedEventArgs e)
        {
            GameEntityView.Instance.DataContext = null;
            var listBox = sender as ListBox;
            var newSelection = listBox.SelectedItems.Cast<GameEntity>().ToList();
            var previousSelection = newSelection.Except(e.AddedItems.Cast<GameEntity>())
                .Concat(e.RemovedItems.Cast<GameEntity>()).ToList();
            
            Project.UndoRedo.Add(new UndoRedoAction(
                () => //undo action
                {
                    listBox.UnselectAll();
                    previousSelection.ForEach(x => ((ListBoxItem)listBox.ItemContainerGenerator.ContainerFromItem(x)).IsSelected = true);
                },
                () => //redo action
                {
                    listBox.UnselectAll();
                    previousSelection.ForEach(x => ((ListBoxItem)listBox.ItemContainerGenerator.ContainerFromItem(x)).IsSelected = true);
                },
                "Selection changed"
            ));

            MSGameEntity msGameEntity = null;
            if (newSelection.Any())
            {
                msGameEntity = new MSGameEntity(newSelection);
            }

            GameEntityView.Instance.DataContext = msGameEntity;
        }
    }
}