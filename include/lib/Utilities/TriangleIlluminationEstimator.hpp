#pragma once
#include "LightProbeGroup.hpp"
#include "evoengine-pch.hpp"
#include "Application.hpp"

#include "CUDAModule.hpp"
#include "IPrivateComponent.hpp"

using namespace EvoEngine;
namespace EvoEngine {
    class TriangleIlluminationEstimator : public IPrivateComponent {
        LightProbeGroup m_lightProbeGroup;
    public:
        void PrepareLightProbeGroup();
        void SampleLightProbeGroup(const RayProperties& rayProperties, int seed, float pushNormalDistance);
        float m_totalArea = 0.0f;
        glm::vec3 m_totalFlux = glm::vec3(0.0f);
        glm::vec3 m_averageFlux = glm::vec3(0.0f);
        void OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) override;

        void Serialize(YAML::Emitter &out) override;
        void Deserialize(const YAML::Node &in) override;
    };


} // namespace SorghumFactory
