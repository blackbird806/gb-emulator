#pragma once

#include <string>

#include "cpu.hpp"
#include "memory.hpp"

struct Gameboy
{
	void loadCardridge(uint8_t* data, size_t size);
	void start();

	void cpuStep();
	std::string disassembleInstruction(uint16_t address);
	
	Registers registers;
	MMU mmu;
};
