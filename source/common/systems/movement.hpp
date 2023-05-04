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
    class MovementSystem {
    public:

        // This should be called every frame to update all entities containing a MovementComponent. 
        void update(World* world, float deltaTime) {
            // For each entity in the world
            // -8 -> 8
            auto roadStartX = -8.0f;
            auto roadEndX = 8.0f;
            for(auto entity : world->getEntities()){
                // Get the movement component if it exists
                MovementComponent* movement = entity->getComponent<MovementComponent>();
                // If the movement component exists
                
                if(movement){
                    if(movement->name == "car"){
                        std::cout <<entity->localTransform.position[2] << std::endl;
                        if ( -8.0f <= entity->localTransform.position[2] &&entity->localTransform.position[2] <= 8.0f)
                        {
                            // std::cout << "outside road width" << std::endl;
                            entity->localTransform.position += deltaTime * movement->linearVelocity;
                            /* code */
                        } else  
                        {
                            entity->localTransform.position[2] = -8.0f;
                            // std::cout << "inside road width" << std::endl;
                            /* code */
                        }
                        
                    // std::cout << "car" << std::endl;
                    }
                    // std::cout << "ncar" << std::endl;
                    // Change the position and rotation based on the linear & angular velocity and delta time.
                    // entity->localTransform.rotation += deltaTime * movement->angularVelocity;
                }
            }
        }

    };

}
