// Cpu6502.cpp - built-in NMOS 6502 for the RMT tracker driver (see Cpu6502.h)

#include "Cpu6502.h"

#include <cstdio>

namespace rmt_emu {

enum : uint8_t { FC = 0x01,
                 FZ = 0x02,
                 FI = 0x04,
                 FD = 0x08,
                 FB = 0x10,
                 FU = 0x20,
                 FV = 0x40,
                 FN = 0x80 };

void Cpu6502::Adc(uint8_t v)
{
    unsigned c = m_p & FC;
    if (m_p & FD) { // NMOS decimal mode
        unsigned lo = (m_a & 0x0F) + (v & 0x0F) + c;
        unsigned hi = (m_a & 0xF0) + (v & 0xF0);
        if (lo > 9) {
            lo += 6;
            hi += 0x10;
        }
        m_p &= (uint8_t) ~(FN | FZ | FV | FC);
        if (!(uint8_t)(m_a + v + c)) m_p |= FZ;
        if (hi & 0x80) m_p |= FN;
        if (~(m_a ^ v) & (m_a ^ hi) & 0x80) m_p |= FV;
        if (hi > 0x90) hi += 0x60;
        if (hi > 0xFF) m_p |= FC;
        m_a = (uint8_t)((hi & 0xF0) | (lo & 0x0F));
        return;
    }
    unsigned r = m_a + v + c;
    m_p &= (uint8_t) ~(FC | FV);
    if (r > 0xFF) m_p |= FC;
    if (~(m_a ^ v) & (m_a ^ r) & 0x80) m_p |= FV;
    m_a = (uint8_t)r;
    SetNZ(m_a);
}

void Cpu6502::Sbc(uint8_t v)
{
    if (m_p & FD) {
        unsigned borrow = (m_p & FC) ? 0 : 1;
        int lo = (m_a & 0x0F) - (v & 0x0F) - (int)borrow;
        int hi = (m_a & 0xF0) - (v & 0xF0);
        unsigned r = m_a - v - borrow;
        if (lo < 0) {
            lo -= 6;
            hi -= 0x10;
        }
        if (hi < 0) hi -= 0x60;
        m_p &= (uint8_t) ~(FC | FV);
        if (r < 0x100) m_p |= FC;
        if ((m_a ^ v) & (m_a ^ r) & 0x80) m_p |= FV;
        SetNZ((uint8_t)r);
        m_a = (uint8_t)((hi & 0xF0) | (lo & 0x0F));
        return;
    }
    Adc((uint8_t)~v);
}

void Cpu6502::Cmp(uint8_t r, uint8_t v)
{
    m_p = (uint8_t)((m_p & ~FC) | (r >= v ? FC : 0));
    SetNZ((uint8_t)(r - v));
}

uint8_t Cpu6502::Asl(uint8_t v)
{
    m_p = (uint8_t)((m_p & ~FC) | (v >> 7));
    v <<= 1;
    SetNZ(v);
    return v;
}
uint8_t Cpu6502::Lsr(uint8_t v)
{
    m_p = (uint8_t)((m_p & ~FC) | (v & 1));
    v >>= 1;
    SetNZ(v);
    return v;
}
uint8_t Cpu6502::Rol(uint8_t v)
{
    uint8_t r = (uint8_t)((v << 1) | (m_p & FC));
    m_p = (uint8_t)((m_p & ~FC) | (v >> 7));
    SetNZ(r);
    return r;
}
uint8_t Cpu6502::Ror(uint8_t v)
{
    uint8_t r = (uint8_t)((v >> 1) | ((m_p & FC) << 7));
    m_p = (uint8_t)((m_p & ~FC) | (v & 1));
    SetNZ(r);
    return r;
}

// taken: +1 cycle, +2 when the target is on another page
void Cpu6502::Branch(bool taken)
{
    int8_t off = (int8_t)Rd(m_pc++);
    if (!taken) return;
    uint16_t t = (uint16_t)(m_pc + off);
    m_extra += ((t & 0xFF00) != (m_pc & 0xFF00)) ? 2 : 1;
    m_pc = t;
}

int Cpu6502::Step()
{
    uint8_t op = Rd(m_pc++);
    uint16_t ea;
    m_extra = 0;

#define RMW(mode, fn, cyc)  \
    {                       \
        ea = mode;          \
        Wr(ea, fn(Rd(ea))); \
        return cyc;         \
    }
#define LD(reg, mode, cyc)    \
    {                         \
        reg = Rd(mode);       \
        SetNZ(reg);           \
        return cyc + m_extra; \
    }

    switch (op) {
        // ORA AND EOR ADC LDA CMP SBC: (zp,x) zp #imm abs (zp),y zp,x abs,y abs,x
        case 0x01:
            m_a |= Rd(Izx());
            SetNZ(m_a);
            return 6;
        case 0x05:
            m_a |= Rd(Zp());
            SetNZ(m_a);
            return 3;
        case 0x09:
            m_a |= Rd(Imm());
            SetNZ(m_a);
            return 2;
        case 0x0D:
            m_a |= Rd(Abs());
            SetNZ(m_a);
            return 4;
        case 0x11:
            m_a |= Rd(Izy(true));
            SetNZ(m_a);
            return 5 + m_extra;
        case 0x15:
            m_a |= Rd(Zpx());
            SetNZ(m_a);
            return 4;
        case 0x19:
            m_a |= Rd(Absi(m_y, true));
            SetNZ(m_a);
            return 4 + m_extra;
        case 0x1D:
            m_a |= Rd(Absi(m_x, true));
            SetNZ(m_a);
            return 4 + m_extra;
        case 0x21:
            m_a &= Rd(Izx());
            SetNZ(m_a);
            return 6;
        case 0x25:
            m_a &= Rd(Zp());
            SetNZ(m_a);
            return 3;
        case 0x29:
            m_a &= Rd(Imm());
            SetNZ(m_a);
            return 2;
        case 0x2D:
            m_a &= Rd(Abs());
            SetNZ(m_a);
            return 4;
        case 0x31:
            m_a &= Rd(Izy(true));
            SetNZ(m_a);
            return 5 + m_extra;
        case 0x35:
            m_a &= Rd(Zpx());
            SetNZ(m_a);
            return 4;
        case 0x39:
            m_a &= Rd(Absi(m_y, true));
            SetNZ(m_a);
            return 4 + m_extra;
        case 0x3D:
            m_a &= Rd(Absi(m_x, true));
            SetNZ(m_a);
            return 4 + m_extra;
        case 0x41:
            m_a ^= Rd(Izx());
            SetNZ(m_a);
            return 6;
        case 0x45:
            m_a ^= Rd(Zp());
            SetNZ(m_a);
            return 3;
        case 0x49:
            m_a ^= Rd(Imm());
            SetNZ(m_a);
            return 2;
        case 0x4D:
            m_a ^= Rd(Abs());
            SetNZ(m_a);
            return 4;
        case 0x51:
            m_a ^= Rd(Izy(true));
            SetNZ(m_a);
            return 5 + m_extra;
        case 0x55:
            m_a ^= Rd(Zpx());
            SetNZ(m_a);
            return 4;
        case 0x59:
            m_a ^= Rd(Absi(m_y, true));
            SetNZ(m_a);
            return 4 + m_extra;
        case 0x5D:
            m_a ^= Rd(Absi(m_x, true));
            SetNZ(m_a);
            return 4 + m_extra;
        case 0x61: Adc(Rd(Izx())); return 6;
        case 0x65: Adc(Rd(Zp())); return 3;
        case 0x69: Adc(Rd(Imm())); return 2;
        case 0x6D: Adc(Rd(Abs())); return 4;
        case 0x71: Adc(Rd(Izy(true))); return 5 + m_extra;
        case 0x75: Adc(Rd(Zpx())); return 4;
        case 0x79: Adc(Rd(Absi(m_y, true))); return 4 + m_extra;
        case 0x7D: Adc(Rd(Absi(m_x, true))); return 4 + m_extra;
        case 0xA1: LD(m_a, Izx(), 6);
        case 0xA5: LD(m_a, Zp(), 3);
        case 0xA9: LD(m_a, Imm(), 2);
        case 0xAD: LD(m_a, Abs(), 4);
        case 0xB1: LD(m_a, Izy(true), 5);
        case 0xB5: LD(m_a, Zpx(), 4);
        case 0xB9: LD(m_a, Absi(m_y, true), 4);
        case 0xBD: LD(m_a, Absi(m_x, true), 4);
        case 0xC1: Cmp(m_a, Rd(Izx())); return 6;
        case 0xC5: Cmp(m_a, Rd(Zp())); return 3;
        case 0xC9: Cmp(m_a, Rd(Imm())); return 2;
        case 0xCD: Cmp(m_a, Rd(Abs())); return 4;
        case 0xD1: Cmp(m_a, Rd(Izy(true))); return 5 + m_extra;
        case 0xD5: Cmp(m_a, Rd(Zpx())); return 4;
        case 0xD9: Cmp(m_a, Rd(Absi(m_y, true))); return 4 + m_extra;
        case 0xDD: Cmp(m_a, Rd(Absi(m_x, true))); return 4 + m_extra;
        case 0xE1: Sbc(Rd(Izx())); return 6;
        case 0xE5: Sbc(Rd(Zp())); return 3;
        case 0xE9: Sbc(Rd(Imm())); return 2;
        case 0xED: Sbc(Rd(Abs())); return 4;
        case 0xF1: Sbc(Rd(Izy(true))); return 5 + m_extra;
        case 0xF5: Sbc(Rd(Zpx())); return 4;
        case 0xF9: Sbc(Rd(Absi(m_y, true))); return 4 + m_extra;
        case 0xFD: Sbc(Rd(Absi(m_x, true))); return 4 + m_extra;

        // STA
        case 0x81: Wr(Izx(), m_a); return 6;
        case 0x85: Wr(Zp(), m_a); return 3;
        case 0x8D: Wr(Abs(), m_a); return 4;
        case 0x91: Wr(Izy(false), m_a); return 6;
        case 0x95: Wr(Zpx(), m_a); return 4;
        case 0x99: Wr(Absi(m_y, false), m_a); return 5;
        case 0x9D: Wr(Absi(m_x, false), m_a); return 5;

        // LDX LDY STX STY
        case 0xA2: LD(m_x, Imm(), 2);
        case 0xA6: LD(m_x, Zp(), 3);
        case 0xB6: LD(m_x, Zpy(), 4);
        case 0xAE: LD(m_x, Abs(), 4);
        case 0xBE: LD(m_x, Absi(m_y, true), 4);
        case 0xA0: LD(m_y, Imm(), 2);
        case 0xA4: LD(m_y, Zp(), 3);
        case 0xB4: LD(m_y, Zpx(), 4);
        case 0xAC: LD(m_y, Abs(), 4);
        case 0xBC: LD(m_y, Absi(m_x, true), 4);
        case 0x86: Wr(Zp(), m_x); return 3;
        case 0x96: Wr(Zpy(), m_x); return 4;
        case 0x8E: Wr(Abs(), m_x); return 4;
        case 0x84: Wr(Zp(), m_y); return 3;
        case 0x94: Wr(Zpx(), m_y); return 4;
        case 0x8C: Wr(Abs(), m_y); return 4;

        // transfers, stack
        case 0xAA:
            m_x = m_a;
            SetNZ(m_x);
            return 2;
        case 0x8A:
            m_a = m_x;
            SetNZ(m_a);
            return 2;
        case 0xA8:
            m_y = m_a;
            SetNZ(m_y);
            return 2;
        case 0x98:
            m_a = m_y;
            SetNZ(m_a);
            return 2;
        case 0xBA:
            m_x = m_s;
            SetNZ(m_x);
            return 2;
        case 0x9A: m_s = m_x; return 2;
        case 0x48: Push(m_a); return 3;
        case 0x68:
            m_a = Pull();
            SetNZ(m_a);
            return 4;
        case 0x08: Push((uint8_t)(m_p | FB | FU)); return 3;
        case 0x28: m_p = (uint8_t)(Pull() | FU); return 4;

        // INC DEC INX DEX INY DEY
        case 0xE8:
            m_x++;
            SetNZ(m_x);
            return 2;
        case 0xCA:
            m_x--;
            SetNZ(m_x);
            return 2;
        case 0xC8:
            m_y++;
            SetNZ(m_y);
            return 2;
        case 0x88:
            m_y--;
            SetNZ(m_y);
            return 2;
        case 0xE6:
            ea = Zp();
            Wr(ea, (uint8_t)(Rd(ea) + 1));
            SetNZ(Rd(ea));
            return 5;
        case 0xF6:
            ea = Zpx();
            Wr(ea, (uint8_t)(Rd(ea) + 1));
            SetNZ(Rd(ea));
            return 6;
        case 0xEE:
            ea = Abs();
            Wr(ea, (uint8_t)(Rd(ea) + 1));
            SetNZ(Rd(ea));
            return 6;
        case 0xFE:
            ea = Absi(m_x, false);
            Wr(ea, (uint8_t)(Rd(ea) + 1));
            SetNZ(Rd(ea));
            return 7;
        case 0xC6:
            ea = Zp();
            Wr(ea, (uint8_t)(Rd(ea) - 1));
            SetNZ(Rd(ea));
            return 5;
        case 0xD6:
            ea = Zpx();
            Wr(ea, (uint8_t)(Rd(ea) - 1));
            SetNZ(Rd(ea));
            return 6;
        case 0xCE:
            ea = Abs();
            Wr(ea, (uint8_t)(Rd(ea) - 1));
            SetNZ(Rd(ea));
            return 6;
        case 0xDE:
            ea = Absi(m_x, false);
            Wr(ea, (uint8_t)(Rd(ea) - 1));
            SetNZ(Rd(ea));
            return 7;

        // shifts
        case 0x0A: m_a = Asl(m_a); return 2;
        case 0x4A: m_a = Lsr(m_a); return 2;
        case 0x2A: m_a = Rol(m_a); return 2;
        case 0x6A: m_a = Ror(m_a); return 2;
        case 0x06: RMW(Zp(), Asl, 5)
        case 0x16: RMW(Zpx(), Asl, 6)
        case 0x0E: RMW(Abs(), Asl, 6)
        case 0x1E: RMW(Absi(m_x, false), Asl, 7)
        case 0x46: RMW(Zp(), Lsr, 5)
        case 0x56: RMW(Zpx(), Lsr, 6)
        case 0x4E: RMW(Abs(), Lsr, 6)
        case 0x5E: RMW(Absi(m_x, false), Lsr, 7)
        case 0x26: RMW(Zp(), Rol, 5)
        case 0x36: RMW(Zpx(), Rol, 6)
        case 0x2E: RMW(Abs(), Rol, 6)
        case 0x3E: RMW(Absi(m_x, false), Rol, 7)
        case 0x66: RMW(Zp(), Ror, 5)
        case 0x76: RMW(Zpx(), Ror, 6)
        case 0x6E: RMW(Abs(), Ror, 6)
        case 0x7E: RMW(Absi(m_x, false), Ror, 7)

        // compares, BIT
        case 0xE0: Cmp(m_x, Rd(Imm())); return 2;
        case 0xE4: Cmp(m_x, Rd(Zp())); return 3;
        case 0xEC: Cmp(m_x, Rd(Abs())); return 4;
        case 0xC0: Cmp(m_y, Rd(Imm())); return 2;
        case 0xC4: Cmp(m_y, Rd(Zp())); return 3;
        case 0xCC: Cmp(m_y, Rd(Abs())); return 4;
        case 0x24: {
            uint8_t v = Rd(Zp());
            m_p = (uint8_t)((m_p & ~(FN | FV | FZ)) | (v & (FN | FV)) | ((m_a & v) ? 0 : FZ));
            return 3;
        }
        case 0x2C: {
            uint8_t v = Rd(Abs());
            m_p = (uint8_t)((m_p & ~(FN | FV | FZ)) | (v & (FN | FV)) | ((m_a & v) ? 0 : FZ));
            return 4;
        }

        // branches: 2 cycles, +1 taken, +2 taken to another page
        case 0x10: Branch(!(m_p & FN)); return 2 + m_extra;
        case 0x30: Branch(m_p & FN); return 2 + m_extra;
        case 0x50: Branch(!(m_p & FV)); return 2 + m_extra;
        case 0x70: Branch(m_p & FV); return 2 + m_extra;
        case 0x90: Branch(!(m_p & FC)); return 2 + m_extra;
        case 0xB0: Branch(m_p & FC); return 2 + m_extra;
        case 0xD0: Branch(!(m_p & FZ)); return 2 + m_extra;
        case 0xF0: Branch(m_p & FZ); return 2 + m_extra;

        // jumps
        case 0x4C: m_pc = Abs(); return 3;
        case 0x6C: // NMOS: the vector does not cross a page
            ea = Abs();
            m_pc = (uint16_t)(Rd(ea) | (Rd((uint16_t)((ea & 0xFF00) | ((ea + 1) & 0xFF))) << 8));
            return 5;
        case 0x20:
            ea = Abs();
            m_pc--;
            Push((uint8_t)(m_pc >> 8));
            Push((uint8_t)m_pc);
            m_pc = ea;
            return 6;
        case 0x60:
            m_pc = Pull();
            m_pc |= (uint16_t)(Pull() << 8);
            m_pc++;
            return 6;
        case 0x40:
            m_p = (uint8_t)(Pull() | FU);
            m_pc = Pull();
            m_pc |= (uint16_t)(Pull() << 8);
            return 6;
        case 0x00: // BRK through $FFFE
            m_pc++;
            Push((uint8_t)(m_pc >> 8));
            Push((uint8_t)m_pc);
            Push((uint8_t)(m_p | FB | FU));
            m_p |= FI;
            m_pc = Rd16(0xFFFE);
            return 7;

        // flags, NOP
        case 0x18: m_p &= (uint8_t)~FC; return 2;
        case 0x38: m_p |= FC; return 2;
        case 0x58: m_p &= (uint8_t)~FI; return 2;
        case 0x78: m_p |= FI; return 2;
        case 0xB8: m_p &= (uint8_t)~FV; return 2;
        case 0xD8: m_p &= (uint8_t)~FD; return 2;
        case 0xF8: m_p |= FD; return 2;
        case 0xEA: return 2;
    }
#undef RMW
#undef LD
    return 0;
}

int Cpu6502::Jsr(uint16_t& adr, uint8_t& a, uint8_t& x, uint8_t& y, int& cycles)
{
    if (!m_mem) return 2;
    m_a = a;
    m_x = x;
    m_y = y;
    m_s = 0xFF;
    m_p = FU | FI;
    // the return address of the JSR: the routine ends when its RTS brings the
    // stack back here
    Push(0xFF);
    Push(0xFE);
    m_pc = adr;
    int result = 1;
    while (cycles > 0) {
        uint8_t op = Rd(m_pc);
        int c = Step();
        if (!c) {
            std::fprintf(stderr, "Cpu6502: undocumented opcode $%02X at $%04X\n", op, (unsigned)(m_pc - 1));
            result = 2;
            break;
        }
        cycles -= c;
        if (op == 0x60 && m_s == 0xFF) {
            result = 0;
            break;
        }
    }
    adr = m_pc;
    a = m_a;
    x = m_x;
    y = m_y;
    return result;
}

} // namespace rmt_emu

// ---------------------------------------------------------------------------
// sa_c6502.dll entry points
// ---------------------------------------------------------------------------

static rmt_emu::Cpu6502 s_cpu;

void RmtBuiltin_C6502_Initialise(unsigned char* memory)
{
    s_cpu.SetMemory(memory);
}

int RmtBuiltin_C6502_JSR(unsigned short* adr, unsigned char* a, unsigned char* x, unsigned char* y, int* cycles)
{
    return s_cpu.Jsr(*adr, *a, *x, *y, *cycles);
}

void RmtBuiltin_C6502_About(char** name, char** author, char** description)
{
    static char n[] = "RMT built-in 6502";
    static char au[] = "RASTER Music Tracker";
    static char d[] = "NMOS 6502, documented opcodes, cycle counted (replaces sa_c6502.dll)";
    *name = n;
    *author = au;
    *description = d;
}
