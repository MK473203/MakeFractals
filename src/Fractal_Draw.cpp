// Code for mapping colors to iterations, drawing images etc.

#include "Fractal_Draw.h"
bool FractalImage::shadowFx = false;
bool FractalImage::debugColors = false;
int FractalImage::ColorAmount = 4096;
int FractalImage::ColorOffset = 0;
float FractalImage::shadowStrength = 0.5f; 
fractalAlgorithmFunction FractalImage::currentAlgorithm = &FractalAlgorithms::Mandelbrot;

/*
{"Default",
std::vector<sf::Color>{
	sf::Color(0, 7, 100),
	sf::Color(32, 107, 203),
	sf::Color(237, 255, 255),
	sf::Color(255, 170, 0),
	sf::Color(0, 2, 0)
}},
The sacred default colors. Guard them well.
*/

std::vector<colorPalette> FractalImage::paletteList{
	{"Default",
	std::vector<sf::Color>{
		sf::Color(0, 7, 100),
		sf::Color(32, 107, 203),
		sf::Color(237, 255, 255),
		sf::Color(255, 170, 0),
		sf::Color(0, 2, 0)
	}},
	{"Black and white",
	std::vector<sf::Color>{
		sf::Color(255, 255, 255),
		sf::Color(0, 0, 0)
	}},
	{"Pastel",
	std::vector<sf::Color>{
		sf::Color(220, 204, 162),
		sf::Color(234, 228, 204),
		sf::Color(249, 249, 248),
		sf::Color(165, 173, 158),
		sf::Color(66, 70, 57)
	}},
	{"idk",
	std::vector<sf::Color>{
		sf::Color(),
		sf::Color(0,7,80),
		sf::Color(),
		sf::Color(60,0,120),
		sf::Color(),
		sf::Color(130,58,160),
		sf::Color(),
		sf::Color(200,68,185),
		sf::Color(),
		sf::Color(240,200,220)
	}}
};

colorPalette FractalImage::currentPalette = FractalImage::paletteList[0];



sf::Color FractalImage::setColor = sf::Color();
sf::Color FractalImage::debugColor = sf::Color(255, 255, 0);

std::vector<sf::Color> FractalImage::palette;

std::vector<std::thread> FractalImage::threadPool;

inline sf::Color Lerp(sf::Color c1, sf::Color c2, float t) {
	return sf::Color(
		c1.r + (c2.r - c1.r) * t,
		c1.g + (c2.g - c1.g) * t,
		c1.b + (c2.b - c1.b) * t
	);
}

