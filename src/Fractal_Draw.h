#ifndef FDRAW
#define FDRAW

#include "Fractal_Utils.h"
#include "Fractal_Maths.h"
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Image.hpp>
#include <vector>
#include <map>
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>


class FractalImage
{
public:

	static bool shadowFx;
	static bool debugColors;
	static int ColorAmount;
	static int ColorOffset;
	static float shadowStrength;

	static colorPalette currentPalette;

	static fractalAlgorithmFunction currentAlgorithm;

	static std::vector<colorPalette> paletteList;

	static sf::Color setColor;
	static sf::Color debugColor;

	static std::vector<sf::Color> palette;

	static std::vector<std::thread> threadPool;

	FractalImage(int width = 1920, int height = 1080) :
		width(width),
		height(height)
	{
		iterDiv = 1 << 5;
		iterationMax = 1000000;
		centerX = -0.875;
		centerY = 0.000001;
		scale = 2.4;
		sampleDistance = 1;
		aspectRatio = (float)width / height;
		scaledHeight = height / std::max(sampleDistance, 1);
		scaledWidth = width / std::max(sampleDistance, 1);
		dataArray.resize(width * height, {});
		pixelArray.resize(width * height * 4, 0);
		threadAmount = std::thread::hardware_concurrency() - 2;

	};
	~FractalImage() { }
	FractalImage(int width, int height, int iterationMax, double centerX, double centerY, double scale, int sampleDistance) :
		width(width),
		height(height),
		iterationMax(iterationMax),
		centerX(centerX),
		centerY(centerY),
		scale(scale),
		sampleDistance(sampleDistance)
	{ 
		aspectRatio = (float)width / height;
		scaledHeight = height / std::max(sampleDistance, 1);
		scaledWidth = width / std::max(sampleDistance, 1);
		dataArray.resize(width * height, {});
		pixelArray.resize(scaledHeight * scaledWidth * 4, 0);
		iterDiv = 1 << 5;
		threadAmount = std::thread::hardware_concurrency() - 2;
	}
	FractalImage(const FractalImage &fi) :
		width(fi.width),
		height(fi.height),
		iterationMax(fi.iterationMax),
		centerX(fi.centerX),
		centerY(fi.centerY),
		scale(fi.scale),
		sampleDistance(fi.sampleDistance),
		iterDiv(fi.iterDiv)

	{
		aspectRatio = (float)width / height;
		scaledHeight = height / std::max(sampleDistance, 1);
		scaledWidth = width / std::max(sampleDistance, 1);
		dataArray.resize(width* height, {});
		pixelArray.resize(scaledHeight* scaledWidth * 4, 0);
		threadAmount = std::thread::hardware_concurrency() - 2;
	}

	int height, width, iterationMax, iterDiv, threadAmount, scaledHeight, scaledWidth;

	double centerX, centerY, scale;

	std::vector<fractalData> dataArray;

	std::vector<uint8_t> pixelArray;

	renderStatus renderingStatus = Empty;
	float renderProgress = 0;

	int flags = 0;

	sf::Clock renderClock;
	int lastRenderTime;

	bool interruptThreads = false;

	int sampleDistance;
	float aspectRatio;

	void updatePixels();

	void cancelUpdate();

	void saveToImage();

	void generatePalette();

	sf::Color MapIterations(fractalData data);

	void refreshVisuals();

	std::string debugInfo();

	friend std::ostream& operator<<(std::ostream& os, const FractalImage& fi) {
		os << std::setprecision(std::numeric_limits<double>::digits10 + 1)
		<< fi.centerX << '\n'
		<< fi.centerY << '\n'
		<< fi.scale << '\n'
		<< fi.iterationMax << '\n'
		<< fi.iterDiv << '\n'
		<< fi.shadowFx << '\n'
		<< fi.ColorOffset;
		return os;
	}

	friend std::istream& operator>>(std::istream& is, FractalImage& fi) {
		is >> std::setprecision(std::numeric_limits<double>::digits10 + 1) >>
			fi.centerX >>
			fi.centerY >>
			fi.scale >>
			fi.iterationMax >>
			fi.iterDiv >>
			fi.shadowFx >>
			fi.ColorOffset;
		return is;
	}

	void savePaletteList();
	void loadPaletteList();

private:


};


sf::Color Lerp(sf::Color c1, sf::Color c2, float t);

void drawThreadTask(int threadIndex, FractalImage& fi);

#endif // !FDRAW