#pragma once

#include "../ecs/world.hpp"
#include "../components/camera.hpp"
#include "../components/free-camera-controller.hpp"
#include "../components/movement.hpp"

#include "../application.hpp"
#include "forward-renderer.hpp"
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
        float levelEnd[5] = {-16.0f, -16.0f, -16.0f, -16.0f, -25.0f}; // The end of the levels
        float waterWidth = 2.0f;                                      // The width of the water
        float widthLeft = -8.f;                                       // left width of the level
        float widthRight = 8.f;                                       // right width of the level
        float startFrog = 9.0f;                                       // The start of the frog
        int enteredCoins = 1;                                         // number of coins randomed
        bool frogAboveTrunk = false;
        int maxHeightAtWin = 10;
        float mazeMaxRight = 7.0f;
        vector<glm::vec3> positionsOfCoins; // positions of the current coins
        ForwardRenderer *renderer = nullptr;

        // Entities in the game
        Entity *monkey = nullptr;
        Entity *cup = nullptr;

        std::vector<std::pair<std::pair<float, float>, std::pair<float, float>>> mazeTiles;
        int currentTile = 0;

    public:
        // When a state enters, it should call this function and give it the pointer to the application
        void enter(Application *app)
        {
            this->app = app;
            app->setGameState(GameState::PLAYING);
            ISoundEngine *soundEngine = app->getSoundEngine();
            if (soundEngine == nullptr)
                app->setSoundEngine(createIrrKlangDevice());
            int level = app->getLevel();

            // true to make it repeat infinitly
            playAudio("level_1.ogg", true, true);

            // Establis the maze
            mazeTiles.push_back(std::make_pair(std::make_pair(2.3, 18.3), std::make_pair(-2, 16.45)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-1.3, 18.1), std::make_pair(-2, 10.1)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-1.3, 15), std::make_pair(-6, 13.2)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-3.5, 15), std::make_pair(-4.3, -3.1)));
            mazeTiles.push_back(std::make_pair(std::make_pair(2.6, 7.3), std::make_pair(-4, 5.6)));

            mazeTiles.push_back(std::make_pair(std::make_pair(-4, -1.5), std::make_pair(-6.15, -3)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-5.22, -2), std::make_pair(-6, -19.7)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-6, -8), std::make_pair(-8.8, -9.85)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-7.9, -0.3), std::make_pair(-8.8, -15.3)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-6.8, -13.65), std::make_pair(-8.8, -15.3)));
            mazeTiles.push_back(std::make_pair(std::make_pair(-3.44, -17.8), std::make_pair(-4.35, -24)));
            mazeTiles.push_back(std::make_pair(std::make_pair(8, -21.9), std::make_pair(-4.35, -24)));
            mazeTiles.push_back(std::make_pair(std::make_pair(5.95, -4.65), std::make_pair(4.95, -22)));

            mazeTiles.push_back(std::make_pair(std::make_pair(-3.46, -18), std::make_pair(-6, -19.75)));
            mazeTiles.push_back(std::make_pair(std::make_pair(8, -4.65), std::make_pair(5.95, -7.25)));
        }

        // This should be called every frame to update all entities containing a FreeCameraControllerComponent
        void update(World *world, float deltaTime, ForwardRenderer *renderer)
        {
            // First of all, we search for an entity containing both a CameraComponent and a FreeCameraControllerComponent
            // As soon as we find one, we break
            world->deleteMarkedEntities();
            CameraComponent *camera = nullptr;
            this->renderer = renderer;
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

            if (app->getKeyboard().isPressed(GLFW_KEY_1))
            {
                app->decreaseSound();
            }
            if (app->getKeyboard().isPressed(GLFW_KEY_2))
            {
                app->increaseSound();
            }

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
            else if (app->getGameState() == GameState::FINISH)
            {
                if (app->getKeyboard().isPressed(GLFW_KEY_ENTER))
                {
                    app->resetGame();
                    auto &config = app->getConfig()["scene"];
                    if (config.contains("world_level_1"))
                    {
                        world->clear();
                        world->deserialize(config["world_level_1"]);
                    }
                }
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
                    //? generate random coins in map
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> disX(widthLeft, widthRight);
                    std::uniform_real_distribution<float> disZ((levelEnd[app->getLevel() - 1] + 2) / 2, (startFrog - 2) / 2);
                    glm::vec3 randomPosition = glm::vec3(disX(gen), -0.5f, disZ(gen));
                    if (enteredCoins < 4)
                    {
                        entity->localTransform.position = randomPosition;
                        positionsOfCoins.push_back(randomPosition);
                        enteredCoins++;
                    }

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
                else if (name == "cup")
                {
                    cup = entity;
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

                playAudio("frog_move.ogg");
                // std::thread audioThread(this->playAudio, "frog_move.ogg");
                // audioThread.detach();

                // UP
                if (app->getKeyboard().isPressed(GLFW_KEY_UP))
                {
                    // prevent the frog from passing through the wall
                    if (frog->localTransform.position.z < levelEnd[app->getLevel() - 1])
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
                    if (frog->localTransform.position.x > levelWidth / 2 || (app->getLevel() == 5 && position.x >= mazeMaxRight))
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
                float x = frog->localTransform.position.x;
                float z = frog->localTransform.position.z;
                float x1 = mazeTiles[currentTile].first.first;
                float x2 = mazeTiles[currentTile].second.first;
                float z1 = mazeTiles[currentTile].first.second;
                float z2 = mazeTiles[currentTile].second.second;

                if (app->getLevel() == 5 && !((x <= x1 && x >= x2) && (z <= z1 && z >= z2)))
                {
                    int i;
                    for (i = 0; i < mazeTiles.size(); i++)
                    {
                        x1 = mazeTiles[i].first.first;
                        x2 = mazeTiles[i].second.first;
                        z1 = mazeTiles[i].first.second;
                        z2 = mazeTiles[i].second.second;
                        if ((x <= x1 && x >= x2) && (z <= z1 && z >= z2))
                        {
                            // std::cout << "Transition to " << i << std::endl;
                            currentTile = i;
                            break;
                        }
                    }
                    if (i == mazeTiles.size())
                    {
                        this->gameOver();
                    }
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
                    this->renderer->applyPostPreprocessing = true;
                    this->gameOver();
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
                        this->gameOver();
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
                    playAudio("coins.mp3");      //? playing audio at collision detection
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
            app->setGameState(GameState::GAME_OVER);

            playAudio("game_over.ogg");
        }

        //  Plays game over audio
        void playAudio(std::string audioFileName, bool repeat = false, bool stopAll = false)
        {
            ISoundEngine *soundEngine = app->getSoundEngine();
            std::string audioPath = "assets/audios/" + audioFileName;
            if (!soundEngine)
                return;
            if (stopAll)
            {
                soundEngine->stopAllSounds();
            }
            if (!soundEngine->isCurrentlyPlaying(audioPath.c_str()))
            {
                // repeat is a boolean
                // when this boolean is true the audio repeats after it's finished
                soundEngine->play2D(audioPath.c_str(), repeat);
            }
        }

        void finishLevel(World *world)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            bool upgraded = app->upgradeLevel();
            if (!upgraded)
            {
                app->setGameState(GameState::FINISH);
                if (cup)
                {
                    cup->localTransform.position.y = 0;
                }
                return;
            }
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
            resetCoins();
            if (newLevel == 1)
            {
                playAudio("level_1.ogg", true, true);
            }
            else if (newLevel == 2)
            {
                playAudio("level_2.mp3", true, true);
            }
            else if (newLevel == 3)
            {

                playAudio("level_3.mp3", true, true);
            }
            else if (newLevel == 4)
            {
                playAudio("level_4.mp3", true, true);
            }
            else if (newLevel == 5)
            {
                playAudio("level_5.mp3", true, true);
            }
        }

        void restartLevel(World *world)
        {
            this->renderer->applyPostPreprocessing = false;
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            app->setGameState(GameState::PLAYING);
            int currentLives = app->getLives();
            auto &config = app->getConfig()["scene"];
            std::string levelName;
            if (currentLives == 0)
            {
                playAudio("game_over.mp3", false, true);
                app->changeState("end-game");
                return;
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
            resetCoins();
        }
        void resetCoins()
        {
            // clear coins
            enteredCoins = 1;
            positionsOfCoins.clear();
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
    Application *our::FreeCameraControllerSystem::app = nullptr;
}