//
// Created by PinkySmile on 23/06/2026.
//

#include <thread>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

int main()
{
	sf::RenderWindow window{sf::VideoMode::getDesktopMode(), "test-audio"};
	std::int16_t samples[1024];

	for (size_t i = 0; i < std::size(samples); i++)
		samples[i] = std::sin(i * 44100 / 440) * 5000; // A 440Hz

	sf::SoundBuffer buffer{samples, std::size(samples), 1, 44100, {sf::SoundChannel::Mono}};
	sf::Sound sound{buffer};
	unsigned timer = 0;

	window.setFramerateLimit(60);
	while (window.isOpen()) {
		while (auto event = window.pollEvent()) {
			if (event->is<sf::Event::Closed>())
				window.close();
			else if (auto btn = event->getIf<sf::Event::MouseButtonPressed>()) {
				if (btn->button == sf::Mouse::Button::Left) {
					sound.stop();
					sound.play();
					timer = 1;
				} else if (btn->button == sf::Mouse::Button::Right) {
					window.clear(sf::Color::Yellow);
					window.display();
					std::this_thread::sleep_for(std::chrono::milliseconds(250));
				}
			}
		}
		if (timer) {
			timer++;
			window.clear(sf::Color::Red);
		} else
			window.clear(sf::Color::Green);
		if (timer >= 60) {
			timer = 0;
			sound.stop();
			sound.play();
		}
		window.display();
	}
	return 0;
}