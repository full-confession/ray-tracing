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
#include "materials/conductor_material.hpp"
#include "materials/plastic_material.hpp"
#include "materials/standard_material.hpp"
#include "materials/transmission_material.hpp"
#include "acceleration_structures/brute_force_acceleration_structure.hpp"
#include "acceleration_structures/bvh_acceleration_structure.hpp"
#include "light_distributions/uniform_light_distribution.hpp"
#include "lights/const_diffuse_area_light.hpp"
#include "lights/const_infinity_area_light.hpp"
#include "lights/texture_infinity_area_light.hpp"
#include "textures/image_texture.hpp"
#include "textures/checker_texture.hpp"
#include "textures/const_texture.hpp"
#include "core/assets.hpp"
#include "samplers/stratified_sampler.hpp"
#include "allocators/fixed_size_allocator.hpp"
#include "bxdfx_tester.hpp"

#include "scene_material_ball.hpp"

void test_mis();
void test_dragon();
void test();
void test_ball();
void test_mask();
void test_balls();
void test_normals();

int main()
{

    //test_mask();
    //test_ball();
    //test_dragon();
    //test_balls();
    //test_normals();
    //fc::scene_material_ball();
    //fc::scene_glass();
    //fc::scene_room();
    //fc::scene_normals();
    fc::scene_mask();
    return 0;
}

void test_mis()
{
    //std::shared_ptr<fc::surface> l0{new fc::sphere_surface{{{0.0, 0.0, 0.0}}, 0.05}};
    //std::shared_ptr<fc::surface> l1{new fc::sphere_surface{{{0.0, 0.0, 3.0}}, 0.2}};
    //std::shared_ptr<fc::surface> l2{new fc::sphere_surface{{{0.0, 0.0, 6.0}}, 0.5}};
    //std::shared_ptr<fc::surface> l3{new fc::sphere_surface{{{0.0, 0.0, 9.0}}, 1.0}};

    //std::shared_ptr<fc::surface> p0{new fc::plane_surface{{{0.0, -2.0, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-47.8)}}, {3.0, 12.0}}};
    //std::shared_ptr<fc::surface> p1{new fc::plane_surface{{{3.0, -4.4, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-26.5)}}, {3.0, 12.0}}};
    //std::shared_ptr<fc::surface> p2{new fc::plane_surface{{{6.5, -5.8, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-13.5)}}, {3.0, 12.0}}};

    //std::shared_ptr<fc::surface> p3{new fc::plane_surface{{{-2.0, 0.0, 4.5}, {0.0, 0.0, fc::math::deg_to_rad(-90.0)}}, {20.0, 20.0}}};

    //std::shared_ptr<fc::material> m0{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.1, 0.1}}};
    //std::shared_ptr<fc::material> m1{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.2, 0.2}}};
    //std::shared_ptr<fc::material> m2{new fc::mirror_material{{1.0, 1.0, 1.0}, {0.4, 0.4}}};

    //std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{nullptr}};

    //std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{l0.get(), {1.0, 0.055998, 0.14469}, 125.0}};
    //std::shared_ptr<fc::area_light> al1{new fc::const_diffuse_area_light{l1.get(), {0.458948, 0.206491, 1.0}, 20.0}};
    //std::shared_ptr<fc::area_light> al2{new fc::const_diffuse_area_light{l2.get(), {0.209686, 0.782212, 1.0}, 4.0}};
    //std::shared_ptr<fc::area_light> al3{new fc::const_diffuse_area_light{l3.get(), {1.0, 0.811331, 0.156019}, 1.0}};

    //std::vector<fc::entity> entities{};
    //entities.push_back({p0, m0, nullptr});
    //entities.push_back({p1, m1, nullptr});
    //entities.push_back({p2, m2, nullptr});

    //entities.push_back({l0, diffuse_material, al0});
    //entities.push_back({l1, diffuse_material, al1});
    //entities.push_back({l2, diffuse_material, al2});
    //entities.push_back({l3, diffuse_material, al3});

    //entities.push_back({p3, diffuse_material, nullptr});

    //fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    //fc::uniform_light_distribution_factory uldf{};
    //fc::uniform_spatial_light_distribution_factory usldf{};

    //std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf}};

    //fc::random_sampler_1d_factory random_sampler_1d_factory{};
    //fc::random_sampler_2d_factory random_sampler_2d_factory{};
    //fc::perspective_camera_factory camera_factory{{{15, -3.9, 4.5}, {0.0, fc::math::deg_to_rad(-90), 0.0}}, fc::math::deg_to_rad(45.0)};

    //std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{2, false}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{2}};
    //fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 16, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    //renderer.run(512);
    //renderer.export_image("normals");
}

