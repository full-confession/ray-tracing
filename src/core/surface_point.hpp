#pragma once
#include "math.hpp"

namespace fc
{
    class standard_light;
    class material;
    class measurement;
    class surface;
    class medium;

    class surface_point
    {
    public:
        vector3 const& get_position() const { return position_; }
        vector3 const& get_normal() const { return normal_; }
        vector2 const& get_uv() const { return uv_; }

        vector3 const& get_shading_tangent() const { return shading_tangent_; }
        vector3 const& get_shading_normal() const { return shading_normal_; }
        vector3 const& get_shading_bitangent() const { return shading_bitangent_; }

        standard_light const* get_light() const { return light_; }
        material const* get_material() const { return material_; }
        measurement const* get_measurement() const { return measurement_; }
        surface const* get_surface() const { return surface_; }
        medium const* get_medium() const { return medium_; }

        void set_position(vector3 const& position) { position_ = position; }
        void set_normal(vector3 const& normal) { normal_ = normal; }
        void set_uv(vector2 const& uv) { uv_ = uv; }

        void set_shading_tangent(vector3 const& shading_tangent) { shading_tangent_ = shading_tangent; }
        void set_shading_normal(vector3 const& shading_normal) { shading_normal_ = shading_normal; }
        void set_shading_bitangent(vector3 const& shading_bitangent) { shading_bitangent_ = shading_bitangent; }

        void set_light(standard_light const* light) { light_ = light; }
        void set_material(material const* material) { material_ = material; }
        void set_measurement(measurement const* measurement) { measurement_ = measurement; }
        void set_surface(surface const* surface) { surface_ = surface; }
        void set_medium(medium const* medium) { medium_ = medium; }

        void* get_measurement_data() const { return measurement_data_; }
        void set_measurement_data(void* data) { measurement_data_ = data; }

        int get_priority() const { return priority_; }
        void set_priority(int priority) { priority_ = priority; }

        double get_ior() const { return ior_; }
        void set_ior(double ior) { ior_ = ior; }

    private:
        vector3 position_{};
        vector3 normal_{};
        vector2 uv_{};

        vector3 shading_tangent_{};
        vector3 shading_normal_{};
        vector3 shading_bitangent_{};

        standard_light const* light_{};
        material const* material_{};
        surface const* surface_{};
        medium const* medium_{};

        measurement const* measurement_{};
        void* measurement_data_{};

        int priority_{};
        double ior_{};
    };
}