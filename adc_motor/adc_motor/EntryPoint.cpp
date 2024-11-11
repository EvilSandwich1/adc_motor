#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#pragma warning (disable : 4996)
#include "ADC.h"
#include "render.h"
#include "stepper.h"

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
stepper motor{};
bool ShowColormapSelector(const char* label);
static bool connectDone = false;
static bool connectDoneMotor = false;
char output[1024] = { 0 };
char cmd[1024] = { 0 };

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void ShowAboutBox(bool* p_open) {
	if (!ImGui::Begin("About", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::End();
		return;
	}
	ImGui::Dummy(ImVec2(70.0f, 20.0f));
	ImGui::SameLine();
	ImGui::Text("Version 1.0");
	/*ImGui::Text("CPU handle = %p", ren.my_texture_srv_cpu_handle.ptr);
	ImGui::Text("GPU handle = %p", ren.my_texture_srv_gpu_handle.ptr);
	ImGui::Text("size = %d x %d", ren.my_image_width, ren.my_image_height);*/
	// Note that we pass the GPU SRV handle here, *not* the CPU handle. We're passing the internal pointer value, cast to an ImTextureID
	ImGui::Image((ImTextureID)ren.my_texture_srv_gpu_handle.ptr, ImVec2((float)ren.my_image_width, (float)ren.my_image_height));
	ImGui::End();
}

void ShowEditStyle(bool* p_open_style) {
	if (!ImGui::Begin("Edit styles", p_open_style, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::End();
		return;
	}
	const char* items_styles[] = { "Light", "Dark", "Classic" };
	static int current_style = 1;
	ImGui::Combo("Style", &current_style, items_styles, IM_ARRAYSIZE(items_styles));

	switch (current_style) {
	case 0:
		ImGui::StyleColorsLight();
		ImPlot::StyleColorsLight();
		break;
	case 1:
		ImGui::StyleColorsDark();
		ImPlot::StyleColorsDark();
		break;
	case 2:
		ImGui::StyleColorsClassic();
		ImPlot::StyleColorsClassic();
		break;
	default:
		ImGui::StyleColorsLight();
		ImPlot::StyleColorsLight();
		break;
	}

	ShowColormapSelector("Line Style");

	ImGui::End();
}

bool ShowColormapSelector(const char* label) {
	ImPlotContext& gp = *GImPlot;
	bool set = false;
	if (ImGui::BeginCombo(label, gp.ColormapData.GetName(gp.Style.Colormap))) {
		for (int i = 0; i < gp.ColormapData.Count; ++i) {
			const char* name = gp.ColormapData.GetName(i);
			if (ImGui::Selectable(name, gp.Style.Colormap == i)) {
				gp.Style.Colormap = i;
				ImPlot::BustItemCache();
				set = true;
			}
		}
		ImGui::EndCombo();
	}
	return set;
}

int main() {
	static float f = 0.0f;
	std::vector<float>& buffer_long = adc.buffer_long;
	std::vector<float> tmp_buffer(1024, 0);
	std::vector<float> tmp_fourier(1024, 0);
	static int npoints = 1024;
	static int samples = 512;
	setlocale(LC_ALL, "en_us.utf8");

	static bool p_open = false;
	static bool p_open_ovr = false;
	static bool p_open_style = false;

	StpCoord current_coord = { 0 };

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

		if (ren.isConnected && !connectDone) {
			if (adc.init_e440() && adc.stream_setup()) {
				connectDone = true;
			}
			else {
				ren.isConnected = false;
				connectDone = false;
			}
		}
		if (ren.disconnect) {
			adc.disconnect();
			ren.disconnect = false;
			connectDone = false;
		}

		if (ren.isMotorConnected && !connectDoneMotor) {
			if (motor.initialize())
				connectDoneMotor = true;
			else {
				ren.isMotorConnected = false;
				connectDoneMotor = false;
			}
		}
		if (ren.disconnectMotor) {
			motor.close();
			ren.disconnectMotor = false;
			connectDoneMotor = false;
		}
		
		float avg = 0;
		if (ren.isConnected) {
			avg = adc.data_avg(adc.data_proc());
		}
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		bool show_demo_window = true;
		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
			ImPlot::ShowDemoWindow(&show_demo_window);
		}

		if (p_open) {
			ShowAboutBox(&p_open);
		}
		if (p_open_style) {
			ShowEditStyle(&p_open_style);
		}

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Quit")) {
					goto quit;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Edit styles", NULL, &p_open_style);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("About"))
			{
				ImGui::MenuItem("About", NULL, &p_open);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		static bool use_work_area = true;
		static ImGuiWindowFlags flags = NULL;//ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBringToFrontOnFocus;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
		ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

		ImGui::Begin("Main", NULL, flags);
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_Reorderable;
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		if (ImGui::BeginTabBar("Plots/plots"), tab_bar_flags) {
			if (ImGui::BeginTabItem("Oscilloscope")) {
				if (ren.isConnected) {
					if (adc.ready_swap) {
						tmp_buffer = buffer_long;
					}
				}
				if (tmp_buffer.size() <= 3) {
					tmp_buffer.resize(32);
					memset(&tmp_buffer[0], 0, 32 * sizeof tmp_buffer[0]);
				}
				ren.oscilloscope(tmp_buffer, samples);
				ren.connect();
				//char* cmd = ren.stepper_input();
				memset(cmd, 0, 1024);
				strcpy(cmd, ren.stepper_input());
				strcat_s(cmd, "\n");
				assert(sizeof(cmd) == 1024);
				if (ren.sendCmd) {
					motor.write_cmd(cmd);
					memset(output, 0, 1024 * sizeof(char));
					Sleep(100);
					strcpy(output, motor.read());
				}
				ren.stepper_output(output);
				if(ImGui::Button("Update"))
					current_coord = motor.get_current_coord();
				ImGui::Text("x = %f", current_coord.x); ImGui::SameLine();
				ImGui::Text("y = %f", current_coord.y); ImGui::SameLine();
				ImGui::Text("z = %f", current_coord.z);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Fourier")) {
				if (ren.isConnected) {
					adc.pffft(npoints, ren.current_window);
					tmp_fourier = adc.pffft_buffer;
				}
				if (tmp_fourier.size() <= 3) {
					tmp_fourier.resize(32);
					memset(&tmp_fourier[0], 0, 32 * sizeof tmp_fourier[0]);
				}
				ren.fourier(tmp_fourier, npoints);
				ren.connect();
				char* cmd = ren.stepper_input();
				if (ren.sendCmd) {
					motor.write_cmd(cmd);
					ren.stepper_output(motor.read());
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Scrolling")) {
				ren.scrolling(avg);
				ren.connect();
				char* cmd = ren.stepper_input();
				if (ren.sendCmd) {
					motor.write_cmd(cmd);
					ren.stepper_output(motor.read());
				}
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
	quit:
	ren.cleanup(L"MyAppClass");
	if (ren.isConnected) {
		adc.disconnect();
	}
	motor.close();
	return 0;
}