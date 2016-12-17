#include <include/views/cef_browser_view.h>
#include <include/views/cef_window.h>
#include <include/wrapper/cef_helpers.h>
#include <include/cef_parser.h>
#include <include/cef_task.h>
#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>
#include <include/cef_life_span_handler.h>
#include <include/internal/cef_types_wrappers.h>
#include <include/internal/cef_win.h>
#include <include/internal/cef_types.h>
#include <include/cef_origin_whitelist.h>
#include <include/cef_browser.h>
#include <include/cef_command_line.h>

#ifdef _DEBUG
#pragma comment(lib, "libcef_Debug_x64.lib")
#pragma comment(lib, "libcef_dll_wrapper_Debug_x64.lib")
#else
#pragma comment(lib, "libcef_Release_x64.lib")
#pragma comment(lib, "libcef_dll_wrapper_Release_x64.lib")
#endif

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow) {
	CefMainArgs main_args(hInstance);
	return CefExecuteProcess(main_args, nullptr, nullptr);
}