void drawThreadTask(int threadIndex, FractalImage& fi) {


	for (int i = threadIndex * 4; i < fi.scaledWidth * fi.scaledHeight * 4; i += fi.threadAmount * 4) {

		if (fi.interruptThreads) {
			return;
		}

		if (i >= fi.threadAmount * 4) {
			fi.pixelArray[i] = 127;
			fi.pixelArray[i + 1] = 127;
			fi.pixelArray[i + 2] = 127;
			fi.pixelArray[i + 3] = 255;
		}

		int x = (i / 4) % fi.scaledWidth;
		int y = (i / 4) / fi.scaledWidth;

		sf::Color c;

		if (fi.sampleDistance == 0) {
			fi.sampleDistance = 1;
		}

		if (fi.sampleDistance >= -1) {
			fi.dataArray[i / 4] = FractalAlgorithms::FractalAlgorithm(fi.currentAlgorithm, x, y, fi.scaledWidth, fi.scaledHeight, fi.iterationMax, fi.centerX, fi.centerY, fi.scale, fi.aspectRatio, fi.flags);
			c = fi.MapIterations(fi.dataArray[i / 4]);
		}
		else {
			unsigned int r = 0;
			unsigned int g = 0;
			unsigned int b = 0;
			int superSampling = -fi.sampleDistance;
			bool isInside = false;

			int dataArrayIndex = i / 4 * superSampling * superSampling;

			fi.dataArray[dataArrayIndex] = FractalAlgorithms::FractalAlgorithm(fi.currentAlgorithm, x * superSampling, y * superSampling, fi.scaledWidth * superSampling, fi.scaledHeight * superSampling, fi.iterationMax, fi.centerX, fi.centerY, fi.scale, fi.aspectRatio, fi.flags);

			sf::Color z = fi.MapIterations(fi.dataArray[dataArrayIndex]);

			r += z.r;
			g += z.g;
			b += z.b;

			for (int innerY = superSampling -1 ; innerY >= 0; innerY--) {
				for (int innerX = superSampling - 1; innerX >= 0; innerX--) {

					if (innerX == 0 && innerY == 0) {
						continue;
					}

					dataArrayIndex = i / 4 * superSampling * superSampling + innerX + innerY * superSampling;

					if (!isInside) {
						fi.dataArray[dataArrayIndex] = FractalAlgorithms::FractalAlgorithm(fi.currentAlgorithm, x * superSampling + innerX, y * superSampling + innerY, fi.scaledWidth * superSampling, fi.scaledHeight * superSampling, fi.iterationMax, fi.centerX, fi.centerY, fi.scale, fi.aspectRatio, fi.flags);
					}
					else {
						fi.dataArray[dataArrayIndex] = fi.dataArray[i / 4 * superSampling * superSampling];
					}

					if (innerY == superSampling - 1 && innerX == superSampling - 1 && fi.dataArray[dataArrayIndex].iterResult != NotInside && fi.dataArray[i / 4 * superSampling * superSampling].iterResult != NotInside) {
						isInside = true;
					}

					z = fi.MapIterations(fi.dataArray[dataArrayIndex]);

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
			c = sf::Color(r, g, b);
		}


		fi.pixelArray[i] = c.r;
		fi.pixelArray[i + 1] = c.g;
		fi.pixelArray[i + 2] = c.b;
		fi.pixelArray[i + 3] = 255;

		if (threadIndex == fi.threadAmount - 1) {
			fi.renderProgress = (float)i / fi.pixelArray.size();
		}

	}

	if(threadIndex == fi.threadAmount - 1) fi.lastRenderTime = fi.renderClock.restart().asMilliseconds();
	fi.renderingStatus = Ready;

}

void FractalImage::generatePalette() {

	palette.clear();

	int colorsPerLerp = ColorAmount / currentPalette.paletteColors.size();

	for (size_t i = 0; i < currentPalette.paletteColors.size(); i++) {
		sf::Color c1 = currentPalette.paletteColors[i];
		sf::Color c2 = currentPalette.paletteColors[(i + 1) % currentPalette.paletteColors.size()];
		for (size_t j = 0; j < colorsPerLerp; j++) {
			float t = j / (float)colorsPerLerp;

			t = std::clamp(t, 0.0f, 1.0f);
			palette.push_back(Lerp(c1, c2, t));
		}
	}

}

sf::Color FractalImage::MapIterations(fractalData data) {
	if (data.iterResult < NotInside && !debugColors) {
		return setColor;
	}
	else if (data.iterResult == NotInside) {

		int iterDivInt = floor(data.smoothValue * iterDiv);

		int c1iter = (data.iterations * iterDiv + iterDivInt + ColorOffset);

		sf::Color c1 = palette[c1iter % palette.size()];

		if (shadowFx) {
			sf::Color c4 = Lerp(c1, sf::Color(), shadowStrength);

			return Lerp(c1, c4, data.shadowValue);
		}
		else {
			return c1;
		}

	}
	else if (debugColors) {
		if (data.iterResult == CircleTest) {
			return sf::Color(0, 0, 255); // Blue
		}
		else if (data.iterResult == LoopTest) {
			return sf::Color(0, 255, 0); // Green
		}
		else if (data.iterResult == DerTest) {
			return sf::Color(255, 0, 0); // Red
		}
		else if (data.iterResult == MaxIter) {
			return sf::Color(); // Black
		}
		else if (data.iterResult == Undefined) {
			return sf::Color(255, 255, 255); // White
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

	scaledHeight = height / std::max(sampleDistance, 1);
	scaledWidth = width / std::max(sampleDistance, 1);
	if (sampleDistance < 0) {
		dataArray.resize(width * (-sampleDistance) * height * (-sampleDistance), {});
	}
	else {
		dataArray.resize(scaledWidth * scaledHeight);
	}
	pixelArray.resize(scaledWidth * scaledHeight * 4);

	threadPool.clear();
	threadPool.resize(threadAmount);

	renderClock.restart();

	for (int i = 0; i < threadAmount; i++)
	{
		threadPool[i] = std::thread(drawThreadTask, i, std::ref(*this));
	}

}

void FractalImage::refreshVisuals() {

	std::vector<std::thread> threads(threadAmount);

	for (int threadIndex = 0; threadIndex < threadAmount; threadIndex++)
	{
		threads[threadIndex] = std::thread([this, threadIndex]() {
			for (int i = threadIndex * 4; i < pixelArray.size(); i += threadAmount * 4)
			{
				sf::Color c;

				if (sampleDistance >= -1) {

					c = MapIterations(dataArray[i / 4]);

				}
				else {

					unsigned int r = 0;
					unsigned int g = 0;
					unsigned int b = 0;
					int superSampling = -sampleDistance;

					for (int j = 0; j < superSampling * superSampling; j++)
					{
						int dataArrayIndex = i / 4 * superSampling * superSampling + j;

						sf::Color z = MapIterations(dataArray[dataArrayIndex]);

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
					c = sf::Color(r, g, b);

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

void FractalImage::cancelUpdate() {

	interruptThreads = true;

	for (int i = 0; i < threadPool.size(); i++)
	{
		if (threadPool[i].joinable()) {
			threadPool[i].join();
			threadPool[i].~thread();
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
	std::ofstream ofs("palette-list.txt");

	if (paletteList.size() == 0) {
		ofs << "White\n";
		ofs << 4294967295 << '\n';
		ofs << 0 << '\n';
		ofs << -1;
	}

	for (int i = 0; i < paletteList.size(); i++)
	{
		colorPalette temp = paletteList[i];

		ofs << temp.name << '\n';

		for (int j = 0; j < temp.paletteColors.size(); j++)
		{
			ofs << temp.paletteColors[j].toInteger() << '\n';
		}
		ofs << 0 << '\n';
	}

	ofs << -1;

	ofs.close();
}

void FractalImage::loadPaletteList() {

	paletteList.clear();

	std::ifstream ifs("palette-list.txt");

	while (true)
	{
		colorPalette tempPalette;
		std::string tempString;

		ifs >> std::ws;

		std::getline(ifs, tempString);

		if (tempString == "-1") break;

		tempPalette.name = tempString;

		uint32_t tempInt;
		while (true)
		{
			ifs >> tempInt;

			if (tempInt == 0) break;
				
			tempPalette.paletteColors.push_back(sf::Color(tempInt));

		}

		paletteList.push_back(tempPalette);
	}

	ifs.close();
}

void FractalImage::saveToImage() {
	int nameindex = 0;

	std::filesystem::path filePath = std::filesystem::current_path() / "images/";
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

	sf::Image image;
	image.create(width, height, pixelArray.data());
	if (image.saveToFile(filePath)) {
		renderingStatus = ImageSaved;
	}

}