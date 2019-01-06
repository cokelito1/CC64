#include "MOS6502.h"
#include "VICII.h"

#include <cstdio>
#include <iostream>
#include <fstream>

#define CONDITIONAL_SET(mask, x) if((x)) { set((mask)); } else { clear((mask)); }

MOS6502::MOS6502(bool TestSuite) {
	ProccessorStatus = MASK_EXP;
	Accumulator = 0x00;
	IndexRegX = 0x00;
	IndexRegY = 0x00;
	PC.DWORD = 0x0000;
	SP.DWORD = 0x01FF;

	for (unsigned int i = 0; i < Memory.size(); i++) {
		Memory[i] = 0xFF;
	}

	for (unsigned int i = 0; i < IO.size(); i++) {
		IO[i] = 0x00;
	}

	try {
		if (!TestSuite) {
			LoadCharset(LoadFile("charset.u5", 4096));
			LoadKernal(LoadFile("kernal.u4", 8192));
			LoadBasicInterpreter(LoadFile("basic.u3", 8192));
		}
		else {
			LoadTestSuite(LoadFile("6502_functional_test.bin", 0x10000));
		}
		DumpMemory("dump.bin");
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
	for (unsigned int i = 0; i < 0x100; i++) {
		RegisterOpcode(i, &MOS6502::StubOpcode);
	}

	RegisterOpcode(0x00, &MOS6502::BRK);
	RegisterOpcode(0x05, &MOS6502::ORAZeroPage);
	RegisterOpcode(0x06, &MOS6502::ASLZeroPage);
	RegisterOpcode(0x08, &MOS6502::PHP);
	RegisterOpcode(0x09, &MOS6502::ORAImm);
	RegisterOpcode(0x0A, &MOS6502::ASLA);
	RegisterOpcode(0x0D, &MOS6502::ORAAbsolute);
	RegisterOpcode(0x10, &MOS6502::BPL);
	RegisterOpcode(0x11, &MOS6502::ORAIndirectIndexedY);
	RegisterOpcode(0x16, &MOS6502::ASLZeroPageIndexed);
	RegisterOpcode(0x18, &MOS6502::CLC);
	RegisterOpcode(0x20, &MOS6502::JSR);
	RegisterOpcode(0x21, &MOS6502::ANDIndirectIndexedX);
	RegisterOpcode(0x24, &MOS6502::BITZeroPage);
	RegisterOpcode(0x25, &MOS6502::ANDZeroPage);
	RegisterOpcode(0x26, &MOS6502::ROLZeroPage);
	RegisterOpcode(0x28, &MOS6502::PLP);
	RegisterOpcode(0x29, &MOS6502::ANDImm);
	RegisterOpcode(0x2A, &MOS6502::ROLA);
	RegisterOpcode(0x2C, &MOS6502::BITAbsolute);
	RegisterOpcode(0x2D, &MOS6502::ANDAbsolute);
	RegisterOpcode(0x30, &MOS6502::BMI);
	RegisterOpcode(0x31, &MOS6502::ANDIndirectIndexedY);
	RegisterOpcode(0x35, &MOS6502::ANDZeroPageIndexed);
	RegisterOpcode(0x38, &MOS6502::SEC);
	RegisterOpcode(0x39, &MOS6502::ANDAbsoluteIndexedY);
	RegisterOpcode(0x3D, &MOS6502::ANDAbsoluteIndexedX);
	RegisterOpcode(0x40, &MOS6502::RTI);
	RegisterOpcode(0x45, &MOS6502::EORZeroPage);
	RegisterOpcode(0x46, &MOS6502::LSRZeroPage);
	RegisterOpcode(0x48, &MOS6502::PHA);
	RegisterOpcode(0x49, &MOS6502::EORImm);
	RegisterOpcode(0x4A, &MOS6502::LSRA);
	RegisterOpcode(0x4C, &MOS6502::JMPImm);
	RegisterOpcode(0x50, &MOS6502::BVC);
	RegisterOpcode(0x55, &MOS6502::EORZeroPageIndexedX);
	RegisterOpcode(0x56, &MOS6502::LSRZeroPageIndexed);
	RegisterOpcode(0x58, &MOS6502::CLI);
	RegisterOpcode(0x60, &MOS6502::RTS);
	RegisterOpcode(0x61, &MOS6502::ADCIndirectIndexedX);
	RegisterOpcode(0x65, &MOS6502::ADCZeroPage);
	RegisterOpcode(0x68, &MOS6502::PLA);
	RegisterOpcode(0x69, &MOS6502::ADCImm);
	RegisterOpcode(0x6C, &MOS6502::JMPIndirect);
	RegisterOpcode(0x6D, &MOS6502::ADCAbsolute);
	RegisterOpcode(0x70, &MOS6502::BVS);
	RegisterOpcode(0x71, &MOS6502::ADCIndirectIndexedY);
	RegisterOpcode(0x75, &MOS6502::ADCZeroPageIndexed);
	RegisterOpcode(0x78, &MOS6502::SEI);
	RegisterOpcode(0x79, &MOS6502::ADCAbsoluteIndexedY);
	RegisterOpcode(0x7D, &MOS6502::ADCAbsoluteIndexedX);
	RegisterOpcode(0x84, &MOS6502::STYZeroPage);
	RegisterOpcode(0x85, &MOS6502::STAZeroPage);
	RegisterOpcode(0x86, &MOS6502::STXZeroPage);
	RegisterOpcode(0x88, &MOS6502::DEY);
	RegisterOpcode(0x8A, &MOS6502::TXA);
	RegisterOpcode(0x8C, &MOS6502::STYAbsolute);
	RegisterOpcode(0x8D, &MOS6502::STAAbsolute);
	RegisterOpcode(0x8E, &MOS6502::STXAbsolute);
	RegisterOpcode(0x90, &MOS6502::BCC);
	RegisterOpcode(0x91, &MOS6502::STAIndirectIndexedY);
	RegisterOpcode(0x94, &MOS6502::STYZeroPageIndexed);
	RegisterOpcode(0x95, &MOS6502::STAZeroPageIndexed);
	RegisterOpcode(0x98, &MOS6502::TYA);
	RegisterOpcode(0x99, &MOS6502::STAAbsoluteIndexedY);
	RegisterOpcode(0x9A, &MOS6502::TXS);
	RegisterOpcode(0x9D, &MOS6502::STAAbsoluteIndexedX);
	RegisterOpcode(0xA0, &MOS6502::LDYImm);
	RegisterOpcode(0xA2, &MOS6502::LDXImm);
	RegisterOpcode(0xA4, &MOS6502::LDYZeroPage);
	RegisterOpcode(0xA5, &MOS6502::LDAZeroPage);
	RegisterOpcode(0xA6, &MOS6502::LDXZeroPage);
	RegisterOpcode(0xA8, &MOS6502::TAY);
	RegisterOpcode(0xA9, &MOS6502::LDAImm);
	RegisterOpcode(0xAA, &MOS6502::TAX);
	RegisterOpcode(0xAC, &MOS6502::LDYAbsolute);
	RegisterOpcode(0xAE, &MOS6502::LDXAbsolute);
	RegisterOpcode(0xAD, &MOS6502::LDAAbsolute);
	RegisterOpcode(0xB0, &MOS6502::BCS);
	RegisterOpcode(0xB1, &MOS6502::LDAIndirectIndexedY);
	RegisterOpcode(0xB4, &MOS6502::LDYZeroPageIndexed);
	RegisterOpcode(0xB5, &MOS6502::LDAZeroPageIndexed);
	RegisterOpcode(0xB8, &MOS6502::CLV);
	RegisterOpcode(0xB9, &MOS6502::LDAAbsoluteIndexedY);
	RegisterOpcode(0xBA, &MOS6502::TSX);
	RegisterOpcode(0xBD, &MOS6502::LDAAbsoluteIndexedX);
	RegisterOpcode(0xC0, &MOS6502::CPYImm);
	RegisterOpcode(0xC4, &MOS6502::CPYZeroPage);
	RegisterOpcode(0xC5, &MOS6502::CMPZeroPage);
	RegisterOpcode(0xC6, &MOS6502::DECZeroPage);
	RegisterOpcode(0xC8, &MOS6502::INY);
	RegisterOpcode(0xC9, &MOS6502::CMPImm);
	RegisterOpcode(0xCA, &MOS6502::DEX);
	RegisterOpcode(0xCD, &MOS6502::CMPAbsolute);
	RegisterOpcode(0xCE, &MOS6502::DECAbsolute);
	RegisterOpcode(0xD0, &MOS6502::BNE);
	RegisterOpcode(0xD1, &MOS6502::CMPIndirectIndexedY);
	RegisterOpcode(0xD8, &MOS6502::CLD);
	RegisterOpcode(0xDD, &MOS6502::CMPAbsoluteIndexedX);
	RegisterOpcode(0xE0, &MOS6502::CMXImm);
	RegisterOpcode(0xE4, &MOS6502::CMXZeroPage);
	RegisterOpcode(0xE5, &MOS6502::SBCZeroPage);
	RegisterOpcode(0xE6, &MOS6502::INCZeroPage);
	RegisterOpcode(0xE8, &MOS6502::INX);
	RegisterOpcode(0xE9, &MOS6502::SBCImm);
	RegisterOpcode(0xEA, &MOS6502::NOP);
	RegisterOpcode(0xEC, &MOS6502::CMXAbsolute);
	RegisterOpcode(0xED, &MOS6502::SBCAbsolute);
	RegisterOpcode(0xEE, &MOS6502::INCAbsolute);
	RegisterOpcode(0xF0, &MOS6502::BEQ);
	RegisterOpcode(0xF8, &MOS6502::SED);
	RegisterOpcode(0xFD, &MOS6502::SBCAbsoluteIndexedX);

	current = CONF_RAM_RAM_RAM_BASICROM_RAM_IO_KERNAL;

	Vic = new VIC2(Memory, Charset, this);

	PC.DWORD = ReadMemory(0xFFFC);
	PC.DWORD |= (ReadMemory(0xFFFD) << 8);

}

MOS6502::~MOS6502() {
	delete Vic;
}

std::vector<sf::Uint8> MOS6502::UpdateFrame() {
	std::vector<sf::Uint8> ret = { 0xCA, 0xFE };
	if (ReadMemory(0xD010) <= 5) {
		return Vic->UpdateFrame();
	}
	return ret;
}

int MOS6502::Exec() {
	Vic->Clock();
	if (interrupt) {
		WriteMemory(SP.DWORD, PC.bytes.HIGH);
		SP.DWORD--;
		WriteMemory(SP.DWORD, PC.bytes.LOW);
		SP.DWORD--;
		//clear(MASK_BREAK);
		WriteMemory(SP.DWORD, ProccessorStatus);
		SP.DWORD--;

		uint16_t Vector = ReadMemory(NextIRQVector);
		Vector |= (ReadMemory(NextIRQVector + 1) << 8);
		PC.DWORD = Vector;
		interrupt = 0;
	}
	CurrentOpcode = ReadMemory(PC.DWORD);
 	PC.DWORD++;
	ProccessorStatus |= MASK_EXP;
	if (SP.bytes.HIGH != 0x01) {
		SP.bytes.HIGH = 0x01;
	}
	int ret = (this->*(OpcodeTable[CurrentOpcode]))();
	lastopcode = CurrentOpcode;
	cycles += ret;
	if (cycles >= (1024000 / 60)) {
		cycles = 0;
	}
	return ret;
}

int MOS6502::ADCImm() {
	uint8_t Val = ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, Sum == 0);
	CONDITIONAL_SET(MASK_CARRY, (Sum & 0x0100) == 0x0100);
	CONDITIONAL_SET(MASK_NEGATIVE, (Sum & 0x80) == 0x80);
	
	Accumulator = Sum & 0xFF;
	return 2;
}

