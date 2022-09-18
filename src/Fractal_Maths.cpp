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

int startingIter = 0;

apfloat A[2], B[2], C[2], D[2];

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
	apfloat Cx = std::move(x);
	apfloat Cy = std::move(y);

	apfloat Zx = Cx;
	apfloat Zy = Cy;
	apfloat xtemp;

	long iter;

	for (iter = 0; iter < max_iter; iter++) {

		if (Zx * Zx + Zy * Zy >= 4.0) {
			if (iter < startingIter) {
				startingIter = std::max(iter, 1l);
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

	/*for (int _x = -1; _x < 2; _x++)
	{
		for (int _y = -1; _y < 2; _y++)
		{
			calculateStartingIter(x + _x * scale, y + _y * scale, max_iter);
		}
	}*/


	apfloat Cx = std::move(x);
	apfloat Cy = std::move(y);

	apfloat Zx = 0;
	apfloat Zy = 0;
	apfloat xtemp;

	startingIter = 0;

	apfloat tempA[2], tempB[2], tempC[2], tempD[2] = { 0, 0 };

	A[0] = 0;
	A[1] = 0;
	B[0] = 0;
	B[1] = 0;
	C[0] = 0;
	C[1] = 0;
	D[0] = 0;
	D[1] = 0;

	long iter;

	for (iter = 0; iter < max_iter; iter++) {

		if (startingIter == 0) {
			tempA[0] = 2.0 * A[0] * Zx - 2.0 * A[1] * Zy + 1.0;
			tempA[1] = 2.0 * A[0] * Zy + 2.0 * A[1] * Zx;
			tempB[0] = A[0] * A[0] - A[1] * A[1] + 2.0 * B[0] * Zx - 2.0 * B[1] * Zy;
			tempB[1] = 2.0 * A[0] * A[1] + 2.0 * B[0] * Zy + 2.0 * B[1] * Zx;
			tempC[0] = 2.0 * A[0] * B[0] - 2.0 * A[1] * B[1] + 2.0 * C[0] * Zx - 2.0 * C[1] * Zy;
			tempC[1] = 2.0 * A[0] * B[1] + 2.0 * A[1] * B[0] + 2.0 * C[0] * Zy + 2.0 * C[1] * Zx;
			tempD[0] = 2.0 * A[0] * C[0] - 2.0 * A[1] * C[1] + B[0] * B[0] - B[1] * B[1] + 2.0 * D[0] * Zx - 2.0 * D[1] * Zy;
			tempD[1] = 2.0 * A[0] * C[1] + 2.0 * A[1] * C[0] + 2.0 * B[0] * B[1] + 2.0 * D[0] * Zy + 2.0 * D[1] * Zx;
			A[0] = tempA[0];
			A[1] = tempA[1];
			B[0] = tempB[0];
			B[1] = tempB[1];
			C[0] = tempC[0];
			C[1] = tempC[1];
			D[0] = tempD[0];
			D[1] = tempD[1];

			if (iter > 10) {
				if (sqrt(B[0] * B[0] + B[1] * B[1]) / (sqrt(C[0] * C[0] + C[1] * C[1]) * scale) < pow(10.0, 12.0)) {
					startingIter = iter +1;
				}
			}

		}
		else {
			refx[iter] = Zx;
			refy[iter] = Zy;
		}

		xtemp = Zx * Zx - Zy * Zy + Cx;
		Zy = (Zx + Zx) * Zy + Cy;
		Zx = xtemp;

	}

	std::cout << std::setprecision(std::numeric_limits<apfloat>::digits10 + 1);
	std::cout << Cx << std::endl;
	std::cout << Cy << std::endl;
	std::cout << A[0] << std::endl;
	std::cout << A[1] << std::endl;
	std::cout << B[0] << std::endl;
	std::cout << B[1] << std::endl;
	std::cout << C[0] << std::endl;
	std::cout << C[1] << std::endl;
	std::cout << D[0] << std::endl;
	std::cout << D[1] << std::endl;



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


	apfloat Cx = std::move(x);
	apfloat Cy = std::move(y);

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

			float log_zn = log(Zx * Zx + Zy * Zy).convert_to<float>() / 2.0f;
			float nu = log(log_zn / log(2)) / log(2);

			nu = 4 - nu;

			results.smoothValue = nu;
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

	fractalData results = {};

	//double Ex = (A[0] * x - A[1] * y + B[0] * (x * x - y * y) - 2 * B[1] * x * y + C[0] * x * (x * x - 3 * y * y) - C[1] * y * (3 * x * x - y * y)).convert_to<double>();
	//double Ey = (A[0] * y + A[1] * x + 2 * B[0] * x * y + B[1] * (x * x - y * y) + C[0] * y * (3 * x * x - y * y) + C[1] * x * (x * x - 3 * y * y)).convert_to<double>();
	
	//double Ex = (A[0] * x - A[1] * y + B[0] * (x * x - y * y) - 2 * B[1] * x * y + C[0] * x * (x * x - 3 * y * y) - C[1] * (3 * x * x - y * y) * y + x * x * x * x * D[0] - 4 * x * x * x * y * D[1] - 6 * x * x * y * y * D[0] + 4 * x * y * y * y * D[1] + y * y * y * y * D[0]).convert_to<double>();
	double Ex = (x * (A[0] - y * (2 * B[1] + y * (3 + 6 * x * D[0])) + x * (B[0] - x * (4 * y * D[1] - C[0] + x * D[0]))) + y * (y * (y * (C[1] + 4 * x + y) - B[0]) - A[1] - 3 * C[1] * x * x)).convert_to<double>();


	//double Ey = (A[0] * y + A[1] * x + 2 * B[0] * x * y + B[1] * (x * x - y * y) + C[0] * (3 * x * x - y * y) * y + C[1] * x * (x * x - 3 * y * y) + x * x * x * x * D[1] + 4 * x * x * x * y * D[0] - 6 * x * x * y * y * D[1] - 4 * x * y * y * y * D[0] + y * y * y * y * D[1]).convert_to<double>();
	double Ey = (x * (A[1] + 2 * B[0] * y + x * (B[1] + x * (C[1] + y * (x * (3 * C[0] + 4 * D[0]) - 6 * D[1] * y) + D[1] * x))) + y * (A[0] - y * (B[1] + 3 * C[1] * x + y * (3 * C[0] + 4 * D[0] * x - D[1] * y)))).convert_to<double>();

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

			float log_zn = log(Fx * Fx + Fy * Fy).convert_to<float>() / 2.0;
			float nu = log(log_zn / log(2)) / log(2);

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


	apfloat Cx = std::move(x);
	apfloat Cy = std::move(y);

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