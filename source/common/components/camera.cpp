#include "camera.hpp"
#include "../ecs/entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace our
{
    // Reads camera parameters from the given json object
    void CameraComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        std::string cameraTypeStr = data.value("cameraType", "perspective");
        if (cameraTypeStr == "orthographic")
        {
            cameraType = CameraType::ORTHOGRAPHIC;
        }
        else
        {
            cameraType = CameraType::PERSPECTIVE;
        }
        near = data.value("near", 0.01f);
        far = data.value("far", 100.0f);
        fovY = data.value("fovY", 90.0f) * (glm::pi<float>() / 180);
        orthoHeight = data.value("orthoHeight", 1.0f);
    }

    // Creates and returns the camera view matrix
    glm::mat4 CameraComponent::getViewMatrix() const
    {
        auto owner = getOwner();
        auto M = owner->getLocalToWorldMatrix();
        // DONE: (Req 8) Complete this function
        // HINT:
        //  In the camera space:
        //  - eye is the origin (0,0,0)
        //  - center is any point on the line of sight. So center can be any point (0,0,z) where z < 0. For simplicity, we let center be (0,0,-1)
        //  - up is the direction (0,1,0)
        //  but to use glm::lookAt, we need eye, center and up in the world state.
        //  Since M (see above) transforms from the camera to thw world space, you can use M to compute:
        //  - the eye position which is the point (0,0,0) but after being transformed by M
        //  - the center position which is the point (0,0,-1) but after being transformed by M
        //  - the up direction which is the vector (0,1,0) but after being transformed by M
        //  then you can use glm::lookAt
        glm::vec3 eye = glm::vec3(M * glm::vec4(0, 0, 0, 1));     // eye position which is the point (0,0,0) but after being transformed by M
        glm::vec3 center = glm::vec3(M * glm::vec4(0, 0, -1, 1)); // center position which is the point (0,0,-1) but after being transformed by M
        glm::vec3 up = glm::vec3(M * glm::vec4(0, 1, 0, 0));      // up direction which is the vector (0,1,0) but after being transformed by M
        return glm::lookAt(eye, center, up);                      // use glm::lookAt to compute the view matrix from the eye, center and up
    }

    // Creates and returns the camera projection matrix
    // "viewportSize" is used to compute the aspect ratio
    glm::mat4 CameraComponent::getProjectionMatrix(glm::ivec2 viewportSize) const
    {
        // DONE: (Req 8) Wrtie this function
        //  NOTE: The function glm::ortho can be used to create the orthographic projection matrix
        //  It takes left, right, bottom, top. Bottom is -orthoHeight/2 and Top is orthoHeight/2.
        //  Left and Right are the same but after being multiplied by the aspect ratio
        //  For the perspective camera, you can use glm::perspective
        //  It takes fovY, aspect ratio, near and far
        float aspectRatio = static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y); // compute the aspect ratio
        if (cameraType == CameraType::PERSPECTIVE)                                                   // if the camera type is perspective
            return glm::perspective(fovY, aspectRatio, near, far);                                   // use glm::perspective to compute the projection matrix

        return glm::ortho(-orthoHeight / 2.0f * aspectRatio, orthoHeight / 2.0f * aspectRatio, -orthoHeight / 2.0f, orthoHeight / 2.0f, near, far); // else (camera type is orthographic), use glm::ortho to compute the projection matrix
    }
}