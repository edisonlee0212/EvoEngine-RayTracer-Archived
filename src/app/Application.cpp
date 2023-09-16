// PlantFactory.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <Application.hpp>
#include <EditorLayer.hpp>
#include "WindowLayer.hpp"
#include "RenderLayer.hpp"
#ifdef BUILD_RAYTRACER
#include <RayTracerLayer.hpp>
#include "BTFMeshRenderer.hpp"
#include "TriangleIlluminationEstimator.hpp"
#endif


#ifdef BUILD_RAYTRACER
using namespace RayTracerFacility;
#endif

int main() {
    const bool enableRayTracing = true;
    ApplicationInfo applicationInfo;

    Application::PushLayer<WindowLayer>();
    Application::PushLayer<EditorLayer>();
    Application::PushLayer<RenderLayer>();
#ifdef BUILD_RAYTRACER
    Application::PushLayer<RayTracerLayer>();
#endif
    Application::Initialize(applicationInfo);

#pragma region Engine Loop
    Application::Start();
    Application::Run();
#pragma endregion
    Application::End();
}
