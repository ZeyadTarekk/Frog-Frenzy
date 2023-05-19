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
#include <thread>
#include <filesystem>
#include <utility>
// #include <unistd.h>
#include <chrono>
#include <irrKlang.h>
using namespace irrklang;

namespace our
{

    //  The game state is used to determine the state of the game

    // The free camera controller system is responsible for moving every entity which contains a FreeCameraControllerComponent.
    // This system is added as a slightly complex example for how use the ECS framework to implement logic.
    // For more information, see "common/components/free-camera-controller.hpp"
    class FreeCameraControllerSystem
    {
        static Application *app;                                      // The application in which the state runs
        bool mouse_locked = false;                                    // Is the mouse locked
        float levelWidth = 19.0f;                                     // The width of the level
        float levelStart = 24.5f;                                     // The start of the level
        float levelEnd[5] = {-16.0f, -16.0f, -16.0f, -16.0f, -25.0f}; // The end of the level
        float waterWidth = 2.0f;                                      // The width of the water
        float widthLeft = -8.f;                                       // left width of the level
        float widthRight = 8.f;                                       // right width of the level
        float startFrog = 9.0f;                                       // The start of the frog
        int entered = 1;
        bool frogAboveTrunk = false;
        int maxHeightAtWin = 10;

        vector<glm::vec3> positionsOfCoins;

        static bool isFrogMovementAudioRunning;

        // Entities in the game
        Entity *monkey = nullptr;

        std::map<std::pair<int, int>, int> maze = {
            {{2, 18}, 1}, {{1, 18}, 1}, {{0, 18}, 1}, {{-1, 18}, 1}, {{-2, 18}, 1},
            {{-2, 17}, 1}, {{-2, 16}, 1}, {{-2, 15}, 1}, {{-2, 14}, 1}, {{-2, 13}, 1}, {{-2, 12}, 1}, {{-2, 11}, 1}, {{-2, 10}, 1},
            {{-3, 15}, 1}, {{-4, 15}, 1}, {{-5, 15}, 1}, {{-6, 15}, 1},
            {{-3, 14}, 1}, {{-4, 14}, 1}, {{-5, 14}, 1}, {{-6, 14}, 1},
            {{-3, 13}, 1}, {{-4, 13}, 1}, {{-5, 13}, 1}, {{-6, 13}, 1},
            {{-4, 12}, 1}, {{-4, 11}, 1}, {{-4, 10}, 1}, {{-4, 9}, 1}, {{-4, 8}, 1}, {{-4, 7}, 1}, {{-4, 6}, 1}, {{-4, 5}, 1}, {{-4, 4}, 1}, {{-4, 3}, 1}, {{-4, 2}, 1}, {{-4, 1}, 1}, {{-4, 0}, 1}, {{-4, -1}, 1}, {{-4, -2}, 1},
            {{-5, -1}, 1}, {{-6, -1}, 1}, 
            {{-5, -2}, 1}, {{-6, -2}, 1}, 
            {{-6, -3}, 1}, {{-6, -4}, 1}, {{-6, -5}, 1}, {{-6, -6}, 1}, {{-6, -7}, 1}, {{-6, -8}, 1}, {{-6, -9}, 1}, {{-6, -10}, 1}, {{-6, -11}, 1}, {{-6, -12}, 1}, {{-6, -13}, 1}, {{-6, -14}, 1}, {{-6, -15}, 1}, {{-6, -16}, 1}, {{-6, -17}, 1}, {{-6, -18}, 1}, {{-6, -19}, 1}, 
            {{-6, -18}, 1}, {{-5, -18}, 1}, {{-4, -18}, 1},
            {{-6, -19}, 1}, {{-5, -19}, 1}, {{-4, -19}, 1},
            {{-4, -20}, 1}, {{-4, -21}, 1}, {{-4, -22}, 1}, {{-4, -23}, 1}, {{-4, -24}, 1},
            {{-4, -22}, 1}, {{-3, -22}, 1}, {{-2, -22}, 1}, {{-1, -22}, 1}, {{0, -22}, 1}, {{1, -22}, 1}, {{2, -22}, 1}, {{3, -22}, 1}, {{4, -22}, 1}, {{5, -22}, 1}, {{6, -22}, 1},
            {{-4, -23}, 1}, {{-3, -23}, 1}, {{-2, -23}, 1}, {{-1, -23}, 1}, {{0, -23}, 1}, {{1, -23}, 1}, {{2, -23}, 1}, {{3, -23}, 1}, {{4, -23}, 1}, {{5, -23}, 1},
            {{-4, -24}, 1}, {{-3, -24}, 1}, {{-2, -24}, 1}, {{-1, -24}, 1}, {{0, -24}, 1}, {{1, -24}, 1}, {{2, -24}, 1}, {{3, -24}, 1}, {{4, -24}, 1}, {{5, -24}, 1},
            {{5, -21}, 1}, {{5, -20}, 1}, {{5, -19}, 1}, {{5, -18}, 1}, {{5, -17}, 1}, {{5, -16}, 1}, {{5, -15}, 1}, {{5, -14}, 1}, {{5, -13}, 1}, {{5, -12}, 1}, {{5, -11}, 1}, {{5, -10}, 1}, {{5, -9}, 1}, {{5, -8}, 1}, {{5, -7}, 1}, {{5, -6}, 1}, {{5, -5}, 1},
            {{6, -21}, 1}, {{6, -20}, 1}, {{6, -19}, 1}, {{6, -18}, 1}, {{6, -17}, 1}, {{6, -16}, 1}, {{6, -15}, 1}, {{6, -14}, 1}, {{6, -13}, 1}, {{6, -12}, 1}, {{6, -11}, 1}, {{6, -10}, 1}, {{6, -9}, 1}, {{6, -8}, 1}, {{6, -7}, 1}, {{6, -6}, 1}, {{6, -5}, 1},
            {{7, -5}, 1}, {{7, -6}, 1}, {{7, -7}, 1},
        };

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
            app->setGameState(GameState::PLAYING);
            if (app->getLevel() == 1)
            {
                // std::thread audioThread(this->playAudio, "level_1.ogg");
                // audioThread.detach();
            }
            else if (app->getLevel() == 2)
            {
                // std::thread audioThread(this->playAudio, "level_2.ogg");
                // audioThread.detach();
            }
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

