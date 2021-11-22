#include "renderer/renderer.hpp"
#include "renderer/cameras/perspective_camera.hpp"
#include "integrators/forward_mis_integrator.hpp"
#include "integrators/forward_bsdf_integrator.hpp"
#include "integrators/backward_integrator.hpp"
#include "integrators/bidirectional_integrator.hpp"
#include "surfaces/sphere_surface.hpp"
#include "surfaces/plane_surface.hpp"
#include "surfaces/mesh_surface.hpp"
#include "materials/diffuse_material.hpp"
#include "materials/mirror_material.hpp"
#include "materials/glass_material.hpp"
#include "acceleration_structures/brute_force_acceleration_structure.hpp"
#include "acceleration_structures/bvh_acceleration_structure.hpp"
#include "light_distributions/uniform_light_distribution.hpp"
#include "lights/const_diffuse_area_light.hpp"
#include "lights/const_infinity_area_light.hpp"
#include "lights/texture_infinity_area_light.hpp"
#include "textures/image_texture.hpp"
#include "core/assets.hpp"
#include "samplers/stratified_sampler.hpp"
#include "allocators/fixed_size_allocator.hpp"

void test_mis();
void test_dragon();
void test_shader_ball();

int main()
{
    test_dragon();
    return 0;
    fc::assets assets{};

    auto mesh{assets.get_mesh("xyz_dragon")};


    std::shared_ptr<fc::mesh_surface> surface3{new fc::mesh_surface{{{0.0, 0.0, 0.0}, {}, {0.03, 0.03, 0.03}}, mesh}};
    //std::shared_ptr<fc::surface> surface3{new fc::sphere_surface{{{0.0, 2.0, 0.0}}, 0.5}};
    std::shared_ptr<fc::plane_surface> surface2{new fc::plane_surface{{}, {10.0, 10.0}}};
    std::shared_ptr<fc::plane_surface> surface4{new fc::plane_surface{{{0.0, 8.0, 0.0}, {fc::math::deg_to_rad(180), 0.0, 0.0}}, {2.0, 2.0}}};

    std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{}};

    std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{surface4.get(), {1.0, 1.0, 1.0}, 25.0}};

    std::vector<fc::entity> entities{};
    entities.push_back({surface4, diffuse_material, al0});
    entities.push_back({surface2, diffuse_material, nullptr});
    entities.push_back({surface3, diffuse_material, nullptr});


    fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    fc::uniform_light_distribution_factory uldf{};
    fc::uniform_spatial_light_distribution_factory usldf{};

    //auto image{assets.get_image("env-loft-hall")};
    //std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    //std::shared_ptr<fc::texture_infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{}, texture, 1.0, image->get_resolution()}};
    //std::shared_ptr<fc::const_infinity_area_light> infinity_area_light{new fc::const_infinity_area_light{{0.5, 0.5, 0.5}, 0.3}};
    std::shared_ptr<fc::infinity_area_light> infinity_area_light{};
    std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf}};


    fc::random_sampler_1d_factory random_sampler_1d_factory{};
    fc::random_sampler_2d_factory random_sampler_2d_factory{};
    //fc::stratified_sampler_1d_factory random_sampler_1d_factory{true};
    //fc::stratified_sampler_2d_factory random_sampler_2d_factory{true};


    fc::perspective_camera_factory camera_factory{{{-4.0, 7.0, -4.0}, {fc::math::deg_to_rad(45.0), fc::math::deg_to_rad(45.0), 0.0}}, fc::math::deg_to_rad(45.0)};
    //fc::perspective_camera_factory camera_factory{{{2.367, 3.216, 6.485}, {0.0, fc::math::deg_to_rad(196.42), 0.0}}, fc::math::deg_to_rad(45.0)};
    //fc::perspective_camera_factory camera_factory{{{15, -3.9, 4.5}, {0.0, fc::math::deg_to_rad(-90), 0.0}}, fc::math::deg_to_rad(45.0)};
    
    std::shared_ptr<fc::forward_mis_integrator> integrator{new fc::forward_mis_integrator{2, false}};
   // std::shared_ptr<fc::forward_bsdf_integrator> integrator{new fc::forward_bsdf_integrator{2}};
    //std::shared_ptr<fc::backward_integrator> integrator{new fc::backward_integrator{2}};
    fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 16, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    renderer.run(512);
    renderer.export_image("normals");

    return 0;
}

