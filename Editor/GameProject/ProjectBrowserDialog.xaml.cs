using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Animation;

namespace Editor.GameProject
{
    /// <summary>
    /// Interaction logic for ProjectBrowserDialog.xaml
    /// </summary>
    public partial class ProjectBrowserDialog : Window
    {
        private readonly CubicEase _easing = new CubicEase() { EasingMode = EasingMode.EaseInOut };
        public ProjectBrowserDialog()
        {
            InitializeComponent();
            Loaded += OnProjectBrowserDialogLoaded;
        }
        private void AnimateToCreateProject()
        {
            var highlightAnimation = new DoubleAnimation(200, 400, new Duration(TimeSpan.FromSeconds(0.2)));
            highlightAnimation.EasingFunction = _easing;
            highlightAnimation.Completed += (s, e) =>
            {
                var animation = new ThicknessAnimation(new Thickness(0), new Thickness(-1600, 0, 0, 0), new Duration(TimeSpan.FromSeconds(0.5)));
                animation.EasingFunction = _easing;
                BrowserContent.BeginAnimation(MarginProperty, animation);
            };
            HighlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void AnimateToOpenProject()
        {
            var highlightAnimation = new DoubleAnimation(400, 200, new Duration(TimeSpan.FromSeconds(0.2)));
            highlightAnimation.EasingFunction = _easing;
            highlightAnimation.Completed += (s, e) =>
            {
                var animation = new ThicknessAnimation(new Thickness(-1600, 0, 0, 0), new Thickness(0), new Duration(TimeSpan.FromSeconds(0.5)));
                animation.EasingFunction = _easing;
                BrowserContent.BeginAnimation(MarginProperty, animation);
            };
            HighlightRect.BeginAnimation(Canvas.LeftProperty, highlightAnimation);
        }

        private void OnProjectBrowserDialogLoaded(object sender, RoutedEventArgs e)
        {
            Loaded -= OnProjectBrowserDialogLoaded;
            if (!OpenProject.Projects.Any())
            {
                OpenProjectButton.IsEnabled = false;
                OpenProjectView.Visibility = Visibility.Hidden;
                OnToggleButton_Click(CreateProjectButton, new RoutedEventArgs());
            }
        }

        private void OnToggleButton_Click(object sender, RoutedEventArgs e)
        {
            if(sender == OpenProjectButton)
            {
                if(CreateProjectButton.IsChecked == true)
                {
                    CreateProjectButton.IsChecked = false;
                    AnimateToOpenProject();
                    OpenProjectView.IsEnabled = true;
                    CreateProjectView.IsEnabled = false;
                }
                OpenProjectButton.IsChecked = true;
            }
            else
            {
                if(OpenProjectButton.IsChecked == true)
                {
                    OpenProjectButton.IsChecked = false;
                    AnimateToCreateProject();
                    OpenProjectView.IsEnabled = false;
                    CreateProjectView.IsEnabled = true;
                }
                CreateProjectButton.IsChecked = true;
            }
        }
    }
}
