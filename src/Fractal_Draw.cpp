// Code for mapping colors to iterations, drawing images etc.

#include "Fractal_Draw.h"
bool FractalImage::shadowFx = false;
bool FractalImage::debugColors = false;
int FractalImage::ColorAmount = 1 << 13;
int FractalImage::ColorOffset = 0;
float FractalImage::shadowStrength = 0.5f; 
fractalAlgorithmFunction FractalImage::currentAlgorithm = &Mandelbrot;
indexMapAlgorithm FractalImage::indexMapper = &topToBottom;
bool FractalImage::reverseRenderOrder = false;

/*
{"Default",
std::vector<SDL_Color>{
	{0, 7, 100},
	{32, 107, 203},
	{237, 255, 255},
	{255, 170, 0},
	{0, 2, 0}
}},
The sacred default colors. Guard them well.
*/

std::vector<colorPalette> FractalImage::defaultPaletteList{
	{"Default",
	std::vector<SDL_Color>{
		{0, 7, 100},
		{32, 107, 203},
		{237, 255, 255},
		{255, 170, 0},
		{0, 2, 0}
	}},
	{"Black and white",
	std::vector<SDL_Color>{
		{255, 255, 255},
		{0, 0, 0}
	}},
	{"Pastel",
	std::vector<SDL_Color>{
		{220, 204, 162},
		{234, 228, 204},
		{249, 249, 248},
		{165, 173, 158},
		{66, 70, 57}
	}}
};

std::vector<colorPalette> FractalImage::paletteList{
	{"Default",
	std::vector<SDL_Color>{
		{0, 7, 100},
		{32, 107, 203},
		{237, 255, 255},
		{255, 170, 0},
		{0, 2, 0}
	}},
	{"Black and white",
	std::vector<SDL_Color>{
		{255, 255, 255},
		{0, 0, 0}
	}},
	{"Pastel",
	std::vector<SDL_Color>{
		{220, 204, 162},
		{234, 228, 204},
		{249, 249, 248},
		{165, 173, 158},
		{66, 70, 57}
	}},
	{"idk",
	std::vector<SDL_Color>{
		{0, 0, 0},
		{0, 7, 80},
		{0, 0, 0},
		{60,0,120},
		{0, 0, 0},
		{130,58,160},
		{0, 0, 0},
		{200,68,185},
		{0, 0, 0},
		{240,200,220}
	}}
};

colorPalette FractalImage::currentPalette = FractalImage::paletteList[0];

colorMapType FractalImage::currentColorMapType = LinearCyclic;

SDL_Color FractalImage::setColor = {0, 0, 0, 255};
SDL_Color FractalImage::debugColor = {255, 0, 0, 255};

std::vector<SDL_Color> FractalImage::palette;

inline SDL_Color Lerp(SDL_Color c1, SDL_Color c2, float t) {
	return {
		(Uint8)(c1.r + (c2.r - c1.r) * t),
		(Uint8)(c1.g + (c2.g - c1.g) * t),
		(Uint8)(c1.b + (c2.b - c1.b) * t),
		255
	};
}

