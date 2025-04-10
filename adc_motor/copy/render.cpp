#include "render.h"

#include ".\include\imgui\imgui.h"
#include ".\include\imgui\imconfig.h"
#include ".\include\imgui\imgui_impl_dx12.h"
#include ".\include\imgui\imgui_impl_win32.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

using namespace std;


// Data
static int const                    NUM_FRAMES_IN_FLIGHT = 3;
static render::FrameContext         g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT                         g_frameIndex = 0;

static int const                    NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = nullptr;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = nullptr;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = nullptr;
static ID3D12CommandQueue* g_pd3dCommandQueue = nullptr;
static ID3D12GraphicsCommandList* g_pd3dCommandList = nullptr;
static ID3D12Fence* g_fence = nullptr;
static HANDLE                       g_fenceEvent = nullptr;
static UINT64                       g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = nullptr;
static HANDLE                       g_hSwapChainWaitableObject = nullptr;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};
static float buffer[8192];
static float x_line[10000];
static bool showCursor;
static bool autoset;
static float wt_vt = 1.0;

static float t{};
static render::ScrollingBuffer dataAnalog;

ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

render::render() {
    for (int i = 0; i < (sizeof(x_line)/sizeof(float)); i++)
        x_line[i] = i;
    npoints_global = 1024;
    samples_global = 512;
    try
    {
        init_window_class();
    }
    catch (const std::exception& e)
    {
        string excpt_data = e.what();
        MessageBox(nullptr, wstring(begin(excpt_data), end(excpt_data)).c_str(), L"������", MB_ICONERROR | MB_OK);
        ExitProcess(EXIT_FAILURE);
    }
}


// Initialisation of ImGui and Win32 window class
void render::init_window_class()
{
    // Create application window
    // HWND hwnd{};
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.lpfnWndProc = render::app_proc;
    wcex.lpszClassName = L"MyAppClass";
    wcex.lpszMenuName = nullptr;
    wcex.style = CS_CLASSDC;

    if (!RegisterClassEx(&wcex))
        throw runtime_error("Error, class not registered!"s);

    RECT _windowRC{ 0, 0, 1200, 800};
    AdjustWindowRect(&_windowRC, WS_OVERLAPPEDWINDOW, false);

    if (m_hWnd = CreateWindow(wcex.lpszClassName, L"Temp title", WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZE | WS_OVERLAPPEDWINDOW,
        (GetSystemMetrics(SM_CXSCREEN) - _windowRC.right) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - _windowRC.bottom) / 2,
        _windowRC.right, _windowRC.bottom, nullptr, nullptr, wcex.hInstance, this); !this->m_hWnd)
        throw runtime_error("Error, can't create main window!"s);

    // Initialize Direct3D
    if (!CreateDeviceD3D(m_hWnd))
    {
        CleanupDeviceD3D();
        UnregisterClassW(wcex.lpszClassName, wcex.hInstance);
    }

    // Show the window
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);

    // Create context and set flags
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(m_hWnd);
    ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
        g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
}

LRESULT CALLBACK render::app_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    render* pRender = NULL;
    if (uMsg == WM_NCCREATE)
    {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pRender = static_cast<render*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pRender));
        pRender->m_hWnd = hWnd;
    }
    else
    {
        pRender = reinterpret_cast<render*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }
    if (pRender)
    {
        return pRender->wnd_proc(hWnd, uMsg, wParam, lParam);
    }
    else return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK render::wnd_proc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    switch (uMsg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED)
        {
            WaitForLastSubmittedFrame();
            CleanupRenderTarget();
            HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
            assert(SUCCEEDED(result) && "Failed to resize swapchain.");
            CreateRenderTarget();
        }
        return 0;
    case WM_DESTROY:
        //close();
        PostQuitMessage(EXIT_SUCCESS);
        return 0;
    /*case WM_COMMAND:
    {
        switch (static_cast<render::CTL_ID>(LOWORD(wParam)))
        {
        case render::CTL_ID::PLACEHOLDER:
        {
            wstringstream wss;
            //wss << frame();
            SetWindowText(this->m_hWndEdit, wss.str().c_str());
        }
        return 0;
        }
    }
    return 0;
   // case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(hWnd, &ps);
    }
   // return 0;
    case WM_TIMER:
        switch (static_cast<render::TIMER_ID>(wParam)) {
        case render::TIMER_ID::IDT_TIMER:
        {
            KillTimer(this->m_hWnd, static_cast<UINT_PTR>(render::TIMER_ID::IDT_TIMER));
            //display();
        }
        return 0;
        }
        return 0;*/
    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);

    }
}
void render::demo() {
    bool show_demo_window = true;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
        ImPlot::ShowDemoWindow(&show_demo_window);
    }
}

