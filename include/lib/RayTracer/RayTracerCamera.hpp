#pragma once
#include "evoengine-pch.hpp"

#include <IPrivateComponent.hpp>
#include <RenderTexture.hpp>


#include "Graphics.hpp"
#include "CUDAModule.hpp"


using namespace EvoEngine;
namespace EvoEngine {
    class RayTracerCamera : public IPrivateComponent {
        friend class RayTracerLayer;
        friend class RayTracer;
        CameraProperties m_cameraProperties;
        bool m_rendered = false;
        bool m_mainCamera = false;
        AssetRef m_skybox;
    public:
        void SetSkybox(const std::shared_ptr<Cubemap>& cubemap);

        void SetMainCamera(bool value);
        bool m_allowAutoResize = true;
        std::shared_ptr<RenderTexture> m_renderTexture;
        RayProperties m_rayProperties;
        glm::uvec2 m_frameSize;
        void Ready(const glm::vec3& position, const glm::quat& rotation);
        void OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) override;
        void SetFov(float value);
        void SetAperture(float value);
        void SetFocalLength(float value);
        void SetMaxDistance(float value);
        void SetOutputType(OutputType value);
        void SetAccumulate(bool value);
        void SetGamma(float value);
        [[nodiscard]] glm::mat4 GetProjection() const;
        void SetDenoiserStrength(float value);
        void OnCreate() override;
        void OnDestroy() override;
        void Serialize(YAML::Emitter &out) override;
        void Deserialize(const YAML::Node &in) override;
        RayTracerCamera& operator=(const RayTracerCamera& source);
        void Render();
        void Render(const RayProperties& rayProperties);
        void Render(const RayProperties& rayProperties, const EnvironmentProperties& environmentProperties);
    };
}