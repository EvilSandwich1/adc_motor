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
#include <shobjidl.h>
#include <windows.h>

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
AlgCoord algorithm_coord = { 0 };
int delay_g = 0;
std::thread motorThread;
std::thread algorithmThread;
std::thread algorithmSmartThread;
std::jthread adcInitThread;
ImGuiContext& g = *GImGui;
std::future<bool> futureAl;
std::future<bool> futureAlSmart;
std::future<bool> futureInit;
std::thread futureJust;

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

bool algoThread(AlgCoord &coord, std::shared_ptr<std::promise<bool>> promise, int delay) {
	//if (g.GroupStack.Size > 0)
	//	ImGui::EndGroup();
	bool res = motor.algorithm(coord, adc, delay);
	promise->set_value(res);
	return res;
}

bool algoSmartThread(AlgCoord& coord, std::shared_ptr<std::promise<bool>> promise, int delay) {
	//if (g.GroupStack.Size > 0)
	//	ImGui::EndGroup();
	bool res = motor.algorithm_smart(coord, adc, delay);
	promise->set_value(res);
	return res;
}

StpCoord MotorControlPanel(StpCoord current_coord) {
	static int counter_10hz = 0;
	std::string cmd;

	cmd = ren.stepper_input();
	cmd += "\n";

	if (ren.sendCmd) {
		motor.write_cmd(cmd);
		memset(output, 0, 1024 * sizeof(char));
		Sleep(100);
		strcpy(output, motor.read());
		Sleep(100);
	}
	ren.stepper_output(output);

	if (connectDoneMotor) {
		++counter_10hz;
		if (counter_10hz >= 4) {
			current_coord = motor.get_current_coord();
			counter_10hz = 0;
		}
	}
	ImGui::Text("x = %f", current_coord.x); ImGui::SameLine();
	ImGui::Text("y = %f", current_coord.y); ImGui::SameLine();
	ImGui::Text("z = %f", current_coord.z);
	//„ÛÔÔ‡ ‰‚ËÊÂÌËÈ
	ImGui::BeginGroup();
	ImGui::Text("X:");
	if (ImGui::Button("Left X")) {
		motor.move(-1.0f, "x");
	} ImGui::SameLine();
	if (ImGui::Button("Right X")) {
		motor.move(1.0f, "x");
	}
	ImGui::Text("Y:");
	if (ImGui::Button("Left Y")) {
		motor.move(-1.0f, "y");
	} ImGui::SameLine();
	if (ImGui::Button("Right Y")) {
		motor.move(1.0f, "y");
	}
	ImGui::Text("Z:");
	if (ImGui::Button("Left Z")) {
		motor.move(-1.0f, "z");
	} ImGui::SameLine();
	if (ImGui::Button("Right Z")) {
		motor.move(1.0f, "z");
	}
	ImGui::EndGroup();

	ImGui::SameLine();
	//„ÛÔÔ‡ Á‡‰‡˜Ë ÍÓÓ‰ËÌ‡Ú
	ImGui::BeginGroup();

	ImGui::Text("");
	ImGui::PushItemWidth(50);

	ImGui::InputFloat("Begin X", &algorithm_coord.begin_x);
	ImGui::SameLine();
	ImGui::InputFloat("Step X", &algorithm_coord.step_x);
	ImGui::SameLine();
	ImGui::InputFloat("End X", &algorithm_coord.end_x);

	ImGui::SameLine();
	ImGui::InputInt("Delay", &delay_g, 0);

	ImGui::InputFloat("Begin Y", &algorithm_coord.begin_y);
	ImGui::SameLine();
	ImGui::InputFloat("Step Y", &algorithm_coord.step_y);
	ImGui::SameLine();
	ImGui::InputFloat("End Y", &algorithm_coord.end_y);

	ImGui::InputFloat("Begin Z", &algorithm_coord.begin_z);
	ImGui::SameLine();
	ImGui::InputFloat("Step Z", &algorithm_coord.step_z);
	ImGui::SameLine();
	ImGui::InputFloat("End Z", &algorithm_coord.end_z);

	ImGui::PopItemWidth();
	ImGui::EndGroup();

	if (ImGui::Button("Start measurement")) {
		if (algorithm_coord.begin_x < algorithm_coord.end_x)
			return current_coord;
		if (algorithm_coord.begin_y < algorithm_coord.end_y)
			return current_coord;
		if (algorithm_coord.begin_z < algorithm_coord.end_z)
			return current_coord;

		//motor.test(algorithm_coord);
		auto promise = std::make_shared<std::promise<bool>>();
		
		futureAl = promise->get_future();
		algorithmThread = std::thread(algoThread, std::ref(algorithm_coord), std::move(promise), std::ref(delay_g));
	}
	if (algorithmThread.joinable()) {
		if (futureAl.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			algorithmThread.join();
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Start smart")) {
		if (algorithm_coord.begin_x < algorithm_coord.end_x)
			return current_coord;
		if (algorithm_coord.begin_y < algorithm_coord.end_y)
			return current_coord;
		if (algorithm_coord.begin_z < algorithm_coord.end_z)
			return current_coord;

		//motor.test(algorithm_coord);
		auto promise = std::make_shared<std::promise<bool>>();

		futureAlSmart = promise->get_future();
		algorithmSmartThread = std::thread(algoSmartThread, std::ref(algorithm_coord), std::move(promise), std::ref(delay_g));
	}
	if (algorithmSmartThread.joinable()) {
		if (futureAlSmart.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
			algorithmSmartThread.join();
		}
	}
	ImGui::SameLine();
	static float speed = 0;
	ImGui::PushItemWidth(50);
	ImGui::InputFloat("Speed", &speed);
	ImGui::SameLine();
	static int counter = 0;
	if (ImGui::Button("Just")) {
		if (motor.global_just_flg == false) {
			motor.global_just_flg = true;
			futureJust = std::thread(&stepper::just, &motor, speed);
		}
		else if (motor.global_just_flg == true){
			motor.global_just_flg = false;
			if (futureJust.joinable())
			futureJust.detach();
		}
	}
	
	return current_coord;
}

bool stepperThread(stepper &motor, std::shared_ptr<std::promise<bool>> promise) {
	if (motor.initialize()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1500));
		motor.home();
		promise->set_value(true);
		return true;
	}
	else {
		promise->set_value(false);
		return false;
	}
}

