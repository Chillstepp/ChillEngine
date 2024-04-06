#pragma once

#include "TransformComponent.h"
#include "ScriptComponent.h"

namespace ChillEngine
{
    namespace game_entity
    {
        DEFINE_TYPE_ID(entity_id);
    
        class entity
        {
        public:
            constexpr explicit entity(entity_id id) : _id(id) {}
            constexpr entity() : _id(id::invalid_id) {}
            constexpr entity_id get_id() const {return _id;}
            constexpr bool is_valid() const {return id::is_valid(_id);}
        
            transform::component transform() const;
            script::component script() const;
            
        private:
            entity_id _id;
        };
    }

    namespace script
    {
        class entity_script : public game_entity::entity
        {
        public:
            virtual ~entity_script() = default;
            virtual void begin_play() {}
            virtual void update(float deltaTime) {}
        protected:
            constexpr explicit entity_script(game_entity::entity entity):
                game_entity::entity(entity.get_id()){}
        };

        namespace detail
        {
            using script_ptr = std::unique_ptr<entity_script>;
            using script_creator = script_ptr(*)(game_entity::entity entity);

            u8 register_script(size_t, script_creator);
            script_creator get_script_creator(size_t tag);
#ifdef USE_WITH_EDITOR
            u8 add_script_name(const char* name);
#endif
            
            template<class script_class>
            script_ptr create_script(game_entity::entity entity)
            {
                return std::make_unique<script_class>(entity);
            }
        }

#ifdef USE_WITH_EDITOR
#define REGISTER_SCRIPT(TYPE)                   \
        class TYPE;                             \
        namespace                               \
        {                                       \
            const u8 _reg_##TYPE(               \
                ChillEngine::script::detail::register_script(std::hash<std::string>()(#TYPE),                                \
                                                             &ChillEngine::script::detail::create_script<TYPE>)              \
            );                                                                                                               \
            const u8 _name_##TYPE(ChillEngine::script::detail::add_script_name(#TYPE));                                      \
        }
#else
#define REGISTER_SCRIPT(TYPE)                   \
        class TYPE;                             \
        namespace                               \
        {                                       \
            const u8 _reg_##TYPE(               \
                ChillEngine::script::detail::register_script(std::hash<std::string>()(#TYPE),                                \
                                                             &ChillEngine::script::detail::create_script<TYPE>)              \
            );                                                                                                               \
        }
#endif
        
    }
}