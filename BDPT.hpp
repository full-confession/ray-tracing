#pragma once
#include "Camera.hpp"
#include "Sampler.hpp"
#include "Image.hpp"
#include "Scene.hpp"
#include "PathIntegrator.hpp"
namespace Fc
{



    class BDPT : public IIntegrator
    {
    public:
        BDPT(int samplesX, int samplesY, int maxDepth)
            : samplesX_{samplesX}, samplesY_{samplesY}, maxDepth_{maxDepth}
        { }

        virtual void Render(Image& image, ICamera const& camera, Scene const& scene, ISampler& sampler, Bounds2i const& scissor) const override
        {
            MemoryAllocator memoryAllocator{1024 * 1024};
            for(int i{}; i < image.GetResolution().y; ++i)
            {
                for(int j{}; j < image.GetResolution().x; ++j)
                {
                    sampler.BeginPixel(samplesX_, samplesY_, 0, 0);
                    for(int k{}; k < samplesX_ * samplesY_; ++k)
                    {
                        sampler.BeginSample();

                        Run(image, {j, i}, camera, sampler, scene, memoryAllocator);

                        sampler.EndSample();
                    }
                    sampler.EndPixel();
                }
            }
        }
    private:
        int samplesX_{};
        int samplesY_{};
        int maxDepth_{};

        struct Vertex
        {
            SurfacePoint3 p{};
            double pdf_p{};

            Vector3 w{};
            double pdf_w{};

            Vector3 beta{};
            BSDF bsdf{};
        };

        void Run(Image& image, Vector2i const& pixelPosition, ICamera const& camera, ISampler& sampler, Scene const& scene, MemoryAllocator& memoryAllocator) const
        {
            std::vector<Vertex> vertices{};

            auto& cameraVertex{vertices.emplace_back()};
            cameraVertex.beta = camera.SampleImportance(image, pixelPosition, sampler.Get2D(), sampler.Get2D(),
                &cameraVertex.p, &cameraVertex.pdf_p, &cameraVertex.w, &cameraVertex.pdf_w);
            cameraVertex.beta *= std::abs(Dot(cameraVertex.p.GetNormal(), cameraVertex.w)) / cameraVertex.pdf_p;

            RandomWalk(scene, sampler, memoryAllocator, vertices);

            Vector3 I{};

            if(vertices.size() > 1)
            {
                I += vertices[1].p.EmittedRadiance(-vertices[0].w) * vertices[0].beta / vertices[0].pdf_w;
            }

            for(int i{1}; i < vertices.size(); ++i)
            {
                int lightCount{scene.GetLightCount()};
                int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
                Light const* light{scene.GetLights()[lightIndex]};

                Vector3 wL{};
                double pdf_pL{};
                SurfacePoint2 pL{};
                Vector3 L{light->SampleIncomingRadiance(vertices[i].p.GetPosition(), sampler.Get2D(), &wL, &pdf_pL, &pL)};
                if(!scene.Visibility(vertices[i].p, pL)) continue;

                pdf_pL /= lightCount;

                Vector3 f{vertices[i].bsdf.Evaluate(-vertices[i - 1].w, wL)};
                I += L * vertices[i - 1].beta * f * G(vertices[i].p, pL, wL) / (vertices[i - 1].pdf_w * pdf_pL);
            }


           /* for(int i{1}; i < vertices.size(); ++i)
            {
                I += vertices[i].p.EmittedRadiance(-vertices[i - 1].w) * vertices[i - 1].beta / vertices[i - 1].pdf_w;
            }*/

            image.AddSample(pixelPosition, I);
            memoryAllocator.Clear();
        }

        void RandomWalk(Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator, std::vector<Vertex>& vertices) const
        {
            if(vertices.size() == maxDepth_) return;

            SurfacePoint3 p{};
            if(!scene.Raycast(vertices.back().p, vertices.back().w, &p)) return;

            auto& vertex{vertices.emplace_back()};
            auto& prevVertex{vertices[vertices.size() - 2]};
            vertex.p = p;
            vertex.pdf_p = prevVertex.pdf_w * std::abs(Dot(vertex.p.GetNormal(), prevVertex.w)) / LengthSqr(vertex.p.GetPosition() - prevVertex.p.GetPosition());

            vertex.bsdf = vertex.p.EvaluateMaterial(memoryAllocator);
            bool delta{};

            Vector3 f{vertex.bsdf.SampleWi(-prevVertex.w, sampler.Get2D(), &vertex.w, &vertex.pdf_w, &delta)};
            if(vertex.pdf_w == 0.0 || (f.x == 0.0 && f.y == 0.0 && f.z == 0.0)) return;

            vertex.beta = prevVertex.beta * f * std::abs(Dot(vertex.p.GetNormal(), vertex.w)) / prevVertex.pdf_w;

            RandomWalk(scene, sampler, memoryAllocator, vertices);
        }
    };
}