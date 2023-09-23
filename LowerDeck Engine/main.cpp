#include "Engine/Engine/Engine.hpp"


_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	Engine* app{ new Engine(hInstance) };
	try
	{
		app->Initialize();
		app->Run();
		app->Release();
	}
	catch (...)
	{
		delete app;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
