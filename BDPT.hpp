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
            : samplesX_{samplesX}, samplesY_{samplesY}, maxVertices_{maxDepth}
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
        int maxVertices_{};

        struct Vertex
        {
            SurfacePoint3 p{};
            double pdf_p{};

            Vector3 w{};
            double pdf_w{};

            Vector3 beta{};
            BSDF bsdf{};
            Light const* light{};
        };

        void Run(Image& image, Vector2i const& pixelPosition, ICamera const& camera, ISampler& sampler, Scene const& scene, MemoryAllocator& memoryAllocator) const
        {


            // camera to light
            //{

            //    Vector3 I{};

            //    // p0
            //    SurfacePoint1 p0{};
            //    double pdf_p0{};
            //    Vector3 w01{};
            //    double pdf_w01{};
            //    Vector3 importance{camera.SampleImportance(image, pixelPosition, sampler.Get2D(), sampler.Get2D(),
            //        &p0, &pdf_p0, &w01, &pdf_w01)};


            //    // p1
            //    SurfacePoint3 p1{};
            //    if(scene.Raycast(p0, w01, &p1))
            //    {
            //        I += importance * std::abs(Dot(p0.GetNormal(), w01)) * p1.EmittedRadiance(-w01) / (pdf_p0 * pdf_w01);
            //    }

            //    image.AddSample(pixelPosition, I);
            //}

            //light to camera
            //{
            //    Vector3 I{};

            //    int lightCount{scene.GetLightCount()};
            //    int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            //    Light const* light{scene.GetLights()[lightIndex]};

            //    // p1
            //    SurfacePoint2 p1{};
            //    double pdf_p1{};
            //    Vector3 ___a{};
            //    double ___b{};
            //    Vector3 radiance{light->SampleRadiance(sampler.Get2D(), sampler.Get2D(), &p1, &pdf_p1, &___a, &___b)};
            //    pdf_p1 /= lightCount;

            //    // p0
            //    SurfacePoint1 p0{};
            //    double pdf_p0{};
            //    Vector2i pixel{};
            //    Vector3 importance{camera.SampleIncomingImportance(image, p1.GetPosition(), sampler.Get2D(), &pixel, &p0, &pdf_p0)};
            //    if(importance.x != 0.0 || importance.y != 0.0 || importance.z != 0.0)
            //    {
            //        Vector3 w10{Normalize(p0.GetPosition() - p1.GetPosition())};
            //        I += importance * G(p0, p1, w10) * light->EmittedRadiance(p1, w10) / (pdf_p0 * pdf_p1);
            //    }

            //    image.AddLightSample(pixel, I);
            //}


            // camera subpath
            std::vector<Vertex> cameraVertices{GenerateCameraSubpath(image, pixelPosition, camera, sampler, scene, memoryAllocator)};
            
            // light subpath
            std::vector<Vertex> lightVertices{GenerateLightSubpath(sampler, scene, memoryAllocator)};

            std::unordered_map<int, int> pathCount_{};

            for(int i{1}; i <= cameraVertices.size(); ++i)
            {
                for(int j{}; j <= lightVertices.size(); ++j)
                {
                    if(i > 1 && j == 0)
                    {
                        // connection a)
                        pathCount_[i] += 1;
                    }
                    else if(i > 1 && j == 1)
                    {
                        // connection c)
                        pathCount_[i + j] += 1;
                    }
                    else if(i == 1 && j > 1)
                    {
                        // connection d)
                        pathCount_[i + j] += 1;
                    }
                    else if(i > 1 && j > 1)
                    {

                        // connection f)
                        pathCount_[i + j] += 1;
                    }
                }
            }

            Vector3 I{};
            for(int i{1}; i <= cameraVertices.size(); ++i)
            {
                for(int j{}; j <= lightVertices.size(); ++j)
                {
                    if(i > 1 && j == 0)
                    {
                        // connection a)
                        I += ConnectionA(i, cameraVertices) / static_cast<double>(pathCount_[i + j]);
                    }
                    else if(i > 1 && j == 1)
                    {
                        // connection c)
                        I += ConnectionC(i, cameraVertices, sampler, scene) / static_cast<double>(pathCount_[i + j]);
                    }
                    else if(i == 1 && j > 1)
                    {
                        // connection d)
                        Vector2i pixel{-1, 0};
                        Vector3 value{ConnectionD(j, lightVertices, sampler, scene, camera, image, &pixel) / static_cast<double>(pathCount_[i + j])};
                        if(pixel.x != -1)
                        {
                            image.AddLightSample(pixel, value);
                        }
                        else
                        {
                            image.AddLightSample();
                        }

                    }
                    else if(i > 1 && j > 1)
                    {

                        // connection f)
                        I += ConnectionF(i, cameraVertices, j, lightVertices, scene) / static_cast<double>(pathCount_[i + j]);
                        //pathCount_[i + j] += 1;
                    }
                }
            }

            image.AddSample(pixelPosition, I);

           /* for(int i{}; i < lightVertices.size(); ++i)
            {
                Vector3 I{};

                auto const& v{lightVertices[i]};
                Vector2i pixelPosition{};
                SurfacePoint1 p0{};
                double pdf_p0{};
                Vector3 importance{camera.SampleIncomingImportance(image, v.p.GetPosition(), sampler.Get2D(), &pixelPosition, &p0, &pdf_p0)};
                if(importance.x != 0.0 || importance.y != 0.0 || importance.z != 0.0)
                {
                    if(scene.Visibility(p0, v.p))
                    {

                        Vector3 w{Normalize(p0.GetPosition() - v.p.GetPosition())};
                        double beta0{1.0 / pdf_p0};

                        if(i == 0)
                        {
                            I += beta0 * importance * G(p0, v.p, w) * v.light->EmittedRadiance(v.p, w) * v.beta;
                        }
                        else
                        {
                            auto const& w_prev{-lightVertices[i - 1].w};
                            I += beta0 * importance * G(p0, v.p, w) * v.bsdf.Evaluate(w, w_prev) * v.beta;
                        }
                    }
                }

                image.AddLightSample(pixelPosition, I);
            }*/

            memoryAllocator.Clear();
        }

        Vector3 ConnectionA(int i, std::vector<Vertex> const& cameraVertices) const
        {
            Vector3 w{-cameraVertices[i - 2].w};
            return cameraVertices[i - 1].beta * cameraVertices[i - 1].p.EmittedRadiance(w);
        }

        Vector3 ConnectionC(int i, std::vector<Vertex> const& cameraVertices, ISampler& sampler, Scene const& scene) const
        {
            int lightCount{scene.GetLightCount()};
            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            Light const* light{scene.GetLights()[lightIndex]};

            Vector3 wi{};
            double pdf_p0;
            SurfacePoint2 p0{};
            Vector3 radiance{light->SampleIncomingRadiance(cameraVertices[i - 1].p.GetPosition(), sampler.Get2D(), &wi, &pdf_p0, &p0)};
            pdf_p0 /= lightCount;

            if(!scene.Visibility(cameraVertices[i - 1].p, p0)) return {};

            Vector3 wo{-cameraVertices[i - 2].w};
            return cameraVertices[i - 1].beta * cameraVertices[i - 1].bsdf.Evaluate(wo, wi) * G(p0, cameraVertices[i - 1].p, wi) * radiance / pdf_p0;
        }

        Vector3 ConnectionD(int j, std::vector<Vertex> const& lightVertices, ISampler& sampler, Scene const& scene, ICamera const& camera, Image const& image,
            Vector2i* pixelPosition) const
        {
            SurfacePoint1 p0{};
            double pdf_p0{};
            Vector3 importance{camera.SampleIncomingImportance(image, lightVertices[j - 1].p.GetPosition(), sampler.Get2D(), pixelPosition, &p0, &pdf_p0)};
            if(importance.x == 0.0 && importance.y == 0.0 && importance.z == 0.0) return {};

            if(!scene.Visibility(lightVertices[j - 1].p, p0)) return {};
            Vector3 wi{-lightVertices[j - 2].w};
            Vector3 wo{Normalize(p0.GetPosition() - lightVertices[j - 1].p.GetPosition())};

            return importance * G(p0, lightVertices[j - 1].p, wo) * lightVertices[j - 1].bsdf.Evaluate(wo, wi) * lightVertices[j - 1].beta / pdf_p0;
        }

        Vector3 ConnectionF(int i, std::vector<Vertex> const& cameraVertices, int j, std::vector<Vertex> const& lightVertices, Scene const& scene) const
        {
            Vector3 w0{-cameraVertices[i - 2].w};
            Vector3 w1{Normalize(cameraVertices[i - 1].p.GetPosition() - lightVertices[j - 1].p.GetPosition())};
            Vector3 w2{-lightVertices[j - 2].w};

            if(!scene.Visibility(cameraVertices[i - 1].p, lightVertices[j - 1].p)) return {};

            return cameraVertices[i - 1].beta * cameraVertices[i - 1].bsdf.Evaluate(w0, -w1) * G(cameraVertices[i - 1].p, lightVertices[j - 1].p, w1)
                * lightVertices[j - 1].bsdf.Evaluate(w1, w2) * lightVertices[j - 1].beta;
        }

        std::vector<Vertex> GenerateCameraSubpath(Image& image, Vector2i const& pixelPosition, ICamera const& camera, ISampler& sampler, Scene const& scene, MemoryAllocator& memoryAllocator) const
        {
            std::vector<Vertex> cameraVertices{};
            cameraVertices.reserve(2);

            auto& t0{cameraVertices.emplace_back()};
            Vector3 importance{camera.SampleImportance(image, pixelPosition, sampler.Get2D(), sampler.Get2D(),
                &t0.p, &t0.pdf_p, &t0.w, &t0.pdf_w)};
            t0.beta = 1.0 / t0.pdf_p;

            SurfacePoint3 p1{};
            if(!scene.Raycast(t0.p, t0.w, &p1)) return cameraVertices;

            auto& t1{cameraVertices.emplace_back()};
            t1.p = p1;
            t1.beta = cameraVertices[0].beta * importance * std::abs(Dot(t0.p.GetNormal(), t0.w)) / t0.pdf_w;

            t1.bsdf = t1.p.EvaluateMaterial(memoryAllocator);
            bool delta{};

            Vector3 f{t1.bsdf.SampleWi(-t0.w, sampler.Get2D(), &t1.w, &t1.pdf_w, &delta)};

            for(int i{2}; i < maxVertices_; ++i)
            {
                SurfacePoint3 pi{};
                if(!scene.Raycast(cameraVertices.back().p, cameraVertices.back().w, &pi)) return cameraVertices;

                auto& ti{cameraVertices.emplace_back()};
                auto const& ti1{cameraVertices[cameraVertices.size() - 2]};

                ti.p = pi;
                ti.beta = ti1.beta * f * std::abs(Dot(ti1.p.GetNormal(), ti1.w)) / ti1.pdf_w;
                ti.bsdf = ti.p.EvaluateMaterial(memoryAllocator);
                f = ti.bsdf.SampleWi(-ti1.w, sampler.Get2D(), &ti.w, &ti.pdf_w, &delta);
            }

            return cameraVertices;
        }

        std::vector<Vertex> GenerateLightSubpath(ISampler& sampler, Scene const& scene, MemoryAllocator& memoryAllocator) const
        {
            std::vector<Vertex> lightVertices{};
            lightVertices.reserve(2);

            int lightCount{scene.GetLightCount()};
            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            Light const* light{scene.GetLights()[lightIndex]};

            auto& s0{lightVertices.emplace_back()};
            Vector3 radiance{light->SampleRadiance(sampler.Get2D(), sampler.Get2D(), &s0.p, &s0.pdf_p, &s0.w, &s0.pdf_w)};
            s0.pdf_p /= lightCount;
            s0.beta = 1.0 / s0.pdf_p;
            s0.light = light;

            SurfacePoint3 p1{};
            if(!scene.Raycast(s0.p, s0.w, &p1)) return lightVertices;

            auto& s1{lightVertices.emplace_back()};
            s1.p = p1;
            s1.beta = std::abs(Dot(s0.p.GetNormal(), s0.w)) * radiance / s0.pdf_w * s0.beta;

            s1.bsdf = s1.p.EvaluateMaterial(memoryAllocator);
            bool delta{};

            Vector3 f{s1.bsdf.SampleWi(-s0.w, sampler.Get2D(), &s1.w, &s1.pdf_w, &delta)};

            for(int i{2}; i < maxVertices_; ++i)
            {
                SurfacePoint3 pi{};
                if(!scene.Raycast(lightVertices.back().p, lightVertices.back().w, &pi)) return lightVertices;

                auto& si{lightVertices.emplace_back()};
                auto const& si1{lightVertices[lightVertices.size() - 2]};

                si.p = pi;
                si.beta = si1.beta * f * std::abs(Dot(si1.p.GetNormal(), si1.w)) / si1.pdf_w;
                si.bsdf = si.p.EvaluateMaterial(memoryAllocator);
                f = si.bsdf.SampleWi(-si1.w, sampler.Get2D(), &si.w, &si.pdf_w, &delta);
            }

            return lightVertices;
        }

        void RandomWalk(Scene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator, std::vector<Vertex>& vertices) const
        {
            if(vertices.size() == maxVertices_) return;

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