void test_mis()
{
    std::shared_ptr<fc::surface> l0{new fc::sphere_surface{{{0.0, 0.0, 0.0}}, 0.05}};
    std::shared_ptr<fc::surface> l1{new fc::sphere_surface{{{0.0, 0.0, 3.0}}, 0.2}};
    std::shared_ptr<fc::surface> l2{new fc::sphere_surface{{{0.0, 0.0, 6.0}}, 0.5}};
    std::shared_ptr<fc::surface> l3{new fc::sphere_surface{{{0.0, 0.0, 9.0}}, 1.0}};

    std::shared_ptr<fc::surface> p0{new fc::plane_surface{{{0.0, -2.0, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-47.8)}}, {3.0, 12.0}}};
    std::shared_ptr<fc::surface> p1{new fc::plane_surface{{{3.0, -4.4, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-26.5)}}, {3.0, 12.0}}};
    std::shared_ptr<fc::surface> p2{new fc::plane_surface{{{6.5, -5.8, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-13.5)}}, {3.0, 12.0}}};

    std::shared_ptr<fc::surface> p3{new fc::plane_surface{{{-2.0, 0.0, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-90.0)}}, {20.0, 20.0}}};

    std::shared_ptr<fc::material> m0{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.1, 0.1}}};
    std::shared_ptr<fc::material> m1{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.2, 0.2}}};
    std::shared_ptr<fc::material> m2{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.4, 0.4}}};

    std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{}};

    std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{l0.get(), {1.0, 0.055998, 0.14469}, 125.0}};
    std::shared_ptr<fc::area_light> al1{new fc::const_diffuse_area_light{l1.get(), {0.458948, 0.206491, 1.0}, 20.0}};
    std::shared_ptr<fc::area_light> al2{new fc::const_diffuse_area_light{l2.get(), {0.209686, 0.782212, 1.0}, 4.0}};
    std::shared_ptr<fc::area_light> al3{new fc::const_diffuse_area_light{l3.get(), {1.0, 0.811331, 0.156019}, 1.0}};

    std::vector<fc::entity> entities{};
    entities.push_back({p0, m0, nullptr});
    entities.push_back({p1, m1, nullptr});
    entities.push_back({p2, m2, nullptr});

    entities.push_back({l0, diffuse_material, al0});
    entities.push_back({l1, diffuse_material, al1});
    entities.push_back({l2, diffuse_material, al2});
    entities.push_back({l3, diffuse_material, al3});

    entities.push_back({p3, diffuse_material, nullptr});

    fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    fc::uniform_light_distribution_factory uldf{};
    fc::uniform_spatial_light_distribution_factory usldf{};

    std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf}};

    fc::random_sampler_1d_factory random_sampler_1d_factory{};
    fc::random_sampler_2d_factory random_sampler_2d_factory{};
    fc::perspective_camera_factory camera_factory{{{15, -3.9, 4.5}, {0.0, fc::math::deg_to_rad(-90), 0.0}}, fc::math::deg_to_rad(45.0)};

    std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{2, false}};
    //std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{2}};
    fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 16, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    renderer.run(512);
    renderer.export_image("normals");
}

