// Fractal algorithms and other math stuff for Fractal Remake

#include "Fractal_Maths.h"

#include <utility>

std::map<const char*, fractalAlgorithmFunction> fractalAlgorithms {
	{"Quick Mandelbrot", &Mandelbrot},
	{"Deep Mandelbrot (naive)", &BigNumMandelbrot},
	{"Deep Mandelbrot (perturbation & series approximation)", &MandelbrotSAPerturbation},
	{"EvenOddMandelbrot", &EvenOddMandelbrot},
	{"Burning Ship", &BurningShip}
};

double eps = 1e-30;

float h2 = 1.5;
int shadowAngle = 45;

float vx = cos(pi / 180 * shadowAngle);
float vy = sin(pi / 180 * shadowAngle);

std::vector<apfloat> refx;
std::vector<apfloat> refy;

unsigned int startingIter = 0;
unsigned int highestIter = 0;
constexpr int seriesLength = 60;

apcomplex cf[seriesLength];

fractalData FractalAlgorithm(fractalAlgorithmFunction algorithm, int _x, int _y, int Width, int Height, int max_iter, 
							 const apfloat& centerx, const apfloat& centery, const deltafloat& scale, float aspectRatio, int flags) {


	if (algorithm == &MandelbrotSAPerturbation) {

		apfloat Cx = (_x - (Width >> 1)) / (double)Width * scale * aspectRatio;
		apfloat Cy = (_y - (Height >> 1)) / (double)Height * scale;

		return algorithm(Cx, Cy, max_iter, flags);

	}

	apfloat Cx = centerx + (_x - (Width >> 1)) / (double)Width * scale * aspectRatio;
	apfloat Cy = centery + (_y - (Height >> 1)) / (double)Height * scale;

	return algorithm(Cx, Cy, max_iter, flags);

}

void calculateStartingIter(apfloat x, apfloat y, int max_iter) {
	apfloat Cx = x;
	apfloat Cy = y;

	apfloat Zx = Cx;
	apfloat Zy = Cy;
	apfloat xtemp;

	long iter;

	for (iter = 0; iter < max_iter; iter++) {

		if (Zx * Zx + Zy * Zy >= 4.0) {
			if (iter < startingIter) {
				startingIter = std::max(iter, 1l);
			} else if(iter > highestIter) {
				highestIter = iter;
			}
			return;
		}

		refx[iter] = Zx;
		refy[iter] = Zy;

		xtemp = Zx * Zx - Zy * Zy + Cx;
		Zy = (Zx + Zx) * Zy + Cy;
		Zx = xtemp;

	}
}

void calculateReferenceMandelbrot(apfloat x, apfloat y, const deltafloat& scale, int max_iter) {


	refx.clear();
	refx.resize(max_iter);
	refy.clear();
	refy.resize(max_iter);

	for(int i = 0; i < seriesLength; ++i) {

		cf[i] = { 0, 0 };
		
	}

	apcomplex tempcf[seriesLength];

	for (int i = 0; i < seriesLength; ++i) {

		tempcf[i] = { 0, 0 };

	}

	startingIter = max_iter;
	highestIter = 0;

	int samplePoints = 9;

	for (float _x = -1; _x <= 1; _x += 2.0f / (samplePoints - 1))
	{
		for (float _y = -1; _y <= 1; _y += 2.0f / (samplePoints - 1))
		{
			calculateStartingIter(x + _x * scale * 0.6, y + _y * scale * 0.6, max_iter);
		}
	}
	
	apcomplex C = { x, y };
	apcomplex Z = { 0, 0 };

	apfloat xtemp;

	long iter;

	for (iter = 0; iter < max_iter; iter++) {

		if (iter < startingIter) {

			for (int i = 0; i < seriesLength; i++) {

				tempcf[i] = 2 * Z * cf[i];

				if(i == 0) {
					tempcf[i] += 1;
				} else if (i % 2 == 1) {

					for (int j = 0; j < i / 2; j++) {
						tempcf[i] += 2 * cf[j] * cf[i - j - 1];
					}

					tempcf[i] += cf[i / 2] * cf[i / 2];
					
				} else {
					for (int j = 0; j < i / 2; j++) {
						tempcf[i] += 2 * cf[j] * cf[i - j - 1];
					}
				}

			}

			for (int i = 0; i < seriesLength; i++) {
				cf[i] = tempcf[i];
			}

		}
		else {

			mpc_real(refx[iter].backend().data(), Z.backend().data(), MPFR_RNDN);
			mpc_imag(refy[iter].backend().data(), Z.backend().data(), MPFR_RNDN);

		}

		Z = Z * Z + C;

	}

	/*std::cout << std::setprecision(std::numeric_limits<apcomplex>::digits10 + 1);

	for (int i = 0; i <= seriesLength; ++i) {

		std::cout << cf[i].str() << std::endl;

	}*/




}

