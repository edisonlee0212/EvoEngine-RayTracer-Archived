#pragma once
#include "EvoEngine-pch.hpp"
#include "IPrivateComponent.hpp"

using namespace EvoEngine;
namespace RayTracerFacility {
    class BTFMeshRenderer : public IPrivateComponent {
    public:
        AssetRef m_mesh;
        AssetRef m_btf;

        void OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) override;
        void Serialize(YAML::Emitter &out) override;
        void Deserialize(const YAML::Node &in) override;

        void CollectAssetRef(std::vector<AssetRef> &list) override;
    };
} // namespace RayTracerFacility
