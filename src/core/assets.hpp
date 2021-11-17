#pragma once
#include "image.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace fc
{
    class assets
    {
    public:
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
        std::unordered_map<std::string, std::shared_ptr<image>> images_{};

        std::shared_ptr<image> load_image(std::string const& name);
    };
}