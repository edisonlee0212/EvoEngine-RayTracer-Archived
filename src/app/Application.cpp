// PlantFactory.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <Application.hpp>
#include <EditorLayer.hpp>
#include "WindowLayer.hpp"
#include "RenderLayer.hpp"
#include <RayTracerLayer.hpp>
#include "BTFMeshRenderer.hpp"
#include "TriangleIlluminationEstimator.hpp"

using namespace EvoEngine;

int main() {
    ApplicationInfo applicationInfo;

    Application::PushLayer<WindowLayer>();
    Application::PushLayer<EditorLayer>();
    Application::PushLayer<RenderLayer>();
    Application::PushLayer<RayTracerLayer>();
    Application::Initialize(applicationInfo);

#pragma region Engine Loop
    Application::Start();
    Application::Run();
#pragma endregion
    Application::End();
}
