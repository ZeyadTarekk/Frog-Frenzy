#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/movement.hpp"

#include "../application.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <random>

namespace our
{

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic.
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem
    {
        Application *app;          // The application in which the state runs
        bool mouse_locked = false; // Is the mouse locked
        float levelWidth = 19.0f;  // The width of the level
        float levelStart = 24.5f;  // The start of the level
        float levelEnd = -16.0f;   // The end of the level
        float waterWidth = 2.0f;   // The width of the water
        float widthLeft = -8.f;    // left width of the level
        float widthRight = 8.f;    // right width of the level
        float startFrog = 9.0f;    // The start of the frog
        int entered = 1;
        bool isGameOver = false; // Is the game over ?
        vector<glm::vec3> positionsOfCoins;

        // Entities in the game
        Entity *monkey = nullptr;

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent
        void update(World *world, float deltaTime)
        {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            world->deleteMarkedEntities();
            CameraComponent *camera = nullptr;
            FreeCameraControllerComponent *controller = nullptr;
            for (auto entity : world->getEntities())
            {
                camera = entity->getComponent<CameraComponent>();
                controller = entity->getComponent<FreeCameraControllerComponent>();
                if (camera && controller)
                    break;
            }
            // If there is no entity with both a CameraComponent and a FreeCameraControllerComponent, we can do nothing so we return
            if (!(camera && controller))
                return;
            // Get the entity that we found via getOwner of camera (we could use controller->getOwner())
            Entity *entity = camera->getOwner();

            // If the left mouse button is pressed, we lock and hide the mouse. This common in First Person Games.
            if (app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && !mouse_locked)
            {
                app->getMouse().lockMouse(app->getWindow());
                mouse_locked = true;
                // If the left mouse button is released, we unlock and unhide the mouse.
            }
            else if (!app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1) && mouse_locked)
            {
                app->getMouse().unlockMouse(app->getWindow());
                mouse_locked = false;
            }

            // We get a reference to the entity's position and rotation
            glm::vec3 &position = entity->localTransform.position;
            glm::vec3 &rotation = entity->localTransform.rotation;

            // If the left mouse button is pressed, we get the change in the mouse location
            // and use it to update the camera rotation
            if (app->getMouse().isPressed(GLFW_MOUSE_BUTTON_1))
            {
                glm::vec2 delta = app->getMouse().getMouseDelta();
                rotation.x -= delta.y * controller->rotationSensitivity; // The y-axis controls the pitch
                rotation.y -= delta.x * controller->rotationSensitivity; // The x-axis controls the yaw
            }

            // We prevent the pitch from exceeding a certain angle from the XZ plane to prevent gimbal locks
            if (rotation.x < -glm::half_pi<float>() * 0.99f)
                rotation.x = -glm::half_pi<float>() * 0.99f;
            if (rotation.x > glm::half_pi<float>() * 0.99f)
                rotation.x = glm::half_pi<float>() * 0.99f;
            // This is not necessary, but whenever the rotation goes outside the 0 to 2*PI range, we wrap it back inside.
            // This could prevent floating point error if the player rotates in single direction for an extremely long time.
            rotation.y = glm::wrapAngle(rotation.y);

            // We update the camera fov based on the mouse wheel scrolling amount
            float fov = camera->fovY + app->getMouse().getScrollOffset().y * controller->fovSensitivity;
            fov = glm::clamp(fov, glm::pi<float>() * 0.01f, glm::pi<float>() * 0.99f); // We keep the fov in the range 0.01*PI to 0.99*PI
            camera->fovY = fov;

            // We get the camera model matrix (relative to its parent) to compute the front, up and right directions
            glm::mat4 matrix = entity->localTransform.toMat4();

            glm::vec3 front = glm::vec3(matrix * glm::vec4(0, 0, -1, 0)),
                      up = glm::vec3(matrix * glm::vec4(0, 1, 0, 0)),
                      right = glm::vec3(matrix * glm::vec4(1, 0, 0, 0));

            glm::vec3 current_sensitivity = controller->positionSensitivity;
            // If the LEFT SHIFT key is pressed, we multiply the position sensitivity by the speed up factor
            if (app->getKeyboard().isPressed(GLFW_KEY_LEFT_SHIFT))
                current_sensitivity *= controller->speedupFactor;

            // We change the camera position based on the keys WASD/QE
            // S & W moves the player back and forth
            if (app->getKeyboard().isPressed(GLFW_KEY_W))
                position += front * (deltaTime * current_sensitivity.z);
            if (app->getKeyboard().isPressed(GLFW_KEY_S))
                position -= front * (deltaTime * current_sensitivity.z);
            // Q & E moves the player up and down
            if (app->getKeyboard().isPressed(GLFW_KEY_Q))
                position += up * (deltaTime * current_sensitivity.y);
            if (app->getKeyboard().isPressed(GLFW_KEY_E))
                position -= up * (deltaTime * current_sensitivity.y);
            // A & D moves the player left or right
            if (app->getKeyboard().isPressed(GLFW_KEY_D))
                position += right * (deltaTime * current_sensitivity.x);
            if (app->getKeyboard().isPressed(GLFW_KEY_A))
                position -= right * (deltaTime * current_sensitivity.x);

            if (isGameOver)
            {
                return;
            }

            // Entities in the frame
            Entity *frog = nullptr;
            Entity *water = nullptr;
            Entity *holdingComponent = nullptr;
            Entity *woodenBox = nullptr;

            std::vector<Entity *> cars;
            std::vector<Entity *> trunks;
            std::vector<Entity *> coins;
            for (auto entity : world->getEntities())
            {
                std::string name = entity->name;
                if (!frog && name == "frog")
                {
                    frog = entity;
                }
                else if (!holdingComponent && name == "holdingComponent")
                {
                    holdingComponent = entity;
                }
                else if (!water && name == "water")
                {
                    water = entity;
                }
                else if (name == "car")
                {
                    cars.push_back(entity);
                }
                else if (name == "trunkWood")
                {
                    trunks.push_back(entity);
                }
                else if (name == "coin")
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> disX(widthLeft, widthRight);
                    std::uniform_real_distribution<float> disZ((levelEnd + 2) / 2, (startFrog - 2) / 2);
                    glm::vec3 randomPosition = glm::vec3(disX(gen), 0.0f, disZ(gen));
                    if (entered == 1)
                    {
                        entity->localTransform.position = randomPosition;
                        // cout << randomPosition.x << " " << randomPosition.y << " " << randomPosition.z << endl;
                        positionsOfCoins.push_back(randomPosition);
                        entered++;
                    }
                    else if (entered == 2)
                    {
                        entity->localTransform.position = randomPosition;
                        positionsOfCoins.push_back(randomPosition);
                        entered++;
                    }
                    // cout << "X: " << entity->localTransform.position.x << " Y: " << entity->localTransform.position.y << " Z: " << entity->localTransform.position.z << endl;
                    coins.push_back(entity);
                }
                else if (!woodenBox && name == "woodenBox")
                {
                    woodenBox = entity;
                }
                else if (!monkey && name == "monkey")
                {
                    monkey = entity;
                }
            }
            if (!frog)
                return;