int MOS6502::ADCZeroPage() {
	uint16_t DataPointer = 0x00;
	DataPointer |= ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, Sum == 0);
	CONDITIONAL_SET(MASK_CARRY, (Sum & 0x0100) == 0x0100);
	CONDITIONAL_SET(MASK_NEGATIVE, (Sum & 0x80) == 0x80);

	Accumulator = Sum & 0xFF;
	return 3;
}

int MOS6502::ADCZeroPageIndexed() {
	uint16_t DataPointer = GenerateZeroPageIndexedAddress();
	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, Sum == 0);
	CONDITIONAL_SET(MASK_CARRY, (Sum & 0x0100) == 0x0100);
	CONDITIONAL_SET(MASK_NEGATIVE, (Sum & 0x80) == 0x80);

	Accumulator = Sum & 0xFF;
	return 4;
}

int MOS6502::ADCAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, Sum == 0);
	CONDITIONAL_SET(MASK_CARRY, (Sum & 0x0100) == 0x0100);
	CONDITIONAL_SET(MASK_NEGATIVE, (Sum & 0x80) == 0x80);

	Accumulator = Sum & 0xFF;
	return 4;
}

int MOS6502::ADCAbsoluteIndexedX() {
	int PageCrossed = 0;

	uint16_t DataPointer = GenerateAbsoluteIndexedX(PageCrossed);
	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, ((((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (Sum == 0));
	CONDITIONAL_SET(MASK_CARRY, ((Sum & 0x0100) == 0x0100));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Sum & 0x80) == 0x80));

	Accumulator = Sum & 0xFF;
	return 4 + PageCrossed;
}

