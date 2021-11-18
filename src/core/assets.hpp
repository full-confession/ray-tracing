#pragma once
#include "image.hpp"
#include "mesh.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace fc
{
    class assets
    {
    public:
        std::shared_ptr<mesh> get_mesh(std::string const& name)
        {
            auto it{meshes_.find(name)};
            if(it != meshes_.end())
            {
                return it->second;
            }

            auto mesh{load_mesh(name)};
            meshes_.insert({name, mesh});

            return mesh;
        }

        std::shared_ptr<image> get_image(std::string const& name)
        {
            auto it{images_.find(name)};
            if(it != images_.end())
            {
                return it->second;
            }

            auto image{load_image(name)};
            images_.insert({name, image});

            return image;
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<mesh>> meshes_{};
        std::unordered_map<std::string, std::shared_ptr<image>> images_{};

        std::shared_ptr<mesh> load_mesh(std::string const& name);
        std::shared_ptr<image> load_image(std::string const& name);
    };
}