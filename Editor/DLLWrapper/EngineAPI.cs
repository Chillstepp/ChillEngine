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
        private const string _dllName = "EngineDll.dll";

        //EngineAPI.cpp method: EDITOR_INTERFACE id::id_type CreateGameEntity(game_entity_descriptor* e)
        [DllImport(_dllName)]
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
        [DllImport(_dllName)]
        private static extern void RemoveGameEntity(int id);

        public static void RemoveGameEntity(GameEntity entity)
        {
            RemoveGameEntity(entity.EntityId);
        }
    }
}