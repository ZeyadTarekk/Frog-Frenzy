#pragma once

#include <unordered_set>
#include "entity.hpp"
#include <iostream>
using namespace std;

namespace our
{

    // This class holds a set of entities
    class World
    {
        std::unordered_set<Entity *> entities;         // These are the entities held by this world
        std::unordered_set<Entity *> markedForRemoval; // These are the entities that are awaiting to be deleted
                                                       // when deleteMarkedEntities is called
    public:
        World() = default;

        // This will deserialize a json array of entities and add the new entities to the current world
        // If parent pointer is not null, the new entities will be have their parent set to that given pointer
        // If any of the entities has children, this function will be called recursively for these children
        void deserialize(const nlohmann::json &data, Entity *parent = nullptr);

        // This adds an entity to the entities set and returns a pointer to that entity
        // WARNING The entity is owned by this world so don't use "delete" to delete it, instead, call "markForRemoval"
        // to put it in the "markedForRemoval" set. The elements in the "markedForRemoval" set will be removed and
        // deleted when "deleteMarkedEntities" is called.
        Entity *add()
        {
            // DONE: (Req 8) Create a new entity, set its world member variable to this,
            //  and don't forget to insert it in the suitable container.
            Entity *entity = new Entity(); // create a new entity
            entity->world = this;          // set its world member variable to this world
            entities.insert(entity);       // insert the entity into the entities set
            return entity;                 // return a pointer to the entity
        }

        // This returns and immutable reference to the set of all entites in the world.
        const std::unordered_set<Entity *> &getEntities()
        {
            return entities;
        }

        // This marks an entity for removal by adding it to the "markedForRemoval" set.
        // The elements in the "markedForRemoval" set will be removed and deleted when "deleteMarkedEntities" is called.
        void markForRemoval(Entity *entity)
        {
            // DONE: (Req 8) If the entity is in this world, add it to the "markedForRemoval" set.
            if (entities.find(entity) != entities.end())
            {                                    // if the entity is in this world
                markedForRemoval.insert(entity); // add it to the "markedForRemoval" set
            }
        }

        // This removes the elements in "markedForRemoval" from the "entities" set.
        // Then each of these elements are deleted.
        void deleteMarkedEntities()
        {
            // DONE: (Req 8) Remove and delete all the entities that have been marked for removal
            for (auto entity : markedForRemoval)
            {                           // for each entity in the "markedForRemoval" set
                entities.erase(entity); // remove the entity from the "entities" set
                delete entity;          // delete the entity
            }
            markedForRemoval.clear(); // clear the "markedForRemoval" set
        }

        // This deletes all entities in the world
        void clear()
        {
            // DONE: (Req 8) Delete all the entites and make sure that the containers are empty
            for (auto entity : entities)
            {                  // for each entity in the "entities" set
                delete entity; // delete the entity
            }
            entities.clear();         // clear the "entities" set
            markedForRemoval.clear(); // clear the "markedForRemoval" set
        }

        // Since the world owns all of its entities, they should be deleted alongside it.
        ~World()
        {
            clear();
        }

        // The world should not be copyable
        World(const World &) = delete;
        World &operator=(World const &) = delete;
    };

}