#include "renderer/renderer.hpp"
#include "renderer/cameras/perspective_camera.hpp"
#include "integrators/forward_mis_integrator.hpp"
#include "integrators/forward_bsdf_integrator.hpp"
#include "integrators/backward_integrator.hpp"
#include "surfaces/sphere_surface.hpp"
#include "surfaces/plane_surface.hpp"
#include "materials/diffuse_material.hpp"
#include "materials/mirror_material.hpp"
#include "acceleration_structures/brute_force_acceleration_structure.hpp"
#include "light_distributions/uniform_light_distribution.hpp"
#include "lights/const_diffuse_area_light.hpp"
#include "lights/const_infinity_area_light.hpp"
#include "lights/texture_infinity_area_light.hpp"
#include "textures/image_texture.hpp"
#include "core/assets.hpp"
#include "samplers/stratified_sampler.hpp"
#include "allocators/fixed_size_allocator.hpp"
int main()
{
    std::shared_ptr<fc::sphere_surface> surface{new fc::sphere_surface{{{0.0, 1.0, 0.0}}, 1.0}};
    std::shared_ptr<fc::diffuse_material> diffuse_material{new fc::diffuse_material{}};
    std::shared_ptr<fc::mirror_material> mirror_material{new fc::mirror_material{}};
    //std::shared_ptr<fc::area_light> area_light{new fc::const_diffuse_area_light{surface.get(), {1.0, 1.0, 1.0}, 1.0}};

    std::shared_ptr<fc::plane_surface> surface2{new fc::plane_surface{{}, {10.0, 10.0}}};

    std::vector<fc::entity> entities{};
    entities.push_back({surface, mirror_material, nullptr});
    entities.push_back({surface2, diffuse_material, nullptr});

    fc::brute_force_acceleration_structure_factory bfasf{};
    fc::uniform_light_distribution_factory uldf{};
    fc::uniform_spatial_light_distribution_factory usldf{};

    fc::assets assets{};
    auto image{assets.get_image("env-loft-hall")};
    std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    std::shared_ptr<fc::texture_infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{}, texture, 1.0, image->get_resolution()}};
    //std::shared_ptr<fc::const_infinity_area_light> infinity_area_light{new fc::const_infinity_area_light{{0.5, 0.5, 0.5}, 0.3}};

    std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, bfasf, uldf, usldf}};


    //fc::random_sampler_1d_factory random_sampler_1d_factory{};
    //fc::random_sampler_2d_factory random_sampler_2d_factory{};
    fc::stratified_sampler_1d_factory random_sampler_1d_factory{true};
    fc::stratified_sampler_2d_factory random_sampler_2d_factory{true};

    fc::perspective_camera_factory camera_factory{{{-4.0, 2.0, -4.0}, {fc::math::deg_to_rad(15.0), fc::math::deg_to_rad(45.0), 0.0}}, fc::math::deg_to_rad(45.0)};
    //std::shared_ptr<fc::forward_mis_integrator> integrator{new fc::forward_mis_integrator{10}};
    //std::shared_ptr<fc::forward_bsdf_integrator> integrator{new fc::forward_bsdf_integrator{10}};
    std::shared_ptr<fc::backward_integrator> integrator{new fc::backward_integrator{10}};
    fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 16, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    renderer.run(512);
    renderer.export_image("normals");

    return 0;
}