int MOS6502::ADCAbsoluteIndexedY() {
	int PageCrossed = 0;

	uint16_t DataPointer = GenerateAbsoluteIndexedY(PageCrossed);
	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, (Sum == 0));
	CONDITIONAL_SET(MASK_CARRY, ((Sum & 0x0100) == 0x0100));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Sum & 0x80) == 0x80));

	Accumulator = Sum & 0xFF;
	return 4 + PageCrossed;
}

int MOS6502::ADCIndirectIndexedX() {
	uint16_t Address = GenerateIndirectIndexedX();
	uint8_t Val = ReadMemory(Address);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, ((((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (Sum == 0));
	CONDITIONAL_SET(MASK_CARRY, ((Sum & 0x0100) == 0x0100));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Sum & 0x80) == 0x80));

	Accumulator = Sum & 0xFF;
	return 6;
}

int MOS6502::ADCIndirectIndexedY() {
	int PageCrossed = 0;
	
	uint16_t DataPointer = GenerateIndirectIndexedY(PageCrossed);
	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sum = Accumulator + Val + get(MASK_CARRY);
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sum) & 0x10) == 0x10) {
			Sum += 0x6;
		}
		if ((Sum & 0xF0) > 0x90) {
			Sum += 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sum) & (Val ^ Sum)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, Sum == 0);
	CONDITIONAL_SET(MASK_CARRY, (Sum & 0x0100) == 0x0100);
	CONDITIONAL_SET(MASK_NEGATIVE, (Sum & 0x80) == 0x80);

	Accumulator = Sum & 0xFF;
	return 5 + PageCrossed;
}

int MOS6502::SBCImm() {
	uint8_t Val = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint16_t Sub = Accumulator - Val - ~(get(MASK_CARRY));
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sub) & 0x10) == 0x10) {
			Sub -= 0x6;
		}
		if ((Sub & 0xF0) > 0x90) {
			Sub -= 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sub) & (Val ^ Sub)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, (Sub == 0));
	CONDITIONAL_SET(MASK_CARRY, (Sub >= 0));
	CONDITIONAL_SET(MASK_NEGATIVE, (((Sub & 0x80)) == 0x80));

	Accumulator = (Sub & 0x00FF);
	return 2;
}

int MOS6502::SBCZeroPage() {
	uint8_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sub = Accumulator - Val - ~(get(MASK_CARRY));
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sub) & 0x10) == 0x10) {
			Sub -= 0x6;
		}
		if ((Sub & 0xF0) > 0x90) {
			Sub -= 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sub) & (Val ^ Sub)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, (Sub == 0));
	CONDITIONAL_SET(MASK_CARRY, (Sub >= 0));
	CONDITIONAL_SET(MASK_NEGATIVE, (((Sub & 0x80)) == 0x80));

	Accumulator = (Sub & 0x00FF);
	return 3;
}

int MOS6502::SBCAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sub = Accumulator - Val - ~(get(MASK_CARRY));
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sub) & 0x10) == 0x10) {
			Sub -= 0x6;
		}
		if ((Sub & 0xF0) > 0x90) {
			Sub -= 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sub) & (Val ^ Sub)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, (Sub == 0));
	CONDITIONAL_SET(MASK_CARRY, (Sub >= 0));
	CONDITIONAL_SET(MASK_NEGATIVE, (((Sub & 0x80)) == 0x80));

	Accumulator = (Sub & 0x00FF);
	return 4;
}

