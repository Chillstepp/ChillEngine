using Editor.Utilities;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using Editor.GameProject;
using EXCEPINFO = System.Runtime.InteropServices.ComTypes.EXCEPINFO;

namespace Editor.GameDev
{
    static class VisualStudio
    {
        private static EnvDTE80.DTE2 _vsInstance = null;
        private static readonly string _progID = "VisualStudio.DTE.17.0";
        public static bool BuildSucceded { get; private set; } = true;
        public static bool BuildDone { get; private set; } = true;

        [DllImport("ole32.dll")]
        private static extern int CreateBindCtx(uint reserved, out IBindCtx ppbc);

        [DllImport("ole32.dll")]
        private static extern int GetRunningObjectTable(uint reserved, out IRunningObjectTable pprot);
        
            
        public static void OpenVisualStudio(string solutionPath)
        {
            if (_vsInstance == null)
            {
                IRunningObjectTable rot = null;
                IEnumMoniker monikerTable = null;
                IBindCtx bindCtx= null;
                try
                {
                    if (_vsInstance == null)
                    {
                        var hResult = GetRunningObjectTable(0, out rot);
                        if (hResult < 0 || rot == null)
                            throw new COMException($"GetRunningObjectTable() returned HResult:{hResult:x8}");

                        rot.EnumRunning(out monikerTable);
                        monikerTable.Reset();

                        hResult = CreateBindCtx(0, out bindCtx);
                        if (hResult < 0 || bindCtx == null)
                            throw new COMException($"CreateBindCtx() returned HResult:{hResult:x8}");

                        IMoniker[] currentMoniker = new IMoniker[1];

                        while (monikerTable.Next(1, currentMoniker, IntPtr.Zero) == 0)
                        {
                            string name = String.Empty;
                            currentMoniker[0].GetDisplayName(bindCtx, null, out name);
                            if (name.Contains(_progID))
                            { 
                            //     hResult = rot.GetObject(currentMoniker[0], out object obj);
                            //     if (hResult < 0 || bindCtx == null)
                            //         throw new COMException(
                            //             $"running object table's GetObject() returned HResult:{hResult:x8}");
                            //     EnvDTE80.DTE2 dte = obj as EnvDTE80.DTE2;
                            //     var solutionName = dte.Solution.FullName;
                            //     if (solutionName == solutionPath)
                            //     {
                            //         _vsInstance = dte;
                            //         break;
                            //     }
                                break;
                            }
                            
                        }

                        if (_vsInstance == null)
                        {
                            Type visualStudioType = Type.GetTypeFromProgID(_progID, true);
                            _vsInstance = Activator.CreateInstance(visualStudioType) as EnvDTE80.DTE2;
                        }


                    }
                }
                catch (Exception e)
                {
                    Debug.WriteLine(e.Message);
                    Logger.Log(MessageType.Error, "failed to open Visual Studio");
                }
                finally
                {
                    if (monikerTable != null) Marshal.ReleaseComObject(monikerTable);
                    if (rot != null) Marshal.ReleaseComObject(rot);
                    if (bindCtx != null) Marshal.ReleaseComObject(bindCtx);
                }
                
            }
        }

        public static void CloseVisualStudio()
        {
            if (_vsInstance?.Solution.IsOpen == true)
            {
                _vsInstance.ExecuteCommand("File.SaveAll");
                _vsInstance.Solution.Close(true);
            }
            _vsInstance?.Quit();
        }

        public static bool AddFilesToSolution(string solution, string projectName, string[] files)
        {
            Debug.Assert(files?.Length > 0);
            OpenVisualStudio(solution);

            try
            {
                if (_vsInstance != null)
                {
                    if(!_vsInstance.Solution.IsOpen) _vsInstance.Solution.Open(solution);
                    else _vsInstance.ExecuteCommand("File.SaveAll");

                    foreach (EnvDTE.Project project in _vsInstance.Solution.Projects)
                    {
                        if (project.UniqueName.Contains(projectName))
                        {
                            foreach (var file in files)
                            {
                                project.ProjectItems.AddFromFile(file);
                            }
                        }
                    }

                    var cpp = files.FirstOrDefault(x => Path.GetExtension(x) == ".cpp");
                    // var h = files.FirstOrDefault(x => Path.GetExtension(x) == ".h");
                    if (!string.IsNullOrEmpty(cpp))
                    {
                        _vsInstance.ItemOperations.OpenFile(cpp, EnvDTE.Constants.vsViewKindTextView).Visible = true;
                    }
                    _vsInstance.MainWindow.Activate();
                    _vsInstance.MainWindow.Visible = true;
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.Message);
                Debug.WriteLine("Failed to add files to project");
                Logger.Log(MessageType.Error, "Failed to add files to project");
                return false;
            }

            return true;
        }

