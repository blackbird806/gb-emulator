#include "cpu.hpp"
#include "gameboy.hpp"

static void nop(Gameboy&)
{

}

static void stop(Gameboy& gb)
{
	
}

static void halt(Gameboy& gb)
{
	__debugbreak(); // TODO
}

static void rlca(Gameboy& gb)
{
	uint8_t const carry = (gb.registers.a & 0x80) >> 7;
	if (carry)
		gb.registers.setFlag(Registers::carryFlag);
	else
		gb.registers.clearFlag(Registers::carryFlag);

	gb.registers.a <<= 1;
	gb.registers.a += carry;
	gb.registers.clearFlag(Registers::halfCarryFlag | Registers::negativeFlag | Registers::zeroFlag);
}

#define EACH_R(M, ...) M(a, __VA_ARGS__) M(b, __VA_ARGS__) M(c, __VA_ARGS__) M(d, __VA_ARGS__) M(e, __VA_ARGS__) M(h, __VA_ARGS__) M(l, __VA_ARGS__)
#define EACH_RR(M) M(af) M(bc) M(de) M(hl)

#define LD_DRR_R(r1, r2) static void ld_##r1##_##r2##(Gameboy& gb) { gb.mmu.memMap[gb.registers.##r1##()] = gb.registers.##r2; }
LD_DRR_R(bc, a)

#define LD_R_DRR(r1, r2) static void ld_##r1##_##r2##(Gameboy& gb) { gb.registers.##r1 = gb.mmu.memMap[gb.registers.##r2##()]; }
LD_R_DRR(a, bc)
LD_R_DRR(a, de)

#define LD_RR(r1, r2) static void ld_##r1##_##r2(Gameboy& gb) { gb.registers.##r1 = gb.registers.##r2; }
LD_RR(a, b)
LD_RR(b, a)
LD_RR(c, a)
LD_RR(b, c)
LD_RR(c, b)
LD_RR(c, d)
LD_RR(c, e)
LD_RR(c, h)
LD_RR(c, l)
LD_RR(d, a)
LD_RR(d, c)
LD_RR(d, e)
LD_RR(d, h)
LD_RR(d, l)
LD_RR(a, d)
LD_RR(d, b)
LD_RR(b, d)
LD_RR(b, e)
LD_RR(b, h)
LD_RR(b, l)
LD_RR(e, b)
LD_RR(e, c)
LD_RR(e, d)
LD_RR(e, h)
LD_RR(e, l)
LD_RR(e, a)
LD_RR(h, b)
LD_RR(h, e)
LD_RR(h, c)
LD_RR(h, d)
LD_RR(h, l)
LD_RR(h, a)
LD_RR(l, b)
LD_RR(l, e)
LD_RR(l, c)
LD_RR(l, d)
LD_RR(l, h)
LD_RR(l, a)

#define LD_R_N(r) static void ld_##r##_n(Gameboy& gb, uint8_t value) { gb.registers.##r = value; }
EACH_R(LD_R_N)

#define LD_RR_NN(r) static void ld_##r##_nn(Gameboy& gb, uint16_t value) { gb.registers.##r##() = value; }
EACH_RR(LD_RR_NN)

#define INC_R(r) static void inc_##r(Gameboy& gb) { gb.registers.##r++; }
EACH_R(INC_R)
INC_R(sp)

#define DEC_R(r) static void dec_##r(Gameboy& gb) { gb.registers.##r--; }
EACH_R(DEC_R)
DEC_R(sp)

#define INC_RR(r) static void inc_##r(Gameboy& gb) { gb.registers.##r()++; }
EACH_RR(INC_RR)

#define DEC_RR(r) static void dec_##r(Gameboy& gb) { gb.registers.##r()--; }
EACH_RR(DEC_RR)

#define BIN_OP(r, opname, op) static void opname##_##r(Gameboy& gb) { gb.registers.a op gb.registers.##r; }
// for some reason this doesn't compiles on MSVC
//EACH_R(BIN_OP, add, +=)

#define ADD_R(r) static void add_##r(Gameboy& gb) { gb.registers.a += gb.registers.##r; }
EACH_R(ADD_R)

#define SUB_R(r) static void sub_##r(Gameboy& gb) { gb.registers.a -= gb.registers.##r; }
EACH_R(SUB_R)

static void add_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a += value;
}

static void sub_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a -= value;
}

#define XOR_R(r) static void xor_##r(Gameboy& gb) { gb.registers.a ^= gb.registers.##r; }
EACH_R(XOR_R)

static void xor_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a ^= value;
}

#define AND_R(r) static void and_##r(Gameboy& gb) { gb.registers.a &= gb.registers.##r; }
EACH_R(AND_R)

static void and_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a &= value;
}

#define OR_R(r) static void or_##r(Gameboy& gb) { gb.registers.a |= gb.registers.##r; }
EACH_R(OR_R)

static void or_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a |= value;
}

static void jp_nn(Gameboy& gb, uint8_t value)
{
	gb.registers.pc = value;
}

