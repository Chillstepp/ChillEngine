using System.ComponentModel;
using System.Runtime.Serialization;

namespace Editor.Common
{
    [DataContract(IsReference = true)]
    public class ViewModeBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged(string propertyName)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

    }
}