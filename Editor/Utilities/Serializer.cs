using System;
using System.IO;

namespace Editor.Utilities
{
    public static class Serializer
    {
        public static void ToFile<T>(T instance, string path)
        {
            try
            {
                using var fs = new FileStream(path, FileMode.Create);
            }
            catch(Exception e)
            {

            }
        }
    }
}