void test_dragon()
{
    //fc::assets assets{};

    ////std::shared_ptr<fc::surface> dragon{new fc::mesh_surface{{{}, {}, {0.03, 0.03, 0.03}}, assets.get_mesh("xyz_dragon")}};
    //
    //std::shared_ptr<fc::surface> spoon{new fc::mesh_surface{{}, assets.get_mesh("spoon")}};
    //std::shared_ptr<fc::surface> plate{new fc::mesh_surface{{}, assets.get_mesh("plate")}};
    //std::shared_ptr<fc::surface> cup{new fc::mesh_surface{{}, assets.get_mesh("cup")}};
    //std::shared_ptr<fc::surface> tea{new fc::mesh_surface{{}, assets.get_mesh("tea")}};
    ////std::shared_ptr<fc::surface> table{new fc::mesh_surface{{}, assets.get_mesh("table")}};

    //std::shared_ptr<fc::surface> plane{new fc::plane_surface{{}, {5.0, 5.0}}};
    //std::shared_ptr<fc::surface> sphere{new fc::sphere_surface{{{0.0, 1.0, 0.0}}, 1.0}};
    ////std::shared_ptr<fc::surface> sphere1{new fc::sphere_surface{{{1.0, 1.0, 0.0}}, 1.0}};
    ////std::shared_ptr<fc::surface> sphere2{new fc::sphere_surface{{{ 3.0, 1.0, 6.0}}, 1.0}};
    ////std::shared_ptr<fc::surface> sphere3{new fc::sphere_surface{{{-11.0, 1.0, 25.0}}, 1.0}};
    ////std::shared_ptr<fc::surface> sphere4{new fc::sphere_surface{{{ 11.0, 1.0, 25.0}}, 1.0}};
    ////std::shared_ptr<fc::surface> sphere2{new fc::sphere_surface{{{0.0, 1.0, 2.0}}, 0.05}};

    ////auto image1{assets.get_image("old_planks_diff")};
    ////std::shared_ptr<fc::image_texture_2d_rgb> texture1{new fc::image_texture_2d_rgb{image1, fc::reconstruction_filter::bilinear, 4}};
    ////auto image2{assets.get_image("old_planks_rough")};
    ////std::shared_ptr<fc::image_texture_2d_r> texture2{new fc::image_texture_2d_r{image2, fc::reconstruction_filter::bilinear, 4}};

    //std::shared_ptr<fc::texture_2d_rgb> checker_texture{new fc::checker_texture_2d_rgb{{0.1, 0.1, 0.1}, {0.05, 0.05, 0.05}, 100.0}};
    //std::shared_ptr<fc::texture_2d_rgb> const_texture{new fc::const_texture_2d_rgb{{0.8, 0.8, 0.8}}};
    //std::shared_ptr<fc::texture_2d_rgb> const1_texture{new fc::const_texture_2d_rgb{{1.0, 1.0, 1.0}}};

    //std::shared_ptr<fc::texture_2d_rgb> const_texture2{new fc::const_texture_2d_rgb{{0.8, 0.4, 0.2}}};
    //std::shared_ptr<fc::material> diffuse_material2{new fc::diffuse_material{const_texture2}};

    //std::shared_ptr<fc::texture_2d_rgb> const_texture3{new fc::const_texture_2d_rgb{{0.6, 0.7, 0.8}}};
    //std::shared_ptr<fc::material> diffuse_material3{new fc::diffuse_material{const_texture3}};

    //std::shared_ptr<fc::texture_2d_rgb> gold_ior{new fc::const_texture_2d_rgb{{0.183, 0.422, 1.373}}};
    //std::shared_ptr<fc::texture_2d_rgb> gold_k{new fc::const_texture_2d_rgb{{4.0, 1.6, 1.15}}};
    //std::shared_ptr<fc::texture_2d_r> gold_roughness{new fc::const_texture_2d_r{0.5}};
    //std::shared_ptr<fc::texture_2d_r> glass_roughness{new fc::const_texture_2d_r{0.2}};

    //std::shared_ptr<fc::texture_2d_rg> roughness_a{new fc::const_texture_2d_rg{{0.3, 0.3}}};
    //std::shared_ptr<fc::texture_2d_rg> roughness0{new fc::const_texture_2d_rg{{0.0, 0.0}}};

    //std::shared_ptr<fc::texture_2d_rg> mirror_roughness{new fc::const_texture_2d_rg{{0.2, 0.2}}};

    //std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{checker_texture}};
    //std::shared_ptr<fc::material> diffuse_material_dragon{new fc::diffuse_material{const_texture}};
    ////std::shared_ptr<fc::material> gold_material{new fc::conductor_material{gold_ior, gold_k, gold_roughness}};
    ////std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{const_texture, const1_texture, glass_roughness, 1.45}};
    //
    //
    //std::shared_ptr<fc::material> cup_material{new fc::glass_material{const1_texture, const1_texture, roughness0}};
    //std::shared_ptr<fc::material> tea_material{new fc::glass_material{const1_texture, const1_texture, roughness0}};

    //std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{const_texture, roughness0}};
    //std::shared_ptr<fc::material> transmission_material{new fc::transmission_material{const1_texture, roughness_a}};


    //std::shared_ptr<fc::texture_2d_r> ior{new fc::const_texture_2d_r{1.5}};
    //std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{
    //    const_texture3,
    //    const1_texture,
    //    roughness_a,
    //    ior
    //}};

    ///*std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{{0.2, 0.4, 0.8}, {0.4, 0.4}}};
    //std::shared_ptr<fc::material> glass_material{new fc::glass_material{{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 1.45}};
    //std::shared_ptr<fc::material> conductor_material{new fc::conductor_material{{0.183, 0.422, 1.373}, {4.0, 1.6, 1.150}, {0.5, 0.5}}};
    //std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{{0.2, 0.4, 0.8}, {1.0, 1.0, 1.0}, 1.45, {0.15, 0.15}}};
    //std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{plane2.get(), {1.0, 1.0, 1.0}, 15.0}};
    //std::shared_ptr<fc::area_light> al1{new fc::const_diffuse_area_light{plane3.get(), {1.0, 1.0, 1.0}, 15.0}};*/


    //std::shared_ptr<fc::medium> glass_medium{new fc::uniform_medium{2, 1.5, {1.0, 1.0, 1.0}, 25.0}};
    //std::shared_ptr<fc::medium> tea_medium{new fc::uniform_medium{1, 1.333, {0.2, 0.8, 0.8}, 150.0}};
    ////std::shared_ptr<fc::medium> glass_medium{new fc::uniform_medium{2, 1.5, {1.0, 1.0, 1.0}, 0.0}};

    ////std::shared_ptr<fc::area_light> al1{new fc::const_diffuse_area_light{sphere2.get(), {1.0, 1.0, 1.0}, 100.0}};

    //std::vector<fc::entity> entities{};
    ////entities.push_back({dragon, plastic_material, nullptr});
    ////entities.push_back({spoon, mirror_material, nullptr});
    ////entities.push_back({cup, cup_material, nullptr, glass_medium});
    ////entities.push_back({tea, tea_material, nullptr, tea_medium});


    //entities.push_back({cup, cup_material, nullptr, glass_medium});
    //entities.push_back({plane, diffuse_material, nullptr});
    //entities.push_back({plate, diffuse_material_dragon, nullptr});
    ////entities.push_back({sphere, tea_material, nullptr, glass_medium});
    ////entities.push_back({sphere1, glass_material, nullptr, nullptr, 1, 1.2});
    ////entities.push_back({sphere2, diffuse_material2, nullptr});
    ////entities.push_back({sphere3, diffuse_material3, nullptr});
    ////entities.push_back({sphere4, diffuse_material3, nullptr});
    ////entities.push_back({sphere2, diffuse_material, al1});

    //auto image{assets.get_image("env-loft-hall")};
    //std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    //std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, fc::math::deg_to_rad(45.0), 0.0}}, texture, 1.0, image->get_resolution()}};
    ////std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::const_infinity_area_light{{1.0, 1.0, 1.0}, 1.0}};

    //fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    //fc::uniform_light_distribution_factory uldf{};
    //fc::uniform_spatial_light_distribution_factory usldf{};

    //std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf}};
    ////fc::random_sampler sampler{4*4};
    //fc::stratified_sampler sampler{32*32};

    ////fc::perspective_camera_factory camera_factory{{{-4.421, 5.753, -5.448}, {fc::math::deg_to_rad(34.309), fc::math::deg_to_rad(35.237), 0.0}}, fc::math::deg_to_rad(27.0)/*, 0.1, 7.5*/};
    //fc::perspective_camera_factory camera_factory{{{-0.202, 0.202, -0.158}, {fc::math::deg_to_rad(34.549), fc::math::deg_to_rad(51.566), 0.0}}, fc::math::deg_to_rad(30.0)};


    ////std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{10}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{10, true}};
    //std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{10, true}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{10}};

    //fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 15, sampler};
    //renderer.run();
    ////renderer.run_pixel({213, 315});
    //renderer.export_image("normals");
}

