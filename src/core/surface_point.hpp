#pragma once
#include "math.hpp"

namespace fc
{
    class standard_light;
    class material;
    class measurement;
    class surface;

    class surface_point
    {
    public:
        vector3 const& get_position() const { return position_; }
        vector3 const& get_normal() const { return normal_; }
        vector2 const& get_uv() const { return uv_; }

        standard_light const* get_light() const { return light_; }
        material const* get_material() const { return material_; }
        measurement const* get_measurement() const { return measurement_; }
        surface const* get_surface() const { return surface_; }

        void set_position(vector3 const& position) { position_ = position; }
        void set_normal(vector3 const& normal) { normal_ = normal; }
        void set_uv(vector2 const& uv) { uv_ = uv; }

        void set_measurement(measurement const* measurement) { measurement_ = measurement; }
        void set_surface(surface const* surface) { surface_ = surface; }
        void set_light(standard_light const* light) { light_ = light; }
        void set_material(material const* material) { material_ = material; }


        void* get_measurement_data() const { return measurement_data_; }
        void set_measurement_data(void* data) { measurement_data_ = data; }

    private:
        vector3 position_{};
        vector3 normal_{};
        vector2 uv_{};

        standard_light const* light_{};
        material const* material_{};
        surface const* surface_{};

        measurement const* measurement_{};
        void* measurement_data_{};
    };
}