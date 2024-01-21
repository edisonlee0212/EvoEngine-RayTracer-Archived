#include "BTFMeshRenderer.hpp"
#include "RayTracerLayer.hpp"
#include "ProjectManager.hpp"
#include "RayTracer.hpp"
#include "EditorLayer.hpp"
#include "RayTracerCamera.hpp"
#include "TriangleIlluminationEstimator.hpp"
#include "BasicPointCloudScanner.hpp"
#include "ClassRegistry.hpp"
#include "StrandsRenderer.hpp"
#include "MeshRenderer.hpp"
#include "Particles.hpp"
#include "SkinnedMeshRenderer.hpp"
#include "CompressedBTF.hpp"
#include "Times.hpp"
using namespace EvoEngine;

std::shared_ptr<RayTracerCamera> RayTracerLayer::m_rayTracerCamera;

void RayTracerLayer::UpdateMeshesStorage(std::unordered_map<uint64_t, RayTracedMaterial>& materialStorage,
	std::unordered_map<uint64_t, RayTracedGeometry>& geometryStorage,
	std::unordered_map<uint64_t, RayTracedInstance>& instanceStorage,
	bool& rebuildInstances, bool& updateShaderBindingTable) const {
	for (auto& i : instanceStorage) i.second.m_removeFlag = true;
	for (auto& i : geometryStorage) i.second.m_removeFlag = true;
	for (auto& i : materialStorage) i.second.m_removeFlag = true;
	auto scene = GetScene();

	if (const auto* rayTracedEntities =
		scene->UnsafeGetPrivateComponentOwnersList<StrandsRenderer>();
		rayTracedEntities && m_renderStrandsRenderer) {
		for (auto entity : *rayTracedEntities) {
			if (!scene->IsEntityEnabled(entity))
				continue;
			auto strandsRendererRenderer =
				scene->GetOrSetPrivateComponent<StrandsRenderer>(entity).lock();
			if (!strandsRendererRenderer->IsEnabled())
				continue;
			auto strands = strandsRendererRenderer->m_strands.Get<Strands>();
			auto material = strandsRendererRenderer->m_material.Get<Material>();
			if (!material || !strands || strands->UnsafeGetStrandPoints().empty() || strands->UnsafeGetSegments().empty())
				continue;
			auto globalTransform = scene->GetDataComponent<GlobalTransform>(entity).m_value;
			bool needInstanceUpdate = false;
			bool needMaterialUpdate = false;

			auto entityHandle = scene->GetEntityHandle(entity);
			auto geometryHandle = strands->GetHandle();
			auto materialHandle = material->GetHandle();
			auto& rayTracedInstance = instanceStorage[strandsRendererRenderer->GetHandle().GetValue()];
			auto& rayTracedGeometry = geometryStorage[geometryHandle];
			auto& rayTracedMaterial = materialStorage[materialHandle];
			rayTracedInstance.m_removeFlag = false;
			rayTracedMaterial.m_removeFlag = false;
			rayTracedGeometry.m_removeFlag = false;

			if (rayTracedInstance.m_entityHandle != entityHandle
				|| rayTracedInstance.m_privateComponentHandle != strandsRendererRenderer->GetHandle().GetValue()
				|| rayTracedInstance.m_version != strandsRendererRenderer->GetVersion()
				|| globalTransform != rayTracedInstance.m_globalTransform) {
				needInstanceUpdate = true;
			}
			if (rayTracedGeometry.m_handle == 0 || rayTracedGeometry.m_version != strands->GetVersion()) {
				rayTracedGeometry.m_updateFlag = true;
				needInstanceUpdate = true;
				rayTracedGeometry.m_rendererType = RendererType::Curve;
				rayTracedGeometry.m_curveSegments = &strands->UnsafeGetSegments();
				rayTracedGeometry.m_curvePoints = &strands->UnsafeGetStrandPoints();
				rayTracedGeometry.m_version = strands->GetVersion();
				rayTracedGeometry.m_geometryType = PrimitiveType::CubicBSpline;
				rayTracedGeometry.m_handle = geometryHandle;
			}
			if (CheckMaterial(rayTracedMaterial, material)) needInstanceUpdate = true;
			if (needInstanceUpdate) {
				rayTracedInstance.m_entityHandle = entityHandle;
				rayTracedInstance.m_privateComponentHandle = strandsRendererRenderer->GetHandle().GetValue();
				rayTracedInstance.m_version = strandsRendererRenderer->GetVersion();
				rayTracedInstance.m_globalTransform = globalTransform;
				rayTracedInstance.m_geometryMapKey = geometryHandle;
				rayTracedInstance.m_materialMapKey = materialHandle;
			}
			updateShaderBindingTable = updateShaderBindingTable || needMaterialUpdate;
			rebuildInstances = rebuildInstances || needInstanceUpdate;
		}
	}
	if (const auto* rayTracedEntities =
		scene->UnsafeGetPrivateComponentOwnersList<MeshRenderer>();
		rayTracedEntities && m_renderMeshRenderer) {
		for (auto entity : *rayTracedEntities) {
			if (!scene->IsEntityEnabled(entity))
				continue;
			auto meshRenderer =
				scene->GetOrSetPrivateComponent<MeshRenderer>(entity).lock();
			if (!meshRenderer->IsEnabled())
				continue;
			auto mesh = meshRenderer->m_mesh.Get<Mesh>();
			auto material = meshRenderer->m_material.Get<Material>();
			if (!material || !mesh || mesh->UnsafeGetVertices().empty())
				continue;
			auto globalTransform = scene->GetDataComponent<GlobalTransform>(entity).m_value;
			bool needInstanceUpdate = false;
			bool needMaterialUpdate = false;

			auto entityHandle = scene->GetEntityHandle(entity);
			auto geometryHandle = mesh->GetHandle();
			auto materialHandle = material->GetHandle();
			auto& rayTracedInstance = instanceStorage[meshRenderer->GetHandle().GetValue()];
			auto& rayTracedGeometry = geometryStorage[geometryHandle];
			auto& rayTracedMaterial = materialStorage[materialHandle];
			rayTracedInstance.m_removeFlag = false;
			rayTracedMaterial.m_removeFlag = false;
			rayTracedGeometry.m_removeFlag = false;

			if (rayTracedInstance.m_entityHandle != entityHandle
				|| rayTracedInstance.m_privateComponentHandle != meshRenderer->GetHandle().GetValue()
				|| rayTracedInstance.m_version != meshRenderer->GetVersion()
				|| globalTransform != rayTracedInstance.m_globalTransform) {
				needInstanceUpdate = true;
			}
			if (rayTracedGeometry.m_handle == 0 || rayTracedGeometry.m_version != mesh->GetVersion()) {
				rayTracedGeometry.m_updateFlag = true;
				needInstanceUpdate = true;
				rayTracedGeometry.m_rendererType = RendererType::Default;
				rayTracedGeometry.m_triangles = &mesh->UnsafeGetTriangles();
				rayTracedGeometry.m_vertices = &mesh->UnsafeGetVertices();
				rayTracedGeometry.m_version = mesh->GetVersion();
				rayTracedGeometry.m_geometryType = PrimitiveType::Triangle;
				rayTracedGeometry.m_handle = geometryHandle;
			}
			if (CheckMaterial(rayTracedMaterial, material)) needInstanceUpdate = true;
			if (needInstanceUpdate) {
				rayTracedInstance.m_entityHandle = entityHandle;
				rayTracedInstance.m_privateComponentHandle = meshRenderer->GetHandle().GetValue();
				rayTracedInstance.m_version = meshRenderer->GetVersion();
				rayTracedInstance.m_globalTransform = globalTransform;
				rayTracedInstance.m_geometryMapKey = geometryHandle;
				rayTracedInstance.m_materialMapKey = materialHandle;
			}
			updateShaderBindingTable = updateShaderBindingTable || needMaterialUpdate;
			rebuildInstances = rebuildInstances || needInstanceUpdate;
		}
	}
	if (const auto* rayTracedEntities =
		scene->UnsafeGetPrivateComponentOwnersList<SkinnedMeshRenderer>();
		rayTracedEntities && m_renderSkinnedMeshRenderer) {
		for (auto entity : *rayTracedEntities) {
			if (!scene->IsEntityEnabled(entity))
				continue;
			auto skinnedMeshRenderer =
				scene->GetOrSetPrivateComponent<SkinnedMeshRenderer>(entity).lock();
			if (!skinnedMeshRenderer->IsEnabled())
				continue;
			auto mesh = skinnedMeshRenderer->m_skinnedMesh.Get<SkinnedMesh>();
			auto material = skinnedMeshRenderer->m_material.Get<Material>();
			if (!material || !mesh || mesh->UnsafeGetSkinnedVertices().empty() ||
				skinnedMeshRenderer->m_boneMatrices->m_value.empty())
				continue;
			auto globalTransform =
				skinnedMeshRenderer->RagDoll()
				? glm::mat4(1.0f)
				: scene->GetDataComponent<GlobalTransform>(entity).m_value;
			bool needInstanceUpdate = false;
			bool needMaterialUpdate = false;

			auto entityHandle = scene->GetEntityHandle(entity);
			auto geometryHandle = skinnedMeshRenderer->GetHandle().GetValue();
			auto materialHandle = material->GetHandle();
			auto& rayTracedInstance = instanceStorage[geometryHandle];
			auto& rayTracedGeometry = geometryStorage[geometryHandle];
			auto& rayTracedMaterial = materialStorage[materialHandle];
			rayTracedInstance.m_removeFlag = false;
			rayTracedMaterial.m_removeFlag = false;
			rayTracedGeometry.m_removeFlag = false;

			if (rayTracedInstance.m_entityHandle != entityHandle
				|| rayTracedInstance.m_privateComponentHandle != skinnedMeshRenderer->GetHandle().GetValue()
				|| rayTracedInstance.m_version != skinnedMeshRenderer->GetVersion()
				|| globalTransform != rayTracedInstance.m_globalTransform) {
				needInstanceUpdate = true;
			}

			if (rayTracedGeometry.m_handle == 0
				|| rayTracedInstance.m_version != skinnedMeshRenderer->GetVersion()
				|| rayTracedGeometry.m_version != mesh->GetVersion()
				|| rayTracedInstance.m_dataVersion != skinnedMeshRenderer->m_boneMatrices->GetVersion()
				|| true) {
				rayTracedGeometry.m_updateFlag = true;
				needInstanceUpdate = true;
				rayTracedGeometry.m_geometryType = PrimitiveType::Triangle;
				rayTracedGeometry.m_rendererType = RendererType::Skinned;
				rayTracedGeometry.m_triangles = &mesh->UnsafeGetTriangles();
				rayTracedGeometry.m_skinnedVertices = &mesh->UnsafeGetSkinnedVertices();
				rayTracedGeometry.m_boneMatrices =
					&skinnedMeshRenderer->m_boneMatrices->m_value;
				rayTracedGeometry.m_version = mesh->GetVersion();
				rayTracedInstance.m_dataVersion = skinnedMeshRenderer->m_boneMatrices->GetVersion();
				rayTracedGeometry.m_handle = geometryHandle;
			}
			if (CheckMaterial(rayTracedMaterial, material)) needInstanceUpdate = true;
			if (needInstanceUpdate) {
				rayTracedInstance.m_entityHandle = entityHandle;
				rayTracedInstance.m_privateComponentHandle = skinnedMeshRenderer->GetHandle().GetValue();
				rayTracedInstance.m_version = skinnedMeshRenderer->GetVersion();
				rayTracedInstance.m_globalTransform = globalTransform;
				rayTracedInstance.m_geometryMapKey = geometryHandle;
				rayTracedInstance.m_materialMapKey = materialHandle;
			}
			updateShaderBindingTable = updateShaderBindingTable || needMaterialUpdate;
			rebuildInstances = rebuildInstances || needInstanceUpdate;
		}
	}
	if (const auto* rayTracedEntities =
		scene->UnsafeGetPrivateComponentOwnersList<Particles>();
		rayTracedEntities && m_renderParticles) {
		for (auto entity : *rayTracedEntities) {
			if (!scene->IsEntityEnabled(entity))
				continue;
			auto particles =
				scene->GetOrSetPrivateComponent<Particles>(entity).lock();
			if (!particles->IsEnabled())
				continue;
			auto mesh = particles->m_mesh.Get<Mesh>();
			auto material = particles->m_material.Get<Material>();
			auto particleInfoList = particles->m_particleInfoList.Get<ParticleInfoList>();
			if (!material || !mesh || !particleInfoList || mesh->UnsafeGetVertices().empty() || particleInfoList->m_particleInfos.empty())
				continue;
			auto globalTransform = scene->GetDataComponent<GlobalTransform>(entity).m_value;
			bool needInstanceUpdate = false;
			bool needMaterialUpdate = false;

			auto entityHandle = scene->GetEntityHandle(entity);
			auto geometryHandle = particles->GetHandle().GetValue();
			auto materialHandle = material->GetHandle();
			auto& rayTracedInstance = instanceStorage[geometryHandle];
			auto& rayTracedGeometry = geometryStorage[geometryHandle];
			auto& rayTracedMaterial = materialStorage[materialHandle];
			rayTracedInstance.m_removeFlag = false;
			rayTracedMaterial.m_removeFlag = false;
			rayTracedGeometry.m_removeFlag = false;

			if (rayTracedInstance.m_entityHandle != entityHandle
				|| rayTracedInstance.m_privateComponentHandle != particles->GetHandle().GetValue()
				|| rayTracedInstance.m_version != particles->GetVersion()
				|| rayTracedInstance.m_dataVersion != particleInfoList->GetVersion()
				|| globalTransform != rayTracedInstance.m_globalTransform) {
				needInstanceUpdate = true;
			}
			if (needInstanceUpdate || rayTracedGeometry.m_handle == 0
				|| rayTracedInstance.m_version != particles->GetVersion()
				|| rayTracedGeometry.m_version != mesh->GetVersion()) {
				rayTracedGeometry.m_updateFlag = true;
				needInstanceUpdate = true;
				rayTracedGeometry.m_geometryType = PrimitiveType::Triangle;
				rayTracedGeometry.m_rendererType = RendererType::Instanced;
				rayTracedGeometry.m_triangles = &mesh->UnsafeGetTriangles();
				rayTracedGeometry.m_vertices = &mesh->UnsafeGetVertices();
				rayTracedGeometry.m_instanceMatrices = reinterpret_cast<std::vector<InstanceMatrix>*>(&particleInfoList->m_particleInfos);
				rayTracedGeometry.m_version = mesh->GetVersion();
				rayTracedGeometry.m_handle = geometryHandle;
				rayTracedInstance.m_dataVersion = particleInfoList->GetVersion();
			}
			if (CheckMaterial(rayTracedMaterial, material)) needInstanceUpdate = true;
			if (needInstanceUpdate) {
				rayTracedInstance.m_entityHandle = entityHandle;
				rayTracedInstance.m_privateComponentHandle = particles->GetHandle().GetValue();
				rayTracedInstance.m_version = particles->GetVersion();
				rayTracedInstance.m_globalTransform = globalTransform;
				rayTracedInstance.m_geometryMapKey = geometryHandle;
				rayTracedInstance.m_materialMapKey = materialHandle;
			}
			updateShaderBindingTable = updateShaderBindingTable || needMaterialUpdate;
			rebuildInstances = rebuildInstances || needInstanceUpdate;
		}
	}
	if (const auto* rayTracedEntities =
		scene->UnsafeGetPrivateComponentOwnersList<BTFMeshRenderer>();
		rayTracedEntities && m_renderBTFMeshRenderer) {
		for (auto entity : *rayTracedEntities) {
			if (!scene->IsEntityEnabled(entity))
				continue;
			auto meshRenderer =
				scene->GetOrSetPrivateComponent<BTFMeshRenderer>(entity).lock();
			if (!meshRenderer->IsEnabled())
				continue;
			auto mesh = meshRenderer->m_mesh.Get<Mesh>();
			auto material = meshRenderer->m_btf.Get<CompressedBTF>();
			if (!material || !material->m_bTFBase.m_hasData || !mesh || mesh->UnsafeGetVertices().empty())
				continue;
			auto globalTransform = scene->GetDataComponent<GlobalTransform>(entity).m_value;
			bool needInstanceUpdate = false;
			bool needMaterialUpdate = false;

			auto entityHandle = scene->GetEntityHandle(entity);
			auto geometryHandle = mesh->GetHandle();
			auto materialHandle = material->GetHandle();
			auto& rayTracedInstance = instanceStorage[meshRenderer->GetHandle().GetValue()];
			auto& rayTracedGeometry = geometryStorage[geometryHandle];
			auto& rayTracedMaterial = materialStorage[materialHandle];
			rayTracedInstance.m_removeFlag = false;
			rayTracedMaterial.m_removeFlag = false;
			rayTracedGeometry.m_removeFlag = false;

			if (rayTracedInstance.m_entityHandle != entityHandle
				|| rayTracedInstance.m_privateComponentHandle != meshRenderer->GetHandle().GetValue()
				|| rayTracedInstance.m_version != meshRenderer->GetVersion()
				|| globalTransform != rayTracedInstance.m_globalTransform) {
				needInstanceUpdate = true;
			}
			if (rayTracedGeometry.m_handle == 0 || rayTracedGeometry.m_version != mesh->GetVersion()) {
				rayTracedGeometry.m_updateFlag = true;
				needInstanceUpdate = true;
				rayTracedGeometry.m_rendererType = RendererType::Default;
				rayTracedGeometry.m_triangles = &mesh->UnsafeGetTriangles();
				rayTracedGeometry.m_vertices = &mesh->UnsafeGetVertices();
				rayTracedGeometry.m_version = mesh->GetVersion();
				rayTracedGeometry.m_geometryType = PrimitiveType::Triangle;
				rayTracedGeometry.m_handle = geometryHandle;
			}
			if (CheckCompressedBTF(rayTracedMaterial, material)) needInstanceUpdate = true;
			if (needInstanceUpdate) {
				rayTracedInstance.m_entityHandle = entityHandle;
				rayTracedInstance.m_privateComponentHandle = meshRenderer->GetHandle().GetValue();
				rayTracedInstance.m_version = meshRenderer->GetVersion();
				rayTracedInstance.m_globalTransform = globalTransform;
				rayTracedInstance.m_geometryMapKey = geometryHandle;
				rayTracedInstance.m_materialMapKey = materialHandle;
			}
			updateShaderBindingTable = updateShaderBindingTable || needMaterialUpdate;
			rebuildInstances = rebuildInstances || needInstanceUpdate;
		}
	}

	for (auto& i : instanceStorage) if (i.second.m_removeFlag) rebuildInstances = true;
}

