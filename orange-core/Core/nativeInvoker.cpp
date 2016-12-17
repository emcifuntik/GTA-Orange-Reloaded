#include "stdafx.h"

void NativeInvoke::Invoke(NativeContext * cxt, uint64_t hash)
{
	auto fn = ScriptEngine::GetNativeHandler(hash);
	if (fn != 0)
		fn(cxt);
}