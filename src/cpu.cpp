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
		gb.registers.setFlags(Registers::carryFlag);
	else
		gb.registers.clearFlags(Registers::carryFlag);

	gb.registers.a <<= 1;
	gb.registers.a += carry;
	gb.registers.clearFlags(Registers::halfCarryFlag | Registers::negativeFlag | Registers::zeroFlag);
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
LD_RR(a, e)
LD_RR(a, c)
LD_RR(a, h)
LD_RR(a, l)
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

#define LD_DNN_RR(r) static void ld_dnn_##r(Gameboy& gb, uint16_t value) { gb.mmu.writeShort(value, gb.registers.##r()); }
EACH_RR(LD_DNN_RR)

static void ldi_dhl_a(Gameboy& gb)
{
	gb.mmu.writeByte(gb.registers.hl(), gb.registers.a);
	gb.registers.hl()++;
}

static void ldi_a_dhl(Gameboy& gb)
{
	gb.registers.a = gb.registers.hl();
	gb.registers.hl()++;
}

static void ldd_dhl_a(Gameboy& gb)
{
	gb.mmu.writeByte(gb.registers.hl(), gb.registers.a);
	gb.registers.hl()--;
}

static void ldd_a_dhl(Gameboy& gb)
{
	gb.registers.a = gb.registers.hl();
	gb.registers.hl()--;
}


//#define LD_RR_RR(rr1, rr2) static void ld_##rr1##_##rr2(Gameboy& gb) { gb.registers.##rr1() = gb.registers.##rr2(); }


static void ld_dnn_sp(Gameboy& gb, uint16_t value)
{
	gb.mmu.writeShort(value, gb.registers.sp);
}

static void ld_dde_a(Gameboy& gb)
{
	gb.mmu.writeByte(gb.registers.de(), gb.registers.a);
}

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

#define ADD_RR(r) static void add_##r(Gameboy& gb) { gb.registers.hl() = gb.registers.##r(); }
EACH_RR(ADD_RR)

static void add_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a += value;
}

static void sub_n(Gameboy& gb, uint8_t value)
{
	gb.registers.a -= value;
}

static void cp_impl(Gameboy& gb, uint8_t r)
{
	if (gb.registers.a == r)
		gb.registers.setFlags(Registers::zeroFlag);
	else
		gb.registers.clearFlags(Registers::zeroFlag);

	if (gb.registers.a < r)
		gb.registers.setFlags(Registers::carryFlag);
	else
		gb.registers.clearFlags(Registers::carryFlag);

	// https://stackoverflow.com/questions/8868396/game-boy-what-constitutes-a-half-carry/8874607#8874607
	if ((r & 0x0f) > (gb.registers.a & 0x0f))
		gb.registers.setFlags(Registers::halfCarryFlag);
	else
		gb.registers.clearFlags(Registers::halfCarryFlag);

	// @Review
	//gb.registers.setFlags(Registers::negativeFlag);
}

#define CP_R(r) static void cp_##r(Gameboy& gb) { cp_impl(gb, gb.registers.##r); }
EACH_R(CP_R)

static void cp_n(Gameboy& gb, uint8_t value) { cp_impl(gb, value); }

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

#define PUSH_RR(rr) static void push_##rr(Gameboy& gb) { gb.registers.sp -= 2; gb.mmu.writeShort(gb.registers.sp, gb.registers.##rr()); }
EACH_RR(PUSH_RR)

#define POP_RR(rr) static void pop_##rr(Gameboy& gb) { gb.registers.##rr() = gb.mmu.readShort(gb.registers.sp); gb.registers.sp += 2; }
EACH_RR(POP_RR)

static void jp_nn(Gameboy& gb, uint16_t value)
{
	gb.registers.pc = value;
}

static void jr_nz_n(Gameboy& gb, uint8_t value)
{
	if (gb.registers.isFlagSet(Registers::zeroFlag))
		gb.ticks += 8;
	else
	{
		gb.registers.pc += value;
		gb.ticks += 12;
	}
}

