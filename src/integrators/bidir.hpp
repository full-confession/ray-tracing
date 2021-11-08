#pragma once
#include "../core/integrator.hpp"
#include "../core/scene.hpp"
#include "../core/sampler.hpp"
#include "../core/allocator.hpp"
namespace Fc
{
    class BidirWalk
    {
    private:
        struct Vertex
        {
            SurfacePoint p{};

            double pdf_forward{};
            double pdf_backward{};

            Vector3 w{};
            IBxDF const* b{};

            Vector3 beta{};
        };

    public:
        static void Sample(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength)
        {
            //if(sampler.GetSampleIndex() == 232103)
            //{
            //    int x = 0;
            //}

            std::vector<Vertex, AllocatorWrapper<Vertex>> cameraVertices{AllocatorWrapper<Vertex>{&allocator}};
            std::vector<Vertex, AllocatorWrapper<Vertex>> lightVertices{AllocatorWrapper<Vertex>{&allocator}};

            cameraVertices.reserve(static_cast<std::size_t>(maxLength) + 1);
            lightVertices.reserve(static_cast<std::size_t>(maxLength));

            CreateCameraPath(camera, scene, sampler, allocator, maxLength, cameraVertices);
            CreateLightPath(scene, sampler, allocator, maxLength, lightVertices);

            Vector3 I{};

            int maxVertices{maxLength + 1};
            int tVertices{static_cast<int>(cameraVertices.size())};
            int sVertices{static_cast<int>(lightVertices.size())};

            if(tVertices >= 1)
            {
                int x{std::min(maxVertices - 1, sVertices)};
                for(int s{2}; s <= x; ++s)
                {
                    // c
                    ConnectT1SN(camera, scene, sampler, cameraVertices, lightVertices, s);
                }
            }

            for(int t{2}; t <= tVertices; ++t)
            {
                // a
                I += ConnectTNS0(scene, cameraVertices, t);
            }

            int y{std::min(maxVertices - 1, tVertices)};
            for(int t{2}; t <= y; ++t)
            {
                // b
                I += ConnectTNS1(scene, cameraVertices, t, lightVertices);
            }

            int z{std::min(maxVertices - 2, tVertices)};
            for(int t{2}; t <= z; ++t)
            {
                int v{std::min(maxVertices - t, sVertices)};
                for(int s{2}; s <= v; ++s)
                {
                    // d
                    I += ConnectTNSN(scene, cameraVertices, t, lightVertices, s);
                }
            }

            camera.AddSample(cameraVertices[0].p, cameraVertices[0].w, I);
            camera.AddSampleCount(1);
        }

    private:
        static void CreateCameraPath(ICamera& camera, Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength, std::vector<Vertex, AllocatorWrapper<Vertex>>& vertices)
        {
            vertices.emplace_back();
            vertices.emplace_back();

            std::size_t v0{0};
            std::size_t v1{1};

            double pdf_w{};
            if(camera.Sample(sampler.Get2D(), sampler.Get2D(),
                &vertices[v0].p, &vertices[v0].pdf_forward, &vertices[v0].w, &pdf_w, &vertices[v1].beta) != SampleResult::Success)
            {
                vertices.pop_back();
                vertices.pop_back();
                return;
            }
            vertices[v0].beta = 1.0 / vertices[v0].pdf_forward;

            if(scene.Raycast(vertices[v0].p, vertices[v0].w, &vertices[v1].p) != RaycastResult::Hit)
            {
                vertices.pop_back();
                return;
            }

            vertices[v1].beta *= vertices[v0].beta * std::abs(Dot(vertices[v0].p.GetNormal(), vertices[v0].w)) / pdf_w;
            vertices[v1].b = vertices[v1].p.GetMaterial()->Evaluate(vertices[v1].p, allocator);
            vertices[v1].pdf_forward = pdf_w * std::abs(Dot(vertices[v1].p.GetNormal(), vertices[v0].w)) / LengthSqr(vertices[v1].p.GetPosition() - vertices[v0].p.GetPosition());

            std::size_t v2{2};
            for(int i{2}; i <= maxLength; ++i)
            {
                vertices.emplace_back();
                double pdf_w12{};
                BxDFFlags flags{};
                if(vertices[v1].b->Sample(-vertices[v0].w, sampler, TransportMode::Radiance, &vertices[v1].w, &pdf_w12, &vertices[v2].beta, &flags) != SampleResult::Success)
                {
                    vertices.pop_back();
                    return;
                }

                if(scene.Raycast(vertices[v1].p, vertices[v1].w, &vertices[v2].p) != RaycastResult::Hit)
                {
                    vertices.pop_back();
                    return;
                }

                vertices[v2].beta *= vertices[v1].beta * std::abs(Dot(vertices[v1].p.GetNormal(), vertices[v1].w)) / pdf_w12;
                vertices[v2].b = vertices[v2].p.GetMaterial()->Evaluate(vertices[v2].p, allocator);

                double pdf2{vertices[v1].b->PDF(-vertices[v0].w, vertices[v1].w)};
                double pdf0{vertices[v1].b->PDF(vertices[v1].w, -vertices[v0].w)};

                vertices[v2].pdf_forward = pdf2 * std::abs(Dot(vertices[v2].p.GetNormal(), vertices[v1].w)) / LengthSqr(vertices[v2].p.GetPosition() - vertices[v1].p.GetPosition());
                vertices[v0].pdf_backward = pdf0 * std::abs(Dot(vertices[v0].p.GetNormal(), vertices[v0].w)) / LengthSqr(vertices[v0].p.GetPosition() - vertices[v1].p.GetPosition());
                v0 += 1;
                v1 += 1;
                v2 += 1;
            }

        }