void RayTracerLayer::UpdateScene() {
	bool rebuildAccelerationStructure = false;
	bool updateShaderBindingTable = false;
	auto& instanceStorage = CudaModule::GetRayTracer()->m_instances;
	auto& materialStorage = CudaModule::GetRayTracer()->m_materials;
	auto& geometryStorage = CudaModule::GetRayTracer()->m_geometries;
	UpdateMeshesStorage(materialStorage, geometryStorage, instanceStorage, rebuildAccelerationStructure,
		updateShaderBindingTable);
	auto& envSettings = GetScene()->m_environment;
	bool useEnvMap = envSettings.m_environmentType == EnvironmentType::EnvironmentalMap;
	if(m_environmentProperties.m_useEnvironmentalMap != useEnvMap)
	{
		m_environmentProperties.m_useEnvironmentalMap = useEnvMap;
		updateShaderBindingTable = true;
	}
	if(m_environmentalMapHandle != envSettings.m_environmentalMap.GetAssetHandle())
	{
		m_environmentalMapHandle = envSettings.m_environmentalMap.GetAssetHandle();
		if (auto envMap = envSettings.m_environmentalMap.Get<EnvironmentalMap>()) {
			const auto reflectionProbe = envMap->m_reflectionProbe.Get<ReflectionProbe>();
			if (reflectionProbe) {
				m_environmentalMapImage = CudaModule::ImportCubemap(reflectionProbe->GetCubemap());
				m_environmentProperties.m_environmentalMap = m_environmentalMapImage->m_textureObject;
			}
		}else
		{
			envMap = Resources::GetResource<EnvironmentalMap>("DEFAULT_ENVIRONMENTAL_MAP");
			const auto reflectionProbe = envMap->m_reflectionProbe.Get<ReflectionProbe>();
			m_environmentalMapImage = CudaModule::ImportCubemap(reflectionProbe->GetCubemap());
			m_environmentProperties.m_environmentalMap = m_environmentalMapImage->m_textureObject;
		}
		updateShaderBindingTable = true;
	}
	if (envSettings.m_backgroundColor != m_environmentProperties.m_color) {
		m_environmentProperties.m_color = envSettings.m_backgroundColor;
		updateShaderBindingTable = true;
	}
	if (m_environmentProperties.m_skylightIntensity != envSettings.m_ambientLightIntensity) {
		m_environmentProperties.m_skylightIntensity = envSettings.m_ambientLightIntensity;
		updateShaderBindingTable = true;
	}
	if (m_environmentProperties.m_gamma != envSettings.m_environmentGamma) {
		m_environmentProperties.m_gamma = envSettings.m_environmentGamma;
		updateShaderBindingTable = true;
	}

	CudaModule::GetRayTracer()->m_requireUpdate = false;
	if (rebuildAccelerationStructure &&
		(!instanceStorage.empty())) {
		CudaModule::GetRayTracer()->BuildIAS();
		CudaModule::GetRayTracer()->m_requireUpdate = true;
	}
	else if (updateShaderBindingTable) {
		CudaModule::GetRayTracer()->m_requireUpdate = true;
	}
}