void test_ball()
{
   // fc::assets assets{};

   // std::shared_ptr<fc::surface> sphere{new fc::sphere_surface{{{-0.5, -0.5, 5.0}}, 0.4}};
   // std::shared_ptr<fc::surface> sphere1{new fc::sphere_surface{{{ 0.5, -0.5, 5.0}}, 0.4}};
   // std::shared_ptr<fc::surface> sphere2{new fc::sphere_surface{{{-0.5,  0.5, 5.0}}, 0.4}};
   // std::shared_ptr<fc::surface> sphere3{new fc::sphere_surface{{{ 0.5,  0.5, 5.0}}, 0.4}};


   // std::shared_ptr<fc::texture_2d_rgb> const1_texture{new fc::const_texture_2d_rgb{{1.0, 1.0, 1.0}}};
   // std::shared_ptr<fc::texture_2d_rgb> const08_texture{new fc::const_texture_2d_rgb{{0.8, 0.8, 0.8}}};
   // std::shared_ptr<fc::texture_2d_rgb> const_texture{new fc::const_texture_2d_rgb{{0.2, 0.4, 0.8}}};
   // std::shared_ptr<fc::texture_2d_r> glass_roughness{new fc::const_texture_2d_r{0.01}};
   // std::shared_ptr<fc::texture_2d_r> glass_roughness1{new fc::const_texture_2d_r{0.1}};
   // std::shared_ptr<fc::texture_2d_r> glass_roughness2{new fc::const_texture_2d_r{0.5}};
   // std::shared_ptr<fc::texture_2d_r> glass_roughness3{new fc::const_texture_2d_r{1.0}};

   // std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{const_texture}};
   // std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{const1_texture, glass_roughness}};
   // std::shared_ptr<fc::material> glass_material{new fc::glass_material{const_texture, const1_texture, glass_roughness, 1.45}};
   // //std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{const_texture, const1_texture, glass_roughness, 1.45}};

   // std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{const_texture, const1_texture, glass_roughness, 1.45}};
   // std::shared_ptr<fc::material> plastic_material1{new fc::plastic_material{const_texture, const1_texture, glass_roughness1, 1.45}};
   // std::shared_ptr<fc::material> plastic_material2{new fc::plastic_material{const_texture, const1_texture, glass_roughness2, 1.45}};
   // std::shared_ptr<fc::material> plastic_material3{new fc::plastic_material{const_texture, const1_texture, glass_roughness3, 1.45}};

   // std::vector<fc::entity> entities{};
   // entities.push_back({sphere, diffuse_material, nullptr});
   // entities.push_back({sphere1, diffuse_material, nullptr});
   // entities.push_back({sphere2, diffuse_material, nullptr});
   // entities.push_back({sphere3, diffuse_material, nullptr});

   // auto image{assets.get_image("evening_meadow_4k")};
   // std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
   // std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, fc::math::deg_to_rad(5.0), 0.0}}, texture, 1.0, image->get_resolution()}};
   // //std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::const_infinity_area_light{{1.0, 1.0, 1.0}, 0.5}};

   // fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
   // fc::uniform_light_distribution_factory uldf{};
   // fc::uniform_spatial_light_distribution_factory usldf{};

   // std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf}};
   // fc::random_sampler sampler{512};

   // fc::perspective_camera_factory camera_factory{{}, fc::math::deg_to_rad(27.0), 0.1, 5.0};

   // //std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{3}};
   // //std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{3, true}};
   //// std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{3, true}};
   // std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{3}};

   // fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 15, sampler};
   // renderer.run(64);
   // renderer.export_image("normals");
}