void drawThreadTask(FractalImage& fi, int threadIndex) {

	unsigned int _i = threadIndex * 4;

	while (_i < fi.scaledWidth * fi.scaledHeight * 4) {

		if (fi.interruptThreads) {
			return;
		}

		int x, y, i;

		if(FractalImage::reverseRenderOrder) {
			i = fi.scaledWidth * fi.scaledHeight - _i / 4;
		} else {
			i = _i / 4;
		}

		fi.indexMapper(i, &x, &y, fi.scaledWidth, fi.scaledHeight);

		i = (y * fi.scaledWidth + x) * 4;

		if (i >= fi.threadAmount * 4) {
			fi.pixelArray[i] = 255 - fi.pixelArray[i];
			fi.pixelArray[i + 1] = 255 - fi.pixelArray[i + 1];
			fi.pixelArray[i + 2] = 255 - fi.pixelArray[i + 2];
			fi.pixelArray[i + 3] = 255;
		}


		SDL_Color c;

		if (fi.sampleDistance == 0) {
			fi.sampleDistance = 1;
		}

		if (fi.sampleDistance >= -1) {
			fi.dataArray[i / 4] = FractalAlgorithm(fi.currentAlgorithm, x, y, fi.scaledWidth, fi.scaledHeight, fi.iterationMax, fi.centerX, fi.centerY, fi.scale, fi.aspectRatio, fi.flags);
			c = fi.mapFractalData(fi.dataArray[i / 4]);
		}
		else {
			unsigned int r = 0;
			unsigned int g = 0;
			unsigned int b = 0;
			int superSampling = -fi.sampleDistance;
			bool isInside = false;

			int dataArrayIndex = i / 4 * superSampling * superSampling;

			fi.dataArray[dataArrayIndex] = FractalAlgorithm(fi.currentAlgorithm, x * superSampling, y * superSampling, fi.scaledWidth * superSampling, fi.scaledHeight * superSampling, fi.iterationMax, fi.centerX, fi.centerY, fi.scale, fi.aspectRatio, fi.flags);

			SDL_Color z = fi.mapFractalData(fi.dataArray[dataArrayIndex]);

			r += z.r;
			g += z.g;
			b += z.b;

			for (int innerY = superSampling - 1; innerY >= 0; innerY--) {
				for (int innerX = superSampling - 1; innerX >= 0; innerX--) {

					if (innerX == 0 && innerY == 0) {
						continue;
					}

					dataArrayIndex = i / 4 * superSampling * superSampling + innerX + innerY * superSampling;

					if (!isInside) {
						fi.dataArray[dataArrayIndex] = FractalAlgorithm(fi.currentAlgorithm, x * superSampling + innerX, y * superSampling + innerY, fi.scaledWidth * superSampling, fi.scaledHeight * superSampling, fi.iterationMax, fi.centerX, fi.centerY, fi.scale, fi.aspectRatio, fi.flags);
					}
					else {
						fi.dataArray[dataArrayIndex] = fi.dataArray[i / 4 * superSampling * superSampling];
					}

					if (innerY == superSampling - 1 && innerX == superSampling - 1 && fi.dataArray[dataArrayIndex].iterResult != NotInside && fi.dataArray[i / 4 * superSampling * superSampling].iterResult != NotInside) {
						isInside = true;
					}

					z = fi.mapFractalData(fi.dataArray[dataArrayIndex]);

					r += z.r;
					g += z.g;
					b += z.b;
				}
			}

			r /= superSampling * superSampling;
			g /= superSampling * superSampling;
			b /= superSampling * superSampling;
			//r = std::sqrt(r);
			//g = std::sqrt(g);
			//b = std::sqrt(b);
			c = {(unsigned char)r, (unsigned char)g, (unsigned char)b, 255};
		}


		fi.pixelArray[i] = c.r;
		fi.pixelArray[i + 1] = c.g;
		fi.pixelArray[i + 2] = c.b;
		fi.pixelArray[i + 3] = 255;

		if (threadIndex == fi.threadAmount - 1) {
			fi.renderProgress = (float)_i / fi.pixelArray.size();
		}

		_i = fi.lastAssignedIndex + 4;
		fi.lastAssignedIndex = _i;

	}

	if (threadIndex == fi.threadAmount - 1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		fi.lastRenderTime = SDL_GetTicks64() - fi.renderStartTime;
		fi.renderingStatus = Ready;
		if (fi.currentColorMapType == Histogram) {
			fi.generateHistogram();
			fi.refreshVisuals();
		}
	}

}

