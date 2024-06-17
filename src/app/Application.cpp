// PlantFactory.cpp : This file contains the 'main' function. Program execution
// begins and ends there.
//
#include <Application.hpp>
#include <EditorLayer.hpp>
#include <RayTracerLayer.hpp>
#include "RenderLayer.hpp"
#include "WindowLayer.hpp"

using namespace evo_engine;

int main() {
  const ApplicationInfo application_info;

  Application::PushLayer<WindowLayer>();
  Application::PushLayer<EditorLayer>();
  Application::PushLayer<RenderLayer>();
  Application::PushLayer<RayTracerLayer>();
  Application::Initialize(application_info);

#pragma region Engine Loop
  Application::Start();
  Application::Run();
#pragma endregion
  Application::End();
}
