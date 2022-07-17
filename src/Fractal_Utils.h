#ifndef FUTILS
#define FUTILS
#define NOMINMAX

//Code for enums, structs and other utility stuff that doesnt fit better to other files.

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <Windows.h>
#include <chrono>
#include "imgui.h"
#include <SFML/Graphics/Color.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>

enum Fractals
{
	Mandelbrot = 0,
	Julia,
	BurningShip,
	Multibrot
};

enum iterationResult { MaxIter = 0, CircleTest, LoopTest, DerTest, Undefined, NotInside };
enum renderStatus { Ready = 0, NeedUpdate, Rendering, Empty, ImageSaved, VideoSaved };

struct fractalData {
	int iterations = 0;
	float smoothValue = 0;
	float shadowValue = 0;
	char iterResult = Undefined;
};

struct colorPalette {
	std::string name;
	std::vector<sf::Color> paletteColors;
};

#include "Fractal_Maths.h"

void generateDataset(int size, int max_iter);

void printDataset(int size);

bool sfColorEdit3(const char* label, sf::Color* color, ImGuiColorEditFlags flags);

void HelpMarker(const char* desc);

void handleConsoleParams(int argc, char* argv[]);

void BenchmarkMandelbrot();

#endif // !FUTILS