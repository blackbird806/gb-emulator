#include "cpu.hpp"
#include "gameboy.hpp"

static void nop(Gameboy&)
{

}

static void ld_bc_nn(Gameboy& gb, uint16_t value)
{
	gb.registers.bc() = value;
}

#define INC_REG_FN_DECL(r) static void inc_##r(Gameboy& gb) { gb.registers.##r++; }
#define INC_WIDE_REG_FN_DECL(r) static void inc_##r(Gameboy& gb) { gb.registers.##r()++; }

INC_REG_FN_DECL(a)
INC_REG_FN_DECL(f)
INC_REG_FN_DECL(b)
INC_REG_FN_DECL(c)
INC_REG_FN_DECL(d)
INC_REG_FN_DECL(e)
INC_REG_FN_DECL(h)
INC_REG_FN_DECL(l)

INC_WIDE_REG_FN_DECL(af)
INC_WIDE_REG_FN_DECL(bc)
INC_WIDE_REG_FN_DECL(de)
INC_WIDE_REG_FN_DECL(hl)

#define UNDEFINED_INSTRUCTION {0, 0, nop}

Instruction instructions[256] = {
	{0, 4, nop},
	{2, 6, ld_bc_nn},
	UNDEFINED_INSTRUCTION,
	{0, 4, inc_bc},
	{0, 4, inc_b},
};

const char* instructions_names[256] = {
	"NOP",
	"LD BC, 0x%04X",
	"UNDEFINED",
	"INC BC",
	"INC B",
};

const char* disassembleInstruction(int opCode)
{
	return instructions_names[opCode];
}