void render::scrolling(float data) {
    static int counter = 0;
    t += ImGui::GetIO().DeltaTime;
    dataAnalog.AddPoint(t, data);

    ImGui::InputFloat("Watt/volt coefficient", &wt_vt, 0.1f, 0.1f, "%.3f", ImGuiInputTextFlags_None); //ImGuiInputTextFlags_CharsDecimal
    double k_wtvt_max = wt_vt * 10;
    double k_wtvt_min = wt_vt * 0;
    ImPlotAxisFlags flagsx = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Foreground;
    ImPlotAxisFlags flagsy1 = ImPlotAxisFlags_Foreground | ImPlotAxisFlags_Lock;
    ImPlotAxisFlags flagsy2 = ImPlotAxisFlags_Foreground | ImPlotAxisFlags_AuxDefault | ImPlotAxisFlags_Lock;
    if (ImPlot::BeginPlot("##Waveform"))
    {
        ImPlot::SetupAxis(ImAxis_X1, "Time, s", flagsx);
        ImPlot::SetupAxisLimits(ImAxis_X1, t - 10.0, t, 2);

        ImPlot::SetupAxis(ImAxis_Y1, "Volt", flagsy1);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 10);

        ImPlot::SetupAxis(ImAxis_Y2, "Watt", flagsy2);
        ImPlot::SetupAxisLimits(ImAxis_Y2, 0, k_wtvt_max);
        ImPlot::SetupAxisLinks(ImAxis_Y2, &k_wtvt_min, &k_wtvt_max);
        ImPlot::PlotLine("", &dataAnalog.Data[0].x, &dataAnalog.Data[0].y, dataAnalog.Data.size(), 0, dataAnalog.Offset, 2 * sizeof(float));
        ImPlot::EndPlot();
    }

    ImGui::Text("Voltage = %f", data);
}

void render::oscilloscope(std::vector<float> data, int samples) {
    static int start = 0;
    start = 0;
    while (!((data[start] < 0) && (data[start + 2] >= 0))) {
        start++;
        if (start == data.size() - (data.size())/2) {
            start = 0;
            break;
        }
    }
    ImPlotAxisFlags flags = ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Foreground;
    if (ImPlot::BeginPlot("##Oscilloscope")) {
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, 10, 2);
        ImPlot::SetupAxis(ImAxis_X1, "Time, s", flags);
        ImPlot::SetupAxisLimits(ImAxis_Y1, *min_element(data.begin(), data.end()), *max_element(data.begin(), data.end()));
        ImPlot::PlotLine("", &x_line[0], &data[start + 2], samples, 0, NULL, sizeof(float));
        if (showCursor) {
            static double drag_tag = data[0];
            static double drag_tag1 = data[0] + 0.1;
            if (autoset) {
                drag_tag = *max_element(data.begin(), data.end());
                drag_tag1 = *min_element(data.begin(), data.end());
            }
            ImPlot::DragLineY(0, &drag_tag, ImVec4(1, 0, 0, 1), 1, ImPlotDragToolFlags_NoFit);
            ImPlot::TagY(drag_tag, ImVec4(1, 0, 0, 1), "%f", drag_tag);
            ImPlot::DragLineY(1, &drag_tag1, ImVec4(0, 0, 1, 1), 1, ImPlotDragToolFlags_NoFit);
            ImPlot::TagY(drag_tag1, ImVec4(0, 0, 1, 1), "%f", drag_tag1);
        }
        ImPlot::EndPlot();
    }
    const char* items_samples[] = { "32", "64", "128", "256", "512"};
    static int current_sample = 4;
    ImGui::Combo("Samples #", &current_sample, items_samples, IM_ARRAYSIZE(items_samples));

    switch (current_sample) {
    case 0:
        samples_global = 32;
        break;
    case 1:
        samples_global = 64;
        break;
    case 2:
        samples_global = 128;
        break;
    case 3:
        samples_global = 256;
        break;
    case 4:
        samples_global = 512;
        break;
    default:
        npoints_global = 512;
        break;
    }

    ImGui::Checkbox("Show cursors", &showCursor);
    if (!showCursor) {
        ImGui::BeginDisabled();
    }
    ImGui::Checkbox("Autoset cursors", &autoset);
    if (!showCursor) {
        ImGui::EndDisabled();
    }
}

