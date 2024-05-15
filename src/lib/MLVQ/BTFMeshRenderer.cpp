//
// Created by lllll on 9/3/2021.
//

#include "BTFMeshRenderer.hpp"

using namespace EvoEngine;

#include "EditorLayer.hpp"
#include "CompressedBTF.hpp"
#include "Mesh.hpp"
using namespace EvoEngine;

bool BTFMeshRenderer::OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) {
    bool changed = false;

	if(editorLayer->DragAndDropButton<Mesh>(m_mesh, "Mesh")) changed = true;
    if(editorLayer->DragAndDropButton<CompressedBTF>(m_btf, "CompressedBTF")) changed = true;

    return changed;
}

void BTFMeshRenderer::Serialize(YAML::Emitter &out) const {
    m_mesh.Save("m_mesh", out);
    m_btf.Save("m_btf", out);
}

void BTFMeshRenderer::Deserialize(const YAML::Node &in) {
    m_mesh.Load("m_mesh", in);
    m_btf.Load("m_btf", in);
}

void BTFMeshRenderer::CollectAssetRef(std::vector<AssetRef> &list) {
    list.push_back(m_mesh);
    list.push_back(m_btf);
}