fractalData Mandelbrot				(const apfloat& x, const apfloat& y, int max_iter, int flags) {
#pragma region variables


	double Cx = x.convert_to<double>();
	double Cy = y.convert_to<double>();

	fractalData results = {};

	double Zx = 0;
	double Zy = 0;

	double xx = 0;
	double yy = 0;

	double derx = 0;
	double dery = 0;

	double derxtemp;

	long iter;

	int check = 3;
	int checkCounter = 0;

	int update = 10;
	int updateCounter = 0;

	double hx = 0.0;
	double hy = 0.0;

	int escapeR = 1 << 16;

#pragma endregion

#pragma region iteration


	for (iter = 0; iter < max_iter; iter++) {


		if (xx + yy > escapeR) {
			break;
		}

		derxtemp = 2 * (derx * Zx - dery * Zy) + 1;
		dery = 2 * (derx * Zy + dery * Zx);
		derx = derxtemp;


		Zy = (Zx + Zx) * Zy + Cy;
		Zx = xx - yy + Cx;

		xx = Zx * Zx;
		yy = Zy * Zy;

		if ((flags & DisableTests) == 0) {
			if (abs(Zx - hx) < eps / (iter + 1)) {
				if (abs(Zy - hy) < eps / (iter + 1)) {
					results.iterations = iter;
					results.iterResult = LoopTest;
					return results;
				}
			}

			if (check == checkCounter) {
				checkCounter = 0;

				if (update == updateCounter) {
					updateCounter = 0;
					check *= 2;
				}
				updateCounter++;

				hx = Zx;
				hy = Zy;

			}
			checkCounter++;
		}
	}

	results.iterations = iter;

#pragma endregion

#pragma region fancy stuffs


	if (iter < max_iter) {

		results.iterResult = NotInside;

		//Smoothing value calculation

		if ((flags & DisableSmoothing) == 0) {

			float log_zn = log(xx + yy) / 2.0;
			float nu = log(log_zn / log(2)) / log(2);

			nu = 4 - nu;


			results.smoothValue = nu;
		}
		else {
			results.smoothValue = 0.0f;
		}


		//Shadow calculation

		if ((flags & DisableShadowCalculation) == 0) {

			float ux = (Zx * derx + Zy * dery) / (derx * derx + dery * dery);
			float uy = (Zy * derx - Zx * dery) / (derx * derx + dery * dery);

			float absu = sqrt(ux * ux + uy * uy);
			ux = ux / absu;
			uy = uy / absu;

			float t = ux * vx + uy * vy + h2;
			t = t / (1 + h2);
			if (t < 0) t = 0;
			if (t > 1) t = 1;

			results.shadowValue = t;
		}
		else {
			results.shadowValue = 0.0f;
		}
	}
	else {
		results.iterResult = MaxIter;
	}



#pragma endregion

	return results;
}