            if (app->getGameState() == GameState::GAME_OVER)
            {
                restartLevel(world);
                return;
            }

            // Entities in the frame
            Entity *frog = nullptr;
            Entity *holdingComponent = nullptr;
            Entity *woodenBox = nullptr;

            std::vector<Entity *> cars;
            std::vector<Entity *> trunks;
            std::vector<Entity *> coins;
            std::vector<Entity *> water;
            for (auto entity : world->getEntities())
            {
                std::string name = entity->name;
                if (name == "frog")
                {
                    frog = entity;
                }
                else if (!holdingComponent && name == "holdingComponent")
                {
                    holdingComponent = entity;
                }
                else if (name == "water")
                {
                    water.push_back(entity);
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
                    std::uniform_real_distribution<float> disZ((levelEnd[app->getLevel()-1] + 2) / 2, (startFrog - 2) / 2);
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
                else if (name == "woodenBox")
                {
                    woodenBox = entity;
                }
                else if (name == "monkey")
                {
                    monkey = entity;
                }
            }
            if (!frog)
                return;

            if (app->getGameState() == GameState::WIN)
            {
                // make wooden box flying when collision with frog
                glm::vec3 deltaPosition = glm::vec3(0.0f, 5 * deltaTime, 0.0f);
                woodenBox->localTransform.position += deltaPosition; // update position of wooden box
                frog->localTransform.position += deltaPosition;      // update position of frog
                position += deltaPosition;                           // update position of camera

                if (position.y >= maxHeightAtWin)
                {
                    finishLevel(world);
                }
                return;
            }

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

                //  prevent multiple audios playing at the same time
                if (!isFrogMovementAudioRunning)
                {
                    isFrogMovementAudioRunning = true;
                    //  Plays frog movement audio in a separate thread
                    // std::thread audioThread(this->playAudio, "frog_move.ogg");
                    // audioThread.detach();
                }

                // UP
                if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                {
                    // prevent the frog from passing through the wall
                    if (frog->localTransform.position.z < levelEnd[app->getLevel()-1])
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

                //  Maze
                int x = round(frog->localTransform.position.x);
                int z = round(frog->localTransform.position.z);

                if (app->getLevel() == 5 && !maze[{x, z}])
                {
                    this->gameOver(world);
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
                    this->gameOver(world);
                }
            }
            frogAboveTrunk = false;
            for (auto trunk : trunks)
            {
                // Move the frog with the trunk
                if (frog->localTransform.position.x < trunk->localTransform.position.x + 1.7f &&
                    frog->localTransform.position.x > trunk->localTransform.position.x - 1.7f &&
                    frog->localTransform.position.z < trunk->localTransform.position.z + 1.0f &&
                    frog->localTransform.position.z > trunk->localTransform.position.z - 1.0f)
                {
                    frogAboveTrunk = true;
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
            if (!frogAboveTrunk)
                for (auto wat : water)
                {
                    if (
                        frog->localTransform.position.z - waterWidth / 2 < wat->localTransform.position.z &&
                        frog->localTransform.position.z + waterWidth / 2 > wat->localTransform.position.z)
                        this->gameOver(world);
                }

            for (auto coin : coins)
            {
                if ((int(frog->localTransform.position.z) == int(coin->localTransform.position.z)) && (int(frog->localTransform.position.x) == int(coin->localTransform.position.x)))
                {
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> dis(5.0f, 10.0f);
                    app->addCoins(dis(gen));     //? adding extra random time  (5~10)
                    world->markForRemoval(coin); //? removing coin after collision detection
                }
            }
            if (
                frog->localTransform.position.z - woodenBox->localTransform.position.z < 1.0f &&
                frog->localTransform.position.z - woodenBox->localTransform.position.z > -1.0f &&
                frog->localTransform.position.x - woodenBox->localTransform.position.x < 1.0f &&
                frog->localTransform.position.x - woodenBox->localTransform.position.x > -1.0f)
            {
                app->setGameState(GameState::WIN);
            }

            if (app->getTimeDiff() <= 0)
            {
                this->gameOver(world);
            }
        }

        //  When the frog hits the water, collides with a car, or runs out of time, the game is over.
        void gameOver(World *world)
        {
            if (monkey)
            {
                monkey->localTransform.position.y = 0;
            }

            app->setGameState(GameState::GAME_OVER);

            //  Plays game over audio in a separate thread
            // std::thread audioThread(this->playAudio, "game_over.ogg");
            // audioThread.detach();
        }

        //  Plays game over audio
        // static void playAudio(std::string audioFileName)
        // {
        //     // std::string audioPath = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path().parent_path().string() + "/assets/audios/" + audioFileName;
        //     std::string audioPath = "assets/audios/" + audioFileName;
        //     ISoundEngine *engine = createIrrKlangDevice();
        //     std::cout << audioPath << std::endl;
        //     if (!engine)
        //         return;

        //     ISoundSource *sound = engine->addSoundSourceFromFile(audioPath.c_str());

        //     if (!sound)
        //         return;

        //     ISound *audio = engine->play2D(sound);

        //     while (engine->isCurrentlyPlaying(sound))
        //         ;

        //     engine->drop(); // delete engine

        //     if (audioFileName == "frog_move.ogg")
        //     {
        //         isFrogMovementAudioRunning = false;
        //     }
        // }

        void finishLevel(World *world)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            app->upgradeLevel();
            app->setGameState(GameState::PLAYING);
            auto &config = app->getConfig()["scene"];
            int newLevel = app->getLevel();
            std::string levelName = "world_level_" + std::to_string(newLevel);
            if (config.contains(levelName))
            {
                world->clear();
                world->deserialize(config[levelName]);
                app->setScore(app->getScore() * 2);
                int currentScore = app->getScore();
                if (currentScore >= 100)
                {
                    // Check if the score reaches 100 increase the lives by 1
                    app->setScore(currentScore - 100);
                    app->setLives(app->getLives() + 1);
                }
            }
        }

        void restartLevel(World *world)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            app->setGameState(GameState::PLAYING);
            int currentLives = app->getLives();
            auto &config = app->getConfig()["scene"];
            std::string levelName;
            if (currentLives == 0)
            {
                app->setLives(3);
                app->setScore(0);
                levelName = "world_level_1";
            }
            else
            {
                app->setLives(currentLives - 1);

                int currentLevel = app->getLevel();
                levelName = "world_level_" + std::to_string(currentLevel);
            }
            if (config.contains(levelName))
            {
                world->clear();
                world->deserialize(config[levelName]);
            }
            int currentScore = app->getScore();
            if (currentScore >= 100)
            {
                // Check if the score reaches 100 increase the lives by 1
                app->setScore(currentScore - 100);
                app->setLives(app->getLives() + 1);
            }
            app->resetTime();
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

    //  Definition of static data members
    bool our::FreeCameraControllerSystem::isFrogMovementAudioRunning = false;
    Application *our::FreeCameraControllerSystem::app = nullptr;
}