static void jr_z_n(Gameboy& gb, uint8_t value)
{
	if (gb.registers.isFlagSet(Registers::zeroFlag))
	{
		gb.registers.pc += value;
		gb.ticks += 12;
	}
	else
		gb.ticks += 8;
}

static void jr_n(Gameboy& gb, uint8_t value)
{
	gb.registers.pc += value;
	__debugbreak();
}

static void rrca(Gameboy& gb)
{
	uint8_t const carry = gb.registers.a & 0x01;
	if (carry)  
		gb.registers.setFlags(Registers::carryFlag);
	else
		gb.registers.clearFlags(Registers::carryFlag);

	gb.registers.a >>= 1;
	if (carry) 
		gb.registers.a |= 0x80;

	gb.registers.clearFlags(Registers::negativeFlag | Registers::zeroFlag | Registers::halfCarryFlag);
}

static void rla(Gameboy& gb)
{
	uint8_t const carry = gb.registers.isFlagSet(Registers::carryFlag) ? 1 : 0;

	if (gb.registers.a & 0x80) 
		gb.registers.setFlags(Registers::carryFlag);
	else
		gb.registers.clearFlags(Registers::carryFlag);

	gb.registers.a <<= 1;
	gb.registers.a += carry;

	gb.registers.clearFlags(Registers::negativeFlag | Registers::zeroFlag | Registers::halfCarryFlag);
}

static void call_z_nn(Gameboy& gb, uint16_t value)
{
	if (gb.registers.isFlagSet(Registers::zeroFlag))
	{
		gb.registers.sp -= 2;
		gb.mmu.writeShort(gb.registers.sp, gb.registers.pc);
		gb.registers.pc = value;
		gb.ticks += 24;
	}
	else
		gb.ticks += 12;
}

static void call_nn(Gameboy& gb, uint16_t value)
{
	gb.registers.sp -= 2;
	gb.mmu.writeShort(gb.registers.sp, gb.registers.pc);
	gb.registers.pc = value;
}

static void rra(Gameboy& gb)
{
	int const carry = (gb.registers.isFlagSet(Registers::carryFlag) ? 1 : 0) << 7;

	if (gb.registers.a & 0x01)
		gb.registers.setFlags(Registers::carryFlag);
	else
		gb.registers.clearFlags(Registers::carryFlag);

	gb.registers.a >>= 1;
	gb.registers.a += carry;
	
	gb.registers.clearFlags(Registers::negativeFlag | Registers::zeroFlag | Registers::halfCarryFlag);
}

static void daa(Gameboy& gb)
{
	uint16_t s = gb.registers.a;

	if (gb.registers.isFlagSet(Registers::negativeFlag)) 
	{
		if (gb.registers.isFlagSet(Registers::halfCarryFlag)) 
			s = (s - 0x06) & 0xFF;
		if (gb.registers.isFlagSet(Registers::carryFlag)) 
			s -= 0x60;
	}
	else {
		if (gb.registers.isFlagSet(Registers::halfCarryFlag) || (s & 0xF) > 9) 
			s += 0x06;
		if (gb.registers.isFlagSet(Registers::carryFlag) || s > 0x9F) 
			s += 0x60;
	}

	gb.registers.a = s;
	gb.registers.clearFlags(Registers::halfCarryFlag);

	if (gb.registers.a) 
		gb.registers.clearFlags(Registers::zeroFlag);
	else 
		gb.registers.setFlags(Registers::zeroFlag);

	if (s >= 0x100) 
		gb.registers.setFlags(Registers::carryFlag);
}

#define UNDEFINED_INSTRUCTION {0, 0, nop, "UNDEFINED"}

