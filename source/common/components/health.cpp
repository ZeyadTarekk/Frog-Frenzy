#include "health.hpp"
#include "../ecs/entity.hpp"
#include "../deserialize-utils.hpp"

namespace our
{
    // Reads linearVelocity & angularVelocity from the given json object
    void Health::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;
        linearVelocity = data.value("linearVelocity", linearVelocity);
        angularVelocity = glm::radians(data.value("angularVelocity", angularVelocity));
        name = data.value("name", name);
        id = data.value("id", id);
        bool active = true;
        }
}