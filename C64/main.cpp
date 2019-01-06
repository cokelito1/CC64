#include <iostream>
#include <chrono>
#include <ctime>

#include <SFML/Graphics.hpp>

#include "MOS6502.h"
#include "VICII.h"

int main() {
	MOS6502 cpu(false);
	VIC2* vic = cpu.GetVic();
	//cpu.Test();

	sf::RenderWindow window(sf::VideoMode(800, 600), "test");
	sf::View v = window.getView();
	v.setCenter(320 / 2, 200 / 2);
	v.setSize(320, 200);
	window.setView(v);

	sf::Texture texture;
	texture.create(320, 200);

	sf::Sprite frameSprite;
	frameSprite.setTexture(texture);
	frameSprite.setPosition(0, 0);

	window.setFramerateLimit(60);
	std::chrono::system_clock::now();

	const unsigned int CyclesPerFrame = 1024000 / 60;
	while (window.isOpen()) {
		int FrameCycles = 0;
		
		sf::Event evt;
		while (window.pollEvent(evt)) {
			switch (evt.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				cpu.WriteMemory(0xDC00, 0x30);
				cpu.WriteMemory(0xDC01, 0x30);
				cpu.SetInterrupt(true, 0xFFFE);
	//			cpu.SetInterrupt(true, 0xFFFE);
				break;
			case sf::Event::KeyReleased:
				cpu.WriteMemory(0xDC00, 0x00);
				cpu.WriteMemory(0xDC01, 0x00);
				break;
			}
		}

		std::vector<sf::Uint8> Pixels;
		std::vector<sf::Uint8> tmp;
		while (FrameCycles < CyclesPerFrame) {
			std::chrono::duration<double> current_time = std::chrono::system_clock::now().time_since_epoch();
			uint32_t epoc = current_time.count() * std::chrono::system_clock::period::num / std::chrono::system_clock::period::den;
			
			uint8_t seconds = epoc % 60;
			uint8_t minutes = (epoc / 60) % 60;
			uint8_t hours = (epoc / 3600) % 24;

			cpu.WriteMemory(0xDC08, 0x00);
			cpu.WriteMemory(0xDC09, (seconds % 10) | (((seconds / 10) % 10) << 4));
			cpu.WriteMemory(0xDC0A, (minutes % 10) | (((minutes / 10) % 10) << 4));
			cpu.WriteMemory(0xDC0B, (hours % 10) | (((hours / 10) % 10) << 4));
			FrameCycles += cpu.Exec();
		}
		tmp = cpu.UpdateFrame();
		if (tmp[0] != 0xCA && tmp[1] != 0xFE) {
			Pixels = tmp;
		}
		//std::cout << "Cycles executed: " << FrameCycles << std::endl;

		texture.update(Pixels.data(), 320, 200, 0, 0);
		frameSprite.setTexture(texture);

		window.clear();
		window.draw(frameSprite);
		window.display();
	}
	
	return 0;
}