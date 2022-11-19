#ifndef FDRAW
#define FDRAW

// Code for mapping colors to iterations, drawing images etc.

#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <map>
#include <SDL.h>
#include <SDL_image.h>
#include <thread>
#include <vector>

#include "Fractal_Maths.h"
#include "Fractal_Utils.h"


class FractalImage
{
public:

	static bool shadowFx;
	static bool debugColors;
	static int ColorAmount;
	static int ColorOffset;
	static float shadowStrength;

	static colorPalette currentPalette;

	static colorMapType currentColorMapType;

	static fractalAlgorithmFunction currentAlgorithm;

	static indexMapAlgorithm indexMapper;
	static bool reverseRenderOrder;

	static std::vector<colorPalette> defaultPaletteList;
	static std::vector<colorPalette> paletteList;

	static SDL_Color setColor;
	static SDL_Color debugColor;


	static std::vector<SDL_Color> palette;


	FractalImage(int width = 1920, int height = 1080) :
		width(width),
		height(height)
	{
		iterDiv = 1 << 5;
		iterationMax = 10000;
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
	FractalImage(int width, int height, int iterationMax, apfloat centerX, apfloat centerY, deltafloat scale, int sampleDistance) :
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
		dataArray.resize(width * height, {});
		pixelArray.resize(scaledHeight * scaledWidth * 4, 0);
		threadAmount = std::thread::hardware_concurrency() - 2;
	}

	FractalImage& operator = (FractalImage fi)
	{
		width = fi.width;
		height = fi.height;
		iterationMax = fi.iterationMax;
		centerX = fi.centerX;
		centerY = fi.centerY;
		scale = fi.scale;
		sampleDistance = fi.sampleDistance;
		iterDiv = fi.iterDiv;

		aspectRatio = (float)width / height;
		scaledHeight = height / std::max(sampleDistance, 1);
		scaledWidth = width / std::max(sampleDistance, 1);
		dataArray.resize(width * height, {});
		pixelArray.resize(scaledHeight * scaledWidth * 4, 0);
		threadAmount = std::thread::hardware_concurrency() - 2;

		return *this;
	}

	int height, width, iterationMax, iterDiv, threadAmount, scaledHeight, scaledWidth;

	apfloat centerX, centerY; 

	deltafloat scale;

	std::vector<deltafloat> pixelDeltas[2];

	std::vector<fractalData> dataArray;

	std::vector<uint8_t> pixelArray;

	iterHistogram imageHistogram;

	std::vector<std::thread> threadPool;

	std::atomic_bool interruptThreads = false;

	renderStatus renderingStatus = Empty;
	unsigned int lastAssignedIndex = 0;
	float renderProgress = 0;

	int flags = 0;

	int renderStartTime;
	int lastRenderTime = 0;


	int sampleDistance;
	float aspectRatio;

	void updatePixels();

	void cancelUpdate();

	void generateHistogram();

	void saveToImage(const char* relativePath);

	void generateZoomVideo(int& keyFrameCounter, deltafloat finalScale = 3.0);

	void generatePalette();

	SDL_Color mapFractalData(fractalData data);

	void refreshVisuals();

	void resizeArrays();

	std::string debugInfo();

	friend std::ostream& operator<<(std::ostream& os, const FractalImage& fi) {

		int algindex = 0;
		int indexmapperindex = 0;

		for (auto it = fractalAlgorithms.begin(); it != fractalAlgorithms.end(); it++)
		{
			if (it->second == fi.currentAlgorithm) {
				break;
			}
			algindex = (algindex + 1) % fractalAlgorithms.size();
		}

		for (auto it = indexMapAlgorithms.begin(); it != indexMapAlgorithms.end(); it++)
		{
			if (it->second == fi.indexMapper) {
				break;
			}
			indexmapperindex = (indexmapperindex + 1) % indexMapAlgorithms.size();
		}

		os << std::setprecision(std::numeric_limits<apfloat>::digits10 + 1)
			<< fi.centerX << '\n'
			<< fi.centerY << '\n'
			<< fi.scale << '\n'
			<< fi.iterationMax << '\n'
			<< fi.iterDiv << '\n'
			<< fi.shadowFx << '\n'
			<< fi.ColorOffset << "\n"
			<< (int)fi.currentColorMapType << "\n"
			<< algindex << "\n"
			<< fi.sampleDistance << "\n"
			<< indexmapperindex;
		return os;
	}

	friend std::istream& operator>>(std::istream& is, FractalImage& fi) {

		int algindex;
		int cmtindex;
		int indexmapperindex;

		is >> std::setprecision(std::numeric_limits<apfloat>::digits10 + 1) >>
			fi.centerX >>
			fi.centerY >>
			fi.scale >>
			fi.iterationMax >>
			fi.iterDiv >>
			fi.shadowFx >>
			fi.ColorOffset >>
			cmtindex >>
			algindex >>
			fi.sampleDistance >>
			indexmapperindex;

		fi.currentColorMapType = (colorMapType)cmtindex;

		auto it = fractalAlgorithms.begin();
		std::advance(it, algindex);

		fi.currentAlgorithm = it->second;

		auto it2 = indexMapAlgorithms.begin();
		std::advance(it2, indexmapperindex);

		fi.indexMapper = it2->second;

		return is;
	}

	void savePaletteList();
	void loadPaletteList();

private:

	void genDeltas();

	void generateKeyframes(int& keyFrameCounter, deltafloat finalScale);

	void cropFramesFromKeyframes();

};

/*struct FractalPixel {

	SDL_Color color;
	deltafloat deltax;
	deltafloat deltay;

	std::vector<fractalData> data


};*/

SDL_Color Lerp(SDL_Color c1, SDL_Color c2, float t);

void drawThreadTask(FractalImage& fi, int threadIndex);

#endif // !FDRAW