void test_mask()
{
    //fc::assets assets{};
    //std::shared_ptr<fc::surface> mask{new fc::mesh_surface{{}, assets.get_mesh("mask")}};

    ////std::shared_ptr<fc::texture_2d_rgb> gold_ior{new fc::const_texture_2d_rgb{{0.183, 0.422, 1.373}}};
    ////std::shared_ptr<fc::texture_2d_rgb> gold_k{new fc::const_texture_2d_rgb{{4.0, 1.6, 1.15}}};

    //std::shared_ptr<fc::texture_2d_rgb> albedo{new fc::image_texture_2d_rgb{assets.get_image("mask-basecolor"), fc::reconstruction_filter::bilinear, 1}};
    //std::shared_ptr<fc::texture_2d_r> metalness{new fc::image_texture_2d_r{assets.get_image("mask-metalness"), fc::reconstruction_filter::bilinear, 1}};
    //std::shared_ptr<fc::texture_2d_r> roughness{new fc::image_texture_2d_r{assets.get_image("mask-roughness"), fc::reconstruction_filter::bilinear, 1}};
    //std::shared_ptr<fc::texture_2d_rgb> normal{new fc::image_texture_2d_rgb{assets.get_image("mask-normal"), fc::reconstruction_filter::bilinear, 1}};

    //std::shared_ptr<fc::texture_2d_rgb> const_texture2{new fc::const_texture_2d_rgb{{0.8, 0.4, 0.2}}};
    //std::shared_ptr<fc::material> diffuse_material2{new fc::diffuse_material2{const_texture2, normal}};
    //
    ////std::shared_ptr<fc::material> material{new fc::standard_material{albedo, metalness, roughness, 1.45}};

    //std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{
    //    const_texture2,
    //    std::make_shared<fc::const_texture_2d_rg>(fc::vector2{0.4, 0.4})
    //}};

    //std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{
    //    std::make_shared<fc::const_texture_2d_rgb>(fc::vector3{0.8, 0.4, 0.2}),
    //    std::make_shared<fc::const_texture_2d_rgb>(fc::vector3{1.0, 1.0, 1.0}),
    //    std::make_shared<fc::const_texture_2d_rg>(fc::vector2{0.3, 0.3}),
    //    std::make_shared<fc::const_texture_2d_r>(1.45)
    //}};

    //std::shared_ptr<fc::material> mirror_material2{new fc::mirror_material2{
    //    std::shared_ptr<fc::texture_2d_rgb>(new fc::const_texture_2d_rgb{{0.8, 0.4, 0.2}}),
    //    std::shared_ptr<fc::texture_2d_rg>(new fc::const_texture_2d_rg{{0.2, 0.2}}),
    //    std::shared_ptr<fc::texture_2d_rgb>(normal)
    //}};

    //std::shared_ptr<fc::material> standard_material{new fc::standard_material{
    //    albedo,
    //    metalness,
    //    roughness,
    //    std::make_shared<fc::const_texture_2d_r>(1.45),
    //    normal
    //}};

    //std::vector<fc::entity> entities{};
    //entities.push_back({mask, standard_material, nullptr});

    //auto image{assets.get_image("env-loft-hall")};
    //std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    //std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, 0.0, 0.0}}, texture, 1.0, image->get_resolution()}};
    ////std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::const_infinity_area_light{{1.0, 1.0, 1.0}, 1.0}};

    //fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    //fc::uniform_light_distribution_factory uldf{};
    //fc::uniform_spatial_light_distribution_factory usldf{};

    //std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf}};

    ////fc::perspective_camera_factory camera_factory{{{-4.421, 5.753, -5.448}, {fc::math::deg_to_rad(34.309), fc::math::deg_to_rad(35.237), 0.0}}, fc::math::deg_to_rad(27.0), 0.1, 7.5};
    //fc::perspective_camera_factory camera_factory{{{2.367, 3.216, 6.485}, {0.0, fc::math::deg_to_rad(196.42), 0.0}}, fc::math::deg_to_rad(45.0), 0.05, 6.0};
    ////fc::perspective_camera_factory camera_factory{{{-0.114, 3.216, 1.439}, {0.0, fc::math::deg_to_rad(203.5), 0.0}}, fc::math::deg_to_rad(30.0)};

    ////fc::pmj02_sampler5 sampler{0, 256};
    ////fc::random_sampler sampler{32*32};
    //fc::stratified_sampler sampler{64 * 64};

    ////std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{10}};
    //std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{10, true}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{10, true}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{2}};

    //fc::renderer renderer{{600, 900}, camera_factory, integrator, scene, 15, sampler};
    //renderer.run();
    ////renderer.run_pixel({341, 352});
    //renderer.export_image("mask");
}

