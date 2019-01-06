#pragma once

#include <array>
#include <vector>
#include <SFML/Graphics.hpp>

#include "MOS6502.h"

class MOS6502;

class VIC2 {
public:
	VIC2(std::array<uint8_t, 0x10000>& MainMemory, std::array<uint8_t, 0x1000> chara, MOS6502* cpu) : Memory(MainMemory) {
		BankOffset = 0x00;
		Charset = chara;
		CPU = cpu;
	}

	~VIC2() { }

	void Clock() {
		if (ReadMemory(0xD019) & 0x80 != 0) {
			//CPU->SetInterrupt(true, 0xFFFE);
		}
		if (CPU->GetCycles() >= NextRaster) {
			if (interrupt && Raster == IRQRaster) {
			//	CPU->SetInterrupt(true, 0xFFFE);
			}
		}
		Raster += 1;
		if (Raster >= 255) {
			Raster = 0;
		}
		WriteMemory(0xD012, Raster);
	}

	std::vector<sf::Uint8> UpdateFrame() {
		std::vector<sf::Uint8> frame(320 * 200 * 8);

		int pos = 0;
		int posY = 0;
		for (int i = 0x0400; i < 0x0800; i++) {
			for (int y = 0; y < 8; y++) {
				uint8_t character = ReadMemory(0x1000 + (Memory[i] * 8) + y);
				int PixelOffset = 0;
				uint32_t Background = ColorPalette[ReadMemory(0xD021)];
				uint32_t TextColor = ColorPalette[ReadMemory(0x0286)];
				for (int x = 0; x < 8; x++) {
					if ((character & 0x80) == 0x80) {
						frame[(y * 320 * 4) + PixelOffset + (pos * 4 * 8) + (posY * 320 * 4 * 8)] = ((TextColor & 0x00FF0000) >> 16);
						frame[(y * 320 * 4) + PixelOffset + 1 + (pos * 4 * 8) + (posY * 320 * 4 * 8)] = ((TextColor & 0x0000FF00) >> 8);
						frame[(y * 320 * 4) + PixelOffset + 2 + (pos * 4 * 8) + (posY * 320 * 8 * 4)] = ((TextColor & 0x000000FF));
						frame[(y * 320 * 4) + PixelOffset + 3 + (pos * 4 * 8) + (posY * 320 * 8 * 4)] = 0xFF;
					}
					else {
						frame[(y * 320 * 4) + PixelOffset + (pos * 4 * 8) + (posY * 320 * 8 * 4)] = ((Background & 0x00FF0000) >> 16);
						frame[(y * 320 * 4) + PixelOffset + 1 + (pos * 4 * 8) + (posY * 320 * 8 * 4)] = ((Background & 0x0000FF00) >> 8);
						frame[(y * 320 * 4) + PixelOffset + 2 + (pos * 4 * 8) + (posY * 320 * 8 * 4)] = (Background & 0x000000FF);
						frame[(y * 320 * 4) + PixelOffset + 3 + (pos * 4 * 8) + (posY * 320 * 8 * 4)] = 0xFF;
					}
					character <<= 1;
					PixelOffset += 4;
				}
			}
			pos++;
			if (pos == 40) {
				posY++;
				pos = 0;
			}
		}
		return frame;
	}

	void WriteMemory(uint16_t Address, uint8_t Data) {
		if (Address == 0xDD00) {
			switch ((Data & 0x03))
			{
			case 0x03:
				BankOffset = 0x00;
				break;
			case 0x02:
				BankOffset = 0x4000;
				break;
			case 0x01:
				BankOffset = 0x8000;
				break;
			case 0x00:
				BankOffset = 0xC000;
				break;
			default:
				break;
			}
		}
		else {
			CPU->WriteToIO(Address - 0xD000, Data);
		}
	}

	uint8_t ReadMemory(uint16_t Address) {
		if ((Address - BankOffset >= 0x1000 && Address - BankOffset <= 0x2000)) {
			return Charset[Address - 0x1000 - BankOffset];
		}
		else {
			return CPU->ReadMemory(Address);
		}
	}

	void SetIRQRaster(uint16_t IRQRaster) {
		this->IRQRaster = IRQRaster;
	}

	bool GetInterrupt() { return interrupt; }

	void SetInterruptEnable(bool interrupt) {
		this->interrupt = interrupt;
	}
private:
	std::array<uint8_t, 0x10000>& Memory;
	std::array<uint8_t, 0x1000> Charset;

	bool interrupt = false;

	uint32_t Raster = 0;

	MOS6502 *CPU;
	
	uint16_t BankOffset;
	uint8_t IRQRaster;

	uint64_t NextRaster = 0;

	const uint16_t ScreenLines = 314;
	const uint16_t ScrenCols = 504;
	const uint16_t FirstVisibleLine = 14;
	const uint16_t LastVisibleLine = 298;
	

	const uint32_t ColorPalette[16] = {
		0xFF000000,
		0xFFFFFFFF,
		0xFF880000,
		0xFFAAFFEE,
		0xFFCC44CC,
		0xFF00CC55,
		0xFF0000FF,
		0xFFEEEE77,
		0xFFDD8855,
		0xFF664400,
		0xFFFF7777,
		0xFF333333,
		0xFF777777,
		0xFFAAFF66,
		0xFF0088FF,
		0xFFBBBBBB,
	};
};