int MOS6502::SBCAbsoluteIndexedX() {
	int PageCrossed = 0;
	uint16_t DataPointer = GenerateAbsoluteIndexedX(PageCrossed);

	uint8_t Val = ReadMemory(DataPointer);
	uint16_t Sub = Accumulator - Val - ~(get(MASK_CARRY));
	if (get(MASK_DCM)) {
		if (((Accumulator ^ Val ^ Sub) & 0x10) == 0x10) {
			Sub -= 0x6;
		}
		if ((Sub & 0xF0) > 0x90) {
			Sub -= 0x60;
		}
	}

	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator ^ Sub) & (Val ^ Sub)) & 0x80) == 0x80);
	CONDITIONAL_SET(MASK_ZERO, (Sub == 0));
	CONDITIONAL_SET(MASK_CARRY, (Sub >= 0));
	CONDITIONAL_SET(MASK_NEGATIVE, (((Sub & 0x80)) == 0x80));

	Accumulator = (Sub & 0x00FF);
	return 4 + PageCrossed;
}

int MOS6502::ANDImm() {
	uint8_t Val = ReadMemory(PC.DWORD);
	PC.DWORD++;

	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 2;
}

int MOS6502::ANDZeroPage() {
	uint16_t DataPointer = 0x00;
	DataPointer |= ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint8_t Val = ReadMemory(DataPointer);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 3;
}

int MOS6502::ANDZeroPageIndexed() {
	uint16_t DataPointer = GenerateZeroPageIndexedAddress();
	uint8_t Val = ReadMemory(DataPointer);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 4;
}

int MOS6502::ANDAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	uint8_t Val = ReadMemory(DataPointer);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 4;
}

int MOS6502::ANDAbsoluteIndexedX() {
	int PageCrossed = 0;
	
	uint16_t DataPointer = GenerateAbsoluteIndexedX(PageCrossed);
	uint8_t Val = ReadMemory(DataPointer);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 4 + PageCrossed;
}

int MOS6502::ANDAbsoluteIndexedY() {
	int PageCrossed = 0;

	uint16_t DataPointer = GenerateAbsoluteIndexedY(PageCrossed);

	uint8_t Val = ReadMemory(DataPointer);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 4 + PageCrossed;
}

int MOS6502::ANDIndirectIndexedX() {
	uint16_t Address = GenerateIndirectIndexedX();

	uint8_t Val = ReadMemory(Address);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 6;
}

int MOS6502::ANDIndirectIndexedY() {
	int PageCrossed = 0;

	uint16_t DataPointer = GenerateIndirectIndexedY(PageCrossed);
	uint8_t Val = ReadMemory(DataPointer);
	Accumulator &= Val;

	CONDITIONAL_SET(MASK_NEGATIVE, (Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 5 + PageCrossed;
}

int MOS6502::ORAImm() {
	uint8_t Val = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Accumulator |= Val;

	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::ORAZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t Val = ReadMemory(Address);
	Accumulator |= Val;

	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 3;
}

int MOS6502::ORAAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	uint8_t Val = ReadMemory(Address);
	Accumulator |= Val;

	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 4;
}

int MOS6502::ORAIndirectIndexedY() {
	int PageCrossed = 0;
	uint16_t DataPointer = GenerateIndirectIndexedY(PageCrossed);

	uint8_t Val = ReadMemory(DataPointer);
	Accumulator |= Val;

	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 5 + PageCrossed;
}

int MOS6502::EORImm() {
	uint8_t Val = ReadMemory(PC.DWORD);
	PC.DWORD++;

	Accumulator ^= Val;
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 2;
}

int MOS6502::EORZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t Val = ReadMemory(Address);

	Accumulator ^= Val;
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 2;
}

int MOS6502::EORZeroPageIndexedX() {
	uint16_t Address = GenerateZeroPageIndexedAddress();
	uint8_t Val = ReadMemory(Address);

	Accumulator ^= Val;
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 4;
}

int MOS6502::JMPImm() {
	uint16_t NewPC = ReadMemory(PC.DWORD);
	PC.DWORD++;
	NewPC |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	PC.DWORD = NewPC;
	return 3;
}

int MOS6502::JMPIndirect() {
	uint16_t Vector = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Vector |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	uint16_t Address = ReadMemory(Vector);
	if ((Vector & 0x00FF) == 0xFF) {
		Address |= (ReadMemory(Vector & 0xFF00) << 8);
	}
	else {
		Address |= (ReadMemory(Vector + 1) << 8);
	}
	PC.DWORD = Address;
	return 5;
}

int MOS6502::BPL() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (get(MASK_NEGATIVE)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::BMI() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (!get(MASK_NEGATIVE)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}


int MOS6502::BNE() {
	int8_t Offset = ReadMemory(PC.DWORD);
	
	if (get(MASK_ZERO)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::BEQ() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (!get(MASK_ZERO)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::BVS() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (!get(MASK_OVERFLOW)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::BVC() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (get(MASK_OVERFLOW)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::BCC() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (get(MASK_CARRY)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::BCS() {
	int8_t Offset = ReadMemory(PC.DWORD);
	if (!get(MASK_CARRY)) {
		PC.DWORD++;
		return 2;
	}

	uint8_t PCH = PC.bytes.HIGH;
	PC.DWORD += Offset + 1;

	if (PC.bytes.HIGH != PCH) {
		return 4;
	}

	return 3;
}

int MOS6502::LDXImm() {
	IndexRegX = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0));

	return 2;
}

int MOS6502::LDXAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	IndexRegX = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0));
	return 4;
}

int MOS6502::LDXZeroPage() {
	uint8_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	IndexRegX = ReadMemory(DataPointer);

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0));

	return 3;
}

int MOS6502::LDYImm() {
	IndexRegY = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0));

	return 2;
}

int MOS6502::LDYAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	IndexRegY = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0));
	return 4;
}

int MOS6502::LDYZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	IndexRegY = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0));

	return 3;
}

int MOS6502::LDYZeroPageIndexed() {
	uint16_t Address = GenerateZeroPageIndexedAddress();
	IndexRegY = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & 0x80) == 0x80));
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0));

	return 4;
}