void RayTracerLayer::OnCreate() {
	CudaModule::Init();
	ClassRegistry::RegisterPrivateComponent<BTFMeshRenderer>(
		"BTFMeshRenderer");
	ClassRegistry::RegisterPrivateComponent<TriangleIlluminationEstimator>(
		"TriangleIlluminationEstimator");
	ClassRegistry::RegisterPrivateComponent<RayTracerCamera>(
		"RayTracerCamera");
	ClassRegistry::RegisterPrivateComponent<BasicPointCloudScanner>(
		"BasicPointCloudScanner");
	ClassRegistry::RegisterAsset<CompressedBTF>(
		"CompressedBTF", { ".cbtf" });

	m_sceneCamera = Serialization::ProduceSerializable<RayTracerCamera>();
	m_sceneCamera->OnCreate();
	Application::RegisterPostAttachSceneFunction([&](const std::shared_ptr<Scene>& scene) {
		m_rayTracerCamera.reset();
		});
}


void RayTracerLayer::PreUpdate() {
	UpdateScene();
	if (!CudaModule::GetRayTracer()->m_instances.empty()) {
		auto editorLayer = Application::GetLayer<EditorLayer>();
		if (m_showSceneWindow && editorLayer && m_renderingEnabled) {
			m_sceneCamera->Ready(editorLayer->GetSceneCameraPosition(), editorLayer->GetSceneCameraRotation());
			Graphics::ImmediateSubmit([&](VkCommandBuffer commandBuffer)
				{
					m_sceneCamera->m_rendered = CudaModule::GetRayTracer()->RenderToCamera(m_environmentProperties,
					m_sceneCamera->m_cameraProperties,
					m_sceneCamera->m_rayProperties);
				});
		}
		auto scene = GetScene();
		auto* entities = scene->UnsafeGetPrivateComponentOwnersList<RayTracerCamera>();
		m_rayTracerCamera.reset();
		if (entities) {
			bool check = false;
			for (const auto& entity : *entities) {
				if (!scene->IsEntityEnabled(entity)) continue;
				auto rayTracerCamera = scene->GetOrSetPrivateComponent<RayTracerCamera>(entity).lock();
				if (!rayTracerCamera->IsEnabled()) continue;
				auto globalTransform = scene->GetDataComponent<GlobalTransform>(rayTracerCamera->GetOwner()).m_value;
				rayTracerCamera->Ready(globalTransform[3], glm::quat_cast(globalTransform));
				Graphics::ImmediateSubmit([&](VkCommandBuffer commandBuffer)
					{
						rayTracerCamera->m_rendered = CudaModule::GetRayTracer()->RenderToCamera(m_environmentProperties,
						rayTracerCamera->m_cameraProperties,
						rayTracerCamera->m_rayProperties);
					});


				if (!check) {
					if (rayTracerCamera->m_mainCamera) {
						m_rayTracerCamera = rayTracerCamera;
						check = true;
					}
				}
				else {
					rayTracerCamera->m_mainCamera = false;
				}
			}
		}
	}

}

