#pragma once

#include "../ecs/component.hpp"

#include <glm/mat4x4.hpp>

namespace our {

    // An enum that defines the type of the light (DIRECTIONAL or SPOT or POINT) 
    enum class LightType {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    // This component denotes that any renderer should draw the scene relative to this camera.
    // We do not define the eye, center or up here since they can be extracted from the entity local to world matrix
    class LightComponent : public Component {
    public:
        LightType LightType; // The type of the light (DIRECTIONAL or SPOT or POINT)
        // glm::vec3 position; // The position of the light
        glm::vec3 direction ; // The direction of the light
        // glm::vec3 color; // The color of the light
        glm::vec3 attenuation; // The attenuation of the light
        glm::vec2 cone_angles; // The cone angles of the light
        glm::vec3 diffuse ; // The diffuse of the light
        glm::vec3 specular ; // The specular of the light
        
        // The ID of this component type is "Light"
        static std::string getID() { return "Light"; }
        // Reads light parameters from the given json object
        void deserialize(const nlohmann::json& data) override;
    };

}