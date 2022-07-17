#include "Fractal_Utils.h"

void generateDataset(int size, int max_iter) {

	std::ofstream file;
	file.open("data.dat", std::ios::binary | std::ios::out);

	boost::random::mt19937 gen;
	boost::random::uniform_real_distribution distx(-3.0, 1.0);
	boost::random::uniform_real_distribution disty(-2.0, 2.0);


	file.write((char*)&max_iter, sizeof(int));

	for (int i = 0; i < size; i++) {
		double x = distx(gen);
		double y = disty(gen);

		int result = fa::Mandelbrot(x, y, max_iter, fa::None).iterations;

		file.write((char*) &x, sizeof(double));
		file.write((char*) &y, sizeof(double));
		file.write((char*) &result, sizeof(int));

	}

	file.close();

	if (!file.good()) {
		std::cerr << "Error occurred while writing file" << '\n';
	}

}

void printDataset(int size) {

	if (!AllocConsole()) {
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

	std::ifstream file("data.dat", std::ios::out | std::ios::binary);

	int max_iter;

	file.read((char*)&max_iter, sizeof(int));

	std::clog << max_iter << '\n';

	for (int i = 0; i < size; i++)
	{

		double x;
		double y;
		int result;

		file.read((char*)&x, sizeof(double));
		file.read((char*)&y, sizeof(double));
		file.read((char*)&result, sizeof(int));

		std::clog << std::setprecision(std::numeric_limits<double>::max_digits10) << x << '\n';
		std::clog << std::setprecision(std::numeric_limits<double>::max_digits10) << y << '\n';
		std::clog << result << '\n';

	}

	file.close();

	std::clog << "\nPress ENTER to close window...";
	std::cin.get();

	ShowWindow(GetConsoleWindow(), SW_HIDE);
	FreeConsole();

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

			

			fractalData data = fa::Mandelbrot(x, y, 100000, 0);

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

void BenchmarkMandelbrot() {
	if (!AllocConsole()) {
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

	auto start = std::chrono::high_resolution_clock::now();



	auto stop = std::chrono::high_resolution_clock::now();

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

	std::clog << "Benchmark took " << duration.count() << " microseconds" << std::endl;

	std::clog << "\nPress ENTER to close window...";
	std::cin.get();

	ShowWindow(GetConsoleWindow(), SW_HIDE);
	FreeConsole();
}