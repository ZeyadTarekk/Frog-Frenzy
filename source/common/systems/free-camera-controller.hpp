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
        float levelEnd = -14.6f;   // The end of the level
        float waterWidth = 4.0f;   // The width of the water

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

            // handle frog movement
            // We get the frog entity
            Entity *frog = nullptr;
            Entity *water = nullptr;
            Entity *holdingComponent = nullptr;

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
                    coins.push_back(entity);
            }
            if (!frog)
                return;

            if (
                app->getKeyboard().isPressed(GLFW_KEY_UP) ||
                app->getKeyboard().isPressed(GLFW_KEY_DOWN) ||
                app->getKeyboard().isPressed(GLFW_KEY_LEFT) ||
                app->getKeyboard().isPressed(GLFW_KEY_RIGHT))
            {
                // MOVING   =>  Jump Effect
                // make the frog jump
                frog->localTransform.position.y = float(0.05f * sin(glfwGetTime() * 10) + 0.05f) - 1;
                // make the frog rotate
                frog->localTransform.rotation.x = float(0.1f * sin(glfwGetTime() * 10)) - glm::pi<float>() / 2;
                // make the frog scale
                frog->localTransform.scale.y = 0.01f * sin(glfwGetTime() * 10) + 0.05f;

                // UP
                if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                {
                    // prevent the frog from passing through the wall
                    if (frog->localTransform.position.z < levelEnd)
                        return;

                    // prevent the frog from passing through the water
                    if (frog->localTransform.position.z - waterWidth / 2 < water->localTransform.position.z)
                    {
                        // frog dies
                        // TODO: play death sound, end game
                        // return;
                    }

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

                    // prevent the frog from passing through the water
                    if (frog->localTransform.position.z + waterWidth / 2 > water->localTransform.position.z)
                    {
                        // frog dies
                        // TODO: play death sound, end game
                        // return;
                    }

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

            // check if car hits frog
            for (auto car : cars)
            {
                if (
                    frog->localTransform.position.x < car->localTransform.position.x + 2.3f &&
                    frog->localTransform.position.x > car->localTransform.position.x - 2.3f &&
                    frog->localTransform.position.z < car->localTransform.position.z + 1.1f &&
                    frog->localTransform.position.z > car->localTransform.position.z - 1.1f)
                {
                    // frog dies
                    std::cout << "frog dies-" << rand() << std::endl;
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

                    std::cout << "frog move with the trunk-" << rand() << std::endl;
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
            }

            for (auto coin : coins)
            {
                if (((int(frog->localTransform.position.z) == 3 || (int(frog->localTransform.position.z) == 0)) && (int(frog->localTransform.position.x) == (coin->localTransform.position.x))))
                {
                    std::cout << "Collison Coin Occur!!" << std::endl;
                }
            }

            // for (auto entity : world->getEntities())
            // {
            //     Health *health = entity->getComponent<Health>();
            //     std::cout << health->getID() << std::endl;
            //     std::cout << int(entity->localTransform.position.z) << std::endl;

            //     if (health)
            //     {
            //         if (int(frog->localTransform.position.z) == 3 || int(frog->localTransform.position.z) == 0)
            //         {
            //             std::cout << "Collison Occur!!" << std::endl;
            //             health->active = !health->active;
            //         }
            //     }
            // }
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