int MOS6502::SEI() {
	set(MASK_IRQ);
	return 2;
}

int MOS6502::SEC() {
	set(MASK_CARRY);
	return 2;
}

int MOS6502::SED() {
	set(MASK_DCM);
	return 2;
}

int MOS6502::CLD() {
	clear(MASK_DCM);
	return 2;
}

int MOS6502::CLV() {
	clear(MASK_OVERFLOW);
	return 2;
}

int MOS6502::TXS() {
	SP.bytes.LOW = IndexRegX;
	return 2;
}

int MOS6502::TXA() {
	Accumulator = IndexRegX;
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0x00));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::TAX() {
	IndexRegX = Accumulator;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0x00));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::TYA() {
	Accumulator = IndexRegY;
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0x00));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::TAY() {
	IndexRegY = Accumulator;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0x00));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::TSX() {
	IndexRegX = SP.bytes.LOW;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0x00));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::PLA() {
	SP.DWORD++;
	Accumulator = ReadMemory(SP.DWORD);
	
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	return 4;
}

int MOS6502::PLP() {
	SP.DWORD++;
	ProccessorStatus = ReadMemory(SP.DWORD);

	return 4;
}

int MOS6502::PHA() {
	WriteMemory(SP.DWORD, Accumulator);
	SP.DWORD--;

	return 3;
}

int MOS6502::PHP() {
	set(MASK_BREAK);
	set(MASK_EXP);
	WriteMemory(SP.DWORD, ProccessorStatus);
	SP.DWORD--;

	return 3;
}

int MOS6502::CLC() {
	clear(MASK_CARRY);
	return 2;
}

int MOS6502::CLI() {
	clear(MASK_IRQ);
	return 2;
}

int MOS6502::JSR() {
	//printf("JSR: PC: 0x%04x, Address: ", PC.DWORD);
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	//printf("0x%04X\n", Address);

	WriteMemory(SP.DWORD, (((PC.DWORD) & 0xFF00) >> 8));
	SP.DWORD--;
	WriteMemory(SP.DWORD, ((PC.DWORD) & 0x00FF));
	SP.DWORD--;

	PC.DWORD = Address;
	return 6;
}

int MOS6502::RTS() {
	uint16_t tmpAddress;
	SP.DWORD++;
	tmpAddress = ReadMemory(SP.DWORD);
	SP.DWORD++;
	tmpAddress |= (ReadMemory(SP.DWORD) << 8);

	PC.DWORD = tmpAddress + 1;
	return 6;
}

int MOS6502::RTI() {
	SP.DWORD++;
	ProccessorStatus = ReadMemory(SP.DWORD);
	SP.DWORD++;
	PC.bytes.LOW = ReadMemory(SP.DWORD);
	SP.DWORD++;
	PC.bytes.HIGH = ReadMemory(SP.DWORD);

	return 6;
}

int MOS6502::LDAImm() {
	Accumulator = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	return 2;
}

int MOS6502::LDAAbsoluteIndexedX() {
	int PageCrossed = 0;
	uint16_t Address = GenerateAbsoluteIndexedX(PageCrossed);
	Accumulator = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	return 4 + PageCrossed;
}

int MOS6502::LDAAbsoluteIndexedY() {
	int PageCrossed = 0;
	uint16_t Address = GenerateAbsoluteIndexedY(PageCrossed);
	Accumulator = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	return 4 + PageCrossed;
}

int MOS6502::LDAAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	Accumulator = ReadMemory(Address);

	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	return 4;
}

int MOS6502::LDAZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	Accumulator = ReadMemory(Address);
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 3;
}

int MOS6502::LDAZeroPageIndexed() {
	uint16_t Address = GenerateZeroPageIndexedAddress();

	Accumulator = ReadMemory(Address);
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 4;
}

int MOS6502::LDAIndirectIndexedY() {
	int PageCrossed = 0;
	uint16_t Address = GenerateIndirectIndexedY(PageCrossed);

	Accumulator = ReadMemory(Address);
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));

	return 5 + PageCrossed;
}

int MOS6502::CMPImm() {
	uint8_t Value = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_ZERO, ((Accumulator - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (Accumulator >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > Accumulator));

	return 2;
}

int MOS6502::CMPAbsoluteIndexedX() {
	int PageCrossed = 0;
	uint16_t DataPointer = GenerateAbsoluteIndexedX(PageCrossed);

	CONDITIONAL_SET(MASK_ZERO, ((Accumulator - ReadMemory(DataPointer)) == 0));
	CONDITIONAL_SET(MASK_CARRY, (Accumulator >= ReadMemory(DataPointer)));
	CONDITIONAL_SET(MASK_NEGATIVE, (ReadMemory(DataPointer) > Accumulator));

	return 4 + PageCrossed;
}

int MOS6502::CMPIndirectIndexedY() {
	int PageCrossed = 0;
	uint16_t DataPointer = GenerateIndirectIndexedY(PageCrossed);

	CONDITIONAL_SET(MASK_ZERO, ((Accumulator - ReadMemory(DataPointer)) == 0));
	CONDITIONAL_SET(MASK_CARRY, (Accumulator >= ReadMemory(DataPointer)));
	CONDITIONAL_SET(MASK_NEGATIVE, (ReadMemory(DataPointer) > Accumulator));

	return 5 + PageCrossed;
}

int MOS6502::CMPAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_ZERO, ((Accumulator - ReadMemory(DataPointer)) == 0));
	CONDITIONAL_SET(MASK_CARRY, (Accumulator >= ReadMemory(DataPointer)));
	CONDITIONAL_SET(MASK_NEGATIVE, (ReadMemory(DataPointer) > Accumulator));

	return 4;
}