        public static void BuildSolution(Project project, string configName, bool showWindow = true)
        {
            if (IsDebugging())
            {
                Logger.Log(MessageType.Error, "Visual studio is current running a process.");
                return;
            }
            OpenVisualStudio(project.Solution);
            BuildSucceded = false;
            BuildDone = false;
            for (int i = 0; i < 1; i++)
            {
                try
                {
                    if (!_vsInstance.Solution.IsOpen)
                    {
                        bool openFinished = false;
                        do
                        {
                            try
                            {
                                //@todo: fk it, I don't know why this need to sleep awhile, otherwise it will open solution infinite,
                                //i doubt that create instance cause this. I will figure out it later.
                                System.Threading.Thread.Sleep(5000);
                                Debug.WriteLine("whysb3");
                                _vsInstance.Solution.Open(project.Solution);
                                Debug.WriteLine("whysb");
                                openFinished = true;
                            }
                            catch (Exception e)
                            {
                                Debug.WriteLine("whysb2");
                                Debug.WriteLine(e.Message);
                            }
                            Debug.WriteLine("whysb4");
                            
                        } while (!openFinished);
                        
                    }
                    _vsInstance.MainWindow.Visible = showWindow;
                    _vsInstance.Events.BuildEvents.OnBuildProjConfigBegin += OnBuildSolutionBegin;
                    _vsInstance.Events.BuildEvents.OnBuildProjConfigDone += OnBuildSolutionDone;
                    
                    //delete pbd file
                    try
                    {
                        foreach (var pbdFile in Directory.GetFiles(Path.Combine($"{project.Path}", $@"x64\{configName}"), "*.pbd"))
                        {
                            File.Delete(pbdFile);
                        }
                    }
                    catch (Exception e)
                    {
                        Debug.WriteLine(e.Message);
                    }

                    
                    _vsInstance.Solution.SolutionBuild.SolutionConfigurations.Item(configName).Activate();
                    _vsInstance.ExecuteCommand("Build.BuildSolution");
                }
                catch (Exception e)
                {
                    Debug.WriteLine(e.Message);
                    Debug.WriteLine($"Attempt {i}: failed to build {project.Name}");
                    System.Threading.Thread.Sleep(1000);
                }
            }

        }

        private static void OnBuildSolutionDone(string project, string projectconfig, string platform, string solutionconfig, bool success)
        {
            if (BuildDone) return;
            if (success)
            {
                Logger.Log(MessageType.Info, $"Building {projectconfig} configuration succeed");
            }
            else
            {
                Logger.Log(MessageType.Error, $"Building {projectconfig} configuration failed");
            }

            BuildDone = true;
            BuildSucceded = success;
        }

        private static void OnBuildSolutionBegin(string project, string projectconfig, string platform, string solutionconfig)
        {
            Logger.Log(MessageType.Info, $"Building {project}, {projectconfig}, {platform}, {solutionconfig}");
        }

        public static bool IsDebugging()
        {
            bool result = false;

            for (int i = 0; i < 3; ++i)
            {
                try
                {
                    result = _vsInstance != null &&
                             (_vsInstance.Debugger.CurrentProgram != null ||
                              _vsInstance.Debugger.CurrentMode == EnvDTE.dbgDebugMode.dbgRunMode);
                    
                }
                catch (Exception e)
                {
                    Debug.Write(e.Message);
                    if (!result) System.Threading.Thread.Sleep(1000);
                }
            }

            return result;
        }
    }
}