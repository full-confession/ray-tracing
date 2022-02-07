#pragma once
#include "core/assets.hpp"
#include "surfaces/mesh_surface.hpp"
#include "surfaces/plane_surface.hpp"
#include "surfaces/sphere_surface.hpp"
#include "core/scene.hpp"
#include "materials/diffuse_material.hpp"
#include "materials/glass_material.hpp"
#include "materials/mirror_material.hpp"
#include "materials/plastic_material.hpp"
#include "lights/const_diffuse_area_light.hpp"
#include "textures/const_texture.hpp"
#include "textures/checker_texture.hpp"
#include "textures/image_texture.hpp"
#include "acceleration_structures/bvh_acceleration_structure.hpp"
#include "light_distributions/uniform_light_distribution.hpp"
#include "lights/texture_infinity_area_light.hpp"
#include "samplers/stratified_sampler.hpp"
#include "renderer/cameras/perspective_camera.hpp"
#include "integrators/backward_integrator.hpp"
#include "integrators/forward_mis_integrator.hpp"
#include "integrators/bidirectional_integrator.hpp"
#include "integrators/forward_bsdf_integrator.hpp"
#include "renderer/renderer.hpp"
namespace fc
{
    inline void scene_material_ball()
    {
        assets assets{};

        std::vector<entity> entities{};
        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("ball_2")),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.05, 0.05, 0.05})
            )
        });

        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("ball_1")),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.8, 0.6, 0.2})
            )
            /*std::make_shared<glass_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rg>(vector2{0.35, 0.35})
            ), nullptr,
            std::make_shared<uniform_medium>(1, 1.45, vector3{0.2, 0.4, 0.8}, 4.0)*/

            /*std::make_shared<mirror_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.8, 0.8, 0.8}),
                std::make_shared<const_texture_2d_rg>(vector2{0.2, 0.2})
            )*/

            /*std::make_shared<plastic_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.8, 0.6, 0.2}),
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rg>(vector2{0.15, 0.15}),
                std::make_shared<const_texture_2d_r>(1.45)
            )*/

        });
        entities.push_back({
            std::make_shared<plane_surface>(pr_transform{}, vector2{20.0, 20.0}),
            std::make_shared<diffuse_material>(
                std::make_shared<checker_texture_2d_rgb>(vector3{0.8, 0.8, 0.8}, vector3{0.6, 0.6, 0.6}, 10.0)
            )
        });


        auto infinity_area_light{std::make_shared<texture_infinity_area_light>(
            pr_transform{},
            std::make_shared<image_texture_2d_rgb>(assets.get_image("env-loft-hall"), reconstruction_filter::bilinear, 4),
            1.0,
            assets.get_image("env-loft-hall")->get_resolution())
        };

        bvh_acceleration_structure_factory acceleration_structure_factory{};
        uniform_light_distribution_factory uldf{};
        uniform_spatial_light_distribution_factory usldf{};
        auto scene{std::make_shared<entity_scene>(std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf)};

        perspective_camera_factory camera_factory{{{-2.15, 5.5, -3.6}, {math::deg_to_rad(45), math::deg_to_rad(30), 0.0}}, math::deg_to_rad(30.0)};

        stratified_sampler sampler{64 * 64};

        auto integrator{std::make_shared<forward_mis_integrator>(10, true)};
       // auto integrator{std::make_shared<bidirectional_integrator>(10, true)};

        renderer renderer{{512, 512}, camera_factory, integrator, scene, 15, sampler};
        renderer.run();

        renderer.export_image("material_ball");
    }

    inline void scene_glass()
    {
        assets assets{};

        std::vector<entity> entities{};
        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("glass")),
            std::make_shared<glass_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rg>(vector2{0.0, 0.0})
            ), nullptr,
            std::make_shared<uniform_medium>(2, 1.52, vector3{1.0, 1.0, 1.0}, 1.0)
        });

        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("water")),
            std::make_shared<glass_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rg>(vector2{0.0, 0.0})
            ), nullptr,
            std::make_shared<uniform_medium>(1, 1.3614, vector3{0.2, 0.6, 0.8}, 4.0)
        });

        entities.push_back({
           std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("ice")),
           std::make_shared<glass_material>(
               std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
               std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
               std::make_shared<const_texture_2d_rg>(vector2{0.0, 0.0})
           ), nullptr,
           std::make_shared<uniform_medium>(2, 1.35, vector3{0.2, 0.6, 0.8}, 0.0)
        });

        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("tube")),
            std::make_shared<plastic_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.8, 0.1, 0.05}),
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rg>(vector2{0.2, 0.2}),
                std::make_shared<const_texture_2d_r>(1.46)
            )
        });

        entities.push_back({
            std::make_shared<plane_surface>(pr_transform{{-0.05, 0.0, 0.63}}, vector2{6.0, 5.0}),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.5, 0.5, 0.5})
            )
        });

        entities.push_back({
            std::make_shared<plane_surface>(pr_transform{{0.25, 0.95, 2.89}, {math::deg_to_rad(-90.0), 0.0, 0.0}}, vector2{6.0, 5.0}),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.5, 0.5, 0.5})
            )
        });

        auto infinity_area_light{std::make_shared<texture_infinity_area_light>(
            pr_transform{{}, {0.0, math::deg_to_rad(-15.0), 0.0}},
            std::make_shared<image_texture_2d_rgb>(assets.get_image("env-loft-hall"), reconstruction_filter::bilinear, 4),
            1.0,
            assets.get_image("env-loft-hall")->get_resolution())
        };

        bvh_acceleration_structure_factory acceleration_structure_factory{};
        uniform_light_distribution_factory uldf{};
        uniform_spatial_light_distribution_factory usldf{};
        auto scene{std::make_shared<entity_scene>(std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf)};

        perspective_camera_factory camera_factory{{{-2.79, 1.846, -5.127}, {math::deg_to_rad(6.119), math::deg_to_rad(19.319), 0.0}}, math::deg_to_rad(30.0)};

        stratified_sampler sampler{64 * 64};

        //auto integrator{std::make_shared<forward_mis_integrator>(20, true)};
        auto integrator{std::make_shared<bidirectional_integrator>(20, true)};

        renderer renderer{{512, 512}, camera_factory, integrator, scene, 15, sampler};
        renderer.run();

        renderer.export_image("glass");
    }

    inline void scene_room()
    {
        assets assets{};

        std::vector<entity> entities{};
        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("walls")),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.8, 0.8, 0.8})
            )
        });

        entities.push_back({
            std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("lights_1")),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.1, 0.1, 0.1})
            )
        });

        auto light_surface{std::make_shared<mesh_surface>(prs_transform{}, assets.get_mesh("lights_2"))};
        entities.push_back({
            light_surface,
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.1, 0.1, 0.1})
            ),
            std::make_shared<const_diffuse_area_light>(light_surface.get(), vector3{1.0, 1.0, 1.0}, 250.0)
        });

        entities.push_back({
            std::make_shared<sphere_surface>(pr_transform{{4.2, 2.01, 0.0}}, 2.0),
            std::make_shared<diffuse_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.2, 0.4, 0.8})
            )
        });

        entities.push_back({
            std::make_shared<sphere_surface>(pr_transform{{0.0, 2.01, 0.0}}, 2.0),
            std::make_shared<glass_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rgb>(vector3{1.0, 1.0, 1.0}),
                std::make_shared<const_texture_2d_rg>(vector2{0.0, 0.0})
            ), nullptr,
            std::make_shared<uniform_medium>(1, 1.45, vector3{0.6, 0.2, 0.8}, 0.5)
        });

        entities.push_back({
            std::make_shared<sphere_surface>(pr_transform{{-4.2, 2.01, 0.0}}, 2.0),
            std::make_shared<mirror_material>(
                std::make_shared<const_texture_2d_rgb>(vector3{0.8, 0.4, 0.2}),
                std::make_shared<const_texture_2d_rg>(vector2{0.2, 0.2})
            )
        });

       
        bvh_acceleration_structure_factory acceleration_structure_factory{};
        uniform_light_distribution_factory uldf{};
        uniform_spatial_light_distribution_factory usldf{};
        auto scene{std::make_shared<entity_scene>(std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf)};

        perspective_camera_factory camera_factory{{{-7.23, 2.79, 16.37}, {math::deg_to_rad(-4.641), math::deg_to_rad(157.552), 0.0}}, math::deg_to_rad(35.0)};

        stratified_sampler sampler{128 * 128};

        //auto integrator{std::make_shared<forward_bsdf_integrator>(10)};
        //auto integrator{std::make_shared<forward_mis_integrator>(10, true)};
        //auto integrator{std::make_shared<backward_integrator>(10)};
        auto integrator{std::make_shared<bidirectional_integrator>(10, true)};

        renderer renderer{{800, 450}, camera_factory, integrator, scene, 15, sampler};
        renderer.run();

        renderer.export_image("room");
    }
}