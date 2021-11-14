#pragma once
#include "sampling.hpp"
#include "light.hpp"
#include "random.hpp"
#include <vector>

namespace Fc
{
    class ILightDistribution 
    {
    public:
        virtual ~ILightDistribution() = default;

        virtual ILight const* Sample(double u, double* pdf) const = 0;
        virtual double PDF(ILight const* light) const = 0;
    };

    class ISpatialLightDistribution
    {
    public:
        virtual ~ISpatialLightDistribution() = default;

        virtual ILightDistribution const* GetLightDistribution(Vector3 const& position) const = 0;
    };

    class UniformLightDistribution : public ILightDistribution, public ISpatialLightDistribution
    {
    public:
        explicit UniformLightDistribution(std::vector<ILight const*> lights)
            : lights_{std::move(lights)}
        { }

        virtual ILight const* Sample(double u, double* pdf) const override
        {
            std::size_t index{std::min(static_cast<std::size_t>(u * static_cast<double>(lights_.size())), lights_.size() - 1)};
            *pdf = 1.0 / static_cast<double>(lights_.size());

            return lights_[index];
        }

        virtual double PDF(ILight const* light) const override
        {
            return 1.0 / static_cast<double>(lights_.size());
        }

        virtual ILightDistribution const* GetLightDistribution(Vector3 const&) const override
        {
            return this;
        }
    private:
        std::vector<ILight const*> lights_;
    };

    class PowerLightDistribution : public ILightDistribution, public ISpatialLightDistribution
    {
    public:
        explicit PowerLightDistribution(std::vector<ILight const*> lights)
            : lights_{std::move(lights)}
        {
            std::vector<double> powers{};
            powers.reserve(lights_.size());

            for(std::size_t i{}; i < lights_.size(); ++i)
            {
                powers.push_back(Luminance(lights_[i]->Power()));
            }

            powerDistribution_.reset(new Distribution1D{std::move(powers)});
        }

        virtual ILight const* Sample(double u, double* pdf) const override
        {
            std::size_t index{powerDistribution_->SampleDiscrete(u, pdf)};
            return lights_[index];
        }

        virtual double PDF(ILight const* light) const override
        {
            auto it{std::find(lights_.begin(), lights_.end(), light)};
            if(it == lights_.end()) return 0.0;
            std::size_t index{static_cast<std::size_t>(it - lights_.begin())};
            return powerDistribution_->PDFDiscrete(index);
        }

        virtual ILightDistribution const* GetLightDistribution(Vector3 const&) const override
        {
            return this;
        }

    private:
        std::vector<ILight const*> lights_;
        std::unique_ptr<Distribution1D> powerDistribution_;
    };

    class VoxelLightDistribution : public ISpatialLightDistribution
    {
        class Wrapper : public ILightDistribution
        {
        public:
            Wrapper(std::vector<ILight const*> const* lights, std::unique_ptr<Distribution1D> distribution)
                : lights_{lights}, distribution_{std::move(distribution)}
            { }

            virtual ILight const* Sample(double u, double* pdf) const override
            {
                std::size_t index{distribution_->SampleDiscrete(u, pdf)};
                return (*lights_)[index];
            }

            virtual double PDF(ILight const* light) const override
            {
                auto it{std::find(lights_->begin(), lights_->end(), light)};
                if(it == lights_->end()) return 0.0;
                std::size_t index{static_cast<std::size_t>(it - lights_->begin())};
                return distribution_->PDFDiscrete(index);
            }

        private:
            std::vector<ILight const*> const* lights_{};
            std::unique_ptr<Distribution1D> distribution_{};
        };