        static void CreateLightPath(Scene const& scene, ISampler& sampler, Allocator& allocator, int maxLength, std::vector<Vertex, AllocatorWrapper<Vertex>>& vertices)
        {
            vertices.emplace_back();
            vertices.emplace_back();

            std::size_t v0{0};
            std::size_t v1{1};

            int lightCount{scene.GetLightCount()};
            int lightIndex{std::min(static_cast<int>(sampler.Get1D() * lightCount), lightCount - 1)};
            ILight const* light{scene.GetLight(lightIndex)};
            double pdf_w{};
            if(light->Sample(sampler.Get2D(), sampler.Get2D(),
                &vertices[v0].p, &vertices[v0].pdf_backward, &vertices[v0].w, &pdf_w, &vertices[v1].beta) != SampleResult::Success)
            {
                vertices.pop_back();
                vertices.pop_back();
                return;
            }
            vertices[v0].pdf_backward /= lightCount;
            vertices[v0].beta = 1.0 / vertices[v0].pdf_backward;

            if(scene.Raycast(vertices[v0].p, vertices[v0].w, &vertices[v1].p) != RaycastResult::Hit)
            {
                vertices.pop_back();
                return;
            }

            vertices[v1].beta *= vertices[v0].beta * std::abs(Dot(vertices[v0].p.GetNormal(), vertices[v0].w)) / pdf_w;
            vertices[v1].b = vertices[v1].p.GetMaterial()->Evaluate(vertices[v1].p, allocator);
            vertices[v1].pdf_backward = pdf_w * std::abs(Dot(vertices[v1].p.GetNormal(), vertices[v0].w)) / LengthSqr(vertices[v1].p.GetPosition() - vertices[v0].p.GetPosition());

            std::size_t v2{2};
            for(int i{2}; i < maxLength; ++i)
            {
                vertices.emplace_back();
                double pdf_w12{};
                BxDFFlags flags{};
                if(vertices[v1].b->Sample(-vertices[v0].w, sampler, TransportMode::Importance, &vertices[v1].w, &pdf_w12, &vertices[v2].beta, &flags) != SampleResult::Success)
                {
                    vertices.pop_back();
                    return;
                }

                if(scene.Raycast(vertices[v1].p, vertices[v1].w, &vertices[v2].p) != RaycastResult::Hit)
                {
                    vertices.pop_back();
                    return;
                }

                vertices[v2].beta *= vertices[v1].beta * std::abs(Dot(vertices[v1].p.GetNormal(), vertices[v1].w)) / pdf_w12;
                vertices[v2].b = vertices[v2].p.GetMaterial()->Evaluate(vertices[v2].p, allocator);
                double pdf2{vertices[v1].b->PDF(-vertices[v0].w, vertices[v1].w)};
                double pdf0{vertices[v1].b->PDF(vertices[v1].w, -vertices[v0].w)};
                vertices[v2].pdf_backward = pdf2 * std::abs(Dot(vertices[v2].p.GetNormal(), vertices[v1].w)) / LengthSqr(vertices[v2].p.GetPosition() - vertices[v1].p.GetPosition());
                vertices[v0].pdf_forward = pdf0 * std::abs(Dot(vertices[v0].p.GetNormal(), vertices[v0].w)) / LengthSqr(vertices[v0].p.GetPosition() - vertices[v1].p.GetPosition());

                v0 += 1;
                v1 += 1;
                v2 += 1;
            }
        }

