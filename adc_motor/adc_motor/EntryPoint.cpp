#include "ADC.h"
#include "render.h"

#include ".\include\imgui\imgui.h"
#include ".\include\imgui\imconfig.h"
#include ".\include\imgui\imgui_impl_dx12.h"
#include ".\include\imgui\imgui_impl_win32.h"
#include ".\include\imgui\imgui_internal.h"
#include ".\include\imgui\imgui_stdlib.h"
#include <d3d12.h>
#pragma comment(lib, "d3d12")
#pragma comment(lib, "dxgi")
#include <dxgi1_4.h>
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

ADC adc{};
render ren{};


int main() {

	static float f = 0.0f;
	std::vector<float>& buffer_long = adc.buffer_long;
	std::vector<float> tmp_buffer(1024, 0);
	static int npoints = 1024;
	static int samples = 512;
	setlocale(LC_ALL, "en_us.utf8");

	// Main loop
	bool done = false;
	while (!done)
	{
		MSG msg;
		npoints = ren.npoints_global;
		samples = ren.samples_global;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) // message dispatching
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;
		//float* data = adc.frame();
		float avg = adc.data_avg(adc.data_proc());
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool show_demo_window = true;
		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
			ImPlot::ShowDemoWindow(&show_demo_window);
		}

		ImGui::Begin("Main");
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		if (ImGui::BeginTabBar("Plots/plots"), tab_bar_flags) {
			if (ImGui::BeginTabItem("Scrolling")) {
				ren.scrolling(avg);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Oscilloscope")) {
				if (adc.ready_swap) {
					tmp_buffer = buffer_long;
				}
				ren.oscilloscope(tmp_buffer, samples);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Fourier")) {
				adc.pffft(npoints, ren.current_window);
				std::vector<float>& tmp = adc.pffft_buffer;
				ren.fourier(tmp, npoints);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
		ImGui::Render();
		ren.d3dctx();

		//ren.paint(avg);
		//Sleep(1);
		
	}
	ren.cleanup(L"MyAppClass");
	return 0;
}