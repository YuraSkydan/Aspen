#include "Engine.h"
#include "../Log/Log.h"
#include "../Input/Input.h"
#include "../ScriptManager.h"
#include "../Core/Time.h"

#include <chrono>

Engine::Engine()
{
	Log::Init(); // maybe move it somewhere else
	INFO("Engine Start");

	s_Instance = this;
	m_Window = std::make_unique<Window>();

	InitImGui();

	Input::Init();
}

void Engine::Run()
{
	while (m_Running)
	{
		auto start = std::chrono::high_resolution_clock::now();

		glClear(GL_COLOR_BUFFER_BIT);
		
		Time::FrameStart();
		m_Editor->Update();

		ImGuiBegin();
		m_Editor->ImGuiRender();
		ImGuiEnd();

		m_Window->Update();
		Time::FrameEnd();

		//change latter
		m_Running = !glfwWindowShouldClose(m_Window->GetNativeWindow());
		auto end = std::chrono::high_resolution_clock::now();

		std::chrono::duration<double> duration = end - start;
		std::string title = "Aspen " + std::to_string(1.0f / duration.count());
		glfwSetWindowTitle(m_Window->GetNativeWindow(), title.c_str());
	}
}

void Engine::Close()
{
	m_Running = false;
}

void Engine::Set(Engine* engine)
{
	s_Instance = engine;
}