void render::fourier(std::vector<float>& data, int npoints) {
    static float x_data[4096];
    if (data.size() < npoints)
        return;
    for (int i = 0; i < data.size(); i++) {
        if (data[i] < 0) data[i] *= -1;
    }
    for (int i = 0; i < npoints/2-1; i++) {
        //data[i] = sqrtf(2 * powf(data[2*i] / npoints, 2) + powf(data[2*i+1] / npoints, 2));
        //data[i] = (((2*data[2*i] + data[2*i+1]) / npoints)/sqrtf(2));
        //data[i] = ((data[i*2] + data[i*2+1])/npoints)/sqrtf(2);
    }

    for (int i = 0; i < (sizeof(x_data) / sizeof(float)); i++)
        x_data[i] = i*1024/npoints;

    ImPlotAxisFlags flags = ImPlotAxisFlags_Foreground;
    if (ImPlot::BeginPlot("##Spectrum"))
    {
        ImPlot::SetupAxisLimits(ImAxis_X1, 0, 512);
        ImPlot::SetupAxis(ImAxis_X1, "Frequency, Hz", flags);
        ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1);//*max_element(&data[0], &data[npoints]));
        ImPlot::SetupAxis(ImAxis_Y1, "Volt RMS", flags);
        //ImPlot::PlotLine("Voltage", &x_line[0], &data[0], npoints, flags, NULL, sizeof(float));
        ImPlot::PlotStems("", &x_data[0], &data[0], npoints/2 - 1, NULL, flags, 0, sizeof(float));
        ImPlot::EndPlot();
    }
    const char* items_npoints[] = { "256", "512", "1024" };//, "2048", "4096"};
    static int current_npoint = 2;
    ImGui::Combo("FFT points", &current_npoint, items_npoints, IM_ARRAYSIZE(items_npoints));
    switch (current_npoint) {
    case 0:
        npoints_global = 256;
        break;
    case 1:
        npoints_global = 512;
        break;
    case 2:
        npoints_global = 1024;
        break;
/*  case 3:
        npoints_global = 2048;
        break;
    case 4:
        npoints_global = 4096;
        break;*/
    default:
        npoints_global = 1024;
        break;
    }

    const char* items_windows[] = { "Rectangular", "Hamming", "Flat top" };
    ImGui::Combo("Window", &current_window, items_windows, IM_ARRAYSIZE(items_windows));
}

void render::d3dctx() {
    render::FrameContext* frameCtx = WaitForNextFrameResources();
    UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
    frameCtx->CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    g_pd3dCommandList->Reset(frameCtx->CommandAllocator, nullptr);
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Render Dear ImGui graphics
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], clear_color_with_alpha, 0, nullptr);
    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, nullptr);
    g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

    g_pSwapChain->Present(1, 0); // Present with vsync
    //g_pSwapChain->Present(0, 0); // Present without vsync

    UINT64 fenceValue = g_fenceLastSignaledValue + 1;
    g_pd3dCommandQueue->Signal(g_fence, fenceValue);
    g_fenceLastSignaledValue = fenceValue;
    frameCtx->FenceValue = fenceValue;
}

void settext(HWND hwnd, wstring str) 
{
    SetWindowText(hwnd, (LPCWSTR)str.c_str());
    SetTimer(hwnd, static_cast<UINT_PTR>(render::TIMER_ID::IDT_TIMER), 10, (TIMERPROC)NULL);
}

