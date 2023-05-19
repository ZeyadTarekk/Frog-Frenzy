#include "material.hpp"

#include "../asset-loader.hpp"
#include "deserialize-utils.hpp"

namespace our
{

    // This function should setup the pipeline state and set the shader to be used
    void Material::setup() const
    {
        // DONE: (Req 7) Write this function
        this->pipelineState.setup();
        this->shader->use();
    }

    // This function read the material data from a json object
    void Material::deserialize(const nlohmann::json &data)
    {
        if (!data.is_object())
            return;

        if (data.contains("pipelineState"))
        {
            pipelineState.deserialize(data["pipelineState"]);
        }
        shader = AssetLoader<ShaderProgram>::get(data["shader"].get<std::string>());
        transparent = data.value("transparent", false);
    }

    // This function should call the setup of its parent and
    // set the "tint" uniform to the value in the member variable tint
    void TintedMaterial::setup() const
    {
        // DONE: (Req 7) Write this function
        Material::setup();
        shader->set("tint", this->tint);
    }

    // This function read the material data from a json object
    void TintedMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        tint = data.value("tint", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    // This function should call the setup of its parent and
    // set the "alphaThreshold" uniform to the value in the member variable alphaThreshold
    // Then it should bind the texture and sampler to a texture unit and send the unit number to the uniform variable "tex"
    void TexturedMaterial::setup() const
    {
        // DONE: (Req 7) Write this function
        TintedMaterial::setup();
        this->shader->set("alphaThreshold", this->alphaThreshold);
        glActiveTexture(GL_TEXTURE0); // we send it unit 0
        // check if the texture is null call the unbind
        if (this->texture)
            this->texture->bind();
        else
            this->texture->unbind();
        // check if the sampler is null call the unbind
        if (this->sampler)
            this->sampler->bind(0);
        else
            this->sampler->unbind(0);
        shader->set("tex", 0);
    }

    // This function read the material data from a json object
    void TexturedMaterial::deserialize(const nlohmann::json &data)
    {
        TintedMaterial::deserialize(data);
        if (!data.is_object())
            return;
        alphaThreshold = data.value("alphaThreshold", 0.0f);
        texture = AssetLoader<Texture2D>::get(data.value("texture", ""));
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
    }

    void LightMaterial::setup() const
    {
        Material::setup();
        if (this->albedo) {
            // activate the texture unit 0
            glActiveTexture(GL_TEXTURE0);
            // bind the texture
            this->albedo->bind();
            // bind the sampler
            this->sampler->bind(0);
            // send the texture unit to the uniform variable "material.albedo" 
            shader->set("material.albedo", 0);
        }

        if (this->specular) {
            // activate the texture unit 1
            glActiveTexture(GL_TEXTURE1);
            // bind the texture
            this->specular->bind();
            // bind the sampler
            this->sampler->bind(1);
            // send the texture unit to the uniform variable "material.specular"
            shader->set("material.specular", 1);
        }
        if (this->emissive) {
            // activate the texture unit 2
            glActiveTexture(GL_TEXTURE2);
            // bind the texture
            this->emissive->bind();
            // bind the sampler
            this->sampler->bind(2);
            // send the texture unit to the uniform variable "material.emissive"
            shader->set("material.emissive", 2);
        }
        if (this->roughness)
        {
            // activate the texture unit 3
            glActiveTexture(GL_TEXTURE3);
            // bind the texture
            this->roughness->bind();
            // bind the sampler
            this->sampler->bind(3);
            // send the texture unit to the uniform variable "material.roughness"
            shader->set("material.roughness", 3);
        }
        if (this->ambient_occlusion)
        {
            // activate the texture unit 4
            glActiveTexture(GL_TEXTURE4);
            // bind the texture
            this->ambient_occlusion->bind();
            // bind the sampler
            this->sampler->bind(4);
            // send the texture unit to the uniform variable "material.ambient_occlusion"
            shader->set("material.ambient_occlusion", 4);
        }
        
        
    }
    void LightMaterial::deserialize(const nlohmann::json &data)
    {
        Material::deserialize(data);
        if (!data.is_object())
            return;
        sampler = AssetLoader<Sampler>::get(data.value("sampler", ""));
        albedo = AssetLoader<Texture2D>::get(data.value("albedo", ""));
        specular = AssetLoader<Texture2D>::get(data.value("specular", ""));
        emissive = AssetLoader<Texture2D>::get(data.value("emissive", ""));
        roughness = AssetLoader<Texture2D>::get(data.value("roughness", ""));
        ambient_occlusion = AssetLoader<Texture2D>::get(data.value("ambient_occlusion", ""));
    }

}