        static Vector3 ConnectTNS0(Scene const& scene, std::vector<Vertex, AllocatorWrapper<Vertex>>& cameraVertices, std::size_t t)
        {
            Vertex& t0{cameraVertices[t - 1]};
            Vertex& t1{cameraVertices[t - 2]};

            if(t0.p.GetLight() != nullptr)
            {
                Vector3 I{t0.beta * t0.p.GetLight()->EmittedRadiance(t0.p, -t1.w)};

                //// weight
                if(t > 2 && I)
                {
                    RestoreValue rv0{t0.pdf_backward};
                    RestoreValue rv1{t1.pdf_backward};

                    t0.pdf_backward = t0.p.GetLight()->PDF(t0.p) / scene.GetLightCount();
                    t1.pdf_backward = t0.p.GetLight()->PDF(t0.p, -t1.w) * std::abs(Dot(t1.p.GetNormal(), t1.w)) / LengthSqr(t1.p.GetPosition() - t0.p.GetPosition());

                    return I * Weight(cameraVertices, static_cast<int>(t), cameraVertices, 0);
                }

                return I;
            }
            else
            {
                return {};
            }
        }

        static Vector3 ConnectTNS1(Scene const& scene, std::vector<Vertex, AllocatorWrapper<Vertex>>& cameraVertices, std::size_t t, std::vector<Vertex, AllocatorWrapper<Vertex>>& lightVertices)
        {

            Vertex& t0{cameraVertices[t - 1]};
            Vertex& t1{cameraVertices[t - 2]};
            Vertex& s0{lightVertices[0]};

            Vector3 d{t0.p.GetPosition() - s0.p.GetPosition()};
            double lengthSqr{LengthSqr(d)};
            Vector3 w{d / std::sqrt(lengthSqr)};
            Vector3 r{s0.p.GetLight()->EmittedRadiance(s0.p, w)};

            Vector3 f{t0.b->Evaluate(-w, -t1.w)};
            if(!f || !r || scene.Visibility(t0.p, s0.p) == VisibilityResult::Occluded) return {};

            double G{std::abs(Dot(t0.p.GetNormal(), w)) * std::abs(Dot(s0.p.GetNormal(), w)) / lengthSqr};
            Vector3 I{t0.beta * f * G * r * s0.beta};

            if(I)
            {
                RestoreValue rv0{t0.pdf_backward};
                RestoreValue rv1{t1.pdf_backward};
                RestoreValue rv2{s0.pdf_forward};

                t0.pdf_backward = s0.p.GetLight()->PDF(s0.p, w) * std::abs(Dot(t0.p.GetNormal(), w)) / lengthSqr;
                t1.pdf_backward = t0.b->PDF(-w, -t1.w) * std::abs(Dot(t1.p.GetNormal(), t1.w)) / LengthSqr(t1.p.GetPosition() - t0.p.GetPosition());
                s0.pdf_forward = t0.b->PDF(-t1.w, -w) * std::abs(Dot(s0.p.GetNormal(), w)) / lengthSqr;

                I *= Weight(cameraVertices, static_cast<int>(t), lightVertices, 1);
            }

            return I;
        }