void FractalImage::generatePalette() {

	palette.clear();

	int colorsPerLerp = ColorAmount / currentPalette.paletteColors.size();

	for (size_t i = 0; i < currentPalette.paletteColors.size(); i++) {
		SDL_Color c1 = currentPalette.paletteColors[i];
		SDL_Color c2 = currentPalette.paletteColors[(i + 1) % currentPalette.paletteColors.size()];
		for (size_t j = 0; j < colorsPerLerp; j++) {
			float t = j / (float)colorsPerLerp;

			t = std::clamp(t, 0.0f, 1.0f);
			palette.push_back(Lerp(c1, c2, t));
		}
	}

}

SDL_Color FractalImage::mapFractalData(fractalData data) {
	if (data.iterResult < NotInside && !debugColors) {
		return setColor;
	}
	else if (data.iterResult == NotInside) {

		SDL_Color c1{};

		int iterDivInt = floor(data.smoothValue * iterDiv);

		int c1iter = (data.iterations * iterDiv + iterDivInt + ColorOffset);

		if (currentColorMapType == LinearCyclic) {
			c1 = palette[c1iter % palette.size()];
		}
		else if (currentColorMapType == ExponentialCyclic) {

			c1 = palette[(int)pow((float)c1iter, 0.9) % palette.size()];

		}
		else if (currentColorMapType == Histogram) {
			if (renderingStatus == Ready) {
				float hue = 0;
				for (int i = 0; i < data.iterations; i++) {

					hue += (float)imageHistogram.list[i] / imageHistogram.total;

				}

				hue += data.smoothValue * imageHistogram.list[data.iterations] / imageHistogram.total;

				c1 = palette[hue * palette.size() * (1.0 - 1.0 / currentPalette.paletteColors.size())];
			}
			else {
				c1 = {0, 0, 0, 0};
			}

		}

		if (shadowFx) {
			SDL_Color c2 = Lerp(c1, {0, 0, 0}, shadowStrength);

			return Lerp(c1, c2, data.shadowValue);
		}
		else {
			return c1;
		}

	}
	else if (debugColors) {
		if (data.iterResult == CircleTest) {
			return { 0, 0, 255, 255}; // Blue
		}
		else if (data.iterResult == LoopTest) {
			return { 0, 255, 0, 255 }; // Green
		}
		else if (data.iterResult == DerTest) {
			return { 255, 0, 0, 255 }; // Red
		}
		else if (data.iterResult == MaxIter) {
			return {0, 0, 0, 255}; // Black
		}
		else if (data.iterResult == Undefined) {
			return { 255, 255, 255, 255 }; // White
		}
	}
	else {
		return debugColor;
	}

	return setColor;

}

void FractalImage::updatePixels() {

	cancelUpdate();

	renderingStatus = Rendering;

	aspectRatio = (float)width / height;

	resizeArrays();

	threadPool.resize(threadAmount);

	renderStartTime = SDL_GetTicks64();

	if (currentAlgorithm == &MandelbrotSAPerturbation) {
		calculateReferenceMandelbrot(centerX, centerY, scale, iterationMax);
	}

	lastAssignedIndex = threadAmount * 4 - 4;

	for (int i = 0; i < threadAmount; i++)
	{
		threadPool[i] = std::thread(drawThreadTask, std::ref(*this), i);
	}

}

void FractalImage::refreshVisuals() {

	std::vector<std::thread> threads(threadAmount);

	for (int threadIndex = 0; threadIndex < threadAmount; threadIndex++)
	{
		threads[threadIndex] = std::thread([this, threadIndex]() {
			for (int i = threadIndex * 4; i < pixelArray.size(); i += threadAmount * 4)
			{
				SDL_Color c;

				if (sampleDistance >= -1) {

					c = mapFractalData(dataArray[i / 4]);

				}
				else {

					unsigned int r = 0;
					unsigned int g = 0;
					unsigned int b = 0;
					int superSampling = -sampleDistance;

					for (int j = 0; j < superSampling * superSampling; j++)
					{
						int dataArrayIndex = i / 4 * superSampling * superSampling + j;

						SDL_Color z = mapFractalData(dataArray[dataArrayIndex]);

						r += z.r;
						g += z.g;
						b += z.b;
					}

					r /= superSampling * superSampling;
					g /= superSampling * superSampling;
					b /= superSampling * superSampling;
					//r = std::sqrt(r);
					//g = std::sqrt(g);
					//b = std::sqrt(b);
					c = { (unsigned char)r, (unsigned char)g, (unsigned char)b };

				}

				pixelArray[i] = c.r;
				pixelArray[i + 1] = c.g;
				pixelArray[i + 2] = c.b;
				pixelArray[i + 3] = c.a;


			}
			});
	}

	for (int threadIndex = 0; threadIndex < threadAmount; threadIndex++)
	{
	
		threads[threadIndex].join();

	}
	

}

