#pragma once
#include "mesh.hpp"
#include "image.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace Fc
{
    class Assets
    {
    public:
        std::shared_ptr<IMesh> GetMesh(std::string const& name)
        {
            auto it{meshes_.find(name)};
            if(it != meshes_.end())
            {
                return it->second;
            }

            auto mesh{LoadMesh(name)};
            meshes_.insert({name, mesh});

            return mesh;
        }

        std::shared_ptr<IImage> GetImage(std::string const& name)
        {
            auto it{images_.find(name)};
            if(it != images_.end())
            {
                return it->second;
            }

            auto image{LoadImage(name)};
            images_.insert({name, image});

            return image;
        }

    private:
        std::unordered_map<std::string, std::shared_ptr<IMesh>> meshes_{};
        std::unordered_map<std::string, std::shared_ptr<IImage>> images_{};

        static std::shared_ptr<IMesh> LoadMesh(std::string const& name);
        static std::shared_ptr<IImage> LoadImage(std::string const& name);
    };
}