void test()
{
    //fc::assets assets{};

    //std::shared_ptr<fc::mesh_surface> a{new fc::mesh_surface{{}, assets.get_mesh("test_a")}};
    //std::shared_ptr<fc::mesh_surface> b{new fc::mesh_surface{{}, assets.get_mesh("test_b")}};
    //std::shared_ptr<fc::mesh_surface> c{new fc::mesh_surface{{}, assets.get_mesh("test_c")}};
    //std::shared_ptr<fc::mesh_surface> d{new fc::mesh_surface{{}, assets.get_mesh("test_d")}};
    //std::shared_ptr<fc::mesh_surface> e{new fc::mesh_surface{{}, assets.get_mesh("test_e")}};
    //std::shared_ptr<fc::mesh_surface> f{new fc::mesh_surface{{}, assets.get_mesh("test_f")}};
    //std::shared_ptr<fc::mesh_surface> g{new fc::mesh_surface{{}, assets.get_mesh("test_g")}};
    ////std::shared_ptr<fc::mesh_surface> g2{new fc::mesh_surface{{}, assets.get_mesh("test_omega2")}};
    //std::shared_ptr<fc::mesh_surface> h{new fc::mesh_surface{{}, assets.get_mesh("test_h")}};

    //std::shared_ptr<fc::mesh_surface> ar{new fc::mesh_surface{{}, assets.get_mesh("test_ar")}};
    //std::shared_ptr<fc::mesh_surface> br{new fc::mesh_surface{{}, assets.get_mesh("test_br")}};
    //std::shared_ptr<fc::mesh_surface> cr{new fc::mesh_surface{{}, assets.get_mesh("test_cr")}};
    //std::shared_ptr<fc::mesh_surface> ab{new fc::mesh_surface{{}, assets.get_mesh("test_ab")}};
    //std::shared_ptr<fc::mesh_surface> bb{new fc::mesh_surface{{}, assets.get_mesh("test_bb")}};
    //std::shared_ptr<fc::mesh_surface> cb{new fc::mesh_surface{{}, assets.get_mesh("test_cb")}};
    //std::shared_ptr<fc::mesh_surface> r{new fc::mesh_surface{{}, assets.get_mesh("test_r")}};

    //std::shared_ptr<fc::mesh_surface> lucy{new fc::mesh_surface{{}, assets.get_mesh("test_lucy")}};
    //std::shared_ptr<fc::mesh_surface> dragon{new fc::mesh_surface{{}, assets.get_mesh("test_dragon")}};
    //std::shared_ptr<fc::mesh_surface> box{new fc::mesh_surface{{}, assets.get_mesh("test_box")}};

    //std::shared_ptr<fc::mesh_surface> pa{new fc::mesh_surface{{}, assets.get_mesh("test_pa")}};
    //std::shared_ptr<fc::mesh_surface> pb{new fc::mesh_surface{{}, assets.get_mesh("test_pb")}};

    //std::shared_ptr<fc::area_light> top_ligths{new fc::const_diffuse_area_light{e.get(), {1.0, 1.0, 1.0}, 200.0}};
    //std::shared_ptr<fc::area_light> wall_ligths{new fc::const_diffuse_area_light{h.get(), {1.0, 1.0, 1.0}, 50.0}};

    //std::shared_ptr<fc::texture_2d_rgb> const_texture{new fc::const_texture_2d_rgb{{0.8, 0.8, 0.8}}};
    //std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{const_texture}};

    //std::shared_ptr<fc::texture_2d_rgb> const_texture2{new fc::const_texture_2d_rgb{{0.1, 0.1, 0.1}}};

    //std::vector<fc::entity> entities{};
    //entities.push_back({a, std::shared_ptr<fc::material>{new fc::plastic_material{{0.066, 0.066, 0.066}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    ////entities.push_back({a, std::shared_ptr<fc::material>{new fc::diffuse_material{const_texture2}}});
    //entities.push_back({b, std::shared_ptr<fc::material>{new fc::plastic_material{{0.8, 0.8, 0.8}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({c, std::shared_ptr<fc::material>{new fc::plastic_material{{0.8, 0.8, 0.8}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({d, std::shared_ptr<fc::material>{new fc::plastic_material{{0.8, 0.8, 0.8}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({e, diffuse_material, top_ligths});
    //entities.push_back({f, std::shared_ptr<fc::material>{new fc::plastic_material{{0.8, 0.8, 0.8}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({g, std::shared_ptr<fc::material>{new fc::mirror_material{{0.8, 0.166, 0.179}, {0.5, 0.5}}}});
    ////entities.push_back({g2, std::shared_ptr<fc::material>{new fc::glass_material{{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 1.45}}});
    //entities.push_back({h, diffuse_material, wall_ligths});

    //entities.push_back({ar, std::shared_ptr<fc::material>{new fc::plastic_material{{1.0, 0.2, 0.2}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({br, std::shared_ptr<fc::material>{new fc::plastic_material{{1.0, 0.3, 0.3}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({cr, std::shared_ptr<fc::material>{new fc::plastic_material{{1.0, 0.444, 0.419}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({ab, std::shared_ptr<fc::material>{new fc::plastic_material{{0.1861, 0.234, 1.0}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({bb, std::shared_ptr<fc::material>{new fc::plastic_material{{0.4095, 0.410, 1.0}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({cb, std::shared_ptr<fc::material>{new fc::plastic_material{{0.6702, 0.670, 1.0}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});
    //entities.push_back({r, std::shared_ptr<fc::material>{new fc::mirror_material{{0.8, 0.8, 0.8}, {0.1, 0.1}}}});

    //entities.push_back({lucy, std::shared_ptr<fc::material>{new fc::mirror_material{{0.8, 0.154, 0.013}, {0.3, 0.3}}}});
    //entities.push_back({dragon, std::shared_ptr<fc::material>{new fc::mirror_material{{0.8, 0.154, 0.013}, {0.3, 0.3}}}});
    //entities.push_back({box, std::shared_ptr<fc::material>{new fc::plastic_material{{0.0024, 0.0024, 0.0024}, {1.0, 1.0, 1.0}, 1.45, {0.5, 0.5}}}});

    //entities.push_back({pa, std::shared_ptr<fc::material>{new fc::mirror_material{{0.168, 0.318, 0.8}, {0.1, 0.1}}}});
    //entities.push_back({pb, std::shared_ptr<fc::material>{new fc::mirror_material{{0.229, 0.238, 0.8}, {0.1, 0.1}}}});

    //fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    //fc::uniform_light_distribution_factory uldf{};
    //fc::uniform_spatial_light_distribution_factory usldf{};
    ////fc::random_sampler_1d_factory random_sampler_1d_factory{};
    ////fc::random_sampler_2d_factory random_sampler_2d_factory{};
    //fc::stratified_sampler_1d_factory random_sampler_1d_factory{true};
    //fc::stratified_sampler_2d_factory random_sampler_2d_factory{true};
    //std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf}};

    //fc::perspective_camera_factory camera_factory{{{2.961, 0.0836, -3.55377}, {0.0, fc::math::deg_to_rad(-46), 0.0}}, fc::math::deg_to_rad(27.0)};

    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{10, false}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{10}};
    //std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{10, false}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{10}};
    //fc::renderer renderer{{640, 360}, camera_factory, integrator, scene, 15, random_sampler_1d_factory, random_sampler_2d_factory, 0};
    //renderer.run(1000);
    //renderer.export_image("normals");
}