    public:
        explicit VoxelLightDistribution(std::vector<ILight const*> lights, Bounds3 const& bounds, Vector3i const& gridSize, int samplesPerVoxel)
            : lights_{lights}, bounds_{bounds}, gridSize_{gridSize}, samplesPerVoxel_{samplesPerVoxel}
        {
            voxelDistributions_.resize(gridSize_.z);
            for(int z{}; z < gridSize_.z; ++z)
            {
                voxelDistributions_[z].resize(gridSize_.y);
                for(int y{}; y < gridSize_.y; ++y)
                {
                    voxelDistributions_[z][y].reserve(gridSize_.x);
                    for(int x{}; x < gridSize_.x; ++x)
                    {
                        voxelDistributions_[z][y].push_back(ComputeVoxel({x, y, z}));
                    }
                }
            }

            outOfBoundsDistribution_.reset(new UniformLightDistribution{lights_});
        }

        virtual ILightDistribution const* GetLightDistribution(Vector3 const& position) const override
        {
            Vector3 offset{(position - bounds_.Min()) / bounds_.Diagonal()};
            Vector3i voxel{
                static_cast<int>(offset.x * gridSize_.x),
                static_cast<int>(offset.y * gridSize_.y),
                static_cast<int>(offset.z * gridSize_.z)
            };

            if(voxel.x < 0 || voxel.x >= gridSize_.x || voxel.y < 0 || voxel.y >= gridSize_.y || voxel.z < 0 || voxel.z >= gridSize_.z)
                return outOfBoundsDistribution_.get();

            return &voxelDistributions_[voxel.z][voxel.y][voxel.x];
        }

    private:
        std::vector<ILight const*> lights_{};
        Bounds3 bounds_{};
        Vector3i gridSize_{};
        int samplesPerVoxel_{};

        std::vector<std::vector<std::vector<Wrapper>>> voxelDistributions_;
        std::unique_ptr<UniformLightDistribution> outOfBoundsDistribution_;

        Wrapper ComputeVoxel(Vector3i const& voxel)
        {
            Vector3 diagonal{bounds_.Diagonal()};
            Vector3 voxelSize{
                static_cast<double>(diagonal.x) / static_cast<double>(gridSize_.x),
                static_cast<double>(diagonal.y) / static_cast<double>(gridSize_.y),
                static_cast<double>(diagonal.z) / static_cast<double>(gridSize_.z)
            };

            Vector3 offset{bounds_.Min() + Vector3{voxel} * voxelSize};
            Random rand{0};


            std::vector<double> weights{};
            weights.resize(lights_.size());

            for(int i{}; i < samplesPerVoxel_; ++i)
            {
                Vector3 position{offset + Vector3{rand.UniformFloat(), rand.UniformFloat(), rand.UniformFloat()} * voxelSize};

                for(std::size_t j{}; j < lights_.size(); ++j)
                {
                    if(lights_[j]->IsInfinityAreaLight())
                    {
                        Vector3 w{};
                        double pdf_w{};
                        Vector3 r{};
                        if(lights_[j]->Sample(Vector2{rand.UniformFloat(), rand.UniformFloat()}, &w, &pdf_w, &r) == SampleResult::Success)
                        {
                            weights[j] += Luminance(r) / pdf_w;
                        }
                    }
                    else
                    {
                        SurfacePoint p{};
                        double pdf_p{};
                        Vector3 r{};
                        if(lights_[j]->Sample(position, Vector2{rand.UniformFloat(), rand.UniformFloat()}, &p, &pdf_p, &r) == SampleResult::Success)
                        {
                            Vector3 dir{p.GetPosition() - position};
                            double lengthSqr{LengthSqr(dir)};
                            Vector3 w{dir / std::sqrt(lengthSqr)};
                            double pdf_w{pdf_p * lengthSqr / std::abs(Dot(p.GetNormal(), w))};
                            weights[j] += Luminance(r) / pdf_w;
                        }
                    }
                }
            }

            double weightSum{};
            for(double weight : weights) weightSum += weight;
            double averageWeight{weightSum / (samplesPerVoxel_ * static_cast<double>(lights_.size()))};
            double minWeight{averageWeight > 0.0 ? 0.001 * averageWeight : 1.0};
            for(double& weight : weights)
            {
                weight = std::max(weight, minWeight);
            }

            return Wrapper{&lights_, std::unique_ptr<Distribution1D>{new Distribution1D{std::move(weights)}}};
        }
    };
}