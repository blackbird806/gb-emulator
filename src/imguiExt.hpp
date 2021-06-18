#pragma once
#include "imgui/imgui.h"

// https://github.com/ocornut/imgui/issues/211#issuecomment-812293268
namespace ImGui
{
	inline void PushEnabled(bool _enabled)
	{
		extern void PushItemFlag(int option, bool enabled);
		PushItemFlag(1 << 2 /*ImGuiItemFlags_Disabled*/, !_enabled);
		//PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * (_enabled ? 1.0f : 0.5f));
	}

	inline void PopEnabled()
	{
		extern void PopItemFlag();
		PopItemFlag();
		//PopStyleVar();
	}

} // namespace 