void FractalImage::resizeArrays() {
	scaledHeight = height / std::max(sampleDistance, 1);
	scaledWidth = width / std::max(sampleDistance, 1);
	if (sampleDistance < 0) {
		dataArray.resize(width * (-sampleDistance) * height * (-sampleDistance), {});
	}
	else {
		dataArray.resize(scaledWidth * scaledHeight);
	}
	pixelArray.resize(scaledWidth * scaledHeight * 4, 0);
}

void FractalImage::genDeltas() {

	pixelDeltas[0].clear();
	pixelDeltas[1].clear();

	pixelDeltas[0].resize(scaledWidth * scaledHeight);
	pixelDeltas[1].resize(scaledWidth * scaledHeight);

	int x, y;

	for (int i = 0; i < dataArray.size(); i++) {

		indexMapper(i, &x, &y, scaledWidth, scaledHeight);

		pixelDeltas[0][i] = (x - (scaledWidth >> 1)) / (double)scaledWidth * scale * aspectRatio;
		pixelDeltas[1][i] = (y - (scaledHeight >> 1)) / (double)scaledHeight * scale;
	}
}

void FractalImage::generateHistogram() {

	imageHistogram.total = 0;
	imageHistogram.list.clear();

	imageHistogram.list.resize(iterationMax - 1);
	imageHistogram.list.shrink_to_fit();

	for (int i = 0; i < dataArray.size(); i++) {
		if (dataArray[i].iterResult != NotInside) {
			continue;
		}
		imageHistogram.list[dataArray[i].iterations]++;
	}

	for (int i = 0; i < imageHistogram.list.size(); i++) {
		imageHistogram.total += imageHistogram.list[i];
	}

}

void FractalImage::cancelUpdate() {

	interruptThreads = true;

	for (auto i = 0; i < threadPool.size(); i++)
	{
		if (threadPool[i].joinable()) {
			//std::cout << i << " is joinable..." << std::endl;
			threadPool[i].join();
			//std::cout << i << " joined." << std::endl;
		}
	}

	interruptThreads = false;

}

std::string FractalImage::debugInfo() {

	int debugScaledWidth = width;
	int debugScaledHeight = height;

	if (sampleDistance < 0) {

		debugScaledWidth *= -sampleDistance;
		debugScaledHeight *= -sampleDistance;

	}
	else {
		debugScaledWidth /= sampleDistance;
		debugScaledHeight /= sampleDistance;
	}

	std::ostringstream stream;

	stream << "Image size: " << width << " x " << height << '\n';
	stream << "Scaled size: " << debugScaledWidth << " x " << debugScaledHeight << '\n';
	stream << "Iteration max: " << iterationMax << '\n';
	stream << "Center coordinates:\n";
	stream << std::setprecision(std::numeric_limits<double>::max_digits10);
	stream << "X: " << centerX << '\n';
	stream << "Y: " << centerY << '\n';
	stream << std::setprecision(3);
	stream << "Scale: " << scale << '\n';

	return stream.str();

	
}