#define UNDEFINED_INSTRUCTION {0, 0, nop, "UNDEFINED"}

Instruction instructions[256] = {
	{ 1, 4, nop, "NOP"},
	{ 3, 6, ld_bc_nn, "LD BC, 0x%04X" },
	{ 1, 8, ld_bc_a, "LD (BC), A" },
	{ 1, 8, inc_bc, "INC BC" }, // 03
	{ 1, 4, inc_b, "INC B" }, // 04
	{ 1, 4, dec_b, "DEC B" },
	{ 2, 4, ld_b_n, "LD B, 0x%02X" },
	{ 1, 4, rlca, "RLCA" },
	UNDEFINED_INSTRUCTION,
	{ 1, 8, ld_a_bc, "LD A, (BC)" }, // 0A
	{ 1, 8, dec_bc, "DEC BC" }, //0B
	{ 1, 4, inc_c, "INC C" }, //0C
	{ 1, 4, dec_c, "DEC C" }, // 0D
	UNDEFINED_INSTRUCTION, // 0E
	UNDEFINED_INSTRUCTION, // 0F
	{ 1, 0, stop, "STOP" }, // 10
	{ 3, 8, ld_de_nn, "LD DE, 0x%04X" },
	UNDEFINED_INSTRUCTION,
	{ 1, 8, inc_de, "INC DE" },
	{ 1, 4, inc_d, "INC D" }, // 14
	{ 1, 4, dec_d, "DEC D" },
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 8, ld_a_de, "LD A,(DE)" }, // 1A
	{ 1, 8, dec_de, "DEC DE" },
	{ 1, 4, inc_e, "INC E" },
	{ 1, 4, dec_e, "DEC E" }, // 1D
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION, // 1F
	UNDEFINED_INSTRUCTION, // 20
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 8, inc_hl, "INC HL" }, // 23
	{ 1, 4, inc_h, "INC H" },
	{ 1, 4, dec_h, "DEC H" },
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 8, dec_hl, "DEC HL" }, // 2B
	{ 1, 4, inc_l, "INC L" },
	{ 1, 4, dec_l, "DEC L" },
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION, // 30
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 8, inc_sp, "INC SP" }, // 33
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 8, dec_sp, "DEC SP" }, // 3B
	{ 1, 4, inc_a, "INC A" },
	{ 1, 4, dec_a, "DEC A" }, // 3D
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 4, nop, "LD B, B" }, // 40
	{ 1, 4, ld_b_c, "LD B, C" }, // 41
	{ 1, 4, ld_b_d, "LD B, D" },
	{ 1, 4, ld_b_e, "LD B, E" },
	{ 1, 4, ld_b_h, "LD B, H" },
	{ 1, 4, ld_b_l, "LD B, L" }, // 45
	UNDEFINED_INSTRUCTION,
	{ 1, 4, ld_b_a, "LD B, A" },
	{ 1, 4, ld_c_b, "LD C, B" },
	{ 1, 4, nop, "LD C, C" },
	{ 1, 4, ld_c_d, "LD C, D" },
	{ 1, 4, ld_c_e, "LD C, E" },
	{ 1, 4, ld_c_h, "LD C, H" },
	{ 1, 4, ld_c_l, "LD C, L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, ld_c_a, "LD C, A" },
	{ 1, 4, ld_d_b, "LD D, B" }, // 50
	{ 1, 4, ld_d_c, "LD D, C" },
	{ 1, 4, nop, "LD D, D" },
	{ 1, 4, ld_d_e, "LD D, E" },
	{ 1, 4, ld_d_h, "LD D, H" },
	{ 1, 4, ld_d_l, "LD D, L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, ld_d_a, "LD D, A" },
	{ 1, 4, ld_e_b, "LD E, B" },
	{ 1, 4, ld_e_c, "LD E, C" },
	{ 1, 4, ld_e_d, "LD E, D" },
	{ 1, 4, nop, "LD E, E" },
	{ 1, 4, ld_e_h, "LD E, H" },
	{ 1, 4, ld_e_l, "LD E, L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, ld_e_a, "LD E, A" }, // 5f
	{ 1, 4, ld_h_b, "LD H, B" },
	{ 1, 4, ld_h_c, "LD H, C" },
	{ 1, 4, ld_h_d, "LD H, D" },
	{ 1, 4, ld_h_e, "LD H, E" },
	{ 1, 4, nop, "LD H, H" },
	{ 1, 4, ld_h_l, "LD H, L" }, // 65
	UNDEFINED_INSTRUCTION,
	{ 1, 4, ld_h_a, "LD H, A" },
	{ 1, 4, ld_l_b, "LD L, B" },
	{ 1, 4, ld_l_c, "LD L, C" },
	{ 1, 4, ld_l_d, "LD L, D" },
	{ 1, 4, ld_l_e, "LD L, E" },
	{ 1, 4, ld_l_h, "LD L, H" },
	{ 1, 4, nop, "LD L, L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, ld_l_a, "LD L, A" }, // 6f
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 4, halt, "HALT" }, // 75
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
};
