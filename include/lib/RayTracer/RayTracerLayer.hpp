#pragma once

#include "evoengine-pch.hpp"
#include "CUDAModule.hpp"
#include "Cubemap.hpp"
#include "ILayer.hpp"
#include "CompressedBTF.hpp"
#include "memory"

#include "Material.hpp"
using namespace EvoEngine;
namespace EvoEngine {
	class RayTracerCamera;
	class RayTracerLayer : public ILayer {
	protected:
		void UpdateMeshesStorage(
			const std::shared_ptr<Scene>& scene,
			std::unordered_map<uint64_t, RayTracedMaterial>& materialStorage,
			std::unordered_map<uint64_t, RayTracedGeometry>& geometryStorage,
			std::unordered_map<uint64_t, RayTracedInstance>& instanceStorage, bool& rebuildInstances,
			bool& updateShaderBindingTable) const;

		void SceneCameraWindow();

		void RayCameraWindow();

		friend class RayTracerCamera;

		static std::shared_ptr<RayTracerCamera> m_rayTracerCamera;

		bool CheckMaterial(RayTracedMaterial& rayTracerMaterial, const std::shared_ptr<Material>& material) const;

		bool CheckCompressedBTF(RayTracedMaterial& rayTracerMaterial, const std::shared_ptr<CompressedBTF>& compressedBtf) const;

		glm::ivec2 m_sceneCameraResolution = glm::ivec2(0);
	public:
		

		bool m_showSceneInfo = false;
		bool m_renderMeshRenderer = true;
		bool m_renderStrandsRenderer = true;
		bool m_renderParticles = true;
		bool m_renderBTFMeshRenderer = true;
		bool m_renderSkinnedMeshRenderer = false;
		[[nodiscard]] glm::ivec2 GetSceneCameraResolution() const;
		bool m_showRayTracerSettingsWindow = false;
		EnvironmentProperties m_environmentProperties;
		std::shared_ptr<CudaImage> m_environmentalMapImage;
		Handle m_environmentalMapHandle = 0;

		bool m_showSceneWindow = false;
		bool m_showCameraWindow = false;

		bool m_renderingEnabled = true;
		float m_resolutionMultiplier = 1.0f;
		std::shared_ptr<RayTracerCamera> m_sceneCamera;

		bool UpdateScene(const std::shared_ptr<Scene>& scene);

		void Update() override;

		void OnCreate() override;

		void PreUpdate() override;

		void OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) override;

		void OnDestroy() override;
	};
} // namespace EvoEngine