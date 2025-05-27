#include "CpuGlobalContext.h"
#include "Dx12GlobalContext.h"
#include <algorithm>

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	std::string commandLine(lpCmdLine);

	std::unique_ptr<GlobalContext> globalContext;

	if (commandLine.find("--cpu") != std::string::npos)
	{
		globalContext = std::make_unique<CpuGlobalContext>(hInstance, "Render CPU", 1280, 720);
	}
	else
	{
		globalContext = std::make_unique<Dx12GlobalContext>(hInstance, "Render DX12", 1920, 1080);
	}

	std::vector<std::pair<std::string, std::string>> modelsTexturesPaths;

	modelsTexturesPaths.emplace_back("./assets/sponza/Sponza.gltf", "./assets/sponza/textures/");

	globalContext->Run(modelsTexturesPaths);

	return 0;
}
