#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

namespace our
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic.
    // For more information, see "common/components/movement.hpp"
    class MovementSystem
    {
    public:
        // This should be called every frame to update all entities containing a MovementComponent.
        void update(World *world, float deltaTime)
        {
            // For each entity in the world
            for (auto entity : world->getEntities())
            {
                // Get the movement component if it exists
                MovementComponent *movement = entity->getComponent<MovementComponent>();
                // If the movement component exists
                if (movement)
                {
                    // Change the position and rotation based on the linear & angular velocity and delta time.
                    if (movement->name == "car")
                    {

                        if (-8.0f <= entity->localTransform.position[0] && entity->localTransform.position[0] <= 8.0f)
                        {
                            entity->localTransform.position += deltaTime * movement->linearVelocity;
                        }
                        else
                        {
                            entity->localTransform.position[0] = -8.0f;
                        }
                    }
                    if (movement->name == "monkey" || movement->name == "moon" || movement->name == "coin" || movement->name == "woodenBox")
                    {
                        entity->localTransform.position += deltaTime * movement->linearVelocity;
                        entity->localTransform.rotation += deltaTime * movement->angularVelocity;
                    }
                    if (movement->name == "trunkWood")
                    {
                        if (-8.0f <= entity->localTransform.position[0] && entity->localTransform.position[0] <= 12.0f)
                        {
                            // inside the water
                            entity->localTransform.position += deltaTime * movement->linearVelocity;
                        }
                        else
                        {
                            // outside the width so bring it inside
                            entity->localTransform.position[0] = -8.0f;
                        }
                        // std::cout << "wood " << entity->localTransform.position[0] << " " << entity->localTransform.position[2] << std::endl;
                    }
                }
            }
        }
    };

}
