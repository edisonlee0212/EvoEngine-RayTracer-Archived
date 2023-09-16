//
// Created by lllll on 9/3/2021.
//

#include "BTFMeshRenderer.hpp"

using namespace EvoEngine;

#include "EditorLayer.hpp"
#include "CompressedBTF.hpp"
#include "Mesh.hpp"
using namespace EvoEngine;

void BTFMeshRenderer::OnInspect(const std::shared_ptr<EditorLayer>& editorLayer) {
    editorLayer->DragAndDropButton<Mesh>(m_mesh, "Mesh");
    editorLayer->DragAndDropButton<CompressedBTF>(m_btf, "CompressedBTF");
}

void BTFMeshRenderer::Serialize(YAML::Emitter &out) {
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