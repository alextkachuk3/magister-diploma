#include <Windows.h>
#include <cmath>
#include <vector>
#include "Utils.h"
#include "GlobalContext.h"
#include "V2.h"
#include "V3.h"
#include "M4.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
    GlobalContext globalContext(hInstance, "Render", 1920, 1080);

	globalContext.Run();

	globalContext.ReleaseResources();

	return 0;
}
