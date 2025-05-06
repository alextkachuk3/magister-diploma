#include "Dx12GlobalContext.h"

Dx12GlobalContext::Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height) : GlobalContext(hInstance, windowTitle, width, height) {}

void Dx12GlobalContext::Run()
{
	isRunning = true;
	while (isRunning)
	{
		ProcessSystemMessages();
	}
}