void RayTracerLayer::OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			if (ImGui::BeginMenu("Editor"))
			{
				if (ImGui::BeginMenu("Scene"))
				{
					ImGui::Checkbox("Show Scene (RT) Window", &m_showSceneWindow);
					if (m_showSceneWindow)
					{
						ImGui::Checkbox("Show Scene (RT) Window Info", &m_showSceneInfo);
					}
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Camera"))
				{
					ImGui::Checkbox("Show Camera (RT) Window", &m_showCameraWindow);
					ImGui::EndMenu();
				}
				ImGui::Checkbox("Ray Tracer Settings", &m_showRayTracerSettingsWindow);
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	if (m_showRayTracerSettingsWindow) {
		if (ImGui::Begin("Ray Tracer Settings")) {
			ImGui::Checkbox("Mesh Renderer", &m_renderMeshRenderer);
			ImGui::Checkbox("Strand Renderer", &m_renderStrandsRenderer);
			ImGui::Checkbox("Particles", &m_renderParticles);
			ImGui::Checkbox("Skinned Mesh Renderer", &m_renderSkinnedMeshRenderer);
			ImGui::Checkbox("BTF Mesh Renderer", &m_renderBTFMeshRenderer);

			if (ImGui::TreeNode("Scene Camera Settings")) {
				m_sceneCamera->OnInspect(editorLayer);
				ImGui::TreePop();
			}
			if (ImGui::TreeNodeEx("Environment Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
				m_environmentProperties.OnInspect();
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}
	if (m_showCameraWindow) RayCameraWindow();
	if (m_showSceneWindow) SceneCameraWindow();
}

void RayTracerLayer::OnDestroy() { CudaModule::Terminate(); }

void RayTracerLayer::SceneCameraWindow() {
	const auto editorLayer = Application::GetLayer<EditorLayer>();
	if (!editorLayer) return;
	auto sceneCameraRotation = editorLayer->GetSceneCameraRotation();
	auto sceneCameraPosition = editorLayer->GetSceneCameraPosition();
	const auto scene = GetScene();
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	if (ImGui::Begin("Scene (RT)")) {
		if (ImGui::BeginChild("RaySceneRenderer", ImVec2(0, 0), false)) {
			static int corner = 1;
			const ImVec2 overlayPos = ImGui::GetWindowPos();

			ImVec2 viewPortSize = ImGui::GetWindowSize();
			m_sceneCameraResolution = glm::ivec2(viewPortSize.x, viewPortSize.y);
			if (m_sceneCamera->m_allowAutoResize)
				m_sceneCamera->m_frameSize = glm::vec2(viewPortSize.x, viewPortSize.y) * m_resolutionMultiplier;
			if (m_sceneCamera->m_rendered) {
				ImGui::Image(m_sceneCamera->m_renderTexture->GetColorImTextureId(),
					ImVec2(viewPortSize.x, viewPortSize.y), ImVec2(0, 1), ImVec2(1, 0));
				editorLayer->CameraWindowDragAndDrop();
			}
			else
				ImGui::Text("No mesh in the scene!");

			const auto windowPos = ImVec2(
				(corner & 1) ? (overlayPos.x + viewPortSize.x) : (overlayPos.x),
				(corner & 2) ? (overlayPos.y + viewPortSize.y) : (overlayPos.y));
			if (m_showSceneInfo) {
				const auto windowPosPivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
				ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, windowPosPivot);
				ImGui::SetNextWindowBgAlpha(0.35f);
				constexpr auto windowFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking |
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_NoFocusOnAppearing;
				if (ImGui::BeginChild("Info", ImVec2(300, 300), true, windowFlags)) {
					ImGui::Text("Info & Settings");
					ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
					std::string drawCallInfo = {};
					ImGui::PushItemWidth(100);
					ImGui::DragFloat("Resolution multiplier", &m_resolutionMultiplier,
						0.01f, 0.1f, 1.0f);
					m_sceneCamera->m_cameraProperties.OnInspect();
					m_sceneCamera->m_rayProperties.OnInspect();
					ImGui::PopItemWidth();
				}
				ImGui::EndChild();
			}

			auto mousePosition = glm::vec2(FLT_MAX, FLT_MIN);
			if (ImGui::IsWindowFocused()) {
				auto mp = ImGui::GetMousePos();
				auto wp = ImGui::GetWindowPos();
				mousePosition = glm::vec2(mp.x - wp.x, mp.y - wp.y);
				static bool isDraggingPreviously = false;
				bool mouseDrag = true;
#pragma region Scene Camera Controller
				if (mousePosition.x < 0 || mousePosition.y < 0 || mousePosition.x > viewPortSize.x ||
					mousePosition.y > viewPortSize.y ||
					editorLayer->GetKey(GLFW_MOUSE_BUTTON_RIGHT) != KeyActionType::Hold) {
					mouseDrag = false;
				}
				static float prevX = 0;
				static float prevY = 0;
				if (mouseDrag && !isDraggingPreviously) {
					prevX = mousePosition.x;
					prevY = mousePosition.y;
				}
				const float xOffset = mousePosition.x - prevX;
				const float yOffset = mousePosition.y - prevY;
				prevX = mousePosition.x;
				prevY = mousePosition.y;
				isDraggingPreviously = mouseDrag;
				if (mouseDrag && !editorLayer->m_lockCamera) {

					glm::vec3 front =
						sceneCameraRotation *
						glm::vec3(0, 0, -1);
					const glm::vec3 right =
						sceneCameraRotation *
						glm::vec3(1, 0, 0);
					if (editorLayer->GetKey(GLFW_KEY_W) == KeyActionType::Hold) {
						sceneCameraPosition +=
							front * static_cast<float>(Times::DeltaTime()) *
							editorLayer->m_velocity;
					}
					if (editorLayer->GetKey(GLFW_KEY_S) == KeyActionType::Hold) {
						sceneCameraPosition -=
							front * static_cast<float>(Times::DeltaTime()) *
							editorLayer->m_velocity;
					}
					if (editorLayer->GetKey(GLFW_KEY_A) == KeyActionType::Hold) {
						sceneCameraPosition -=
							right * static_cast<float>(Times::DeltaTime()) *
							editorLayer->m_velocity;
					}
					if (editorLayer->GetKey(GLFW_KEY_D) == KeyActionType::Hold) {
						sceneCameraPosition +=
							right * static_cast<float>(Times::DeltaTime()) *
							editorLayer->m_velocity;
					}
					if (editorLayer->GetKey(GLFW_KEY_LEFT_SHIFT) == KeyActionType::Hold) {
						sceneCameraPosition.y +=
							editorLayer->m_velocity *
							static_cast<float>(Times::DeltaTime());
					}
					if (editorLayer->GetKey(GLFW_KEY_LEFT_CONTROL) == KeyActionType::Hold) {
						sceneCameraPosition.y -=
							editorLayer->m_velocity *
							static_cast<float>(Times::DeltaTime());
					}
					if (xOffset != 0.0f || yOffset != 0.0f) {
						front = glm::rotate(front, glm::radians(-xOffset * editorLayer->m_sensitivity), glm::vec3(0, 1, 0));
						const glm::vec3 right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
						if ((front.y < 0.99f && yOffset < 0.0f) || (front.y > -0.99f && yOffset > 0.0f)) {
							front = glm::rotate(front, glm::radians(-yOffset * editorLayer->m_sensitivity), right);
						}
						const glm::vec3 up = glm::normalize(glm::cross(right, front));
						sceneCameraRotation = glm::quatLookAt(front, up);
					}
					editorLayer->SetCameraPosition(editorLayer->GetSceneCamera(), sceneCameraPosition);
					editorLayer->SetCameraRotation(editorLayer->GetSceneCamera(), sceneCameraRotation);
				}
#pragma endregion
			}
		}
		ImGui::EndChild();
		auto* window = ImGui::FindWindowByName("Scene (RT)");
		m_renderingEnabled = !(window->Hidden && !window->Collapsed);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void RayTracerLayer::RayCameraWindow() {
	auto editorLayer = Application::GetLayer<EditorLayer>();
	if (!editorLayer) return;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	if (ImGui::Begin("Camera (RT)")) {
		if (ImGui::BeginChild("RayCameraRenderer", ImVec2(0, 0), false,
			ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar)) {
			ImVec2 viewPortSize = ImGui::GetWindowSize();
			if (m_rayTracerCamera) {
				if (m_rayTracerCamera->m_allowAutoResize)
					m_rayTracerCamera->m_frameSize = glm::vec2(viewPortSize.x, viewPortSize.y);
				if (m_rayTracerCamera->m_rendered) {
					ImGui::Image(m_rayTracerCamera->m_renderTexture->GetColorImTextureId(),
						ImVec2(viewPortSize.x, viewPortSize.y), ImVec2(0, 1), ImVec2(1, 0));
					editorLayer->CameraWindowDragAndDrop();
				}
				else
					ImGui::Text("No mesh in the scene!");
			}
			else {
				ImGui::Text("No camera attached!");
			}
		}
		ImGui::EndChild();
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void RayTracerLayer::Update() {
	if (m_showCameraWindow) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		if (ImGui::Begin("Camera (RT)")) {
			if (ImGui::BeginChild("RayCameraRenderer", ImVec2(0, 0), false,
				ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar)) {
				if (m_rayTracerCamera && m_rayTracerCamera->m_rendered && ImGui::IsWindowFocused()) {
					Application::GetLayer<EditorLayer>()->m_mainCameraFocusOverride = true;
				}
			}
			ImGui::EndChild();
		}
		ImGui::End();
		ImGui::PopStyleVar();
	}
}

bool
RayTracerLayer::CheckMaterial(RayTracedMaterial& rayTracerMaterial, const std::shared_ptr<Material>& material) const {
	bool changed = false;
	if (rayTracerMaterial.m_materialType == MaterialType::Default && material->m_vertexColorOnly) {
		changed = true;
		rayTracerMaterial.m_materialType = MaterialType::VertexColor;
	}
	else if (rayTracerMaterial.m_materialType == MaterialType::VertexColor && !material->m_vertexColorOnly) {
		changed = true;
		rayTracerMaterial.m_materialType = MaterialType::Default;
	}

	if (changed || rayTracerMaterial.m_version != material->GetVersion()) {
		rayTracerMaterial.m_handle = material->GetHandle();
		rayTracerMaterial.m_version = material->GetVersion();
		rayTracerMaterial.m_materialProperties = material->m_materialProperties;
		
		const auto albedoTexture = material->GetAlbedoTexture();
		if (albedoTexture &&
			albedoTexture->GetVkImage() != VK_NULL_HANDLE) {
			rayTracerMaterial.m_albedoTexture = CudaModule::ImportTexture2D(albedoTexture);
		}
		else {
			rayTracerMaterial.m_albedoTexture = nullptr;
		}
		const auto normalTexture = material->GetNormalTexture();
		if (normalTexture &&
			normalTexture->GetVkImage() != VK_NULL_HANDLE) {
			rayTracerMaterial.m_normalTexture = CudaModule::ImportTexture2D(normalTexture);
		}
		else {
			rayTracerMaterial.m_normalTexture = nullptr;
		}
		const auto roughnessTexture = material->GetRoughnessTexture();
		if (roughnessTexture &&
			roughnessTexture->GetVkImage() != VK_NULL_HANDLE) {
			rayTracerMaterial.m_roughnessTexture = CudaModule::ImportTexture2D(roughnessTexture);
		}
		else {
			rayTracerMaterial.m_roughnessTexture = nullptr;
		}
		const auto metallicTexture = material->GetMetallicTexture();
		if (metallicTexture &&
			metallicTexture->GetVkImage() != VK_NULL_HANDLE) {
			rayTracerMaterial.m_metallicTexture = CudaModule::ImportTexture2D(metallicTexture);
		}
		else {
			rayTracerMaterial.m_metallicTexture = nullptr;
		}
		
		changed = true;
	}

	return changed;
}

bool RayTracerLayer::CheckCompressedBTF(RayTracedMaterial& rayTracerMaterial,
	const std::shared_ptr<CompressedBTF>& compressedBtf) const {
	bool changed = false;
	if (rayTracerMaterial.m_materialType != MaterialType::CompressedBTF) {
		changed = true;
		rayTracerMaterial.m_materialType = MaterialType::CompressedBTF;
	}
	if (rayTracerMaterial.m_version != compressedBtf->m_version) {
		changed = true;
		rayTracerMaterial.m_version = compressedBtf->m_version;
		rayTracerMaterial.m_btfBase = &compressedBtf->m_bTFBase;
	}
	return changed;
}

glm::ivec2 RayTracerLayer::GetSceneCameraResolution() const
{
	return m_sceneCameraResolution;
}

