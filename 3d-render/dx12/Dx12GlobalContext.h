#pragma once

#include "GlobalContext.h"

class Dx12GlobalContext : public GlobalContext
{
public:
	Dx12GlobalContext(HINSTANCE hInstance, const char* windowTitle, int width, int height);

	void Run() override;
};
