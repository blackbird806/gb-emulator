#include "app.hpp"

#include <imgui/imgui.h>
#include <glad/glad.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>
#include <cstdio>
#include <fstream>

#define AINI_IMPLEMENTATION
#include "aini.hpp"

static void initDockspace()
{
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	static bool p_open = true;
	ImGui::Begin("DockSpace", &p_open, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();
}

static std::string readTextFile(std::filesystem::path const& path)
{
	std::ifstream const settingsFile(path);
	std::ostringstream sstream;
	sstream << settingsFile.rdbuf();
	return sstream.str();
}

static std::vector<uint8_t> readBinFile(std::filesystem::path const& path)
{
	std::ifstream settingsFile(path, std::ios::ate | std::ios::binary);
	size_t const size = settingsFile.tellg();
	std::vector<uint8_t> content;
	content.resize(size);
	settingsFile.seekg(0, std::ios::beg);
	settingsFile.read(reinterpret_cast<char*>(content.data()), size);
	return content;
}

void App::init()
{
	if (std::filesystem::exists(fileSettingsPath))
	{
		aini::Reader reader(readTextFile(fileSettingsPath));
		if (reader.has_key("lastRomPath"))
		{
			try {
				loadRom(reader.get_string("lastRomPath"));
			}
			catch(std::exception& e) {
				fprintf(stderr, e.what());
			}
		}
	}
	
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "error initializing SDL: %s\n", SDL_GetError());
		throw std::runtime_error("failed to init SDL");
    }

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    window = SDL_CreateWindow("gb-emulator",
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress))) {
			fprintf(stderr, "Failed to initialize GLAD\n");
			throw std::runtime_error("failed to init GLAD");
    }
	
	// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();

	mem_edit.Open = false;
}

void App::startFrame()
{
	// Handle events
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		// Forward to Imgui
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT
			|| (event.type == SDL_WINDOWEVENT
			&& event.window.event == SDL_WINDOWEVENT_CLOSE
			&& event.window.windowID == SDL_GetWindowID(window))) {
			endApp = true;
		}

	}
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	initDockspace();
}

void App::endFrame()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	ImGui::Render();
	
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

void App::run()
{
	while (!endApp)
	{
		startFrame();
		update();
		onGUI();
		endFrame();
	}
}

void App::update()
{
	if (gbStarted)
	{
		gb.cpuStep();
	}
}

void App::onGUI()
{
	static bool showDemo = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open"))
			{
				fileDialog.SetTitle("File browser");
				fileDialog.Open();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Game"))
		{
			gameOpen = !gameOpen;
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools"))
		{
			if (ImGui::MenuItem("Memory editor"))
			{
				mem_edit.Open = !mem_edit.Open;
			}
			if (ImGui::MenuItem("Disassembler"))
			{
				disassemblerOpen = !disassemblerOpen;
			}
			if (ImGui::MenuItem("demo win"))
			{
				showDemo = !showDemo;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (gameOpen)
	{
		ImGui::Begin("Game");
		if (ImGui::Button("Start"))
		{
			gb.start();
			gbStarted = true;
		}
		
		ImGui::End();
	}

	if (mem_edit.Open)
		mem_edit.DrawWindow("Memory Editor", gb.mmu.memMap, sizeof(gb.mmu.memMap));

	fileDialog.Display();
	if (fileDialog.HasSelected())
	{
		std::string const filePathStr = fileDialog.GetSelected().string();
		printf("selected file \"%s\"\n", filePathStr.c_str());
		aini::Writer writer;
		writer.set_string("lastRomPath", filePathStr);
		std::ofstream settings(fileSettingsPath);
		settings << writer.write();
		
		loadRom(fileDialog.GetSelected());
		fileDialog.ClearSelected();
	}

	if (disassemblerOpen)
	{
		ImGui::Begin("Disassembler", &disassemblerOpen);
		static int minAddress = 0, maxAddress = MMU::romSize / 16;
		ImGui::DragInt("min address", &minAddress, 8, 0, maxAddress);
		ImGui::DragInt("max address", &maxAddress, 8, minAddress, MMU::romSize);
		ImGuiTableFlags constexpr flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (ImGui::BeginTable("disassembler table", 3, flags))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImU32>(0xFF00FF));
			ImGui::TableSetupColumn("address");
			ImGui::TableSetupColumn("byte");
			ImGui::TableSetupColumn("code");
			ImGui::PopStyleColor();
			ImGui::TableHeadersRow();
			
			for (uint16_t i = minAddress; i < maxAddress;)
			{
				ImGui::TableNextColumn();
				if (gb.registers.pc == i)
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
				ImGui::Text("0x%04X\n", i);
				ImGui::TableNextColumn();
				ImGui::Text("0x%02X\n", gb.mmu.rom()[i]);
				ImGui::TableNextColumn();
				ImGui::TextUnformatted(gb.disassembleInstruction(i).c_str());
				if (gb.registers.pc == i)
					ImGui::PopStyleColor();
				i += instructions[gb.mmu.rom()[i]].len > 0 ? instructions[gb.mmu.rom()[i]].len : 1;
			}
			ImGui::EndTable();
		}
		ImGui::End();
	}
	if (showDemo)
		ImGui::ShowDemoWindow(&showDemo);
}

App::~App()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void App::loadRom(std::filesystem::path const& romPath)
{
	std::ifstream file(romPath, std::ios::binary | std::ios::ate);
	size_t const size = file.tellg();

	if (size > MMU::romSize)
	{
		fprintf(stderr, "error : cardrige is too big\n");
		fileDialog.ClearSelected();
		return;
	}
	file.seekg(0, std::ios::beg);
	file.read(reinterpret_cast<char*>(gb.mmu.rom()), size);
}
