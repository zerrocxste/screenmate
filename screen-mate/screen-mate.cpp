#include "includes.h"

const ImVec2 WindowSize{ 60, 30 };

void InitImGui();

void DestroyImGui();

void WndProcHandlerCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WndProcMouseMoveCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WndProcLButtonDownCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WndProcLButtonUpCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void RenderCallback();

void BeginSceneCallback();

void ResetCallback();

LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

int __stdcall WinMain(HINSTANCE hinst, HINSTANCE prevhinst, LPSTR lpcmd, int cmd)
{
	std::srand(time(NULL));

	m_pScene = std::make_unique<CScene>();

	auto question_result = MessageBoxW(NULL, L"Load cat images from memory or use your own?", L"", MB_ICONQUESTION | MB_YESNO);

	if (question_result == IDYES)
		m_pScene->use_image_in_memory = true;

	process_enumerator::create_process_enumerator_thread();

	DXWFInitialization(hinst);

	DXWFWndProcCallbacks(DXWF_WNDPROC_WNDPROCHANDLER_, WndProcHandlerCallback);
	DXWFWndProcCallbacks(DXWF_WNDPROC_WM_MOUSEMOVE_, WndProcMouseMoveCallback);
	DXWFWndProcCallbacks(DXWF_WNDPROC_WM_LBUTTONDOWN_, WndProcLButtonDownCallback);
	DXWFWndProcCallbacks(DXWF_WNDPROC_WM_LBUTTONUP_, WndProcLButtonUpCallback);

	DXWFRendererCallbacks(DXWF_RENDERER_RESET_, ResetCallback);
	DXWFRendererCallbacks(DXWF_RENDERER_LOOP_, RenderCallback);
	DXWFRendererCallbacks(DXWF_RENDERER_BEGIN_SCENE_LOOP_, BeginSceneCallback);

	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);
	int x = screen_rect.right / 2 - WindowSize.x / 2;
	int y = screen_rect.bottom / 2 - WindowSize.y / 2;

	if (DXWFCreateWindow("Screen mate",
		x, y,
		WindowSize.x, WindowSize.y,
		WS_POPUP,
		WS_EX_TOPMOST | WS_EX_TRANSPARENT, //WS_EX_LAYERED
		user_dx_flags::ENABLE_WINDOW_ALPHA))
	{
		InitImGui();
		DXWFRenderLoop();
	}

	DestroyImGui();

	DXWFTerminate();

	m_pScene.release();
}

void InitImGui()
{
	ImGui::CreateContext();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 28.9f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	ImGui_ImplDX9_Init(DXWFGetD3DDevice());
	ImGui_ImplWin32_Init(DXWFGetHWND());

	ImGui::StyleColorsLight();
	ImGui::GetStyle().AntiAliasedFill = true;
	ImGui::GetStyle().AntiAliasedLines = true;
}

void DestroyImGui()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void WndProcHandlerCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam);
}

POINT g_ptMousePos;
bool g_bIsMouseCaptured = false;

void WndProcMouseMoveCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_bIsMouseCaptured)
	{
		RECT rc;
		POINT ptCursor;
		POINT ptDelta;

		GetWindowRect(hWnd, &rc);
		GetCursorPos(&ptCursor);
		ptDelta.x = g_ptMousePos.x - ptCursor.x;
		ptDelta.y = g_ptMousePos.y - ptCursor.y;

		MoveWindow(hWnd, rc.left - ptDelta.x, rc.top - ptDelta.y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
		g_ptMousePos.x = ptCursor.x;
		g_ptMousePos.y = ptCursor.y;
	}
}

void WndProcLButtonDownCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!g_bIsMouseCaptured)
	{
		SetCapture(hWnd);
		g_bIsMouseCaptured = true;
		GetCursorPos(&g_ptMousePos);
	}
}

void WndProcLButtonUpCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_bIsMouseCaptured)
	{
		ReleaseCapture();
		g_bIsMouseCaptured = false;
	}
}

void RenderCallback()
{
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(1.f, 1.f, 1.f, 0.f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::Begin("##BackBuffer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);
	ImGui::SetWindowPos(ImVec2(), ImGuiCond_Always);
	ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

	m_pScene->Run();

	ImGui::GetCurrentWindow()->DrawList->PushClipRectFullScreen();
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	ImGui::EndFrame();
}

void BeginSceneCallback()
{
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void ResetCallback()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}