#include "forward-renderer.hpp"
#include "../mesh/mesh-utils.hpp"
#include "../texture/texture-utils.hpp"

namespace our
{

    void ForwardRenderer::initialize(glm::ivec2 windowSize, const nlohmann::json &config)
    {
        // First, we store the window size for later use
        this->windowSize = windowSize;

        // Then we check if there is a sky texture in the configuration
        if (config.contains("sky"))
        {
            // First, we create a sphere which will be used to draw the sky
            this->skySphere = mesh_utils::sphere(glm::ivec2(16, 16));

            // We can draw the sky using the same shader used to draw textured objects
            ShaderProgram *skyShader = new ShaderProgram();
            skyShader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
            skyShader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
            skyShader->link();

            // DONE: (Req 10) Pick the correct pipeline state to draw the sky
            //  Hints: the sky will be draw after the opaque objects so we would need depth testing but which depth funtion should we pick?
            //  We will draw the sphere from the inside, so what options should we pick for the face culling.
            PipelineState skyPipelineState{};

            //? depth configuration

            // enable depth testing
            skyPipelineState.depthTesting.enabled = true;
            // fragments will pass the depth test if their depth value is less
            // than or equal to the depth value of the existing fragment in the framebuffer.
            skyPipelineState.depthTesting.function = GL_LEQUAL;

            // face culling configuration
            skyPipelineState.faceCulling.enabled = true;
            skyPipelineState.faceCulling.culledFace = GL_FRONT;
            skyPipelineState.faceCulling.frontFace = GL_CCW;

            // Load the sky texture (note that we don't need mipmaps since we want to avoid any unnecessary blurring while rendering the sky)
            std::string skyTextureFile = config.value<std::string>("sky", "");
            Texture2D *skyTexture = texture_utils::loadImage(skyTextureFile, false);

            // Setup a sampler for the sky
            Sampler *skySampler = new Sampler();
            skySampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            skySampler->set(GL_TEXTURE_WRAP_S, GL_REPEAT);
            skySampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Combine all the aforementioned objects (except the mesh) into a material
            this->skyMaterial = new TexturedMaterial();
            this->skyMaterial->shader = skyShader;
            this->skyMaterial->texture = skyTexture;
            this->skyMaterial->sampler = skySampler;
            this->skyMaterial->pipelineState = skyPipelineState;
            this->skyMaterial->tint = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            this->skyMaterial->alphaThreshold = 1.0f;
            this->skyMaterial->transparent = false;
        }

        // Then we check if there is a postprocessing shader in the configuration
        if (config.contains("postprocess"))
        {
            // DONE: (Req 11) Create a framebuffer
            glGenFramebuffers(1, &postprocessFrameBuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);

            // DONE: (Req 11) Create a color and a depth texture and attach them to the framebuffer
            //  Hints: The color format can be (Red, Green, Blue and Alpha components with 8 bits for each channel).
            //  The depth format can be (Depth component with 24 bits).
            colorTarget = texture_utils::empty(GL_RGBA8, this->windowSize);
            depthTarget = texture_utils::empty(GL_DEPTH_COMPONENT24, this->windowSize);

            // Attach the color buffer texture to the framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTarget->getOpenGLName(), 0);
            // Attach the depth buffer renderbuffer to the framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTarget->getOpenGLName(), 0);

            // DONE: (Req 11) Unbind the framebuffer just to be safe
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Create a vertex array to use for drawing the texture
            glGenVertexArrays(1, &postProcessVertexArray);

            // Create a sampler to use for sampling the scene texture in the post processing shader
            Sampler *postprocessSampler = new Sampler();
            postprocessSampler->set(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            postprocessSampler->set(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            postprocessSampler->set(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            // Create the post processing shader
            ShaderProgram *postprocessShader = new ShaderProgram();
            postprocessShader->attach("assets/shaders/fullscreen.vert", GL_VERTEX_SHADER);
            postprocessShader->attach(config.value<std::string>("postprocess", ""), GL_FRAGMENT_SHADER);
            postprocessShader->link();

            // Create a post processing material
            postprocessMaterial = new TexturedMaterial();
            postprocessMaterial->shader = postprocessShader;
            postprocessMaterial->texture = colorTarget;
            postprocessMaterial->sampler = postprocessSampler;
            // The default options are fine but we don't need to interact with the depth buffer
            // so it is more performant to disable the depth mask
            postprocessMaterial->pipelineState.depthMask = false;
        }
    }

    void ForwardRenderer::destroy()
    {
        // Delete all objects related to the sky
        if (skyMaterial)
        {
            delete skySphere;
            delete skyMaterial->shader;
            delete skyMaterial->texture;
            delete skyMaterial->sampler;
            delete skyMaterial;
        }
        // Delete all objects related to post processing
        if (postprocessMaterial)
        {
            glDeleteFramebuffers(1, &postprocessFrameBuffer);
            glDeleteVertexArrays(1, &postProcessVertexArray);
            delete colorTarget;
            delete depthTarget;
            delete postprocessMaterial->sampler;
            delete postprocessMaterial->shader;
            delete postprocessMaterial;
        }
    }

    void ForwardRenderer::render(World *world)
    {
        // First of all, we search for a camera and for all the mesh renderers
        CameraComponent *camera = nullptr;
        opaqueCommands.clear();
        transparentCommands.clear();
        lightComponents.clear();
        for (auto entity : world->getEntities())
        {
            // If we hadn't found a camera yet, we look for a camera in this entity
            if (!camera)
                camera = entity->getComponent<CameraComponent>();
            // If this entity has a mesh renderer component
            if (auto meshRenderer = entity->getComponent<MeshRendererComponent>(); meshRenderer)
            {
                // if this component has a light component, we add it to the light components list
                if (auto light = entity->getComponent<LightComponent>(); light)
                {
                    lightComponents.push_back(light);
                }
                // We construct a command from it
                RenderCommand command;
                command.localToWorld = meshRenderer->getOwner()->getLocalToWorldMatrix();
                command.center = glm::vec3(command.localToWorld * glm::vec4(0, 0, 0, 1));
                command.mesh = meshRenderer->mesh;
                command.material = meshRenderer->material;
                // if it is transparent, we add it to the transparent commands list
                if (command.material->transparent)
                {
                    transparentCommands.push_back(command);
                }
                else
                {
                    // Otherwise, we add it to the opaque command list
                    opaqueCommands.push_back(command);
                }
            }
        }

        // If there is no camera, we return (we cannot render without a camera)
        if (camera == nullptr)
            return;

        // DONE: (Req 9) Modify the following line such that "cameraForward" contains a vector pointing the camera forward direction
        //  HINT: See how you wrote the CameraComponent::getViewMatrix, it should help you solve this one
        //? calculate the forward direction of the camera by subtracting the eye position from the center position,
        //? which is the direction the camera is pointing towards.
        //? This is achieved by transforming the points (0, 0, -1) and (0, 0, 0) from local space to world space.
        //? last component of the vector is detect position or direction ,, position=1 , direction=0

        auto M = camera->getOwner()->getLocalToWorldMatrix(); //? get local matrix
        glm::vec3 centerTransparency = M * glm::vec4(0.0, 0.0, -1.0, 1.0);
        glm::vec3 eyeTransparency = M * glm::vec4(0.0, 0.0, 0.0, 1.0);
        glm::vec3 cameraForward = glm::normalize(centerTransparency - eyeTransparency);
        std::sort(transparentCommands.begin(), transparentCommands.end(), [cameraForward](const RenderCommand &first, const RenderCommand &second)
                  {
            //DONE: (Req 9) Finish this function
            // HINT: the following return should return true "first" should be drawn before "second".
            //? Sorting the transparent rendering commands based on their distance from the camera's position along its forward direction.
            return (glm::dot(cameraForward,first.center)>glm::dot(cameraForward,second.center) ); });

        // DONE: (Req 9) Get the camera ViewProjection matrix and store it in VP
        glm::mat4 VP = camera->getProjectionMatrix(this->windowSize) * camera->getViewMatrix();

        // DONE: (Req 9) Set the OpenGL viewport using viewportStart and viewportSize
        //? Sets the viewport to cover the entire window.
        glViewport(0, 0, this->windowSize.x, this->windowSize.y);

        // DONE: (Req 9) Set the clear color to black and the clear depth to 1
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClearDepth(1.0);

        // DONE: (Req 9) Set the color mask to true and the depth mask to true (to ensure the glClear will affect the framebuffer)
        glColorMask(true, true, true, true);
        glDepthMask(true);

        // If there is a postprocess material, bind the framebuffer
        if (postprocessMaterial)
        {
            // DONE: (Req 11) bind the framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postprocessFrameBuffer);
        }

        // DONE: (Req 9) Clear the color and depth buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // DONE: (Req 9) Draw all the opaque commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto command : opaqueCommands)
        {
            //* Responsible for rendering all the opaque objects in the scene

            //? 1- calculates the model-view-projection matrix= multiplying the camera view-projection matrix VP by the local-to-world matrix of the object.
            //? 2- sets up the material of the object by calling setup func. that sets the material properties
            //? 3- binding to crossponding shader ("transform")
            //? 4- draw mesh  to render object
            // check if the command  is a lighted material or not
            
            if (auto material = dynamic_cast<LightMaterial *>(command.material))
            {
                if (material != nullptr)
                {
                 
                material->setup();
                // vertex shader
                // send the camera position to the shader
                material->shader->set("eye", eyeTransparency);
                // send the view projection matrix to the shader
                material->shader->set("VP", VP);
                // send the model matrix to the shader
                material->shader->set("M", command.localToWorld);
                // send the model view matrix to the shader
                material->shader->set("M_IT", glm::transpose(glm::inverse(command.localToWorld)));
                // fragment shader
                // sky light color data
                // send the sky light color data to the shader
                material->shader->set("Sky.top", glm::vec3(0.0f, 1.0f, 0.5f));
                material->shader->set("Sky.middle", glm::vec3(0.3f, 0.3f, 0.3f));
                material->shader->set("Sky.bottom", glm::vec3(0.1f, 0.1f, 0.1f));

                //  send the light count
                material->shader->set("light_count",(GLint)lightComponents.size());
                // loop over the light components and send the light data to the shader
                for (auto i = 0; i < (int)lightComponents.size(); i++)
                {
                    material->shader->set("lights[" + std::to_string(i) + "].type", (GLint)lightComponents[i]->LightType);
                    // in case of directional light we need to send the direction of the light only
                    if(lightComponents[i]->LightType==LightType::DIRECTIONAL)
                    {
                        glm::vec3 directional_direction = glm::normalize(lightComponents[i]->direction);
                        material->shader->set("lights[" + std::to_string(i) + "].direction", directional_direction);
                    }
                    // in case of point light we need to send the position of the light only
                    else if (lightComponents[i] ->LightType == LightType::POINT)
                    {
                        glm::vec3 position = glm::vec3(lightComponents[i]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
                        material->shader->set("lights[" + std::to_string(i) + "].position",position );
                    }
                    // in case of spot light we need to send the position and direction of the light 
                    else if (lightComponents[i]->LightType == LightType::SPOT)
                    {
                        glm::vec3 directional_direction = glm::normalize(lightComponents[i]->direction);
                        // we multiply local to world matrix by (0,0,0,1) to get the vec3 and drop the w component
                        glm::vec3 position = glm::vec3(lightComponents[i]->getOwner()->getLocalToWorldMatrix() * glm::vec4(0.0, 0.0, 0.0, 1.0));
                        material->shader->set("lights[" + std::to_string(i) + "].position", position);
                        material->shader->set("lights[" + std::to_string(i) + "].direction", directional_direction );
                        material->shader->set("lights[" + std::to_string(i) + "].cone_angles", lightComponents[i]->cone_angles);
                    }
                    // material->shader->set("lights[" + std::to_string(i) + "].color", lightComponents[i]->color);
                    material->shader->set("lights[" + std::to_string(i) + "].attenuation", lightComponents[i]->attenuation);
                    material->shader->set("lights[" + std::to_string(i) + "].diffuse", lightComponents[i]->diffuse);
                    material->shader->set("lights[" + std::to_string(i) + "].specular", lightComponents[i]->specular);
                }
                }
            }
            else {
            glm::mat4 modelViewProjection = VP * command.localToWorld;
            command.material->setup();
            command.material->shader->set("transform", modelViewProjection);
            }
            
            command.mesh->draw();
        }

        // If there is a sky material, draw the sky
        if (this->skyMaterial)
        {
            // DONE: (Req 10) setup the sky material
            this->skyMaterial->setup();

            // DONE: (Req 10) Get the camera position
            glm::vec3 cameraPosition = M * glm::vec4(0.0, 0.0, 0.0, 1.0);

            // DONE: (Req 10) Create a model matrix for the sy such that it always follows the camera (sky sphere center = camera position)
            glm::mat4 identity = glm::mat4(1.0f);
            glm::mat4 skyModelMatrix = glm::translate(identity, cameraPosition);

            // DONE: (Req 10) We want the sky to be drawn behind everything (in NDC space, z=1)
            //  We can acheive the is by multiplying by an extra matrix after the projection but what values should we put in it?
            glm::mat4 alwaysBehindTransform = glm::mat4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 1.0f);
            // DONE: (Req 10) set the "transform" uniform
            glm::mat4 transform = alwaysBehindTransform * VP * skyModelMatrix;
            skyMaterial->shader->set("transform", transform);
            // DONE: (Req 10) draw the sky sphere
            this->skySphere->draw();
        }
        // DONE: (Req 9) Draw all the transparent commands
        //  Don't forget to set the "transform" uniform to be equal the model-view-projection matrix for each render command
        for (auto command : transparentCommands)
        {
            //* Responsible for rendering all the transparent objects in the scene

            //? 1- calculates the model-view-projection matrix= multiplying the camera view-projection matrix VP by the local-to-world matrix of the object.
            //? 2- sets up the material of the object by calling setup func. that sets the material properties
            //? 3- binding to crossponding shader ("transform")
            //? 4- draw mesh  to render object

            glm::mat4 modelViewProjection = VP * command.localToWorld;
            command.material->setup();
            command.material->shader->set("transform", modelViewProjection);
            command.mesh->draw();
        }

        // If there is a postprocess material, apply postprocessing
        if (postprocessMaterial)
        {
            // DONE: (Req 11) Return to the default framebuffer
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

            // DONE: (Req 11) Setup the postprocess material and draw the fullscreen triangle
            glBindVertexArray(postProcessVertexArray);
            postprocessMaterial->setup();
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }
    }

}