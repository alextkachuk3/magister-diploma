#pragma once

#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define AssertMsg(Msg) { OutputDebugStringA(Msg); Assert(false);}
