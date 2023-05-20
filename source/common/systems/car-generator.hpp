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

    // The Car Generator system is responsible for randomize  and moving every car entity which contains a MovementComponent.
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
                    //  check is this movement component is a car
                    if (movement->name == "car" ) {
                        // std::cout << movement->linearVelocity[1] << std::endl;
                        // check if the car is inside the game area
                        if (-1.2f <= entity->localTransform.position[1] && entity->localTransform.position[1] <= 1.6f)
                        {
                            // std::cout << "inside" << std::endl;
                            // move the car
                            entity->localTransform.position += deltaTime * movement->linearVelocity;
                            // check if the car passed the half of the game area
                            if (entity->localTransform.position[1] <= 0.0f)
                            {
                                // id = 1  id%2 = 1
                                // id=  2  id%2 = 0
                                // id = 3  id%2 = 1
                                // id = 4  id%2 = 0
                                // each car has an unique id , each Lane road will have 2 cars and there id will be one after the other
                                // so if the car id is even then the other car id will be odd and vice versa
                                // for the following line we car id
                                int  sID =  std::stoi(movement->id);
                                // here we calculate the other car id
                                int searchId = sID % 2 == 0 ? sID - 1 : sID + 1;
                                // std::cout << "searchId" << searchId << std::endl;
                                // we loop over all the entities in the world
                                for (auto entity2 : world->getEntities()){
                                    MovementComponent *movement2 = entity2->getComponent<MovementComponent>();
                                    if(movement2 && movement2->name == "car") {
                                    int  sID =  std::stoi( movement2->id);
                                    // here we check if the other car id is equal to the search id and the car is at the starting point
                                    if (sID == searchId && entity2->localTransform.position[1] == 1.2f) {
                                        // std::cout << "found" << searchId << std::endl;
                                        srand(time(nullptr));  // seed the random number generator with the current time
                                        // generate a random number between 1 and 3
                                        int random_num = rand() % 8 + 1;
                                        // set the velocity of the two cars to be the same so that each two cars in same lane will not collide
                                        movement->linearVelocity[1] = -0.1f * random_num;
                                        movement2->linearVelocity[1] = -0.1f * random_num;
                                        // we add the following condition to make sure that atleast on car in same lane must move
                                        if(movement->linearVelocity[1] == 0.0f && movement2->linearVelocity[1] == 0.0f) {
                                            movement->linearVelocity[1] = -0.1f;
                                        }
                                        // std::cout << "random " << random_num << std::endl;
                                        break;
                                    }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // inside the width
                            // std::cout << "start" << std::endl;
                            // set the car to the starting point
                            entity->localTransform.position[1] = 1.2f;
                            // set the velocity to zero so that the car will not move
                            movement->linearVelocity[1] = 0.0;
                        }
                    }
                    // same as the above code but for cars moving in the opposite direction
                    if (movement->name == "car2" ) {
                        // std::cout << movement->linearVelocity[1] << std::endl;
                        // std::cout << entity->localTransform.position[1] << std::endl;
                        if (-1.6f <= entity->localTransform.position[1] && entity->localTransform.position[1] <= 1.2f)
                        {
                            // std::cout << "nside" << std::endl;
                            entity->localTransform.position += deltaTime * movement->linearVelocity;
                            if (entity->localTransform.position[1] >= 0.0f)
                            {
                                // id = 1  id%2 = 1
                                // id=  2  id%2 = 0
                                // id = 3  id%2 = 1
                                // id = 4  id%2 = 0
                                // movement->linearVelocity[1] = -0.5f;
                                int  sID =  std::stoi(movement->id);
                                int searchId = sID % 2 == 0 ? sID - 1 : sID + 1;
                                // std::cout << "searchId" << searchId << std::endl;
                                for (auto entity2 : world->getEntities()){
                                    MovementComponent *movement2 = entity2->getComponent<MovementComponent>();
                                    if(movement2 && movement2->name == "car2") {
                                    int  sID =  std::stoi( movement2->id);
                                    // std::cout << "you" << entity2->localTransform.position[1] << std::endl;
                                    if (sID == searchId && entity2->localTransform.position[1] <= -1) {
                                        // std::cout << "found" << searchId << std::endl;
                                        srand(time(nullptr));  // seed the random number generator with the current time
                                        // generate a random number between 1 and 3
                                        int random_num = rand() % 8 + 1;
                                        movement->linearVelocity[1] = 0.1f * random_num;
                                        movement2->linearVelocity[1] = 0.1f * random_num;
                                        // std::cout << "random " << random_num << std::endl;
                                        if(movement->linearVelocity[1] <= 0.0f && movement2->linearVelocity[1] <= 0.0f) {
                                            movement->linearVelocity[1] = 0.1f;
                                        }
                                        break;
                                    }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // inside the width
                            // std::cout << "start" << std::endl;
                            entity->localTransform.position[1] = -1.2f;
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
