#pragma once

#include <cstdint>
#include <bit>

struct MMU
{
	static uint16_t constexpr romSize = 0x8000;

	uint8_t memMap[0xFFFF];

	uint8_t readByte(uint16_t address)
	{
		return memMap[address];
	}
	
	uint16_t readShort(uint16_t address)
	{
		return std::bit_cast<uint16_t*>(static_cast<uint8_t*>(memMap))[address];
	}
	
	uint8_t* rom()
	{
		return memMap;
	}

	uint8_t* vram()
	{
		return memMap + romSize;
	}
};