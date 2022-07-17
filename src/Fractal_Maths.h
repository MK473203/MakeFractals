#ifndef FMATHS
#define FMATHS

// Fractal algorithms and other math stuff for Fractal Remake

#include "Fractal_Utils.h"

#include <cmath>
#include <map>

const double pi = 3.14159265359878323;

typedef fractalData(*fractalAlgorithmFunction)(double, double, int, int);

namespace fa {

	extern std::map<const char*, fractalAlgorithmFunction> fractalAlgorithms;

	extern double eps;

	extern float h2;
	extern int shadowAngle;

	extern double vx;
	extern double vy;

	enum IterationFlags {
		None						= 0,
		DisableTests				= 1 << 0,
		DisableSmoothing			= 1 << 1,
		DisableShadowCalculation	= 1 << 2
	};

	inline bool checkCircles(double _x, double _y);

	inline void updateShadowVars() {
		vx = cos(pi / 360 * shadowAngle);
		vy = sin(shadowAngle);
	}

	fractalData FractalAlgorithm(fractalAlgorithmFunction algorithm, int _x, int _y, int Width, int Height, int max_iter, double centerx, double centery, double scale, double aspectRatio, int flags);

	fractalData Mandelbrot(double x, double y, int max_iter, int flags);

	fractalData EvenOddMandelbrot(double x, double y, int max_iter, int flags);

	fractalData BurningShip(double x, double y, int max_iter, int flags);
}

#endif // !FMATHS