#include <Windows.h>
#include <Shobjidl.h>
#include <Shlobj.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LPITEMIDLIST PidlBrowse(HWND hwnd, int nCSIDL, LPWSTR pszDisplayName);
void openDialogBox();
HWND initWindow(HINSTANCE hInstance);
PCWSTR CLASS_NAME = L"Test Class";
HCURSOR cursor;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR pCmdLine, int nCmdShow)
{
	HWND hWnd = initWindow(hInstance);

	if (hWnd == NULL)
	{
		return 0;
	}
	ShowWindow(hWnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;

}

HWND initWindow(HINSTANCE hInstance)
{
	cursor = LoadCursorFromFileA("aero_arrow.cur");
	WNDCLASS wc = {};
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hCursor = cursor;
	
	RegisterClass(&wc);

	HWND wnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Learn to Program Windows",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
		NULL, // Parent Window
		NULL, // Menu
		hInstance,
		NULL // Additional App data
	);
	return wnd;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SETCURSOR:
		SetCursor(cursor);
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
			EndPaint(hWnd, &ps);
		}
		return 0;
	case WM_RBUTTONDOWN:

		openDialogBox();
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void openDialogBox()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileDialog* pFileOpen;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		if (SUCCEEDED(hr))
		{
			pFileOpen->SetOptions(FOS_PICKFOLDERS);
			hr = pFileOpen->Show(NULL);
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						MessageBox(NULL, pszFilePath, L"File Path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
				pFileOpen->Release();
			}
			CoUninitialize();
		}
	}
}

LPITEMIDLIST PidlBrowse(HWND hwnd, int nCSIDL, LPWSTR pszDisplayName)
{
	LPITEMIDLIST pidlRoot = NULL;
	LPITEMIDLIST pidlSelected = NULL;
	BROWSEINFO bi = { 0 };

	if (nCSIDL)
	{
		SHGetFolderLocation(hwnd, nCSIDL, NULL, NULL, &pidlRoot);
	}

	else
	{
		pidlRoot = NULL;
	}

	bi.hwndOwner = hwnd;
	bi.pidlRoot = pidlRoot;
	bi.pszDisplayName = pszDisplayName;
	bi.lpszTitle = L"Choose a folder";
	bi.ulFlags = 0;
	bi.lpfn = NULL;
	bi.lParam = 0;

	pidlSelected = SHBrowseForFolder(&bi);

	if (pidlRoot)
	{
		CoTaskMemFree(pidlRoot);
	}

	return pidlSelected;
}