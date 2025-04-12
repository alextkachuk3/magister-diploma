#pragma once

#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define InvalidCodePath Assert(!"Invalid Code Path")
