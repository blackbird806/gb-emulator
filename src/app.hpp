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
	
	void init();
	void run();
	void update();
	void onGUI();

	~App();

	protected:

	void loadRom(std::filesystem::path const& romPath);
	
	Gameboy gb;
	bool gbStarted = false;
	ImGui::FileBrowser fileDialog;
	MemoryEditor mem_edit;
	bool disassemblerOpen = false;
	bool spriteViewerOpen = false;
	
	void startFrame();
	void endFrame();
	bool endApp = false;
	bool romLoaded = false;
	SDL_Window* window;
	SDL_Renderer* renderer;
};