bool adcConnectThread(ADC& adc, std::shared_ptr<std::promise<bool>> promise) {
	if (adc.init_e440()) {
		promise->set_value(true);
		return true;
	}
	else {
		promise->set_value(false);
		return false;
	}
}

int main() {
	static float f = 0.0f;
	std::vector<float>& buffer_long = adc.buffer_long;
	std::vector<float> tmp_buffer(1024, 0);
	std::vector<float> tmp_fourier(1024, 0);
	static int npoints = 1024;
	static int samples = 512;
	setlocale(LC_ALL, "en_us.utf8");

	std::future<bool> future;

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
		

		if (ren.adcConnectInProgress && ren.connectStartUpADC && !connectDone) {
			auto promise = std::make_shared<std::promise<bool>>();
			futureInit = promise->get_future();
			adcInitThread = std::jthread(adcConnectThread, std::ref(adc), std::move(promise));
			ren.connectStartUpADC = false;
			/*if (adc.init_e440() && adc.stream_setup()) {
				connectDone = true;
			}
			else {
				ren.isConnected = false;
				connectDone = false;
			}*/
		}
		if (ren.adcConnectInProgress && !connectDone) {
			if (futureInit.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				adcInitThread.join();
				if (futureInit.get()) {
					connectDone = true;
					ren.isConnected = true;
					//adc.stream_setup(); // —»Õ’–ŒÕÕ¿ﬂ ≈¡”Àƒ€√¿
				}
				else {
					ren.adcConnectInProgress = false;
					connectDone = false;
				}
			}
		}
		if (ren.disconnect) {
			adc.disconnect();
			ren.disconnect = false;
			connectDone = false;
		}

		if (ren.motorConnectInProgress && !connectDoneMotor && ren.connectStartUp) {
			auto promise = std::make_shared<std::promise<bool>>();
			future = promise->get_future();
			motorThread = std::thread(stepperThread, std::ref(motor), std::move(promise));
			ren.connectStartUp = false;
			/*if (motor.initialize()) {
				Sleep(1500);
				motor.home();
				connectDoneMotor = true;
			}
			else {
				ren.isMotorConnected = false;
				connectDoneMotor = false;
			}*/
		}
		if (ren.motorConnectInProgress && !connectDoneMotor) {
			if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
				motorThread.join();
				if (future.get()) {
					connectDoneMotor = true;
				}
				else {
					ren.motorConnectInProgress = false;
					connectDoneMotor = false;
				}
			}
		}
		if (ren.disconnectMotor) {
			motor.close();
			ren.disconnectMotor = false;
			connectDoneMotor = false;
		}
		
		float avg = 0;
		float data = 0;
		if (ren.isConnected) {
			data = adc.frame();
			//avg = adc.data_avg(adc.data_proc());// —»Õ’–ŒÕÕ¿ﬂ ≈¡”Àƒ€√¿
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
			/*if (ImGui::BeginTabItem("Fourier")) {
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
				current_coord = MotorControlPanel(current_coord);
				ImGui::EndTabItem();
			}*/
			if (ImGui::BeginTabItem("Scrolling")) {
				ren.scrolling(data);
				ren.connect();
				current_coord = MotorControlPanel(current_coord);
				ImGui::EndTabItem();
			}
			/*if (ImGui::BeginTabItem("Oscilloscope")) {
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
				current_coord = MotorControlPanel(current_coord);
				ImGui::EndTabItem();
			}*/
			if (ImGui::BeginTabItem("Visualize")) {
				ren.visualize();
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