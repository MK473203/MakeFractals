#ifndef FUTILS
#define FUTILS
#define NOMINMAX

//Code for enums, structs and other utility stuff that doesn't fit better to other files.

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>
#include <map>
#include <SDL.h>
#include <sstream>
#include <Windows.h>

#include "imgui.h"

typedef void(*indexMapAlgorithm)(int, int*, int*, int, int);

extern std::map<const char*, indexMapAlgorithm> indexMapAlgorithms;

extern std::string prefPath;

enum colorMapType {
	LinearCyclic = 0,
	ExponentialCyclic,
	Histogram,
};

enum iterationResult { MaxIter = 0, CircleTest, LoopTest, DerTest, Undefined, NotInside };
enum renderStatus { Ready = 0, NeedUpdate, Rendering, Empty, ImageSaved, VideoSaved };

struct fractalData {
	int iterations = 0;
	float smoothValue = 0;
	float shadowValue = 0;
	char iterResult = Undefined;
};

struct iterHistogram {
	std::vector<int> list;
	long int total = 0;
};

struct colorPalette {
	std::string name;
	std::vector<SDL_Color> paletteColors;
};


#include "Fractal_Maths.h"

void HelpMarker(const char* desc);

bool sdlColorEdit3(const char* label, SDL_Color* col, ImGuiColorEditFlags flags);

void handleConsoleParams(int argc, char* argv[]);

void topToBottom(int i, int* x, int* y, int width, int height);

void bottomToTop(int i, int* x, int* y, int width, int height);

void leftToRight(int i, int* x, int* y, int width, int height);

void rightToLeft(int i, int* x, int* y, int width, int height);

void outwardsFromMiddleH(int i, int* x, int* y, int width, int height);

void outwardsFromMiddleV(int i, int* x, int* y, int width, int height);

void spiral(int i, int* x, int* y, int width, int height);

#endif // !FUTILS