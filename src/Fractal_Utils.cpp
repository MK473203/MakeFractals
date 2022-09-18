#include "Fractal_Utils.h"

std::map<const char*, indexMapAlgorithm> indexMapAlgorithms{
	{"Top to bottom", &topToBottom},
	{"Bottom to top", &bottomToTop},
	{"Left to right", &leftToRight},
	{"Right to left", &rightToLeft},
	{"Outwards from middle (horizontal)", &outwardsFromMiddleH},
	{"Outwards from middle (vertical)", &outwardsFromMiddleV},
	{"Spiral", &spiral},
};

void HelpMarker(const char* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 25.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

bool sdlColorEdit3(const char* label, SDL_Color* col, ImGuiColorEditFlags flags) {

	float colFloats[3] = { (float)col->r / 255, (float)col->g / 255 , (float)col->b / 255 };

	bool result = ImGui::ColorEdit3(label, colFloats, flags);

	*col = { (unsigned char)(colFloats[0] * 255), (unsigned char)(colFloats[1] * 255) , (unsigned char)(colFloats[2] * 255) };

	return result;

}

void handleConsoleParams(int argc, char* argv[]) {

	if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
		return;
	}

	FILE* fDummy;
	freopen_s(&fDummy, "CONOUT$", "w", stdout);
	freopen_s(&fDummy, "CONOUT$", "w", stderr);
	freopen_s(&fDummy, "CONIN$", "r", stdin);
	std::cout.clear();
	std::clog.clear();
	std::cerr.clear();
	std::cin.clear();

	std::string arg1 = std::string(argv[1]);

	if (arg1 == "-h" || arg1 == "--help") {

		std::cout <<
			"usage: FractalRemake [options] [parameters...]\n"
			"Options:\n"
			"\n"
			"-p      <x> <y> [iterlimit] [fractal]\n"
			"--point <x> <y> [iterlimit] [fractal]\n"
			"\n"
			"Used to get info about [fractal] (defaults to the mandelbrot fractal)\nat point (x, y)\nwith the iteration limit [iterlimit] iterations (default 100000 iterations)"
			
			<< std::endl;

	}
	else if (arg1 == "-p" || arg1 == "--point") {
		try
		{
			double x = std::stod(argv[2]);
			double y = std::stod(argv[3]);

			

			fractalData data = Mandelbrot(x, y, 100000, 0);

			if (data.iterResult == NotInside) {
				std::cout << "Point is not inside the Mandelbrot set, escaping at " << data.iterations << " iterations." << "\n";
			}
			else {
				std::cout << "Point is inside the Mandelbrot set at " << data.iterations << " iterations." << "\n";
			}

		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
		}

	}

	std::cout << "\nPress ENTER to exit...";
	std::cin.get();

	FreeConsole();

}

void topToBottom(int i, int* x, int* y, int width, int height) {

	*x = i % width;
	*y = (height - 1) - i / width;

}

void bottomToTop(int i, int* x, int* y, int width, int height) {

	*x = i % width;
	*y = i / width;

}

void leftToRight(int i, int* x, int* y, int width, int height) {

	*x = i / height;
	*y = i % height;

}

void rightToLeft(int i, int* x, int* y, int width, int height) {

	*x = (width - 1) - i / height;
	*y = i % height;

}

void outwardsFromMiddleH(int i, int* x, int* y, int width, int height) {

	int _x = i / height;
	if (_x % 2 == width % 2) {
		*x = (width + _x) / 2;
	}
	else {
		*x = (width - _x) / 2;
	}

	*y = i % height;

}

void outwardsFromMiddleV(int i, int* x, int* y, int width, int height) {

	int _y = i / width;
	if (_y % 2 == height % 2) {
		*y = (height + _y) / 2;
	}
	else {
		*y = (height - _y) / 2;
	}

	*x = i % width;

}

void spiral(int i, int* x, int* y, int width, int height) {

	if (i >= std::min(height * (height + 1-(height % 2)), width * width)) {
		if (height < width) {
			return outwardsFromMiddleH(i, x, y, width, height);
		}
		else {
			return outwardsFromMiddleV(i, x, y, width, height);
		}
	}

	int dx = 0;
	int dy = 1;

	int seglength = 1;

	int _x = width / 2 ;
	int _y = height / 2;

	int k = i;

	while (k > 0)
	{

		_x += dx * seglength;
		_y += dy * seglength;

		if (_y >= height || _y < 0) {
			return outwardsFromMiddleH(i, x, y, width, height);
		} else if (_x >= width || _x < 0) {
			return outwardsFromMiddleV(i, x, y, width, height);
		}

		k -= seglength;

		if (dy == 0) {
			seglength++;
		}

		seglength = std::min(seglength, k);

		int temp = dx;
		dx = -dy;
		dy = temp;

	}

	*x = _x;
	*y = _y;

}