void test_balls()
{
    //std::shared_ptr<fc::surface> s0{new fc::sphere_surface{{{0.0, 1.0, 0.0}}, 1.0}};
    //std::shared_ptr<fc::surface> s1{new fc::sphere_surface{{{3.16003, 1.0, -2.32747}}, 1.0}};
    //std::shared_ptr<fc::surface> s2{new fc::sphere_surface{{{-2.23849, 1.0, -3.68311}}, 1.0}};
    //std::shared_ptr<fc::surface> s3{new fc::sphere_surface{{{-7.87424, 1.0, -1.24736}}, 1.0}};
    //std::shared_ptr<fc::surface> s4{new fc::sphere_surface{{{-5.9499, 1.0, 2.40197}}, 1.0}};
    //std::shared_ptr<fc::surface> s5{new fc::sphere_surface{{{-0.341524, 1.0, 4.28785}}, 1.0}};
    //std::shared_ptr<fc::surface> s6{new fc::sphere_surface{{{2.99684, 1.0, 5.79599}}, 1.0}};

    //std::shared_ptr<fc::surface> l0{new fc::sphere_surface{{{-0.374679, 0.5, -2.60628}}, 0.5}};
    //std::shared_ptr<fc::surface> l1{new fc::sphere_surface{{{-4.03987, 0.5, 0.045641}}, 0.5}};
    //std::shared_ptr<fc::surface> l2{new fc::sphere_surface{{{-3.09873, 0.5, 4.31356}}, 0.5}};
    //std::shared_ptr<fc::surface> l3{new fc::sphere_surface{{{1.82126, 0.5, 0.928827}}, 0.5}};

    //std::shared_ptr<fc::surface> plane{new fc::plane_surface{{}, {25.0, 25.0}}};

    //std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{l0.get(), {1.0, 0.176762, 0.03322}, 5.0}};
    //std::shared_ptr<fc::area_light> al1{new fc::const_diffuse_area_light{l1.get(), {0.0, 1.0, 0.132231}, 5.0}};
    //std::shared_ptr<fc::area_light> al2{new fc::const_diffuse_area_light{l2.get(), {1.0, 0.0, 0.057055}, 5.0}};
    //std::shared_ptr<fc::area_light> al3{new fc::const_diffuse_area_light{l3.get(), {0.0, 0.311272, 1.0}, 5.0}};

    //std::shared_ptr<fc::texture_2d_rgb> const1_texture{new fc::const_texture_2d_rgb{{1.0, 1.0, 1.0}}};
    //std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{const1_texture}};

    //std::shared_ptr<fc::texture_2d_r> glass_roughness{new fc::const_texture_2d_r{0.3}};
    //std::shared_ptr<fc::material> glass_material{new fc::glass_material{const1_texture, const1_texture, glass_roughness, 1.45}};

    //std::vector<fc::entity> entities{};
    //entities.push_back({s0, glass_material, nullptr});
    //entities.push_back({s1, diffuse_material, nullptr});
    //entities.push_back({s2, diffuse_material, nullptr});
    //entities.push_back({s3, diffuse_material, nullptr});
    //entities.push_back({s4, diffuse_material, nullptr});
    //entities.push_back({s5, diffuse_material, nullptr});
    //entities.push_back({s6, diffuse_material, nullptr});

    //entities.push_back({l0, diffuse_material, al0});
    //entities.push_back({l1, diffuse_material, al1});
    //entities.push_back({l2, diffuse_material, al2});
    //entities.push_back({l3, diffuse_material, al3});

    //entities.push_back({plane, diffuse_material, nullptr});

    //fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    //fc::uniform_light_distribution_factory uldf{};
    //fc::uniform_spatial_light_distribution_factory usldf{};

    //std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf}};

    //fc::stratified_sampler_1d_factory sampler_1d_factory{true};
    //fc::stratified_sampler_2d_factory sampler_2d_factory{true};

    //fc::perspective_camera_factory camera_factory{{{1.987698, 6.547128, -5.249698}, {fc::math::deg_to_rad(46.925), fc::math::deg_to_rad(-29.667), 0.0}}, fc::math::deg_to_rad(45.0)};


    //std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{10}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{10, true}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{10, true}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{10}};

    //fc::renderer renderer{{800, 450}, camera_factory, integrator, scene, 15, sampler_1d_factory, sampler_2d_factory, 0};
    //renderer.run(1000);
    //renderer.export_image("normals");
}