fractalData BigNumMandelbrot		(const apfloat& x, const apfloat& y, int max_iter, int flags) {
#pragma region variables


	apfloat Cx = x;
	apfloat Cy = y;

	fractalData results = {};

	apfloat Zx = 0;
	apfloat Zy = 0;

	apfloat xx = 0;
	apfloat yy = 0;

	apfloat derx = 0;
	apfloat dery = 0;

	apfloat derxtemp;

	long iter;

	int check = 3;
	int checkCounter = 0;

	int update = 10;
	int updateCounter = 0;

	apfloat hx = 0.0;
	apfloat hy = 0.0;

	int escapeR = 1 << 16;

#pragma endregion

#pragma region iteration


	for (iter = 0; iter < max_iter; iter++) {

		derxtemp = 2 * (derx * Zx - dery * Zy) + 1;
		dery = 2 * (derx * Zy + dery * Zx);
		derx = derxtemp;

		Zy = (Zx + Zx) * Zy + Cy;
		Zx = xx - yy + Cx;

		xx = Zx * Zx;
		yy = Zy * Zy;

		if (xx + yy > escapeR) {
			break;
		}

		if ((flags & DisableTests) == 0) {
			if (abs(Zx - hx) < eps) {
				if (abs(Zy - hy) < eps) {
					results.iterations = iter;
					results.iterResult = LoopTest;
					return results;
				}
			}

			if (check == checkCounter) {
				checkCounter = 0;

				if (update == updateCounter) {
					updateCounter = 0;
					check *= 2;
				}
				updateCounter++;

				hx = Zx;
				hy = Zy;

			}
			checkCounter++;
		}
	}

	results.iterations = iter;

#pragma endregion

#pragma region fancy stuffs


	if (iter < max_iter) {

		results.iterResult = NotInside;

		//Smoothing value calculation

		if ((flags & DisableSmoothing) == 0) {

			float log_zn = log((Zx * Zx + Zy * Zy).convert_to<float>()) / 2.0f;

			float nu = log(log_zn / log(2.0f)) / log(2.0f);

			nu = 4 - nu;
		}
		else {
			results.smoothValue = 0.0f;
		}


		//Shadow calculation

		if ((flags & DisableShadowCalculation) == 0) {

			apfloat ux = (Zx * derx + Zy * dery) / (derx * derx + dery * dery);
			apfloat uy = (Zy * derx - Zx * dery) / (derx * derx + dery * dery);

			apfloat absu = sqrt(ux * ux + uy * uy);

			float t = (ux / absu).convert_to<float>() * vx + (uy / absu).convert_to<float>() * vy + h2;
			t = t / (1 + h2);
			if (t < 0) t = 0;
			if (t > 1) t = 1;

			results.shadowValue = t;
		}
		else {
			results.shadowValue = 0.0f;
		}
	}
	else {
		results.iterResult = MaxIter;
	}



#pragma endregion

	return results;
}

fractalData MandelbrotSAPerturbation(const apfloat& x, const apfloat& y, int max_iter, int flags) {
#pragma region variables


	double Dx = x.convert_to<double>();
	double Dy = y.convert_to<double>();

	apcomplex Dcomplex = {x, y};

	fractalData results = {};

	apcomplex Ecomplex = {0, 0};

	for(int i = 0; i < seriesLength; i++) {

		Ecomplex += cf[i] * pow(Dcomplex, i + 1);
		
	}

	double Ex = Ecomplex.real().convert_to<double>();
	double Ey = Ecomplex.imag().convert_to<double>();

	double xtemp;

	apfloat Zx;
	apfloat Zy;

	apfloat Fx;
	apfloat Fy;

	long iter;

	int check = 3;
	int checkCounter = 0;

	int update = 10;
	int updateCounter = 0;

	apfloat hx = 0.0;
	apfloat hy = 0.0;

	int escapeR = 1 << 16;

#pragma endregion

#pragma region iteration

	for (iter = startingIter; iter < max_iter; iter++) {

		Zx = refx[iter];
		Zy = refy[iter];

		xtemp = Dx + Ex * Ex - Ey * Ey + 2 * (Ex * Zx - Ey * Zy).convert_to<double>();
		Ey = Dy + 2 * Ex * Ey + 2 * (Ex * Zy + Ey * Zx).convert_to<double>();
		Ex = xtemp;

		Fx = Zx + Ex;
		Fy = Zy + Ey;

		if (Fx * Fx + Fy * Fy > escapeR) {
			break;
		}

		if ((flags & DisableTests) == 0) {
			if (abs(Fx - hx) < eps) {
				if (abs(Fy - hy) < eps) {
					results.iterations = iter;
					results.iterResult = LoopTest;
					return results;
				}
			}

			if (check == checkCounter) {
				checkCounter = 0;

				if (update == updateCounter) {
					updateCounter = 0;
					check *= 2;
				}
				updateCounter++;

				hx = Fx;
				hy = Fy;

			}
			checkCounter++;
		}

	}

	results.iterations = iter;

#pragma endregion

#pragma region fancy stuffs

	if (iter < max_iter) {
		results.iterResult = NotInside;

		//Smoothing value calculation

		if ((flags & DisableSmoothing) == 0) {

			//float log_zn = log(Fx * Fx + Fy * Fy).convert_to<float>() / 2.0f;
			float log_zn = log((Fx * Fx + Fy * Fy).convert_to<float>()) / 2.0f;
			
			float nu = log(log_zn / log(2.0f)) / log(2.0f);

			nu = 4 - nu;


			results.smoothValue = nu;
		}
		else {
			results.smoothValue = 0.0f;
		}
	}
	else {
		results.iterResult = MaxIter;
	}


#pragma endregion

	return results;

}

