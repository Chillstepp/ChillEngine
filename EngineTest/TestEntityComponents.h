#pragma once

#include <ctime>
#include <iostream>

#include "Test.h"
#include "../Engine/Components/Transform.h"
#include "../Engine/Components/Entity.h"
#include "../Engine/Common/CommonHeaders.h"

using namespace ChillEngine;

class engine_test : public test
{
public:
    bool initialize() override
    {
        srand((u32)time(nullptr));
        return true;
    }
    void run() override
    {
        do
        {
            for(u32 i = 0; i < 10000; ++i)
            {
                create_random();
                remove_random();
                num_entities = entities.size();
            }
            print_result();
        }
        while (getchar() != 'q');
    }
    void shutdown() override
    {
        
    }
private:
    void create_random()
    {
        u32 count = rand() % 20;
        if(entities.empty()) count = 1000;
        transform::init_info transform_info{};
        game_entity::entity_info entity_info{&transform_info};
        while(count > 0)
        {
            ++added;
            game_entity::entity entity = game_entity::create_game_entity(entity_info);
            assert(entity.is_valid());
            entities.push_back(entity);
            --count;
        }
    }
    void remove_random()
    {
        u32 count = rand() % 20;
        if(entities.size() < 1000) return;
        while(count > 0)
        {
            const u32 index = rand() % entities.size();
            const game_entity::entity entity = entities[index];
            assert(entity.is_valid());
            if(entity.is_valid())
            {
                game_entity::remove_game_entity(entity);
                entities.erase(entities.begin() + index);
                assert(!game_entity::is_alive(entity));
                ++removed;
            }
            --count;
        }
    }

    void print_result()
    {
        std::cout<<"Entities created:"<<added <<std::endl;
        std::cout<<"Entities deleted:"<<removed <<std::endl;
    }
    
    utl::vector<game_entity::entity> entities;
    u32 added = 0;
    u32 removed = 0;
    u32 num_entities = 0;
    
};