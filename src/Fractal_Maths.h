#ifndef FMATHS
#define FMATHS

// Fractal algorithms and other math stuff for Fractal Remake

#include <cmath>
#include <boost/multiprecision/gmp.hpp>
#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/mpc.hpp>

#include "Fractal_Utils.h"

const float pi = 3.14159265359878323f;

typedef boost::multiprecision::mpfr_float_100 apfloat;

typedef boost::multiprecision::number<boost::multiprecision::gmp_float<10> > deltafloat;

typedef boost::multiprecision::mpc_complex_100 apcomplex;

typedef fractalData(*fractalAlgorithmFunction)(const apfloat&, const apfloat&, int, int);

extern std::map<const char*, fractalAlgorithmFunction> fractalAlgorithms;

extern double eps;

extern float h2;
extern int shadowAngle;

extern float vx;
extern float vy;

extern std::vector<double> refx;
extern std::vector<double> refy;

extern unsigned int lowestIter;
extern unsigned int perturbationEndIter;
extern unsigned int perturbationStartingIter;
extern unsigned int highestIter;

enum IterationFlags {
	None						= 0,
	DisableTests				= 1 << 0,
	DisableSmoothing			= 1 << 1,
	DisableShadowCalculation	= 1 << 2
};

inline void updateShadowVars() {
	vx = cos(pi / 180 * shadowAngle);
	vy = sin(pi / 180 * shadowAngle);
}

fractalData FractalAlgorithm(fractalAlgorithmFunction algorithm, int _x, int _y, int Width, int Height, int max_iter,
                             const apfloat& centerx, const apfloat& centery, const deltafloat& scale, float aspectRatio, int flags);

void calculateReferenceMandelbrot(apfloat x, apfloat y, const deltafloat& scale, int max_iter);

fractalData Mandelbrot				(const apfloat& x, const apfloat& y, int max_iter, int flags);

fractalData BigNumMandelbrot		(const apfloat& x, const apfloat& y, int max_iter, int flags);

fractalData MandelbrotSAPerturbation(const apfloat& x, const apfloat& y, int max_iter, int flags);

fractalData EvenOddMandelbrot		(const apfloat& x, const apfloat& y, int max_iter, int flags);

fractalData BurningShip				(const apfloat& x, const apfloat& y, int max_iter, int flags);

#endif // !FMATHS