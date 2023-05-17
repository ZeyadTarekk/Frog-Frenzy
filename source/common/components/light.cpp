#include "light.hpp"
#include "../ecs/entity.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../deserialize-utils.hpp"

namespace our
{
    // Reads light parameters from the given json object
    void LightComponent::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        std::string lightTypeStr = data.value("lightType", "directional");
        if (lightTypeStr == "directional")
        {
            LightType = LightType::DIRECTIONAL;
        }
        else if (lightTypeStr == "point")
        {
            LightType = LightType::POINT;
        }
        else
        {
            LightType = LightType::SPOT;
        }
        // position = data.value("position", position);
        direction = data.value("direction", direction);
        // color = data.value("color", color);
        attenuation = data.value("attenuation", attenuation);
        cone_angles = data.value("cone_angles", cone_angles);
        diffuse = data.value("diffuse", diffuse);
        specular = data.value("specular", specular);
    }

}