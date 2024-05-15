#pragma once
#include "evoengine-pch.hpp"
#include "IPrivateComponent.hpp"

using namespace EvoEngine;
namespace EvoEngine {
    class BTFMeshRenderer : public IPrivateComponent {
    public:
        AssetRef m_mesh;
        AssetRef m_btf;

        bool OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) override;
        void Serialize(YAML::Emitter &out) const override;
        void Deserialize(const YAML::Node &in) override;

        void CollectAssetRef(std::vector<AssetRef> &list) override;
    };
} // namespace EvoEngine
