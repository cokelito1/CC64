#pragma once

#include <cstdint>
#include <array>


#include <vector>

#include <SFML/System.hpp>

/*
* Shitty notes:
* - The accumulator serves as the input to the inner world and output to the outer world
* - Cycles per second = 1024000/100
* - To detect overflow if the two values are positive the 7 bit should be 0 if not set overflow, on negative numbers the same but
*   the numbers flipped
* - 2 Major mode of indexing, Non-Indexed and Indexed
* - 0xFF pages, one define per PCH increment, 0xFF bytes per pages defined by PCL increment
* - The MOS520X has 2 register for indexing named X and Y, one can use this for Indexed Addresing
*
* - Implied mode is when the instruction has encoded in one byte the src and dst
* - Absolute mode is when the instruction is followed by two bytes that have the dst
* - Immediate mode is when the instruction has a byte next that's constant an use as the second parameter
* - Zero page mode is when the instruction has 2 bytes the first is the instruction the second is the PCL to 0x00;
* -	Relative mode is when the instruction has 2 bytes, the first is the instruction the second is and offset to current PC
* - Indirect mode is when the instruction has 2 bytes, the first is the instruction the second is used get the zero page of location of the PC 
* -
* - Absolute indexed mode is when the instruction is 3 bytes long and you add an index register to the PC of absolute address, set carry if page crossed
* - Zero page indexed mode is when the instruction is 2 byte long and is computed as PCH | (Mem[PC+1] + IndexRegX) % 255  
* - Indirect indexed mode is the same as Indirect non-indexed except adding IndexRegX to the second byte
* -
* - LDA = Load from memory to accumulator, M -> A, Sets Negative, Zero
* - STA = Store from Accumulator to memory, A -> M, Sets none
* - ADC = Add Accumulator with Memory and Carry, A + M + C -> A, Sets Carry Overflow, Zero
* - SBC = Substract Accumulator with Memory and Borrow, A - M - ~(C) -> A, Sets Carry if result >= 0, Overflow, Negative, Zero
* - SEC = Set carry flag, 1 -> C
* - CLC = Clear carry flag, 0 -> C
* - SEI = Set irq disable, 1 -> IRQ
* - CLI = Clear irq disable, 0 -> IRQ
* - SED = Set Decimal, 1 -> DCM
* - CLD = Clear Decimal, 0 -> DCM
* - CLV = Clear overflow, 0 -> Overflow
* - JMP = Jump to mem[PC] | mem[PC+1] << 8, PCL -> M[PC], PCH ->M[PC + 1] 
* - BMI = Branch on Negative set, if(Negative) { PC += mem[PC + 1]; }
* - BPL = Branch on Negative clear, if(!Negative) { PC += mem[PC + 1]; }
* - BCC = Branch on Carry clear, if(!Carry) { PC += mem[PC + 1]; }
* - BCS = Branch on Carry set, if(Carry) { PC += mem[PC + 1]; }
* - BEQ = Branch on Zero, if(Zero) { PC += mem[PC + 1]; }
* - BNE = Branch on not Zero, if(!Zero) { PC += mem[PC + 1]; }
* - BVS = Branch on overflow set, if(Overflow) { PC += mem[PC + 1]; }
* - BVC = Branch on overflow clear, if(!Overflow) { PC += mem[PC + 1]; }
* - CMP = Compare Memory and Accumulator, Sets zero if (Accumulator - Memory == 0), set negative if(Memory > Accumulator), set carry if(Memory <= Accumulator)
* - BIT = Test bit in memory with Accumulator, Sets N to Mem & FLAG_NEGATIVE, Sets Overflow to Mem & FLAG_OVERFLOW, Sets zero to the AND result 
*
* Diagram of internal shits:
*                                                  8 bit bus
*		                                              |
*                                                     v
*  ----------------------------------------------------------------------------------------------------
*  ^                   ^                  ^             ^             ^                 ^             ^
*  |                   |                  |             |             |                 |             |
*  v                   v                  v             v			  v                 v             v
* ALU <---------> Accumulator     Processor Status      PC			Memory          IndexRegX     IndexRegY
*                                                       ^
*                                                       |
*                                                   ----+----
*                                                   |       |
*                                                   v       v
*                                                  PCH     PCL
*
* Memory Diagram
*
* ----- 0x0000
* |RAM|
* |---| 0x3FFF
* |I/O|
* |---| 0x7FFF
* |ROM|
* ----- 0xFFFF
*/

