#pragma once
#include "cpu.hpp"
#include "memory.hpp"

struct Gameboy
{
	void loadCardridge(uint8_t* data, size_t size);
	void start();

	void cpuStep();
	
	Registers registers;
	MMU mmu;
};
