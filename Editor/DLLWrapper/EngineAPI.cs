
using System.Numerics;
using System.Runtime.InteropServices;
using Editor.Components;
using Editor.EngineAPIStructs;

namespace Editor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class transformComponent
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale;
    }
    
    [StructLayout(LayoutKind.Sequential)]
    class GameEntityDescriptor
    {
        public transformComponent Transform = new transformComponent();
    }
}

namespace Editor.DLLWrapper
{
    static class EngineAPI
    {
        private const string _engineDll = "EngineDll.dll";

        [DllImport(_engineDll, CharSet = CharSet.Ansi)]
        public static extern int LoadGameCodeDll(string dllpath);

        [DllImport(_engineDll)]
        public static extern int UnloadGameCodeDll();
        internal static class EntityAPI
        {
            //EngineAPI.cpp method: EDITOR_INTERFACE id::id_type CreateGameEntity(game_entity_descriptor* e)
            [DllImport(_engineDll)]
            private static extern int CreateGameEntity(GameEntityDescriptor desc);

            public static int CreateGameEntity(GameEntity entity)
            {
                GameEntityDescriptor desc = new GameEntityDescriptor();

                //transform component
                {
                    var c = entity.GetComponent<Transform>();
                    desc.Transform.Position = c.Position;
                    desc.Transform.Rotation = c.Rotation;
                    desc.Transform.Scale = c.Scale;
                }

                return CreateGameEntity(desc);
            }

            //EngineAPI.cpp method: EDITOR_INTERFACE void RemoveGameEntity(id::id_type id)
            [DllImport(_engineDll)]
            private static extern void RemoveGameEntity(int id);

            public static void RemoveGameEntity(GameEntity entity)
            {
                RemoveGameEntity(entity.EntityId);
            }
        }
        
    }
}