void test_dragon()
{
    fc::assets assets{};

    auto mesh{assets.get_mesh("xyz_dragon")};

    std::shared_ptr<fc::surface> dragon{new fc::mesh_surface{{{0.0, 0.0, 0.0}, {}, {0.03, 0.03, 0.03}}, mesh}};
    std::shared_ptr<fc::surface> sphere{new fc::sphere_surface{{{0.0, 1.0, 0.0}}, 1.0}};
    std::shared_ptr<fc::surface> sphere2{new fc::sphere_surface{{{0.0, 4.0, 0.0}}, 0.5}};
    std::shared_ptr<fc::surface> plane{new fc::plane_surface{{}, {100.0, 100.0}}};

    std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{}};
    std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{{0.8, 0.4, 0.2}, {0.3, 0.3}}};
    std::shared_ptr<fc::material> glass_material{new fc::glass_material{{1.0, 1.0, 1.0}}};

    std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{sphere2.get(), {1.0, 1.0, 1.0}, 25.0}};

    std::vector<fc::entity> entities{};
    entities.push_back({dragon, glass_material, nullptr});
    //entities.push_back({sphere, glass_material, nullptr});
    entities.push_back({sphere2, diffuse_material, al0});
    entities.push_back({plane, diffuse_material, nullptr});

    fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    fc::uniform_light_distribution_factory uldf{};
    fc::uniform_spatial_light_distribution_factory usldf{};

    auto image{assets.get_image("env-loft-hall")};
    std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    //std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, fc::math::deg_to_rad(45.0), 0.0}}, texture, 1.0, image->get_resolution()}};
    //std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::const_infinity_area_light{{0.5, 0.5, 0.5}, 1.0}};
    
    std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf}};

    //fc::random_sampler_1d_factory random_sampler_1d_factory{};
    //fc::random_sampler_2d_factory random_sampler_2d_factory{};
    fc::stratified_sampler_1d_factory random_sampler_1d_factory{true};
    fc::stratified_sampler_2d_factory random_sampler_2d_factory{true};
    fc::perspective_camera_factory camera_factory{{{0.0, 1.5, -10.0}}, fc::math::deg_to_rad(45.0)};

    //std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{10}};
    //std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{10, true}};
    std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{10, true}};

    fc::renderer renderer{{640, 360}, camera_factory, integrator, scene, 16, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    renderer.run(4096);
    renderer.export_image("normals");
}

void test_shader_ball()
{
    fc::assets assets{};

    auto outside{assets.get_mesh("shader_ball_outside")};
    auto inside{assets.get_mesh("shader_ball_inside")};

    std::shared_ptr<fc::surface> ball_outside{new fc::mesh_surface{{}, outside}};
    std::shared_ptr<fc::surface> ball_inside{new fc::mesh_surface{{}, inside}};
    std::shared_ptr<fc::surface> plane{new fc::plane_surface{{}, {100.0, 100.0}}};

    std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{}};
    std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.3, 0.3}}};

    std::vector<fc::entity> entities{};
    entities.push_back({ball_outside, mirror_material, nullptr});
    entities.push_back({ball_inside, diffuse_material, nullptr});
    entities.push_back({plane, diffuse_material, nullptr});

    fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    fc::uniform_light_distribution_factory uldf{};
    fc::uniform_spatial_light_distribution_factory usldf{};

    auto image{assets.get_image("env-loft-hall")};
    std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, fc::math::deg_to_rad(180.0), 0.0}}, texture, 1.0, image->get_resolution()}};

    std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf}};

    fc::stratified_sampler_1d_factory random_sampler_1d_factory{true};
    fc::stratified_sampler_2d_factory random_sampler_2d_factory{true};
    fc::perspective_camera_factory camera_factory{{{-0.981, 0.967, 0.849}, {fc::math::deg_to_rad(27.0), fc::math::deg_to_rad(130.0), 0.0}}, fc::math::deg_to_rad(45.0)};

    std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{5, false}};

    fc::renderer renderer{{256, 256}, camera_factory, integrator, scene, 16, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    renderer.run(10000);
    renderer.export_image("normals");
}

