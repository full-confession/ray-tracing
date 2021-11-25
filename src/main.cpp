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

void test_mis();
void test_dragon();
void test();
int main()
{
    test_dragon();
    //test_dragon();
    return 0;
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
    fc::assets assets{};

    //std::shared_ptr<fc::surface> dragon{new fc::mesh_surface{{{}, {}, {0.03, 0.03, 0.03}}, assets.get_mesh("xyz_dragon")}};
    //std::shared_ptr<fc::surface> floor{new fc::mesh_surface{{{1.0, 0.05, 1.0}}, assets.get_mesh("floor")}};
    std::shared_ptr<fc::surface> plane{new fc::plane_surface{{}, {25.0, 25.0}}};
    std::shared_ptr<fc::surface> sphere{new fc::sphere_surface{{{0.0, 1.0, 0.0}}, 1.0}};

    auto image1{assets.get_image("old_planks_diff")};
    std::shared_ptr<fc::image_texture_2d_rgb> texture1{new fc::image_texture_2d_rgb{image1, fc::reconstruction_filter::bilinear, 4}};
    auto image2{assets.get_image("old_planks_rough")};
    std::shared_ptr<fc::image_texture_2d_r> texture2{new fc::image_texture_2d_r{image2, fc::reconstruction_filter::bilinear, 4}};

    std::shared_ptr<fc::texture_2d_rgb> checker_texture{new fc::checker_texture_2d_rgb{{0.8, 0.8, 0.8}, {0.6, 0.6, 0.6}, 10.0}};
    std::shared_ptr<fc::texture_2d_rgb> const_texture{new fc::const_texture_2d_rgb{{0.2, 0.4, 0.8}}};
    std::shared_ptr<fc::texture_2d_rgb> const1_texture{new fc::const_texture_2d_rgb{{1.0, 1.0, 1.0}}};

    std::shared_ptr<fc::texture_2d_rgb> gold_ior{new fc::const_texture_2d_rgb{{0.183, 0.422, 1.373}}};
    std::shared_ptr<fc::texture_2d_rgb> gold_k{new fc::const_texture_2d_rgb{{4.0, 1.6, 1.15}}};
    std::shared_ptr<fc::texture_2d_r> gold_roughness{new fc::const_texture_2d_r{0.5}};

    std::shared_ptr<fc::material> diffuse_material{new fc::diffuse_material{texture1}};
    std::shared_ptr<fc::material> diffuse_material_dragon{new fc::diffuse_material{const_texture}};
    std::shared_ptr<fc::material> gold_material{new fc::conductor_material{gold_ior, gold_k, gold_roughness}};
    std::shared_ptr<fc::material> floor_material{new fc::plastic_material{texture1, const1_texture, texture2, 1.3}};
    std::shared_ptr<fc::material> glass_material{new fc::glass_material{const1_texture, const1_texture, 1.45}};

    /*std::shared_ptr<fc::material> mirror_material{new fc::mirror_material{{0.2, 0.4, 0.8}, {0.4, 0.4}}};
    std::shared_ptr<fc::material> glass_material{new fc::glass_material{{1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, 1.45}};
    std::shared_ptr<fc::material> conductor_material{new fc::conductor_material{{0.183, 0.422, 1.373}, {4.0, 1.6, 1.150}, {0.5, 0.5}}};
    std::shared_ptr<fc::material> plastic_material{new fc::plastic_material{{0.2, 0.4, 0.8}, {1.0, 1.0, 1.0}, 1.45, {0.15, 0.15}}};
    std::shared_ptr<fc::area_light> al0{new fc::const_diffuse_area_light{plane2.get(), {1.0, 1.0, 1.0}, 15.0}};
    std::shared_ptr<fc::area_light> al1{new fc::const_diffuse_area_light{plane3.get(), {1.0, 1.0, 1.0}, 15.0}};*/

    //std::shared_ptr<fc::medium> uniform_medium{new fc::uniform_medium{{0.8, 0.4, 0.2}, 10.0}};

    std::vector<fc::entity> entities{};
    //entities.push_back({dragon, gold_material, nullptr});
    entities.push_back({plane, floor_material, nullptr});
    entities.push_back({sphere, glass_material, nullptr});

    auto image{assets.get_image("env-loft-hall")};
    std::shared_ptr<fc::image_texture_2d_rgb> texture{new fc::image_texture_2d_rgb{image, fc::reconstruction_filter::bilinear, 4}};
    std::shared_ptr<fc::infinity_area_light> infinity_area_light{new fc::texture_infinity_area_light{{{}, {0.0, fc::math::deg_to_rad(45.0), 0.0}}, texture, 1.0, image->get_resolution()}};


    fc::bvh_acceleration_structure_factory acceleration_structure_factory{};
    fc::uniform_light_distribution_factory uldf{};
    fc::uniform_spatial_light_distribution_factory usldf{};

    std::shared_ptr<fc::entity_scene> scene{new fc::entity_scene{std::move(entities), infinity_area_light, acceleration_structure_factory, uldf, usldf}};

    fc::stratified_sampler_1d_factory sampler_1d_factory{true};
    fc::stratified_sampler_2d_factory sampler_2d_factory{true};


    //fc::perspective_camera_factory camera_factory{{{-4.421, 5.753, -5.448}, {fc::math::deg_to_rad(34.309), fc::math::deg_to_rad(35.237), 0.0}}, fc::math::deg_to_rad(27.0), 0.1, 7.5};
    fc::perspective_camera_factory camera_factory{{{0.0, 1.5, -9}}, fc::math::deg_to_rad(27.0)};


    //std::shared_ptr<fc::integrator> integrator{new fc::backward_integrator{10}};
    std::shared_ptr<fc::integrator> integrator{new fc::forward_mis_integrator{10, true}};
    //std::shared_ptr<fc::integrator> integrator{new fc::bidirectional_integrator{10, true}};
    //std::shared_ptr<fc::integrator> integrator{new fc::forward_bsdf_integrator{10}};

    fc::renderer renderer{{800, 450}, camera_factory, integrator, scene, 15, sampler_1d_factory, sampler_2d_factory, 0};
    renderer.run(64);
    renderer.export_image("normals");
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