void test_normals()
{
    //std::shared_ptr<fc::surface> a{new fc::plane_surface{{}, {10.0, 10.0}}};
    //std::shared_ptr<fc::surface> b{new fc::plane_surface{{{0.0, 1.0, 6.0}, {fc::math::deg_to_rad(-90), 0.0, 0.0}}, {10.0, 2.0}}};

    //fc::assets assets{};
    //auto image{assets.get_image("wallpaper-normal")};

    //std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material2{
    //    std::shared_ptr<fc::texture_2d_rgb>(new fc::const_texture_2d_rgb{{0.8, 0.8, 0.8}}),
    //    std::shared_ptr<fc::texture_2d_rgb>(new fc::image_texture_2d_rgb(image, fc::reconstruction_filter::bilinear, 1))
    //}};

    //std::shared_ptr<fc::material> mirror_material{new fc::mirror_material2{
    //    std::shared_ptr<fc::texture_2d_rgb>(new fc::const_texture_2d_rgb{{0.8, 0.8, 0.8}}),
    //    std::shared_ptr<fc::texture_2d_rg>(new fc::const_texture_2d_rg{{0.4, 0.4}}),
    //    std::shared_ptr<fc::texture_2d_rgb>(new fc::image_texture_2d_rgb(image, fc::reconstruction_filter::bilinear, 1))
    //}};

    //std::shared_ptr<fc::area_light> area_light{new fc::const_diffuse_area_light{b.get(), {1.0, 1.0, 1.0}, 1.0}};

    //std::vector<fc::entity> entities{};
    //entities.push_back({a, mirror_material});
    //entities.push_back({b, diffuse_material, area_light});


    //fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    //fc::uniform_light_distribution_factory uldf{};
    //fc::uniform_spatial_light_distribution_factory usldf{};

    ////auto aa{assets.get_image("env-loft-hall")};
    ////std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{aa, fc::reconstruction_filter::bilinear, 4}};
    ////std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, 0.0, 0.0}}, texture, 1.0, image->get_resolution()}};

    //std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), nullptr, acceleration_structure_factory, uldf, usldf}};
    //fc::stratified_sampler sampler{32 * 32};

   
    //fc::perspective_camera_factory camera_factory{{{5.72, 2.36, 0.31}, {fc::math::deg_to_rad(27.33), fc::math::deg_to_rad(303.69), 0.0}}, fc::math::deg_to_rad(30.0)};


    ////std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{2}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{2, true}};
    //std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{2, true}};
    ////std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{2}};

    //fc::renderer renderer{{512, 512}, camera_factory, integrator, scene, 15, sampler};
    //renderer.run();
    ////renderer.run_pixel({181, 278});
    //renderer.export_image("normals2");
}