#pragma once

#include <cstdint>

struct MMU
{
	static uint16_t constexpr romSize = 0x8000;

	uint8_t memMap[0xFFFF];
	
	uint8_t* rom()
	{
		return memMap;
	}

	uint8_t* vram()
	{
		return memMap + romSize;
	}
};