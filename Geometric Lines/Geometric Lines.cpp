#include "framework.h"
#include "Geometric Lines.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
ATOM MyRegisterClass(HINSTANCE hInstance);
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef struct {
	ImVec2 pos;
	int type;
}Node;

Node node[400];

int n_node = 80;
int node_size = 5;
int dis = 100;
int delay = 5;

bool setting_menu = false;
double clockToMilliseconds(clock_t ticks) {
	return (ticks / (double)CLOCKS_PER_SEC) * 1000.0;
}

float DistanceOfPointToPoint(ImVec2 p1, ImVec2 p2) {
	return (float)sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2));
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_GEOMETRICLINES, szWindowClass, MAX_LOADSTRING);

	RECT desktop;
	GetWindowRect(GetDesktopWindow(), &desktop);

	MyRegisterClass(hInstance);

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_EX_DLGMODALFRAME,
		(desktop.right - 1280) / 2, (desktop.bottom - 800) / 2, 1280, 800, nullptr, nullptr, hInstance, nullptr);

	if (!CreateDeviceD3D(hWnd))
	{
		CleanupDeviceD3D();
		UnregisterClassW(szWindowClass, hInstance);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);
	io.Fonts->AddFontDefault();
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	io.Fonts->AddFontFromFileTTF("fa-solid-900.ttf", 50.0f, &icons_config, icons_ranges);

	for (int i = 0; i < 400; i++) {
		node[i].pos.x = -100 + rand() % 1481;
		node[i].pos.y = -100 + rand() % 1001;
		node[i].type = 1 + rand() % 8;
	}

	clock_t beginFrame = clock();

	MSG msg;
	ZeroMemory(&msg, sizeof(msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		const ImGuiViewport* vp = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(vp->WorkPos);
		ImGui::SetNextWindowSize(vp->WorkSize);
		ImGui::Begin("GL", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);

		ImGui::SetCursorPos(ImVec2(vp->WorkSize.x - 80, vp->WorkSize.y - 50));
		ImGui::Text(ICON_FA_COG);
		ImGui::SetCursorPos(ImVec2(vp->WorkSize.x - 80, vp->WorkSize.y - 84));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.000f, 0.004f, 0.000f, 0.000f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.000f, 0.004f, 0.000f, 0.000f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.000f, 0.004f, 0.000f, 0.000f));
		if (ImGui::Button("###", ImVec2(50, 50))) setting_menu = true;

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		if (setting_menu) {
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 12.0f);
			if (ImGui::Begin("Setting", &setting_menu, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::SliderInt("Node", &n_node, 20, 400);
				ImGui::SliderInt("Node Size", &node_size, 1, 20);
				ImGui::SliderInt("Distance", &dis, 20, 500);
				ImGui::SliderInt("Move Delay", &delay, 0, 20);
				ImGui::End();
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
			ImGui::PopStyleVar();
		}

		ImDrawList* draw_list = ImGui::GetWindowDrawList();


		for (int i = 0; i < n_node; i++) {

			draw_list->AddCircleFilled(node[i].pos, node_size, IM_COL32(255, 255, 255, 255));
			for (int j = 0; j < n_node; j++)
				if (DistanceOfPointToPoint(node[j].pos, node[i].pos) < (float)dis)
					draw_list->AddLine(node[i].pos, node[j].pos, IM_COL32(255, 255, 255, 255));
		}

		clock_t endFrame = clock();
		if (clockToMilliseconds(endFrame - beginFrame) > (double)delay) {
			beginFrame = clock();
			for (int i = 0; i < n_node; ++i) {
				if (node[i].pos.x > 1380 || node[i].pos.x < -100 || node[i].pos.y < -100 || node[i].pos.y > 900) {
					node[i].pos.x = -100 + rand() % 1481;
					node[i].pos.y = -100 + rand() % 1001;
					node[i].type = 1 + rand() % 8;
				}
				else {
					if (node[i].type == 1) {
						node[i].pos.x++;
					}
					else if (node[i].type == 2) {
						node[i].pos.x--;
					}
					else if (node[i].type == 3) node[i].pos.y++;
					else if (node[i].type == 4) node[i].pos.y--;
					else if (node[i].type == 5) {
						node[i].pos.x++;
						node[i].pos.y++;
					}
					else if (node[i].type == 6) {
						node[i].pos.x--;
						node[i].pos.y--;
					}
					else if (node[i].type == 7) {
						node[i].pos.x++;
						node[i].pos.y--;
					}
					else if (node[i].type == 8) {
						node[i].pos.x--;
						node[i].pos.y++;
					}
				}
			}
		}

		ImGui::End();

		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}

		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(hWnd);
	UnregisterClassW(szWindowClass, hInstance);
	return (int)msg.wParam;
}



ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GEOMETRICLINES));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}
