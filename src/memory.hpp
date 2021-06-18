#pragma once

#include <cstdint>
#include <bit>

struct MMU
{
	static uint16_t constexpr romSize = 0x8000;
	static uint16_t constexpr titleAddress = 0x0134;

	uint8_t memMap[0xFFFF];

	const char* romName() const
	{
		return std::bit_cast<char*>(&memMap[titleAddress]);
	}

	void writeByte(uint16_t address, uint8_t value)
	{
		memMap[address] = value;
	}

	void writeShort(uint16_t address, uint16_t value)
	{
		*std::bit_cast<uint16_t*>(&static_cast<uint8_t*>(memMap)[address]) = value;
	}
	
	uint8_t readByte(uint16_t address)
	{
		return memMap[address];
	}
	
	uint16_t readShort(uint16_t address)
	{
		return *std::bit_cast<uint16_t*>(&static_cast<uint8_t*>(memMap)[address]);
	}
	
	uint8_t* rom()
	{
		return memMap;
	}

	uint8_t* vram()
	{
		return &memMap[romSize];
	}
};