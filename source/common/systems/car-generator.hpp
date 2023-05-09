#pragma once

#include "../ecs/world.hpp"
#include "../components/movement.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>

#include <cstdlib>   // for rand() and srand()
#include <ctime>     // for time()

namespace our
{

    // The movement system is responsible for moving every entity which contains a MovementComponent.
    // This system is added as a simple example for how use the ECS framework to implement logic.
    // For more information, see "common/components/movement.hpp"
    class CarGeneratorSystem
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
                    if (movement->name == "car" ) {

                        std::cout << entity->localTransform.position[1] << std::endl;
                        if (-1.2f <= entity->localTransform.position[1] && entity->localTransform.position[1] <= 1.6f)
                        {
                            std::cout << "nside" << std::endl;
                            entity->localTransform.position += deltaTime * movement->linearVelocity;
                            if (entity->localTransform.position[1] <= 0.0f)
                            {
                                // id = 1  id%2 = 1
                                // id=  2  id%2 = 0
                                // id = 3  id%2 = 1
                                // id = 4  id%2 = 0
                                // movement->linearVelocity[1] = -0.5f;
                                int  sID =  std::stoi(movement->id);
                                int searchId = sID % 2 == 0 ? sID - 1 : sID + 1;
                                std::cout << "searchId" << searchId << std::endl;
                                for (auto entity2 : world->getEntities()){
                                    MovementComponent *movement2 = entity2->getComponent<MovementComponent>();
                                    if(movement2 && movement2->name == "car") {
                                    int  sID =  std::stoi( movement2->id);
                                    if (sID == searchId && entity2->localTransform.position[1] == 1.2f) {
                                        std::cout << "found" << searchId << std::endl;
                                        srand(time(nullptr));  // seed the random number generator with the current time
                                        // generate a random number between 1 and 3
                                        int random_num = rand() % 8 + 1;
                                        movement->linearVelocity[1] = -0.1f * random_num;
                                        movement2->linearVelocity[1] = -0.1f * random_num;
                                        std::cout << "random " << random_num << std::endl;
                                        break;
                                    }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // inside the width
                            std::cout << "start" << std::endl;
                            entity->localTransform.position[1] = 1.2f;
                            // movement->linearVelocity[1] = 0.0f;
                            movement->linearVelocity[1] = 0.0;
                        }
                    }
                    if (movement->name == "tire")
                    {
                        entity->localTransform.position += deltaTime * movement->linearVelocity;
                        entity->localTransform.rotation += deltaTime * movement->angularVelocity;
                    }
                }
            }
        }
    };

}
