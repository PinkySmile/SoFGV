//
// Created by Gegel85 on 13/05/2022.
//

#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <nlohmann/json.hpp>

int main(int argc, char **argv)
{
	if (argc < 2)
		return EXIT_FAILURE;

	std::ofstream stream;
	std::vector<sf::Image> images;
	std::vector<std::vector<sf::Uint32>> colors;
	nlohmann::json json = nlohmann::json::array();

	for (int i = 0; ; i++) {
		std::cout << "Loading file " << argv[1] << "\\" << i << ".png";
		if (!images.emplace_back().loadFromFile(std::string(argv[1]) + "\\" + std::to_string(i) + ".png"))
			break;
		colors.emplace_back();
		if (images[0].getSize().x != images[i].getSize().x || images[0].getSize().y != images[i].getSize().y) {
			std::cerr << "Files " << argv[1] << "\\0.png and " << argv[1] << "\\" << i << ".png don't have the same size!";
			std::cerr << "Aborting..." << std::endl;
			system("pause");
			return EXIT_FAILURE;
		}
	}
	images.pop_back();
	if (images.size() < 2) {
		std::cerr << "There must be at least 2 files to process!" << std::endl;
		std::cerr << "Aborting..." << std::endl;
		system("pause");
		return EXIT_FAILURE;
	}
	std::cout << "Processing..." << std::endl;
	for (unsigned y = 0; y < images[0].getSize().y; y++) {
		std::cout << "Line " << y << std::endl;
		for (unsigned x = 0; x < images[0].getSize().x; x++) {
			auto it = std::find(colors[0].begin(), colors[0].end(), images[0].getPixel(x, y).toInteger());
			int off = 0;
			bool alpha = false;
			bool end = it == colors[0].end();

			for (auto &image : images)
				if (image.getPixel(x, y).a != 255) {
					alpha = true;
					break;
				}
			if (alpha)
				continue;
			if (end)
				colors[0].push_back(images[0].getPixel(x, y).toInteger());
			else
				off = it - colors[0].begin();
			for (unsigned i = 1; i < images.size(); i++) {
				if (end)
					colors[i].push_back(images[i].getPixel(x, y).toInteger());
				else if (colors[i][off] != *it)
					std::cout << "Warning: Pixel (" << x << ", " << y << ") in image " << i << " would need overwrite. Ignoring..." << std::endl;
			}
		}
	}
	std::cout << "Removing same colors" << std::endl;
	for (unsigned i = 0; i < colors[0].size(); i++) {
		bool same = true;

		for (unsigned j = 1; j < colors.size(); j++) {
			if (colors[0][i] != colors[j][i]) {
				same = false;
				break;
			}
		}
		if (same) {
			for (auto &color : colors)
				color.erase(color.begin() + i);
			i--;
		}
	}
	std::cout << "Generating JSON" << std::endl;
	for (auto &arr : colors) {
		json.push_back(nlohmann::json::array());
		for (auto color : arr) {
			sf::Color c{color};

			json.back().push_back({
				{"r", c.r},
				{"g", c.g},
				{"b", c.b},
			});
		}
	}
	std::cout << "Saving file" << std::endl;
	stream.open("output.json");
	stream << json;
	stream.close();
	std::cout << "Job done" << std::endl;
	system("pause");
}