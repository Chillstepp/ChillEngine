using System;
using System.Collections.Generic;
using System.Runtime.Serialization;


namespace Editor.GameProject
{
    [DataContract]
    public class ProjectData
    {
        [DataMember]
        public string ProjectName { get; set; }
        [DataMember]
        public string ProjectPath { get; set; }
        [DataMember]
        public DateTime Data { get; set; }
    }

    [DataContract]
    public class ProjectDataList
    {
        [DataMember]
        public List<ProjectData> Projects { get; set; }
    }
    public class OpenProject
    {
        static OpenProject()
        {
            
        }
    }
}