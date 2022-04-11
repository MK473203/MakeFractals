#include "Fractal_Utils.h"

void generateDataset(int size) {

	std::ofstream file;
	file.open("data.dat", std::ios::binary | std::ios::out);

	boost::random::mt19937 gen;
	boost::random::uniform_real_distribution distx(-3.0, 1.0);
	boost::random::uniform_real_distribution disty(-2.0, 2.0);

	for (int i = 0; i < size; i++) {
		double x = distx(gen);
		double y = disty(gen);

		char result = FractalAlgorithms::Mandelbrot(x, y, 100000000, FractalAlgorithms::None).iterResult;

		if (result == iterationResult::NotInside) {
			result = 0;
		}
		else {
			result = 1;
		}

		file.write((char*) &x, sizeof(double));
		file.write((char*) &y, sizeof(double));
		file.write((char*) &result, sizeof(char));

	}

	file.close();

	if (!file.good()) {
		std::cout << "Error occurred at writing time!" << std::endl;
	}

}

void printDataset(int size) {

	std::ifstream file("data.dat", std::ios::out | std::ios::binary);

	for (int i = 0; i < size; i++)
	{

		double x;
		double y;
		char result;

		file.read((char*)&x, sizeof(double));
		file.read((char*)&y, sizeof(double));
		file.read((char*)&result, sizeof(char));

		std::cout << std::setprecision(std::numeric_limits<double>::max_digits10) << x << std::endl;
		std::cout << std::setprecision(std::numeric_limits<double>::max_digits10) << y << std::endl;
		std::cout << (int)result << std::endl;

	}

	file.close();

}

bool sfColorEdit3(const char* label, sf::Color* color, ImGuiColorEditFlags flags) {
	float col[3] = { (float)color->r / 255.0f, (float)color->g / 255.0f, (float)color->b / 255.0f };

	bool wasChanged = ImGui::ColorEdit3(label, col, flags);

	if (wasChanged) {
		color->r = col[0] * 255;
		color->g = col[1] * 255;
		color->b = col[2] * 255;
	}

	return wasChanged;

}