bool render::CreateDeviceD3D(HWND hwnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC1 sd;
    {
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = NUM_BACK_BUFFERS;
        sd.Width = 0;
        sd.Height = 0;
        sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        sd.Scaling = DXGI_SCALING_STRETCH;
        sd.Stereo = FALSE;
    }

    // [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
    ID3D12Debug* pdx12Debug = nullptr;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
        pdx12Debug->EnableDebugLayer();
#endif

    // Create device
    D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
    if (D3D12CreateDevice(nullptr, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
        return false;

    // [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
    if (pdx12Debug != nullptr)
    {
        ID3D12InfoQueue* pInfoQueue = nullptr;
        g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        pInfoQueue->Release();
        pdx12Debug->Release();
    }
#endif

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        desc.NumDescriptors = NUM_BACK_BUFFERS;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
            return false;

        SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
        for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        {
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    {
        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        desc.NumDescriptors = 1;
        desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
            return false;
    }

    {
        D3D12_COMMAND_QUEUE_DESC desc = {};
        desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 1;
        if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
            return false;
    }

    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
            return false;

    if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, nullptr, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
        g_pd3dCommandList->Close() != S_OK)
        return false;

    if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
        return false;

    g_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (g_fenceEvent == nullptr)
        return false;

    {
        IDXGIFactory4* dxgiFactory = nullptr;
        IDXGISwapChain1* swapChain1 = nullptr;
        if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
            return false;
        if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, m_hWnd, &sd, nullptr, nullptr, &swapChain1) != S_OK)
            return false;
        if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
            return false;
        swapChain1->Release();
        dxgiFactory->Release();
        g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
        g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
    }

    CreateRenderTarget();
    return true;
}

void render::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, nullptr); g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_hSwapChainWaitableObject != nullptr) { CloseHandle(g_hSwapChainWaitableObject); }
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
        if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = nullptr; }
    if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = nullptr; }
    if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = nullptr; }
    if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = nullptr; }
    if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = nullptr; }
    if (g_fence) { g_fence->Release(); g_fence = nullptr; }
    if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void render::create_controls()
{
    using std::runtime_error;
    if (this->m_hWndButton = CreateWindowEx(0, L"BUTTON", L"����", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 0, 100, 300, 30, this->m_hWnd, reinterpret_cast<HMENU>(render::CTL_ID::PLACEHOLDER), nullptr, nullptr); !this->m_hWndButton)
        throw runtime_error("Can't create button"s);
    if (this->m_hWndEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, ES_READONLY | ES_CENTER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 0, 70, 300, 30, this->m_hWnd, reinterpret_cast<HMENU>(render::CTL_ID::TEXTFIELD), nullptr, nullptr); !this->m_hWndEdit)
        throw runtime_error("Can't create edit"s);
}

void render::CleanupRenderTarget()
{
    WaitForLastSubmittedFrame();

    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
        if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = nullptr; }
}

void render::CreateRenderTarget()
{
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
    {
        ID3D12Resource* pBackBuffer = nullptr;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, g_mainRenderTargetDescriptor[i]);
        g_mainRenderTargetResource[i] = pBackBuffer;
    }
}

void render::WaitForLastSubmittedFrame()
{
    FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue == 0)
        return; // No fence was signaled

    frameCtx->FenceValue = 0;
    if (g_fence->GetCompletedValue() >= fenceValue)
        return;

    g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
    WaitForSingleObject(g_fenceEvent, INFINITE);
}

render::FrameContext* render::WaitForNextFrameResources()
{
    UINT nextFrameIndex = g_frameIndex + 1;
    g_frameIndex = nextFrameIndex;

    HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, nullptr };
    DWORD numWaitableObjects = 1;

    FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->FenceValue;
    if (fenceValue != 0) // means no fence was signaled
    {
        frameCtx->FenceValue = 0;
        g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
        waitableObjects[1] = g_fenceEvent;
        numWaitableObjects = 2;
    }

    WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

    return frameCtx;
}

void render::cleanup(LPCWSTR classname)
{
    WaitForLastSubmittedFrame();
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    ImPlot::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(m_hWnd);
    UnregisterClassW(classname, NULL);
}

render::~render() {}