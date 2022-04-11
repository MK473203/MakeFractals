// Fractal algorithms and other math stuff for Fractal Remake

#include "Fractal_Maths.h"


namespace FractalAlgorithms {

	std::map<const char*, fractalAlgorithmFunction> fractalAlgorithms{
		{"Mandelbrot", &Mandelbrot},
		{"EvenOddMandelbrot", &EvenOddMandelbrot},
		{"Burning Ship", &BurningShip},
	};

	double eps = std::numeric_limits<double>::epsilon();

	float h2 = 1.5;
	int shadowAngle = 45;

	double vx = cos(shadowAngle);
	double vy = sin(shadowAngle);

	inline bool checkCircles(double _x, double _y) {

		double _y2 = _y * _y;

		double q = pow(_x - 0.25, 2) + _y2;
		double q2 = q * (q + (_x - 0.25));

		if (q2 < 0.25 * (_y2)) {
			return true;
		}

		return false;
	}

	fractalData FractalAlgorithm(fractalAlgorithmFunction algorithm, int _x, int _y, int Width, int Height, int max_iter, double centerx, double centery, double scale, double aspectRatio, int flags) {

		double Cx = centerx + (_x - (Width >> 1)) / (double)Width * scale * aspectRatio;
		double Cy = centery - (_y - (Height >> 1)) / (double)Height * scale;

		return algorithm(Cx, Cy, max_iter, flags);

	}

	fractalData Mandelbrot(double x, double y, int max_iter, int flags) {
#pragma region variables


		double Cx = x;
		double Cy = y;

		fractalData results = {};

		if ((flags & DisableTests) == 0) {
			if (checkCircles(Cx, Cy)) {
				results.iterations = max_iter;
				results.iterResult = CircleTest;
				return results;
			}
		}

		double Zx = Cx;
		double Zy = Cy;

		double xx = Zx * Zx;
		double yy = Zy * Zy;

		double derx = 1;
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

			if ((flags & DisableTests) == 0) {
				if (derx * derx + dery * dery < eps) {
					results.iterations = max_iter;
					results.iterResult = DerTest;
					return results;
				}
			}

			derxtemp = 2 * (derx * Zx - dery * Zy);
			dery = 2 * (derx * Zy + dery * Zx);
			derx = derxtemp;


			Zy = (Zx + Zx) * Zy + Cy;
			Zx = xx - yy + Cx;

			xx = Zx * Zx;
			yy = Zy * Zy;

			if ((flags & DisableTests) == 0) {
				double xDiff = abs(Zx - hx);
				if (xDiff < eps) {
					double yDiff = abs(Zy - hy);
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

				double log_zn = log(Zx * Zx + Zy * Zy) / 2.0;
				double nu = log(log_zn / log(2)) / log(2);

				nu = 4 - nu;


				results.smoothValue = (float)nu;
			}
			else {
				results.smoothValue = 0.0f;
			}


			//Shadow calculation

			if ((flags & DisableShadowCalculation) == 0) {

				double derx2 = derx * derx;
				double dery2 = dery * dery;

				double ux = (Zx * derx + Zy * dery) / (derx2 + dery2);
				double uy = (Zy * derx - Zx * dery) / (derx2 + dery2);

				double absu = sqrt(ux * ux + uy * uy);
				ux = ux / absu;
				uy = uy / absu;

				float t = (float)(ux * vx + uy * vy + h2);
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

	fractalData EvenOddMandelbrot(double x, double y, int max_iter, int flags) {
#pragma region variables


		double Cx = x;
		double Cy = y;

		fractalData results = {};

		double Zx = Cx;
		double Zy = Cy;

		double xx = Zx * Zx;
		double yy = Zy * Zy;

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

				double log_zn = log(Zx * Zx + Zy * Zy) / 2.0;
				double nu = log(log_zn / log(2)) / log(2);

				nu = 4 - nu;


				results.smoothValue = (float)nu;
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

	fractalData BurningShip(double x, double y, int max_iter, int flags) {
#pragma region variables


		fractalData results = {};

		double Zx = x;
		double Zy = -y;

		double xx = Zx * Zx;
		double yy = Zy * Zy;

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

			Zy = std::abs(2 * Zx * Zy) - y;
			Zx = xx - yy + x;

			xx = Zx * Zx;
			yy = Zy * Zy;

			if ((flags & DisableTests) == 0) {
				double xDiff = abs(Zx - hx);
				if (xDiff < eps) {
					double yDiff = abs(Zy - hy);
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

				double log_zn = log(Zx * Zx + Zy * Zy) / 2.0;
				double nu = log(log_zn / log(2)) / log(2);

				nu = 4 - nu;


				results.smoothValue = (float)nu;
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

}