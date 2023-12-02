#include "Application.hpp"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	Application* app{ new Application(hInstance) };
	app->Initialize();
	app->Run();
	app->Release();
	delete app;

	// TODO:
	// Create Window in THIS project and just pass pointer to it to Engine and Editor
	
	// - Init D3D
	// - Pass D3D to create actual ImGui-powerd window
	// - Enable Editor if in editor mode
	// - Init Renderer layer
	// - ???
	// - Profit

	return 0;
}

