// Cpu6502.h - built-in NMOS 6502 for the RMT tracker driver
//
// Replaces sa_c6502.dll (Altirra's AltirraRMT6502) where it is not available:
// same entry points (C6502_Initialise / C6502_JSR / C6502_About), a flat 64 KB
// memory with no I/O (the tracker driver writes the POKEY registers at $D200
// in memory, the renderer copies them to the POKEY emulation), documented
// opcodes with their cycle counts (page crossing and taken branches included).
//
// Derived from the 6502 core of AT2019/ATARI-Driver/RmtSkeleton/tools/rmtplay.

#pragma once

#include <cstdint>

namespace rmt_emu {

class Cpu6502 {
public:
    void SetMemory(uint8_t* memory) { m_mem = memory; }

    // Run the subroutine at adr (like JSR adr) with A/X/Y until its RTS, or
    // until cycles runs out. cycles is reduced by the cycles run; A/X/Y and adr
    // (the program counter reached) are updated. Returns 0 at RTS, 1 if the
    // cycles ran out, 2 on an undocumented opcode.
    int Jsr(uint16_t& adr, uint8_t& a, uint8_t& x, uint8_t& y, int& cycles);

private:
    uint8_t* m_mem = nullptr;
    uint16_t m_pc = 0;
    uint8_t m_a = 0, m_x = 0, m_y = 0, m_s = 0xFF, m_p = 0x24;
    int m_extra = 0;                    // page crossing / branch cycles of the current instruction

    uint8_t Rd(uint16_t a) const { return m_mem[a]; }
    void Wr(uint16_t a, uint8_t v) { m_mem[a] = v; }
    uint16_t Rd16(uint16_t a) const { return (uint16_t)(Rd(a) | (Rd((uint16_t)(a + 1)) << 8)); }
    uint16_t Rd16Zp(uint8_t a) const { return (uint16_t)(Rd(a) | (Rd((uint8_t)(a + 1)) << 8)); }
    void Push(uint8_t v) { Wr((uint16_t)(0x100 + m_s--), v); }
    uint8_t Pull() { return Rd((uint16_t)(0x100 + ++m_s)); }

    void SetNZ(uint8_t v) { m_p = (uint8_t)((m_p & 0x7D) | (v & 0x80) | (v ? 0 : 0x02)); }
    void Adc(uint8_t v);
    void Sbc(uint8_t v);
    void Cmp(uint8_t r, uint8_t v);
    uint8_t Asl(uint8_t v);
    uint8_t Lsr(uint8_t v);
    uint8_t Rol(uint8_t v);
    uint8_t Ror(uint8_t v);
    void Branch(bool taken);

    // effective addresses; the ",X" ",Y" "(zp),Y" reads add a cycle on a page crossing
    uint16_t Zp() { return Rd(m_pc++); }
    uint16_t Zpx() { return (uint8_t)(Rd(m_pc++) + m_x); }
    uint16_t Zpy() { return (uint8_t)(Rd(m_pc++) + m_y); }
    uint16_t Abs() { uint16_t a = Rd16(m_pc); m_pc += 2; return a; }
    uint16_t Absi(uint8_t i, bool read) {
        uint16_t b = Abs(), a = (uint16_t)(b + i);
        if (read && (a & 0xFF00) != (b & 0xFF00)) m_extra++;
        return a;
    }
    uint16_t Izx() { return Rd16Zp((uint8_t)(Rd(m_pc++) + m_x)); }
    uint16_t Izy(bool read) {
        uint16_t b = Rd16Zp(Rd(m_pc++)), a = (uint16_t)(b + m_y);
        if (read && (a & 0xFF00) != (b & 0xFF00)) m_extra++;
        return a;
    }
    uint16_t Imm() { return m_pc++; }

    // one instruction; returns its cycles, 0 for an undocumented opcode
    int Step();
};

} // namespace rmt_emu

// The sa_c6502.dll entry points, built in
void RmtBuiltin_C6502_Initialise(unsigned char* memory);
int RmtBuiltin_C6502_JSR(unsigned short* adr, unsigned char* a, unsigned char* x, unsigned char* y, int* cycles);
void RmtBuiltin_C6502_About(char** name, char** author, char** description);