class VIC2;

enum FlagsMask {
	MASK_CARRY		= 0x01,
	MASK_ZERO		= 0x02,
	MASK_IRQ		= 0x04,
	MASK_DCM		= 0x08,
	MASK_BREAK		= 0x10,
	MASK_EXP		= 0x20,
	MASK_OVERFLOW	= 0x40,
	MASK_NEGATIVE	= 0x80,
};

enum BankConfiguration {
	CONF_RAM_RAM_RAM_BASICROM_RAM_IO_KERNAL = 31,
	CONF_RAM_RAM_RAM_RAM_RAM_IO_KERNAL = 30,
	CONF_RAM_RAM_RAM_RAM_RAM_IO_RAM = 29,
	CONF_RAM_RAM_RAM_RAM_RAM_RAM_RAM = 28,
	CONF_RAM_RAM_RAM_BASICROM_RAM_CHARSET_KERNAL = 27,
	CONF_RAM_RAM_RAM_RAM_RAM_CHARSET_KERNAL = 26,
	CONF_RAM_RAM_RAM_RAM_CHARSET_RAM = 25,
	CONF_RAM_CARTLO_IO_CARTHI = 23,
	CONF_RAM_RAM_CARTLO_BASICROM_RAM_IO_KERNAL = 15,
	CONF_RAM_RAM_CARTLO_BASICROM_RAM_CHARSET_KERNAL = 11,
	CONF_RAM_RAM_CARTLO_CARTHI_RAM_IO_KERNAL = 7,
	CONF_RAM_RAM_RAM_CARTHI_RAM_IO_KERNAL = 6,
	CONF_RAM_RAM_CARTLO_CARTHI_RAM_CHARSET_KERNAL = 3,
	CONF_RAM_RAM_RAM_CARTHI_RAM_CHARSET_KERNAL = 2,

};

union DWORD_Register {
	struct {
		uint8_t LOW;
		uint8_t HIGH;
	}bytes;
	uint16_t DWORD;
};

class MOS6502 {
public:
	MOS6502(bool TestSuite = false);
	~MOS6502();

	VIC2* GetVic() { return Vic; }

	int Exec();
	std::vector<sf::Uint8> UpdateFrame();

	uint8_t ReadMemory(uint16_t Address);
	uint64_t GetCycles() { return cycles; }
	void WriteMemory(uint16_t Address, uint8_t Data);
	void WriteToIO(uint16_t Address, uint8_t Data);
	void SetInterrupt(bool interrupt, uint16_t IRQVector) { this->interrupt = interrupt; NextIRQVector = IRQVector; }
	void Test();
private:
	typedef int (MOS6502::*Opcode)();

	uint8_t Accumulator;
	uint8_t IndexRegX;
	uint8_t IndexRegY;
	uint8_t ProccessorStatus;

	DWORD_Register PC;
	DWORD_Register SP;

	void inline set(uint8_t mask) { ProccessorStatus |= mask; }
	void inline clear(uint8_t mask) { ProccessorStatus = ProccessorStatus & ~(mask); }
	uint8_t inline get(uint8_t mask) { return ProccessorStatus & mask; }

	
	int BRK();

	int ADCImm();
	int ADCZeroPage();
	int ADCZeroPageIndexed();
	int ADCAbsolute();
	int ADCAbsoluteIndexedX();
	int ADCAbsoluteIndexedY();
	int ADCIndirectIndexedX();
	int ADCIndirectIndexedY();

	int SBCImm();
	int SBCAbsolute();
	int SBCZeroPage();
	int SBCAbsoluteIndexedX();

	int ANDImm();
	int ANDZeroPage();
	int ANDZeroPageIndexed();
	int ANDAbsolute();
	int ANDAbsoluteIndexedX();
	int ANDAbsoluteIndexedY();
	int ANDIndirectIndexedX();
	int ANDIndirectIndexedY();

