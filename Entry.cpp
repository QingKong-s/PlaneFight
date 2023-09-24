#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"D2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"dxguid.lib")

#include "Entry.h"
#include "Resource.h"
#include "CApp.h"
#include "CWndMain.h"

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR pszCmdLine, _In_ int nCmdShow)
{
	CoInitialize(NULL);

	App = new CApp;
	App->Init(hInstance);

	CWndMain::RegisterWndClass();
	CWndMain Wnd{};

	const DWORD dwStyle = WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_VISIBLE | WS_OVERLAPPEDWINDOW;
	const DWORD dwExStyle = 0u;
	
	const int iDpi = GetDpiForSystem();
	RECT rc{ 0,0,512 * iDpi / 96,768 * iDpi / 96 };
	AdjustWindowRectEx(&rc, dwStyle, FALSE, dwExStyle);
	Wnd.Create(L"PlaneFight", 100, 100, rc.right, rc.bottom, dwStyle, dwExStyle);

	MSG msg;
	while (TRUE)
	{
		if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;
			else
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
			}
		}
		else
		{
			Wnd.Tick();
			Utility::Sleep(30);
		}
	}

	delete App;
	CoUninitialize();
	return (int)msg.wParam;
}