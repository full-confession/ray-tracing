#pragma once
#include "IIntegrator.hpp"

namespace Fc
{
    template<typename T>
    class RestoreValue
    {
    public:
        explicit RestoreValue(T& target)
            : target_{target}, copy_{target}
        { }

        ~RestoreValue()
        {
            target_ = copy_;
        }


    private:
        T& target_{};
        T copy_{};
    };

    class BidirectionalPathIntegrator : public PixelIntegrator
    {
    public:
        BidirectionalPathIntegrator(Vector2i const& tileSize, int workerCount, int xSamples, int ySamples, int maxVertices)
            : PixelIntegrator{tileSize, workerCount}, xSamples_{xSamples}, ySamples_{ySamples}, maxVertices_{maxVertices}
        {
            std::cout << "BDPT" << std::endl;
        }


    protected:
        virtual void RenderPixel(Image& image, Vector2i const& pixel, ICamera const& camera,
            IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const override
        {
            sampler.BeginPixel(xSamples_, ySamples_, maxVertices_ * 2, maxVertices_ * 3);
            for(int k{}; k < xSamples_ * ySamples_; ++k)
            {
                sampler.BeginSample();
                Sample(image, pixel, camera, scene, sampler, memoryAllocator);
                sampler.EndSample();
                memoryAllocator.Clear();
            }
            sampler.EndPixel();
        }

    private:
        struct Vertex
        {
            SurfacePoint p{};

            double pdf_p_forward{};
            double pdf_p_backward{};

            Vector3 w{};
            double pdf_w{};

            Vector3 beta{};

            BSDF bsdf{};
        };

        void Sample(Image& image, Vector2i const& pixel, ICamera const& camera, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            Vertex* tVertices{memoryAllocator.Allocate<Vertex[]>(maxVertices_)};
            std::memset(tVertices, 0, sizeof(Vertex) * maxVertices_);
            Vertex* sVertices{memoryAllocator.Allocate<Vertex[]>(maxVertices_)};
            std::memset(sVertices, 0, sizeof(Vertex) * maxVertices_);

            int t{GenerateCameraSubpath(tVertices, image, pixel, camera, scene, sampler, memoryAllocator)};
            int s{GenerateLightSubpath(sVertices, scene, sampler, memoryAllocator)};



            Vector3 value{};
            if(t > 1)
            {
                value += Connect(tVertices, 2, sVertices, 0, image, camera, scene, sampler, nullptr);
            }

            for(int i{3}; i <= maxVertices_; ++i)
            {
                for(int j{i}; j > 1; --j)
                {
                    if(t >= j && s >= i - j)
                    {
                        value += Connect(tVertices, j, sVertices, i - j, image, camera, scene, sampler, nullptr);
                    }
                }

                if(t >= 1 && s >= i - 1)
                {
                    Vector2i lightPixel{};
                    Vector3 v{Connect(tVertices, 1, sVertices, i - 1, image, camera, scene, sampler, &lightPixel)};
                    image.AddLightSample(lightPixel, v);
                }
            }
            image.AddSample(pixel, value);
            image.AddLightSampleCount(1);
        }

        int GenerateCameraSubpath(Vertex* vertices, Image& image, Vector2i const& pixel, ICamera const& camera, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            // 0 vertex
            Vector3 importance{camera.SamplePointAndDirection(image, pixel, sampler.Get2D(), sampler.Get2D(), &vertices[0].p, &vertices[0].pdf_p_forward, &vertices[0].w, &vertices[0].pdf_w)};
            vertices[0].beta = Vector3{1.0} / vertices[0].pdf_p_forward;
            int vertexCount{1};

            // 1 vertex
            if(!scene.Raycast(vertices[0].p, vertices[0].w, &vertices[1].p)) return vertexCount;
            vertices[1].beta = vertices[0].beta * importance * std::abs(Dot(vertices[0].p.Normal(), vertices[0].w)) / vertices[0].pdf_w;
            vertices[1].bsdf = vertices[1].p.Material()->EvaluateAtPoint(vertices[1].p, memoryAllocator);
            vertices[1].pdf_p_forward = vertices[0].pdf_w * std::abs(Dot(vertices[1].p.Normal(), vertices[0].w)) / LengthSqr(vertices[1].p.Position() - vertices[0].p.Position());
            vertexCount += 1;

            // i vertex
            for(int i{2}; i < maxVertices_; ++i)
            {
                Vertex& v0{vertices[i - 2]};
                Vertex& v1{vertices[i - 1]};
                Vertex& v2{vertices[i]};


                Vector3 f210{v1.bsdf.SampleBxDF(0, -v0.w, sampler.Get2D(), &v1.w, &v1.pdf_w)};
                if(!scene.Raycast(v1.p, v1.w, &v2.p)) return vertexCount;

                v2.beta = v1.beta * f210 * std::abs(Dot(v1.p.Normal(), v1.w)) / v1.pdf_w;
                v2.bsdf = v2.p.Material()->EvaluateAtPoint(v2.p, memoryAllocator);
                v2.pdf_p_forward = v1.pdf_w * std::abs(Dot(v2.p.Normal(), v1.w)) / LengthSqr(v2.p.Position() - v1.p.Position());
                
                double pdf_w10{v1.bsdf.PDFBxDF(0, -v0.w, v1.w)};
                v0.pdf_p_backward = pdf_w10 * std::abs(Dot(v0.p.Normal(), v0.w)) / LengthSqr(v1.p.Position() - v0.p.Position());

                vertexCount += 1;
            }

            return vertexCount;
        }

        int GenerateLightSubpath(Vertex* vertices, IScene const& scene, ISampler& sampler, MemoryAllocator& memoryAllocator) const
        {
            int lightCount{scene.LightCount()};
            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            ILight const* light{scene.Light(lightIndex)};

            // 0 vertex
            vertices[0].pdf_p_backward = light->SamplePoint(sampler.Get2D(), &vertices[0].p) / lightCount;
            vertices[0].pdf_w = light->SampleDirection(vertices[0].p, sampler.Get2D(), &vertices[0].w);
            vertices[0].beta = Vector3{1.0} / vertices[0].pdf_p_backward;
            int vertexCount{1};

            // 1 vertex
            if(!scene.Raycast(vertices[0].p, vertices[0].w, &vertices[1].p)) return vertexCount;
            vertices[1].beta = vertices[0].beta * light->EmittedRadiance(vertices[0].p, vertices[0].w) * std::abs(Dot(vertices[0].p.Normal(), vertices[0].w)) / vertices[0].pdf_w;
            vertices[1].bsdf = vertices[1].p.Material()->EvaluateAtPoint(vertices[1].p, memoryAllocator);
            vertices[1].pdf_p_backward = vertices[0].pdf_w * std::abs(Dot(vertices[1].p.Normal(), vertices[0].w)) / LengthSqr(vertices[1].p.Position() - vertices[0].p.Position());
            vertexCount += 1;

            // i vertex
            for(int i{2}; i < maxVertices_; ++i)
            {
                Vertex& v0{vertices[i - 2]};
                Vertex& v1{vertices[i - 1]};
                Vertex& v2{vertices[i]};

                Vector3 f210{v1.bsdf.SampleBxDF(0, -v0.w, sampler.Get2D(), &v1.w, &v1.pdf_w)};
                if(!scene.Raycast(v1.p, v1.w, &v2.p)) return vertexCount;

                v2.beta = v1.beta * f210 * std::abs(Dot(v1.p.Normal(), v1.w)) / v1.pdf_w;
                v2.bsdf = v2.p.Material()->EvaluateAtPoint(v2.p, memoryAllocator);
                v2.pdf_p_backward = v1.pdf_w * std::abs(Dot(v2.p.Normal(), v1.w)) / LengthSqr(v2.p.Position() - v1.p.Position());


                double pdf_w10{v1.bsdf.PDFBxDF(0, v1.w, -v0.w)};
                v0.pdf_p_forward = pdf_w10 * std::abs(Dot(v0.p.Normal(), v0.w)) / LengthSqr(v1.p.Position() - v0.p.Position());


                vertexCount += 1;
            }

            return vertexCount;
        }

        Vector3 Connect(Vertex* tVertices, int t, Vertex* sVertices, int s, Image const& image, ICamera const& camera, IScene const& scene, ISampler& sampler, Vector2i* pixel) const
        {
            if(t > 1 && s == 0 && tVertices[t - 1].p.Light() != nullptr)
            {
                Vector3 value{tVertices[t - 1].beta * tVertices[t - 1].p.Light()->EmittedRadiance(tVertices[t - 1].p, -tVertices[t - 2].w)};

                if(value && t > 2)
                {
                    Vertex const& v0{tVertices[t - 3]};
                    Vertex& v1{tVertices[t - 2]};
                    Vertex& v2{tVertices[t - 1]};

                    RestoreValue rv0{v2.pdf_p_backward};
                    RestoreValue rv1{v1.pdf_p_backward};

                    v2.pdf_p_backward = v2.p.Light()->ProbabilityPoint(v2.p) / scene.LightCount();
                    v1.pdf_p_backward = v2.p.Light()->ProbabilityDirection(v2.p, -v1.w) * std::abs(Dot(v1.p.Normal(), v1.w)) / LengthSqr(v1.p.Position() - v2.p.Position());

                    return Weight(tVertices, t, sVertices, s) * value;
                }
                else
                {
                    return value;
                }
            }
            else if(t > 1 && s == 1 && sVertices[s - 1].p.Light() != nullptr)
            {
                Vertex& v0{tVertices[t - 2]};
                Vertex& v1{tVertices[t - 1]};
                Vertex& v2{sVertices[s - 1]};


                Vector3 wo{-v0.w};
                Vector3 wi{Normalize(v2.p.Position() - v1.p.Position())};
                Vector3 radiance{v2.p.Light()->EmittedRadiance(v2.p, -wi)};

                if(radiance && scene.Visibility(v1.p, v2.p))
                {
                    Vector3 value{v1.beta * v1.bsdf.EvaluateBxDF(0, wo, wi) * G(v1.p, v2.p, wi) * v2.p.Light()->EmittedRadiance(v2.p, -wi) * v2.beta};

                    if(value)
                    {
                        RestoreValue rv0{v2.pdf_p_forward};
                        RestoreValue rv1{v1.pdf_p_backward};
                        RestoreValue rv2{v0.pdf_p_backward};

                        v2.pdf_p_forward = v1.bsdf.PDFBxDF(0, wo, wi) * std::abs(Dot(v2.p.Normal(), wi)) / LengthSqr(v2.p.Position() - v1.p.Position());
                        v1.pdf_p_backward = v2.p.Light()->ProbabilityDirection(v2.p, -wi) * std::abs(Dot(v1.p.Normal(), wi)) / LengthSqr(v2.p.Position() - v1.p.Position());
                        v0.pdf_p_backward = v1.bsdf.PDFBxDF(0, wo, wi) * std::abs(Dot(v0.p.Normal(), wo)) / LengthSqr(v0.p.Position() - v1.p.Position());
                        
                        return Weight(tVertices, t, sVertices, s) * value;
                    }

                }
            }
            // t = 1, s > 1
            else if(t == 1 && s > 1)
            {
                Vertex& v0{tVertices[t - 1]};
                Vertex& v1{sVertices[s - 1]};
                Vertex& v2{sVertices[s - 2]};

                RestoreValue rv0{v0};

                Vector3 importance{camera.SamplePoint(image, v1.p.Position(), sampler.Get2D(),
                    pixel, &v0.p, &v0.pdf_p_forward)};
                v0.beta = Vector3{1.0} / v0.pdf_p_forward;

                if(importance && scene.Visibility(v0.p, v1.p))
                {
                    Vector3 w10{Normalize(v0.p.Position() - v1.p.Position())};
                    Vector3 w12{v1.w};
                    Vector3 value{v0.beta * importance * G(v0.p, v1.p, w10) * v1.bsdf.EvaluateBxDF(0, w10, w12) * v1.beta};

                    if(value)
                    {
                        RestoreValue rv1{v1.pdf_p_forward};
                        RestoreValue rv2{v2.pdf_p_forward};

                        v1.pdf_p_forward = v0.p.Camera()->ProbabilityDirection(image, v0.p, -w10) * std::abs(Dot(v1.p.Normal(), w10)) / LengthSqr(v1.p.Position() - v0.p.Position());
                        v2.pdf_p_forward = v1.bsdf.PDFBxDF(0, w10, w12) * std::abs(Dot(v2.p.Normal(), w12)) / LengthSqr(v2.p.Position() - v1.p.Position());

                        return Weight(tVertices, t, sVertices, s) * value;
                    }
                }
            }
            else if(t > 1 && s > 1)
            {
                Vertex& v0{tVertices[t - 2]};
                Vertex& v1{tVertices[t - 1]};
                Vertex& v2{sVertices[s - 1]};
                Vertex& v3{sVertices[s - 2]};

                if(scene.Visibility(v1.p, v2.p))
                {
                    Vector3 w10{-v0.w};
                    Vector3 w12{Normalize(v2.p.Position() - v1.p.Position())};
                    Vector3 w21{-w12};
                    Vector3 w23{-v3.w};
                    
                    Vector3 value{v1.beta * v1.bsdf.EvaluateBxDF(0, w10, w12) * G(v1.p, v2.p, w12) * v2.bsdf.EvaluateBxDF(0, w21, w23) * v2.beta};

                    if(value)
                    {
                        RestoreValue rv0{v1.pdf_p_backward};
                        RestoreValue rv1{v0.pdf_p_backward};
                        RestoreValue rv2{v2.pdf_p_forward};
                        RestoreValue rv3{v3.pdf_p_forward};

                        v1.pdf_p_backward = v2.bsdf.PDFBxDF(0, w21, w23) * std::abs(Dot(v1.p.Normal(), w21)) / LengthSqr(v1.p.Position() - v2.p.Position());
                        v0.pdf_p_backward = v1.bsdf.PDFBxDF(0, w10, w12) * std::abs(Dot(v0.p.Normal(), w10)) / LengthSqr(v0.p.Position() - v1.p.Position());

                        v2.pdf_p_forward = v1.bsdf.PDFBxDF(0, w10, w12) * std::abs(Dot(v2.p.Normal(), w12)) / LengthSqr(v2.p.Position() - v1.p.Position());
                        v3.pdf_p_forward = v2.bsdf.PDFBxDF(0, w21, w23) * std::abs(Dot(v3.p.Normal(), w23)) / LengthSqr(v3.p.Position() - v2.p.Position());

                        return Weight(tVertices, t, sVertices, s) * value;
                    }
                }
            }

            return {};
        }

        double Weight(Vertex* tVertices, int t, Vertex* sVertices, int s) const
        {
            double sum{1.0};

            double r{1.0};
            for(int i{t - 1}; i > 0; --i)
            {
                r *= tVertices[i].pdf_p_backward / tVertices[i].pdf_p_forward;
                sum += r;
            }

            r = 1.0;
            for(int i{s - 1}; i >= 0; --i)
            {
                r *= sVertices[i].pdf_p_forward / sVertices[i].pdf_p_backward;
                sum += r;
            }

            double weight{1.0 / sum};
            return weight;
            //return 1.0 / 3.0;
        }

        int xSamples_{};
        int ySamples_{};
        int maxVertices_{};
    };
}