        static void ConnectT1SN(ICamera& camera, Scene const& scene, ISampler& sampler, std::vector<Vertex, AllocatorWrapper<Vertex>>& cameraVertices, std::vector<Vertex, AllocatorWrapper<Vertex>>& lightVertices, std::size_t s)
        {
            Vertex& s0{lightVertices[s - 1]};
            Vertex& s1{lightVertices[s - 2]};

            SurfacePoint p{};
            double pdf_p{};
            double pdf_w{};
            Vector3 i{};
            if(camera.Sample(s0.p.GetPosition(), sampler.Get2D(), &p, &pdf_p, &i, &pdf_w) != SampleResult::Success) return;

            Vector3 d{s0.p.GetPosition() - p.GetPosition()};
            double lengthSqr{LengthSqr(d)};
            Vector3 w{d / std::sqrt(lengthSqr)};
            Vector3 f{s0.b->Evaluate(-s1.w, -w)};
            if(!f || scene.Visibility(p, s0.p) == VisibilityResult::Occluded) return;

            double G{std::abs(Dot(p.GetNormal(), w)) * std::abs(Dot(s0.p.GetNormal(), w)) / lengthSqr};
            Vector3 I{i * G * f * s0.beta / pdf_p};
            if(I)
            {
                RestoreValue rv0{s0.pdf_forward};
                RestoreValue rv1{s1.pdf_forward};

                s0.pdf_forward = pdf_w * std::abs(Dot(s0.p.GetNormal(), w)) / lengthSqr;
                s1.pdf_forward = s0.b->PDF(-w, -s1.w) * std::abs(Dot(s1.p.GetNormal(), s1.w)) / LengthSqr(s1.p.GetPosition() - s0.p.GetPosition());
                
                I *= Weight(cameraVertices, 1, lightVertices, s);

                camera.AddSample(p, w, I);
            }
        }

        static Vector3 ConnectTNSN(Scene const& scene, std::vector<Vertex, AllocatorWrapper<Vertex>>& cameraVertices, std::size_t t, std::vector<Vertex, AllocatorWrapper<Vertex>>& lightVertices, std::size_t s)
        {

            Vertex& t0{cameraVertices[t - 1]};
            Vertex& t1{cameraVertices[t - 2]};
            Vertex& s0{lightVertices[s - 1]};
            Vertex& s1{lightVertices[s - 2]};

            Vector3 d{t0.p.GetPosition() - s0.p.GetPosition()};
            double lengthSqr{LengthSqr(d)};
            Vector3 w{d / std::sqrt(lengthSqr)};
            Vector3 ft{t0.b->Evaluate(-w, -t1.w)};
            Vector3 fs{s0.b->Evaluate(-s1.w, w)};
            if(!ft || !fs || scene.Visibility(t0.p, s0.p) == VisibilityResult::Occluded) return {};

            double G{std::abs(Dot(t0.p.GetNormal(), w)) * std::abs(Dot(s0.p.GetNormal(), w)) / lengthSqr};
            Vector3 I{t0.beta * ft * G * fs * s0.beta};

            if(I)
            {
                RestoreValue rv0{t0.pdf_backward};
                RestoreValue rv1{t1.pdf_backward};
                RestoreValue rv2{s0.pdf_forward};
                RestoreValue rv3{s1.pdf_forward};

                s0.pdf_forward = t0.b->PDF(-t1.w, -w) * std::abs(Dot(s0.p.GetNormal(), w)) / lengthSqr;
                s1.pdf_forward = s0.b->PDF(w, -s1.w) * std::abs(Dot(s1.p.GetNormal(), s1.w)) / LengthSqr(s1.p.GetPosition() - s0.p.GetPosition());
                t0.pdf_backward = s0.b->PDF(-s1.w, w) * std::abs(Dot(t0.p.GetNormal(), w)) / lengthSqr;
                t1.pdf_backward = t0.b->PDF(-w, -t1.w) * std::abs(Dot(t1.p.GetNormal(), t1.w)) / LengthSqr(t1.p.GetPosition() - t0.p.GetPosition());

                I *= Weight(cameraVertices, t, lightVertices, s);
            }

            return I;
        }
    
        static double Weight(std::vector<Vertex, AllocatorWrapper<Vertex>>& cameraVertices, int t, std::vector<Vertex, AllocatorWrapper<Vertex>>& lightVertices, int s)
        {
            double sum{1.0};

            double r{1.0};
            for(int i{t - 1}; i > 0; --i)
            {
                r *= cameraVertices[i].pdf_backward / cameraVertices[i].pdf_forward;
                sum += r;
            }

            r = 1.0;
            for(int i{s - 1}; i >= 0; --i)
            {
                r *= lightVertices[i].pdf_forward / lightVertices[i].pdf_backward;
                sum += r;
            }

            return 1.0 / sum;
        }

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
    };
}