int MOS6502::CMPZeroPage() {
	uint8_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t Value = ReadMemory(DataPointer);

	CONDITIONAL_SET(MASK_ZERO, ((Accumulator - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (Accumulator >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > Accumulator));

	return 3;
}


int MOS6502::CMXImm() {
	uint8_t Value = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_ZERO, ((IndexRegX - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (IndexRegX >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > IndexRegX));

	return 2;
}

int MOS6502::CMXZeroPage() {
	uint8_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t Value = ReadMemory(DataPointer);

	CONDITIONAL_SET(MASK_ZERO, ((IndexRegX - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (IndexRegX >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > IndexRegX));

	return 3;
}

int MOS6502::CMXAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;
	uint8_t Value = ReadMemory(DataPointer);

	CONDITIONAL_SET(MASK_ZERO, ((IndexRegX - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (IndexRegX >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > IndexRegX));

	return 4;
}

int MOS6502::CPYImm() {
	uint8_t Value = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_ZERO, ((IndexRegY - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (IndexRegY >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > IndexRegY));

	return 2;
}


int MOS6502::CPYZeroPage() {
	uint8_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t Value = ReadMemory(DataPointer);

	CONDITIONAL_SET(MASK_ZERO, ((IndexRegY - Value) == 0));
	CONDITIONAL_SET(MASK_CARRY, (IndexRegY >= Value));
	CONDITIONAL_SET(MASK_NEGATIVE, (Value > IndexRegY));

	return 3;
}

int MOS6502::DEX() {
	IndexRegX--;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::INX() {
	IndexRegX++;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegX == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegX & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}


int MOS6502::DEY() {
	IndexRegY--;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::INY() {
	IndexRegY++;
	CONDITIONAL_SET(MASK_ZERO, (IndexRegY == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((IndexRegY & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::BRK() {
	PC.DWORD++;
	WriteMemory(SP.DWORD, PC.bytes.HIGH);
	SP.DWORD--;
	WriteMemory(SP.DWORD, PC.bytes.LOW);
	SP.DWORD--;
	set(MASK_BREAK);
	WriteMemory(SP.DWORD, ProccessorStatus);
	set(MASK_IRQ);
	SP.DWORD--;

	uint16_t Vector = ReadMemory(0xFFFE);
	Vector |= (ReadMemory(0xFFFF) << 8);
	PC.DWORD = Vector;

	return 7;
}

int MOS6502::LSRA() {
	uint8_t Bit0 = (Accumulator & 0x01);

	Accumulator >>= 1;
	if (Bit0) {
		Accumulator |= 0x80;
	}
	else {
		Accumulator &= ~(0x80);
	}

	clear(MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0x00));
	CONDITIONAL_SET(MASK_CARRY, (Bit0));

	return 2;
}

int MOS6502::ASLA() {
	uint8_t Bit7 = (Accumulator & 0x80);

	Accumulator <<= 1;
	Accumulator &= ~(0x01);

	clear(MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0x00));
	CONDITIONAL_SET(MASK_CARRY, (Bit7));

	return 2;
}

int MOS6502::ASLZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint8_t Value = ReadMemory(Address);
	uint8_t Bit7 = (Value & 0x80);

	Value <<= 1;
	Value &= ~(0x01);
	WriteMemory(Address, Value);

	clear(MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Value == 0x00));
	CONDITIONAL_SET(MASK_CARRY, (Bit7));

	return 5;
}

int MOS6502::ASLZeroPageIndexed() {
	uint8_t Address = GenerateZeroPageIndexedAddress();

	uint8_t Value = ReadMemory(Address);
	uint8_t Bit7 = (Value & 0x80);

	Value <<= 1;
	Value &= ~(0x01);
	WriteMemory(Address, Value);

	clear(MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (Value == 0x00));
	CONDITIONAL_SET(MASK_CARRY, (Bit7));

	return 6;
}

int MOS6502::LSRZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint8_t Bit0 = (ReadMemory(Address) & 0x01);
	uint8_t Res = (ReadMemory(Address) >> 1);
	if (Bit0) {
		Res |= 0x80;
	}
	else {
		Res &= ~(0x80);
	}

	WriteMemory(Address, Res);

	clear(MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (ReadMemory(Address) == 0x00));
	CONDITIONAL_SET(MASK_CARRY, (Bit0));

	return 5;
}

int MOS6502::LSRZeroPageIndexed() {
	uint16_t Address = GenerateZeroPageIndexedAddress();

	uint8_t Bit0 = (ReadMemory(Address) & 0x01);
	uint8_t Res = (ReadMemory(Address) >> 1);
	if (Bit0) {
		Res |= 0x80;
	}
	else {
		Res &= ~(0x80);
	}

	WriteMemory(Address, Res);

	clear(MASK_NEGATIVE);
	CONDITIONAL_SET(MASK_ZERO, (ReadMemory(Address) == 0x00));
	CONDITIONAL_SET(MASK_CARRY, (Bit0));

	return 6;
}

int MOS6502::ROLA() {
	uint8_t Carry = get(MASK_CARRY);
	uint8_t Bit7 = Accumulator & 0x80;

	Accumulator <<= 1;
	if (Carry) {
		Accumulator |= 0x01;
	}
	else {
		Accumulator &= ~(0x01);
	}

	CONDITIONAL_SET(MASK_CARRY, (Bit7));
	CONDITIONAL_SET(MASK_ZERO, (Accumulator == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Accumulator & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 2;
}

int MOS6502::ROLZeroPage() {
	uint8_t Carry = get(MASK_CARRY);
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t Value = ReadMemory(Address);
	uint8_t Bit7 = Value & 0x80;

	Value <<= 1;
	if (Carry) {
		Value |= 0x01;
	}
	else {
		Value &= ~(0x01);
	}

	CONDITIONAL_SET(MASK_CARRY, (Bit7));
	CONDITIONAL_SET(MASK_ZERO, (Value == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((Value & MASK_NEGATIVE) == MASK_NEGATIVE));

	WriteMemory(Address, Value);
	return 5;
}

int MOS6502::BITZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_ZERO, (((Accumulator & ReadMemory(Address)) == 0)));
	CONDITIONAL_SET(MASK_NEGATIVE, (((Accumulator & ReadMemory(Address)) & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator & ReadMemory(Address)) & MASK_OVERFLOW) == MASK_OVERFLOW));

	return 3;
}

std::vector<uint8_t> MOS6502::LoadFile(const std::string& filename, size_t Size) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::exception("Couldn't open file");
	}
	
	size_t FileSize = file.tellg();
	if (FileSize != Size) {
		throw  std::exception("The size of file didn't match\n");
	}

	file.clear();
	file.seekg(0);

	std::vector<uint8_t> data(Size);
	file.read((char*)data.data(), Size);

	file.close();

	return data;
}

void MOS6502::LoadCharset(std::vector<uint8_t> Charset) {
	for (uint16_t i = 0; i < Charset.size(); i++) {
		this->Charset[i] = Charset[i];
	}
}

void MOS6502::LoadKernal(std::vector<uint8_t> Kernal) {
	for (uint32_t i = 0; i < Kernal.size(); i++) {
		this->Kernal[i] = Kernal[i];
	}
}

void MOS6502::LoadBasicInterpreter(std::vector<uint8_t> Interpreter) {
	for (uint16_t i = 0x00; i < Interpreter.size(); i++) {
		Basic[i] = Interpreter[i];
	}
}

void MOS6502::LoadTestSuite(std::vector<uint8_t> TestSuite) {
	for (uint32_t i = 0x00; i < TestSuite.size(); i++) {
		Memory[i] = TestSuite[i];
	}
	for (uint32_t i = 0xE000; i < 0x10000; i++) {
		Kernal[i - 0xE000] = Memory[i];
	}
	for (uint32_t i = 0xA000; i < 0xC000; i++) {
		Basic[i - 0xA000] = Memory[i];
	}
	for (uint32_t i = 0xD000; i < 0xE000; i++) {
		Charset[i - 0xD000] = Memory[i];
	}
}

void MOS6502::DumpMemory(const std::string& filename) {
	std::ofstream dump(filename);
	if (!dump.is_open()) {
		throw std::exception("Couldn't make dump of memory");
	}

	dump.write((const char*)Memory.data(), 0xFFFF);
	dump.close();
}

uint16_t MOS6502::GenerateZeroPageIndexedAddress() {
	uint16_t DataPointer = 0x00;
	DataPointer |= ReadMemory(PC.DWORD);
	PC.DWORD++;

	DataPointer += IndexRegX;
	if ((DataPointer & 0xFF00) >> 8 != 0x00) {
		DataPointer &= 0x00FF;
	}

	return DataPointer;
}
uint16_t MOS6502::GenerateAbsoluteIndexedX(int& PageCrossed) {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t DataPointerH = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (DataPointerH << 8);
	DataPointer += IndexRegX;

	if ((DataPointer & 0xFF00) >> 8 != DataPointerH) {
		PageCrossed++;
	}

	return DataPointer;
}
uint16_t MOS6502::GenerateAbsoluteIndexedY(int& PageCrossed) {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	uint8_t DataPointerH = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (DataPointerH << 8);
	DataPointer += IndexRegY;

	if ((DataPointer & 0xFF00) >> 8 != DataPointerH) {
		PageCrossed++;
	}

	return DataPointer;
}
uint16_t MOS6502::GenerateIndirectIndexedX() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer += IndexRegX;
	uint16_t Address = ReadMemory(DataPointer & 0x00FF);
	Address |= (ReadMemory(DataPointer + 1) << 8);

	return Address;
}
uint16_t MOS6502::GenerateIndirectIndexedY(int& PageCrossed) {
	uint8_t Vector = ReadMemory(PC.DWORD);
	PC.DWORD++;

	uint16_t DataPointer = ReadMemory(Vector);
	uint8_t DataPointerH = ReadMemory(Vector + 1);
	DataPointer |= (DataPointerH << 8);

	DataPointer += IndexRegY;
	if ((DataPointer & 0xFF00) >> 8 != DataPointerH) {
		PageCrossed++;
	}

	return DataPointer;
}

uint8_t MOS6502::ReadMemory(uint16_t Address) {
	switch (current)
	{
	case CONF_RAM_RAM_RAM_BASICROM_RAM_IO_KERNAL:
		if (Address >= 0xA000 && Address <= 0xBFFF) {
			return Basic[Address - 0xA000];
		}
		else if (Address >= 0xD000 && Address <= 0xDFFF) {
			return IO[Address - 0xD000];
		}
		else if (Address >= 0xE000) {
			return Kernal[Address - 0xE000];
		}
		else {
			return Memory[Address];
		}
		break;
	default:
		break;
	}

	return 0x00;
}

void MOS6502::WriteMemory(uint16_t Address, uint8_t Data) {
	
	if (Address == 0x0001) {
		Memory[Address] = Data;
		switch ((Data & 0x07)) {
		case 0x03:
			current = CONF_RAM_RAM_RAM_BASICROM_RAM_CHARSET_KERNAL;
			std::printf("Chaged bank config\n");
			break;
		}
	}

	switch (current)
	{
	case CONF_RAM_RAM_RAM_BASICROM_RAM_IO_KERNAL:
		if (Address >= 0xA000 && Address <= 0xBFFF) {
			Basic[Address - 0xA000] = Data;
		}
		else if (Address >= 0xD000 && Address <= 0xDFFF) {
			if (Address == 0xD011) {
				Vic->SetInterruptEnable(Data & 0x80);
				return;
			} else if (Address == 0xD012) {
				Vic->SetIRQRaster(Data);
				return;
			}
			IO[Address - 0xD000] = Data;
			if (Address == 0xDD00) {
				Vic->WriteMemory(Address, Data);
			}
		}
		else if (Address >= 0xE000) {
			Kernal[Address - 0xE000] = Data;
		}
		else {
			Memory[Address] = Data;
		}
		break;
	case CONF_RAM_RAM_RAM_BASICROM_RAM_CHARSET_KERNAL:
		if (Address >= 0xA000 && Address <= 0xBFFF) {
			Basic[Address - 0xA000] = Data;
		}
		else if (Address >= 0xD000 && Address <= 0xDFFF) {
			Charset[Address - 0xD000] = Data;
		}
		else if (Address >= 0xE000) {
			Kernal[Address - 0xE000] = Data;
		}
		else {
			Memory[Address] = Data;
		}
		break;
	default:
		break;
	}
}

void MOS6502::WriteToIO(uint16_t Address, uint8_t Data) {
	IO[Address] = Data;
}

int MOS6502::STXAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	WriteMemory(Address, IndexRegX);
	return 4;
}

int MOS6502::STXZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	WriteMemory((uint16_t)Address, IndexRegX);
	return 3;
}

int MOS6502::STYZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	WriteMemory((uint16_t)Address, IndexRegY);
	return 3;
}

int MOS6502::STYZeroPageIndexed() {
	uint16_t Address = GenerateZeroPageIndexedAddress();
	WriteMemory((uint16_t)Address, IndexRegY);
	return 4;
}

int MOS6502::STYAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	WriteMemory(Address, IndexRegY);
	return 4;
}

int MOS6502::STAAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	WriteMemory(Address, Accumulator);
	return 4;
}

int MOS6502::STAAbsoluteIndexedY() {
	int PageCrossed = 0;
	uint16_t Address = GenerateAbsoluteIndexedY(PageCrossed);

	WriteMemory(Address, Accumulator);
	return 5;
}

int MOS6502::STAAbsoluteIndexedX() {
	int PageCrossed = 0;
	uint16_t Address = GenerateAbsoluteIndexedX(PageCrossed);

	WriteMemory(Address, Accumulator);
	return 5;
}

int MOS6502::STAZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	WriteMemory((uint16_t)Address, Accumulator);
	return 3;
}