Instruction instructions[256] = {
	{ 1, 4, nop, "NOP"}, // 00
	{ 3, 6, ld_bc_nn, "LD BC, 0x%04X" }, // 01
	{ 1, 8, ld_bc_a, "LD (BC), A" }, //02
	{ 1, 8, inc_bc, "INC BC" }, // 03
	{ 1, 4, inc_b, "INC B" }, // 04
	{ 1, 4, dec_b, "DEC B" }, // 05
	{ 2, 4, ld_b_n, "LD B, 0x%02X" }, // 06
	{ 1, 4, rlca, "RLCA" }, // 07
	{ 3, 16, ld_dnn_sp, "LD (0x%04X), SP" }, // 08
	{ 1, 8, add_bc, "ADD BC"}, // 09
	{ 1, 8, ld_a_bc, "LD A, (BC)" }, // 0A
	{ 1, 8, dec_bc, "DEC BC" }, // 0B
	{ 1, 4, inc_c, "INC C" }, // 0C
	{ 1, 4, dec_c, "DEC C" }, // 0D
	{ 2, 8, ld_c_n, "LD C, 0x%02X" }, // 0E
	{ 1, 4, rrca, "RRCA"}, // 0F
	{ 1, 0, stop, "STOP" }, // 10
	{ 3, 8, ld_de_nn, "LD DE, 0x%04X" }, // 11
	{ 1, 8, ld_dde_a, "LD (DE), A" }, // 12
	{ 1, 8, inc_de, "INC DE" }, // 13
	{ 1, 4, inc_d, "INC D" }, // 14
	{ 1, 4, dec_d, "DEC D" }, // 15
	{ 2, 8, ld_d_n, "LD D, 0x%02X" }, // 16
	{ 1, 4, rla, "RLA" }, // 17
	{ 2, 12, jr_n, "JR 0x%02X" }, // 18
	{ 1, 8, add_de, "ADD DE" }, // 19
	{ 1, 8, ld_a_de, "LD A,(DE)" }, // 1A
	{ 1, 8, dec_de, "DEC DE" }, // 1B
	{ 1, 4, inc_e, "INC E" }, // 1C
	{ 1, 4, dec_e, "DEC E" }, // 1D
	{ 2, 8, ld_e_n, "LD E, 0x%02X" }, // 1E
	{ 1, 4, rra, "RRA" }, // 1F
	{ 2, 0, jr_nz_n, "JR NZ, 0x%02X" }, // 20
	{ 3, 12, ld_hl_nn, "LD HL, 0x%04X" }, // 21
	{ 1, 8, ldi_dhl_a, "LDI (HL), A" }, // 22
	{ 1, 8, inc_hl, "INC HL" }, // 23
	{ 1, 4, inc_h, "INC H" }, // 24
	{ 1, 4, dec_h, "DEC H" }, // 25
	{ 2, 8, ld_h_n, "LD H, 0x%02X" }, // 26
	{ 1, 4, daa, "DAA" }, // 27 
	{ 2, 0, jr_z_n, "JR Z, 0x%02X" }, // 28
	{ 1, 8, add_hl, "ADD HL"}, // 29
	{ 1, 8, ldi_a_dhl, "LDI A, (HL)" }, // 2a
	{ 1, 8, dec_hl, "DEC HL" }, // 2B
	{ 1, 4, inc_l, "INC L" }, // 2c
	{ 1, 4, dec_l, "DEC L" }, // 2d
	UNDEFINED_INSTRUCTION, // 2e
	UNDEFINED_INSTRUCTION, // 2f
	UNDEFINED_INSTRUCTION, // 30
	UNDEFINED_INSTRUCTION, // 31
	UNDEFINED_INSTRUCTION, // 32
	{ 1, 8, inc_sp, "INC SP" }, // 33
	UNDEFINED_INSTRUCTION, // 34
	UNDEFINED_INSTRUCTION, // 35
	UNDEFINED_INSTRUCTION, // 36
	UNDEFINED_INSTRUCTION, // 37
	UNDEFINED_INSTRUCTION, // 38
	UNDEFINED_INSTRUCTION, // 39
	UNDEFINED_INSTRUCTION, // 3a
	{ 1, 8, dec_sp, "DEC SP" }, // 3B
	{ 1, 4, inc_a, "INC A" }, // 3c
	{ 1, 4, dec_a, "DEC A" }, // 3D
	UNDEFINED_INSTRUCTION, // 3e
	UNDEFINED_INSTRUCTION, // 3f
	{ 1, 4, nop, "LD B, B" }, // 40
	{ 1, 4, ld_b_c, "LD B, C" }, // 41
	{ 1, 4, ld_b_d, "LD B, D" }, // 42
	{ 1, 4, ld_b_e, "LD B, E" }, // 43
	{ 1, 4, ld_b_h, "LD B, H" }, //44
	{ 1, 4, ld_b_l, "LD B, L" }, // 45
	UNDEFINED_INSTRUCTION, // 46
	{ 1, 4, ld_b_a, "LD B, A" }, // 47
	{ 1, 4, ld_c_b, "LD C, B" }, // 48
	{ 1, 4, nop, "LD C, C" }, // 49
	{ 1, 4, ld_c_d, "LD C, D" }, // 4a
	{ 1, 4, ld_c_e, "LD C, E" }, // 4b
	{ 1, 4, ld_c_h, "LD C, H" }, // 4c
	{ 1, 4, ld_c_l, "LD C, L" }, // 4d
	UNDEFINED_INSTRUCTION, // 4e
	{ 1, 4, ld_c_a, "LD C, A" }, // 4f
	{ 1, 4, ld_d_b, "LD D, B" }, // 50
	{ 1, 4, ld_d_c, "LD D, C" }, // 51
	{ 1, 4, nop, "LD D, D" }, // 52
	{ 1, 4, ld_d_e, "LD D, E" }, // 53
	{ 1, 4, ld_d_h, "LD D, H" }, //54
	{ 1, 4, ld_d_l, "LD D, L" }, // 55
	UNDEFINED_INSTRUCTION, // 56
	{ 1, 4, ld_d_a, "LD D, A" }, // 57
	{ 1, 4, ld_e_b, "LD E, B" }, // 58
	{ 1, 4, ld_e_c, "LD E, C" }, // 59
	{ 1, 4, ld_e_d, "LD E, D" }, // 5a
	{ 1, 4, nop, "LD E, E" }, // 5b
	{ 1, 4, ld_e_h, "LD E, H" }, // 5c
	{ 1, 4, ld_e_l, "LD E, L" }, // 5d
	UNDEFINED_INSTRUCTION, // 5e
	{ 1, 4, ld_e_a, "LD E, A" }, // 5f
	{ 1, 4, ld_h_b, "LD H, B" }, // 60
	{ 1, 4, ld_h_c, "LD H, C" }, // 61
	{ 1, 4, ld_h_d, "LD H, D" }, // 62
	{ 1, 4, ld_h_e, "LD H, E" }, // 63
	{ 1, 4, nop, "LD H, H" }, // 64
	{ 1, 4, ld_h_l, "LD H, L" }, // 65
	UNDEFINED_INSTRUCTION, // 66
	{ 1, 4, ld_h_a, "LD H, A" }, // 67 
	{ 1, 4, ld_l_b, "LD L, B" }, // 68
	{ 1, 4, ld_l_c, "LD L, C" }, // 69
	{ 1, 4, ld_l_d, "LD L, D" }, // 6a
	{ 1, 4, ld_l_e, "LD L, E" }, // 6b
	{ 1, 4, ld_l_h, "LD L, H" }, // 6c
	{ 1, 4, nop, "LD L, L" }, // 6d
	UNDEFINED_INSTRUCTION, // 6e
	{ 1, 4, ld_l_a, "LD L, A" }, // 6f
	UNDEFINED_INSTRUCTION, // 70
	UNDEFINED_INSTRUCTION, // 71
	UNDEFINED_INSTRUCTION, // 72
	UNDEFINED_INSTRUCTION, // 73
	UNDEFINED_INSTRUCTION, // 74
	{ 1, 4, halt, "HALT" }, // 75
	UNDEFINED_INSTRUCTION, // 76
	UNDEFINED_INSTRUCTION, // 77
	{ 1, 4, ld_a_b, "LD A, B" }, // 78
	{ 1, 4, ld_a_c, "LD A, C" },
	{ 1, 4, ld_a_d, "LD A, D" },
	{ 1, 4, ld_a_e, "LD A, E" },
	{ 1, 4, ld_a_h, "LD A, H" },
	{ 1, 4, ld_a_l, "LD A, L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, nop, "LD A, A" },
	{ 1, 4, add_b, "ADD B" },
	{ 1, 4, add_c, "ADD C" },
	{ 1, 4, add_d, "ADD D" },
	{ 1, 4, add_e, "ADD E" },
	{ 1, 4, add_h, "ADD H" },
	{ 1, 4, add_l, "ADD L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, add_a, "ADD A" }, // 87
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 4, sub_b, "SUB B" }, // 90
	{ 1, 4, sub_c, "SUB C" },
	{ 1, 4, sub_d, "SUB D" },
	{ 1, 4, sub_e, "SUB E" },
	{ 1, 4, sub_h, "SUB H" },
	{ 1, 4, sub_l, "SUB L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, sub_a, "SUB A" }, // 97
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION, // 9f
	{ 1, 4, and_b, "AND B" }, // a0
	{ 1, 4, and_c, "AND C" },
	{ 1, 4, and_d, "AND D" },
	{ 1, 4, and_e, "AND E" },
	{ 1, 4, and_h, "AND H" },
	{ 1, 4, and_l, "AND L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, and_a, "AND A" },
	{ 1, 4, xor_b, "XOR B" },
	{ 1, 4, xor_c, "XOR C" },
	{ 1, 4, xor_b, "XOR D" },
	{ 1, 4, xor_e, "XOR E" },
	{ 1, 4, xor_h, "XOR H" },
	{ 1, 4, xor_l, "XOR L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, xor_a, "XOR A" },
	{ 1, 4, or_b, "OR B" },
	{ 1, 4, or_c, "OR C" },
	{ 1, 4, or_d, "OR D" },
	{ 1, 4, or_e, "OR E" },
	{ 1, 4, or_h, "OR H" },
	{ 1, 4, or_l, "OR L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, or_a, "OR A" }, // b7
	{ 1, 4, cp_b, "CP B" },
	{ 1, 4, cp_c, "CP C" },
	{ 1, 4, cp_a, "CP D" },
	{ 1, 4, cp_a, "CP E" },
	{ 1, 4, cp_a, "CP H" },
	{ 1, 4, cp_a, "CP L" },
	UNDEFINED_INSTRUCTION,
	{ 1, 4, cp_a, "CP A" },			// bf
	UNDEFINED_INSTRUCTION,			// c0
	{ 1, 16, pop_bc, "POP BC" },	// c1
	UNDEFINED_INSTRUCTION,			// c2
	{ 3, 16, jp_nn, "JP 0x%04X" },	// c3
	UNDEFINED_INSTRUCTION, // c4
	UNDEFINED_INSTRUCTION, // c5
	UNDEFINED_INSTRUCTION, // c6
	UNDEFINED_INSTRUCTION, // c7
	UNDEFINED_INSTRUCTION, // c8
	UNDEFINED_INSTRUCTION, // c9
	UNDEFINED_INSTRUCTION, // ca
	UNDEFINED_INSTRUCTION, // cb 
	{ 3, 0 /*variable ticks*/, call_z_nn, "CALL Z, 0x%04X" }, // cc
	{ 3, 24, call_nn, "CALL 0x%04X" }, // cd
	UNDEFINED_INSTRUCTION, // ce
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
	{ 1, 16, pop_af, "POP AF" }, // f1
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 1, 16, push_af, "PUSH AF" }, // f5
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION, // f7
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	UNDEFINED_INSTRUCTION,
	{ 2, 8, cp_n, "CP 0x%02X" }, //fe
	UNDEFINED_INSTRUCTION, //ff
};