            // handle frog movement
            if (
                app->getKeyboard().isPressed(GLFW_KEY_UP) ||
                app->getKeyboard().isPressed(GLFW_KEY_DOWN) ||
                app->getKeyboard().isPressed(GLFW_KEY_LEFT) ||
                app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
            {
                // MOVING   =>  Jump Effect
                frog->localTransform.position.y = float(0.05f * sin(glfwGetTime() * 10) + 0.05f) - 1;           // make the frog jump
                frog->localTransform.rotation.x = float(0.1f * sin(glfwGetTime() * 10)) - glm::pi<float>() / 2; // make the frog rotate
                frog->localTransform.scale.y = 0.01f * sin(glfwGetTime() * 10) + 0.05f;                         // make the frog scale

                // UP
                if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                {
                    // prevent the frog from passing through the wall
                    if (frog->localTransform.position.z < levelEnd)
                        return;

                    // update the camera position
                    position += front * (deltaTime * current_sensitivity.z);
                    // update the frog position
                    frog->localTransform.position += front * (deltaTime * current_sensitivity.z);
                    // update the frog direction
                    frog->localTransform.rotation.y = 0;
                }
                // DOWN
                else if (app->getKeyboard().isPressed(GLFW_KEY_DOWN))
                {
                    if (frog->localTransform.position.z > levelStart)
                        return;

                    position -= front * (deltaTime * current_sensitivity.z);
                    frog->localTransform.position -= front * (deltaTime * current_sensitivity.z);
                    frog->localTransform.rotation.y = glm::pi<float>();
                }
                // RIGHT
                else if (app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
                {
                    if (frog->localTransform.position.x > levelWidth / 2)
                        return;
                    position += right * (deltaTime * current_sensitivity.x);
                    frog->localTransform.position += right * (deltaTime * current_sensitivity.x);
                    frog->localTransform.rotation.y = glm::pi<float>() * -0.5f;
                }
                // LEFT
                else if (app->getKeyboard().isPressed(GLFW_KEY_LEFT))
                {
                    if (frog->localTransform.position.x < -levelWidth / 2)
                        return;
                    position -= right * (deltaTime * current_sensitivity.x);
                    frog->localTransform.position -= right * (deltaTime * current_sensitivity.x);
                    frog->localTransform.rotation.y = glm::pi<float>() * 0.5f;
                }
            }
            else
            {
                // IDLE     =>  No Jump Effect
                // reset the frog position, rotation and scale
                frog->localTransform.position.y = -1;
                frog->localTransform.rotation.x = -glm::pi<float>() / 2;
                frog->localTransform.scale.y = 0.05f;
            }

            // check if a car hits the frog
            for (auto car : cars)
            {
                glm::mat4 carTransformationMatrix = car->getLocalToWorldMatrix();
                glm::vec3 carPosition = glm::vec3(carTransformationMatrix[3]);
                if (
                    frog->localTransform.position.x < carPosition.x + 2.3f &&
                    frog->localTransform.position.x > carPosition.x - 2.3f &&
                    frog->localTransform.position.z < carPosition.z + 1.1f &&
                    frog->localTransform.position.z > carPosition.z - 1.1f)
                {
                    this->gameOver();
                }
            }
            for (auto trunk : trunks)
            {
                if (frog->localTransform.position.x < trunk->localTransform.position.x + 1.7f &&
                    frog->localTransform.position.x > trunk->localTransform.position.x - 1.7f &&
                    frog->localTransform.position.z < trunk->localTransform.position.z + 1.0f &&
                    frog->localTransform.position.z > trunk->localTransform.position.z - 1.0f)
                {
                    // ! move the frog with the trunk

                    // std::cout << "frog move with the trunk-" << rand() << std::endl;
                    // get the movement component of the trunk to know it's speed
                    MovementComponent *movement = trunk->getComponent<MovementComponent>();
                    // frog->localTransform.position.y += 1;
                    // check if the frog won't go out of the box
                    if (!(frog->localTransform.position.x > levelWidth / 2))
                    {
                        // update the camera position
                        position += right * (deltaTime * movement->linearVelocity.x);
                        // Update the frog's position based on the trunk's movement
                        frog->localTransform.position += deltaTime * movement->linearVelocity;
                    }
                }
                else if (
                    frog->localTransform.position.z - waterWidth / 2 < water->localTransform.position.z &&
                    frog->localTransform.position.z + waterWidth / 2 > water->localTransform.position.z)
                {
                    this->gameOver();
                }
            }

            for (auto coin : coins)
            {
                if ((int(frog->localTransform.position.z) == int(coin->localTransform.position.z)) && (int(frog->localTransform.position.x) == int(coin->localTransform.position.x)))
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> dis(5.0f, 10.0f);
                    app->levelDuration += int(dis(gen)); //? adding extra random time  (5~10)
                    world->markForRemoval(coin);         //? removing coin after collision detection
                }
            }
            if (int(woodenBox->localTransform.position.z) == int(frog->localTransform.position.z))
            {
                // make wooden box flying when collision with frog
                glm::vec3 newPosition = woodenBox->localTransform.position + glm::vec3(0.0f, 5 * deltaTime, 0.0f);
                woodenBox->localTransform.position = newPosition; // update position of wooden box
                frog->localTransform.position = newPosition + glm::vec3(0.0f, 2.5, 0.0f); // update position of frog
                entity->localTransform.position = newPosition + glm::vec3(0.0f, 3, 2.0f); // update position of camera
            }

            if (app->timeDiff <= 0)
            {
                this->gameOver();
            }
        }

        //  When the frog hits the water, collides with a car, or runs out of time, the game is over.
        void gameOver()
        {
            if (monkey)
            {
                monkey->localTransform.position.y = 0;
            }

            this->isGameOver = true;
        }

        // When the state exits, it should call this function to ensure the mouse is unlocked
        void exit()
        {
            if (mouse_locked)
            {
                mouse_locked = false;
                app->getMouse().unlockMouse(app->getWindow());
            }
        }
    };

}