int MOS6502::STAZeroPageIndexed() {
	uint16_t Address = GenerateZeroPageIndexedAddress();
	WriteMemory(Address, Accumulator);
	return 4;
}

int MOS6502::STAIndirectIndexedY() {
	int PageCrossed;
	uint16_t Address = GenerateIndirectIndexedY(PageCrossed);

	WriteMemory(Address, Accumulator);
	return 6;
}

int MOS6502::NOP() {
	return 2;
}

int MOS6502::DECZeroPage() {
	uint8_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	WriteMemory(DataPointer, ReadMemory(DataPointer) - 1);

	CONDITIONAL_SET(MASK_ZERO, (ReadMemory(DataPointer) == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((ReadMemory(DataPointer) & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 5;
}

int MOS6502::DECAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;
	WriteMemory(DataPointer, ReadMemory(DataPointer) - 1);

	CONDITIONAL_SET(MASK_ZERO, (ReadMemory(DataPointer) == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((ReadMemory(DataPointer) & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 6;
}

int MOS6502::INCAbsolute() {
	uint16_t DataPointer = ReadMemory(PC.DWORD);
	PC.DWORD++;
	DataPointer |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;
	WriteMemory(DataPointer, ReadMemory(DataPointer) + 1);

	CONDITIONAL_SET(MASK_ZERO, (ReadMemory(DataPointer) == 0));
	CONDITIONAL_SET(MASK_NEGATIVE, ((ReadMemory(DataPointer) & MASK_NEGATIVE) == MASK_NEGATIVE));

	return 6;
}

int MOS6502::BITAbsolute() {
	uint16_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;
	Address |= (ReadMemory(PC.DWORD) << 8);
	PC.DWORD++;

	CONDITIONAL_SET(MASK_ZERO, (((Accumulator & ReadMemory(Address)) == 0)));
	CONDITIONAL_SET(MASK_NEGATIVE, (((Accumulator & ReadMemory(Address)) & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_OVERFLOW, (((Accumulator & ReadMemory(Address)) & MASK_OVERFLOW) == MASK_OVERFLOW));

	return 4;
}
int MOS6502::INCZeroPage() {
	uint8_t Address = ReadMemory(PC.DWORD);
	PC.DWORD++;

	WriteMemory(Address, ReadMemory(Address) + 1);
	
	CONDITIONAL_SET(MASK_NEGATIVE, ((ReadMemory(Address) & MASK_NEGATIVE) == MASK_NEGATIVE));
	CONDITIONAL_SET(MASK_ZERO, (ReadMemory(Address) == 0));

	return 5;
}

void MOS6502::Test() {
	PC.DWORD = 0x400;
	uint16_t LastPC = 0;
	while (1) {
		if (PC.DWORD == LastPC) {
			std::cout << "We fucked up" << std::endl;
			StubOpcode();
		}
		else if (PC.DWORD == 0x3463) {
			std::cout << "Somehow this shit works";
		}
		LastPC = PC.DWORD;
		this->Exec();
	}
}

int MOS6502::StubOpcode() {
	std::printf("PC: 0x%04X, LastOpcode: 0x%02X, CurrentOpcode: 0x%02X, Accumulator: 0x%02X, Index X: 0x%02X, Index Y: 0x%02X, SP: 0x%04X, PP: 0x%02x\n", PC.DWORD - 1, lastopcode, CurrentOpcode, Accumulator, IndexRegX, IndexRegY, SP.DWORD, ProccessorStatus);
 	return 1;
}