fractalData EvenOddMandelbrot		(const apfloat& x, const apfloat& y, int max_iter, int flags) {
#pragma region variables


	apfloat Cx = x;
	apfloat Cy = y;

	fractalData results = {};

	apfloat Zx = Cx;
	apfloat Zy = Cy;

	apfloat xx = Zx * Zx;
	apfloat yy = Zy * Zy;

	long iter;

	int escapeR = 1 << 16;

#pragma endregion

#pragma region iteration


	for (iter = 0; iter < max_iter; iter++) {


		if (xx + yy > escapeR) {
			break;
		}

		if (iter % 2 == 0) {

			Zy = (Zx + Zx) * Zy + Cy;
			Zx = xx - yy + Cx;

		}
		else {
			Zy = (Zx + Zx) * Zy - Cy;
			Zx = xx - yy - Cx;
		}

		xx = Zx * Zx;
		yy = Zy * Zy;

	}

	results.iterations = iter;

#pragma endregion

#pragma region fancy stuffs


	if (iter < max_iter) {

		results.iterResult = NotInside;

		//Smoothing value calculation

		if ((flags & DisableSmoothing) == 0) {

			float log_zn = log(Zx * Zx + Zy * Zy).convert_to<float>() / 2.0;
			float nu = log(log_zn / log(2)) / log(2);

			nu = 4 - nu;


			results.smoothValue = nu;
		}
		else {
			results.smoothValue = 0.0f;
		}


		//Shadow calculation

		results.shadowValue = 0.0f;

	}
	else {
		results.iterResult = MaxIter;
	}



#pragma endregion

	return results;
}

fractalData BurningShip				(const apfloat& x, const apfloat& y, int max_iter, int flags) {
#pragma region variables


	fractalData results = {};

	apfloat Zx = x;
	apfloat Zy = -y;

	apfloat xx = Zx * Zx;
	apfloat yy = Zy * Zy;

	long iter;

	int check = 3;
	int checkCounter = 0;

	int update = 10;
	int updateCounter = 0;

	apfloat hx = 0.0;
	apfloat hy = 0.0;

	int escapeR = 1 << 16;

#pragma endregion

#pragma region iteration


	for (iter = 0; iter < max_iter; iter++) {


		if (xx + yy > escapeR) {
			break;
		}

		Zy = abs(2.0 * Zx * Zy) - y;
		Zx = xx - yy + x;

		xx = Zx * Zx;
		yy = Zy * Zy;

		if ((flags & DisableTests) == 0) {
			apfloat xDiff = abs(Zx - hx);
			if (xDiff < eps) {
				apfloat yDiff = abs(Zy - hy);
				if (yDiff < eps) {
					results.iterations = max_iter;
					results.iterResult = LoopTest;
					return results;
				}
			}

			if (check == checkCounter) {
				checkCounter = 0;

				if (update == updateCounter) {
					updateCounter = 0;
					check *= 2;
				}
				updateCounter++;

				hx = Zx;
				hy = Zy;

			}
			checkCounter++;
		}

	}

	results.iterations = iter;

#pragma endregion

#pragma region fancy stuffs


	if (iter < max_iter) {

		results.iterResult = NotInside;

		//Smoothing value calculation

		if ((flags & DisableSmoothing) == 0) {

			float log_zn = log(Zx * Zx + Zy * Zy).convert_to<float>() / 2.0;
			float nu = log(log_zn / log(2)) / log(2);

			nu = 4 - nu;


			results.smoothValue = nu;
		}
		else {
			results.smoothValue = 0.0f;
		}


		//Shadow calculation

		results.shadowValue = 0.0f;

	}
	else {
		results.iterResult = MaxIter;
	}



#pragma endregion

	return results;


}