// PlantFactory.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <Application.hpp>
#include <EditorLayer.hpp>
#include "WindowLayer.hpp"
#include "PhysicsLayer.hpp"
#include "RenderLayer.hpp"
#ifdef RAYTRACERFACILITY
#include <RayTracerLayer.hpp>
#include "BTFMeshRenderer.hpp"
#include "TriangleIlluminationEstimator.hpp"
#endif


#ifdef RAYTRACERFACILITY
using namespace RayTracerFacility;
#endif

int main() {
    const bool enableRayTracing = true;
    ApplicationInfo applicationInfo;

    Application::PushLayer<WindowLayer>();
    //Application::PushLayer<PhysicsLayer>();
    Application::PushLayer<EditorLayer>();
    Application::PushLayer<RenderLayer>();
#ifdef RAYTRACERFACILITY
    Application::PushLayer<RayTracerLayer>();
#endif
    Application::Initialize(applicationInfo);

#pragma region Engine Loop
    Application::Start();
    Application::Run();
#pragma endregion
    Application::End();
}