void FractalImage::savePaletteList() {

	std::string listPath = prefPath + "palette-list.txt";

	std::ofstream ofs(listPath);

	if (ofs.good()) {
		for (int i = 0; i < paletteList.size(); i++)
		{
			colorPalette temp = paletteList[i];

			ofs << temp.name << '\n';

			for (int j = 0; j < temp.paletteColors.size(); j++)
			{
				uint32_t tempInt = (uint32_t)255 + ((uint32_t)temp.paletteColors[j].r << 8) + ((uint32_t)temp.paletteColors[j].g << 16) + ((uint32_t)temp.paletteColors[j].b << 24);
				ofs << tempInt << '\n';
			}
			ofs << 0 << '\n';
		}

		ofs << "\0\0\0";
	}

	ofs.close();
}

void FractalImage::loadPaletteList() {

	paletteList.clear();

	std::string listPath = prefPath + "palette-list.txt";

	std::ifstream ifs(listPath);

	if (ifs.good()) {
		unsigned char tempr;
		unsigned char tempg;
		unsigned char tempb;
		uint32_t tempInt;

		while (true)
		{
			colorPalette tempPalette;
			std::string tempString;

			ifs >> std::ws;

			std::getline(ifs, tempString);

			if (tempString == "\0\0\0") break;

			tempPalette.name = tempString;

			while (true)
			{
				ifs >> tempInt;
				if (tempInt == 0) break;

				tempr = (tempInt >> 8) & 255;
				tempg = (tempInt >> 16) & 255;
				tempb = (tempInt >> 24) & 255;


				tempPalette.paletteColors.push_back({ tempr, tempg, tempb });

			}

			paletteList.push_back(tempPalette);
		}
	}

	ifs.close();
}

void FractalImage::saveToImage(const char* absolutePath) {
	int nameindex = 0;

	std::filesystem::path filePath = absolutePath;
	filePath.replace_filename(std::to_string(nameindex));

	if (!std::filesystem::exists(filePath.parent_path())) {
		std::filesystem::create_directory(filePath.parent_path());
	}

	filePath.replace_extension("png");

	while (std::filesystem::exists(filePath)) {
		filePath.replace_filename(std::to_string(++nameindex));
		filePath.replace_extension("png");
	}

	if (renderingStatus == Empty) {
		updatePixels();
		for (int i = 0; i < threadPool.size(); i++)
		{
			threadPool[i].join();
		}
	}

	SDL_Surface* tempSurface = SDL_CreateRGBSurfaceWithFormatFrom(pixelArray.data(), scaledWidth, scaledHeight, 32, 4 * scaledWidth, SDL_PIXELFORMAT_RGBA32);

	// OpenGL textures are flipped vertically compared to SDL surfaces, so we temporarily flip the surface in order to get the correct image.
	SDL_FlipSurface(tempSurface);

	if (IMG_SavePNG(tempSurface, filePath.string().c_str()) == 0) {
		renderingStatus = ImageSaved;
	}

	SDL_FlipSurface(tempSurface);

	SDL_FreeSurface(tempSurface);

}

void FractalImage::generateZoomVideo(int& keyFrameCounter, deltafloat finalScale) {

	generateKeyframes(keyFrameCounter, finalScale);

	cropFramesFromKeyframes();

	// Code to compile a video from the frames here

	renderingStatus = VideoSaved;

}

void FractalImage::generateKeyframes(int& keyFrameCounter, deltafloat finalScale) {

	float zoomIncrement = 0.5;

	deltafloat tempScale = scale;

	int keyFrameAmount = floor(log(tempScale / finalScale) / log(zoomIncrement)).convert_to<int>();

	scale /= zoomIncrement;

	std::filesystem::path filePath = std::filesystem::current_path() / "images/zoomframes/";

	if (!std::filesystem::exists(filePath)) {
		std::filesystem::create_directory(filePath);
	}

	while (scale < finalScale) {
		saveToImage("images/zoomframes/");

		renderingStatus = Empty;

		keyFrameCounter++;

		scale /= zoomIncrement;
	}


}

void FractalImage::cropFramesFromKeyframes() {

}