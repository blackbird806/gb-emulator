#pragma once

#include <cstdint>
#include <variant>
#include <bit>

struct Registers {
	
	uint8_t a;
	uint8_t f;

	uint8_t b;
	uint8_t c;

	uint8_t d;
	uint8_t e;

	uint8_t h;
	uint8_t l;
	
	uint8_t flags;
	uint16_t sp;
	uint16_t pc;

	uint16_t& af()
	{
		return *std::bit_cast<uint16_t*>(&a);
	}
	
	uint16_t& bc()
	{
		return *std::bit_cast<uint16_t*>(&b);
	}

	uint16_t& de()
	{
		return *std::bit_cast<uint16_t*>(&d);
	}

	uint16_t& hl()
	{
		return *std::bit_cast<uint16_t*>(&h);
	}
};

struct Gameboy;

using InstructionOpFn = std::variant<
						void(*)(Gameboy&),
						void(*)(Gameboy&, uint8_t),
						void(*)(Gameboy&, uint16_t)>;

struct Instruction
{
	uint8_t len;
	uint8_t cycles;
	InstructionOpFn op;
	const char* name;
};

extern Instruction instructions[256];