	int ORAImm();
	int ORAZeroPage();
	int ORAAbsolute();
	int ORAIndirectIndexedY();

	int NOP();

	int EORImm();
	int EORZeroPage();
	int EORZeroPageIndexedX();

	int LSRA();
	int LSRZeroPage();
	int LSRZeroPageIndexed();

	int ASLA();
	int ASLZeroPage();
	int ASLZeroPageIndexed();

	int ROLA();
	int ROLZeroPage();

	int INCZeroPage();

	int LDAImm();
	int LDAAbsolute();
	int LDAAbsoluteIndexedX();
	int LDAAbsoluteIndexedY();
	int LDAZeroPage();
	int LDAZeroPageIndexed();
	int LDAIndirectIndexedY();

	int JMPImm();
	int JMPIndirect();

	int BNE();
	int BEQ();
	int BPL();
	int BMI();
	int BCC();
	int BCS();
	int BVS();
	int BVC();

	int JSR();
	int RTS();
	int RTI();

	int BITAbsolute();

	int LDXImm();
	int LDXZeroPage();
	int LDXAbsolute();

	int LDYImm();
	int LDYAbsolute();
	int LDYZeroPage();
	int LDYZeroPageIndexed();

	int SEI();
	int SEC();
	int SED();
	int CLI();
	int CLC();
	int CLD();
	int CLV();

	int PLA();
	int PLP();

	int PHA();
	int PHP();

	int TXS();
	int TXA();
	int TAX();
	int TYA();
	int TAY();
	int TSX();

	int CMPImm();
	int CMPZeroPage();
	int CMPAbsoluteIndexedX();
	int CMPAbsolute();
	int CMPIndirectIndexedY();

	int CMXImm();
	int CMXZeroPage();
	int CMXAbsolute();

	int CPYZeroPage();
	int CPYImm();

	int INCAbsolute();

	int DECZeroPage();
	int DECAbsolute();

	int DEX();
	int INX();
	int DEY();
	int INY();

	int BITZeroPage();

	int STXAbsolute();
	int STXZeroPage();
	int STYZeroPage();
	int STYZeroPageIndexed();
	int STYAbsolute();
	int STAAbsolute();
	int STAAbsoluteIndexedX();
	int STAAbsoluteIndexedY();
	int STAZeroPage();
	int STAZeroPageIndexed();
	int STAIndirectIndexedY();

	std::vector<uint8_t> LoadFile(const std::string& filename, size_t Size);

	void LoadCharset(std::vector<uint8_t> Charset);
	void LoadKernal(std::vector<uint8_t> Kernal);
	void LoadBasicInterpreter(std::vector<uint8_t> Interpreter);
	void LoadTestSuite(std::vector<uint8_t> TestSuite);

	void DumpMemory(const std::string& Filename);

	int StubOpcode();

	Opcode OpcodeTable[0x100];

	void inline RegisterOpcode(uint8_t Address, Opcode op) { OpcodeTable[Address] = op; }

	std::array<uint8_t, 0x10000> Memory;
	std::array<uint8_t, 0xE000 - 0xD000> Charset;
	std::array<uint8_t, 0xE000 - 0xD000 + 1> IO;
	std::array<uint8_t, 0xFFFF - 0xE000 + 1> Kernal;
	std::array<uint8_t, 0xBFFF - 0xA000 + 1> Basic;

	uint16_t GenerateZeroPageIndexedAddress();
	uint16_t GenerateAbsoluteIndexedX(int& PageCrossed);
	uint16_t GenerateAbsoluteIndexedY(int& PageCrossed);
	uint16_t GenerateIndirectIndexedX();
	uint16_t GenerateIndirectIndexedY(int& PageCrossed);

	uint8_t lastopcode = 0x00;
	uint8_t CurrentOpcode = 0x00;
	uint16_t NextIRQVector = 0x00;

	bool interrupt = false;

	uint64_t cycles = 0;
	BankConfiguration current;
	VIC2 *Vic;
};