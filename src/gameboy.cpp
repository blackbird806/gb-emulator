#include "gameboy.hpp"

#include <cstring>

void Gameboy::loadCardridge(uint8_t* data, size_t size)
{
	memcpy(mmu.rom(), data, size);
}

void Gameboy::start()
{
	registers.af() = 0x01B0;
	registers.bc() = 0x0013;
	registers.de() = 0x00D8;
	registers.hl() = 0x014D;
	registers.sp = 0xFFFE;
	registers.pc = 0x100;
	
	mmu.memMap[0xFF05] = 0x00; // TIMA
	mmu.memMap[0xFF06] = 0x00; // TMA
	mmu.memMap[0xFF07] = 0x00; // TAC
	mmu.memMap[0xFF10] = 0x80; // NR10
	mmu.memMap[0xFF11] = 0xBF; // NR11
	mmu.memMap[0xFF12] = 0xF3; // NR12
	mmu.memMap[0xFF14] = 0xBF; // NR14
	mmu.memMap[0xFF16] = 0x3F; // NR21
	mmu.memMap[0xFF17] = 0x00; // NR22
	mmu.memMap[0xFF19] = 0xBF; // NR24
	mmu.memMap[0xFF1A] = 0x7F; // NR30
	mmu.memMap[0xFF1B] = 0xFF; // NR31
	mmu.memMap[0xFF1C] = 0x9F; // NR32
	mmu.memMap[0xFF1E] = 0xBF; // NR34
	mmu.memMap[0xFF20] = 0xFF; // NR41
	mmu.memMap[0xFF21] = 0x00; // NR42
	mmu.memMap[0xFF22] = 0x00; // NR43
	mmu.memMap[0xFF23] = 0xBF; // NR44
	mmu.memMap[0xFF24] = 0x77; // NR50
	mmu.memMap[0xFF25] = 0xF3; // NR51
	mmu.memMap[0xFF26] = 0xF1; // NR52
	mmu.memMap[0xFF40] = 0x91; // LCDC
	mmu.memMap[0xFF42] = 0x00; // SCY
	mmu.memMap[0xFF43] = 0x00; // SCX
	mmu.memMap[0xFF45] = 0x00; // LYC
	mmu.memMap[0xFF47] = 0xFC; // BGP
	mmu.memMap[0xFF48] = 0xFF; // OBP0
	mmu.memMap[0xFF49] = 0xFF; // OBP1
	mmu.memMap[0xFF4A] = 0x00; // WY
	mmu.memMap[0xFF4B] = 0x00; // WX
	// mmu.memMap[0xFFFF] = 0x00; // IE
}

void Gameboy::cpuStep()
{
	Instruction const instr = instructions[mmu.rom()[registers.pc]];
	switch (instr.len)
	{
		case 1:
			std::get<void(*)(Gameboy&)>(instr.op)(*this);
			break;
		case 2:
			std::get<void(*)(Gameboy&, uint8_t)>(instr.op)(*this, mmu.readByte(registers.pc + 1));
			break;
		case 3:
			std::get<void(*)(Gameboy&, uint16_t)>(instr.op)(*this, mmu.readShort(registers.pc + 1));
			break;
		default:
			fprintf(stderr, "instruction not implemented: %s, 0x%02X\n", disassembleInstruction(registers.pc).c_str(), mmu.rom()[registers.pc]);
			__debugbreak();
			break;
	}
	registers.pc++;
}

std::string Gameboy::disassembleInstruction(uint16_t address)
{
	uint8_t const opCode = mmu.rom()[address];
	Instruction const instr = instructions[opCode];

	if (instr.len > 1)
	{
		char buffer[30];
		if (instr.len == 2)
			snprintf(buffer, sizeof(buffer), instr.name, mmu.readByte(address + 1));
		else
			snprintf(buffer, sizeof(buffer), instr.name, mmu.readShort(address + 1));
		return std::string(buffer);
	}
	
	return instr.name;
}
