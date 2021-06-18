#pragma once

#include <filesystem>
#include <SDL.h>
#include <imgui/imgui.h>
#include <imfilebrowser.h>
#include <imgui_memory_editor.h>
#include "gameboy.hpp"

class App
{
	public:

	static auto constexpr fileSettingsPath = "settings.ini";
	App();
	void init();
	void run();
	void update();
	void onGUI();

	~App();

	protected:

	void loadRom(std::filesystem::path const& romPath);
	
	Gameboy gb;
	bool gbStarted = false;
	ImGui::FileBrowser openDialog;
	ImGui::FileBrowser saveDialog;
	MemoryEditor mem_edit;
	bool disassemblerOpen = false;
	bool spriteViewerOpen = false;
	bool debuggerOpen = false;
	bool stepDebug = false;
	bool nextStep = false;
	
	void startFrame();
	void endFrame();
	bool endApp = false;
	bool romLoaded = false;
	SDL_Window* window;
	SDL_Renderer* renderer;
};
