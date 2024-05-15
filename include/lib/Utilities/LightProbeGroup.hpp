#pragma once

#include <evoengine-pch.hpp>
#include <Application.hpp>
#include <CUDAModule.hpp>

using namespace EvoEngine;
namespace EvoEngine {
    struct LightProbeGroup {
        std::vector<IlluminationSampler<glm::vec3>> m_lightProbes;
        void CalculateIllumination(const RayProperties& rayProperties, int seed, float pushNormalDistance);
        bool OnInspect();
    };
}