#include "gameboy.hpp"

#include <cstring>

void Gameboy::loadCardridge(uint8_t* data, size_t size)
{
	memcpy(mmu.rom(), data, size);
}

void Gameboy::start()
{

}

void Gameboy::cpuStep()
{
	
}
