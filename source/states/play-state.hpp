#pragma once

#include <application.hpp>

#include <ecs/world.hpp>
#include <systems/forward-renderer.hpp>
#include <systems/free-camera-controller.hpp>
#include <systems/movement.hpp>
#include <systems/car-generator.hpp>
#include <asset-loader.hpp>
#include <thread>

// This state shows how to use the ECS framework and deserialization.
class Playstate : public our::State
{

    our::World world;
    our::ForwardRenderer renderer;
    our::FreeCameraControllerSystem cameraController;
    our::MovementSystem movementSystem;
    our::CarGeneratorSystem carGeneratorSystem;

    void onInitialize() override
    {
        // First of all, we get the scene configuration from the app config
        auto &config = getApp()->getConfig()["scene"];
        // If we have assets in the scene config, we deserialize them
        if (config.contains("assets"))
        {
            our::deserializeAllAssets(config["assets"]);
        }
        // If we have a world in the scene config, we use it to populate our world
        if (config.contains("world_level_1"))
        {
            world.deserialize(config["world_level_1"]);
        }
        // We initialize the camera controller system since it needs a pointer to the app
        cameraController.enter(getApp());
        // Then we initialize the renderer
        auto size = getApp()->getFrameBufferSize();
        renderer.initialize(size, config["renderer"]);
    }

    void onDraw(double deltaTime) override
    {
        // Here, we just run a bunch of systems to control the world logic
        our::GameState state = getApp()->getGameState();
        if (state != our::GameState::PAUSE)
        {
            movementSystem.update(&world, (float)deltaTime);
            cameraController.update(&world, (float)deltaTime);
            carGeneratorSystem.update(&world, (float)deltaTime);
        }
        // And finally we use the renderer system to draw the scene
        renderer.render(&world);

        // Get a reference to the keyboard object
        auto &keyboard = getApp()->getKeyboard();

        // if (cameraController.getGameState() == our::GameState::PAUSE)
        // {
        //     getApp()->fixTime();
        // }

        if (keyboard.justPressed(GLFW_KEY_ESCAPE))
        {
            // If the escape  key is pressed in this frame, go to the play state
            getApp()->changeState("menu");

            //?  Plays menu  audio in a separate thread
            std::thread audioThread(&our::Application::playAudio, getApp(), "menu.ogg");
            audioThread.detach();
        }

        if (keyboard.justPressed(GLFW_KEY_P))
        {
            if (getApp()->getGameState() == our::GameState::PLAYING)
            {
                getApp()->setGameState(our::GameState::PAUSE);
                getApp()->setTimeDiffOnPause(getApp()->getTimeDiff());
            }
            else if (getApp()->getGameState() == our::GameState::PAUSE)
            {
                getApp()->setCurrentTimeDiff(getApp()->getTimeDiffOnPause());
                getApp()->setGameState(our::GameState::PLAYING);
            }
        }
    }

    void onDestroy() override
    {
        // Don't forget to destroy the renderer
        renderer.destroy();
        // On exit, we call exit for the camera controller system to make sure that the mouse is unlocked
        cameraController.exit();
        // Clear the world
        world.clear();
        // and we delete all the loaded assets to free memory on the RAM and the VRAM
        our::clearAllAssets();
    }
};