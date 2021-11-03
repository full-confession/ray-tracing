#pragma once
#include "camera.hpp"


namespace Fc
{

    class IIntegrator
    {
    public:
        virtual ~IIntegrator() = default;

        virtual void Begin(Vector2i resolution, std::shared_ptr<ICameraFactory> cameraFactory) = 0;

        virtual void SetScissor(Bounds2i const& scissor) = 0;
        virtual void SetMinPathLength(int value) = 0;
        virtual void SetMaxPathLength(int value) = 0;
        virtual void SetWorkerCount(int value) = 0;

        virtual void Run(int samplesPerPixel) = 0;

        virtual void Export() = 0;

        virtual void End() = 0;
    };
}