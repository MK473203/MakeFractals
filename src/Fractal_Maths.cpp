// Fractal algorithms and other math stuff for Fractal Remake

#include "Fractal_Maths.h"

#include <utility>

std::map<const char*, fractalAlgorithmFunction> fractalAlgorithms {
	{"Quick Mandelbrot", &Mandelbrot},
	{"Deep Mandelbrot (brute force, SLOW)", &BigNumMandelbrot},
	{"Deep Mandelbrot (perturbation & series approximation, WIP)", &MandelbrotSAPerturbation},
	{"EvenOddMandelbrot", &EvenOddMandelbrot},
	{"Burning Ship", &BurningShip}
};

double eps = 1e-30;

float h2 = 1.5;
int shadowAngle = 45;

float vx = cos(pi / 180 * shadowAngle);
float vy = sin(pi / 180 * shadowAngle);

apfloat refPointX;
apfloat refPointY;
std::vector<double> refx;
std::vector<double> refy;

unsigned int lowestIter = 0;
unsigned int perturbationStartingIter = 0;
unsigned int perturbationEndIter = 0;
unsigned int highestIter = 0;
int seriesLength = 20;

std::vector<apcomplex> cf;

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
			if (iter < lowestIter) {
				lowestIter = std::max(iter, 1l);
			} else if(iter > highestIter) {
				highestIter = iter;
			}
			return;
		}

		xtemp = Zx * Zx - Zy * Zy + Cx;
		Zy = (Zx + Zx) * Zy + Cy;
		Zx = xtemp;

	}

	highestIter = max_iter;
}

void calculateReferenceMandelbrot(apfloat x, apfloat y, const deltafloat& scale, int max_iter) {

	refPointX = x;
	refPointY = y;

	std::vector<apcomplex> tempcf;

	cf.resize(seriesLength);
	tempcf.resize(seriesLength);

	for(int i = 0; i < cf.size(); i++) {

		cf[i] = { 0, 0 };
		tempcf[i] = { 0, 0 };
		
	}

	lowestIter = max_iter;
	highestIter = 0;
	perturbationEndIter = 0;

	int samplePoints = 9;

	for (float _x = -1; _x <= 1; _x += 2.0f / (samplePoints - 1))
	{
		for (float _y = -1; _y <= 1; _y += 2.0f / (samplePoints - 1))
		{
			if(abs(_y) < 1 && abs(_x) < 1) {
				continue;
			}
			calculateStartingIter(x + _x * scale * 0.5, y + _y * scale * 0.5, max_iter);
		}
	}

	perturbationStartingIter = lowestIter;

	perturbationStartingIter *= 0.99;
	
	apcomplex C = { x, y };
	apcomplex Z = { 0, 0 };

	mpc_t mpc_temp;
	mpc_init2(mpc_temp, mpc_get_prec(C.backend().data()));

	mpc_t mpc4;
	mpc_init2(mpc4, mpc_get_prec(C.backend().data()));
	mpc_set_d_d(mpc4, 4, 0, MPC_RNDNN);
	

	long iter;

	for (iter = 0; iter < max_iter; iter++) {

		if (iter < perturbationStartingIter) {

			for (int i = 0; i < cf.size(); i++) {
				
				//tempcf[i] = 2 * Z * cf[i];
				mpc_mul(mpc_temp, Z.backend().data(), cf[i].backend().data(), MPC_RNDNN);

				mpc_mul_si(mpc_temp, mpc_temp, 2, MPC_RNDNN);

				mpc_set(tempcf[i].backend().data(), mpc_temp, MPC_RNDNN);


				if(i == 0) {
					//tempcf[i] += 1;
					mpc_add_si(tempcf[i].backend().data(), tempcf[i].backend().data(), 1, MPC_RNDNN);
				} else if (i % 2 == 1) {

					for (int j = 0; j < i / 2; j++) {
						//tempcf[i] += 2 * cf[j] * cf[i - j - 1];
						mpc_mul(mpc_temp, cf[j].backend().data(), cf[i - j - 1].backend().data(), MPC_RNDNN);

						mpc_mul_si(mpc_temp, mpc_temp, 2, MPC_RNDNN);

						mpc_add(tempcf[i].backend().data(), tempcf[i].backend().data(), mpc_temp, MPC_RNDNN);

					}

					//tempcf[i] += cf[i / 2] * cf[i / 2];
					mpc_sqr(mpc_temp, cf[i / 2].backend().data(), MPC_RNDNN);

					mpc_add(tempcf[i].backend().data(), tempcf[i].backend().data(), mpc_temp, MPC_RNDNN);
					
				} else {
					for (int j = 0; j < i / 2; j++) {
						//tempcf[i] += 2 * cf[j] * cf[i - j - 1];
						mpc_mul(mpc_temp, cf[j].backend().data(), cf[i - j - 1].backend().data(), MPC_RNDNN);

						mpc_mul_si(mpc_temp, mpc_temp, 2, MPC_RNDNN);

						mpc_add(tempcf[i].backend().data(), tempcf[i].backend().data(), mpc_temp, MPC_RNDNN);
					}
				}

			}

			for (int i = 0; i < cf.size(); i++) {
				cf[i] = tempcf[i];
			}

			if(iter > 10 && abs(cf[cf.size() - 2]) < abs(cf[cf.size() - 1]) * scale) {
				perturbationStartingIter = iter + 1;
			}

		}
		else {

			if(refx.size() != max_iter - perturbationStartingIter) {
				refx.clear();
				refx.resize(max_iter - perturbationStartingIter);
				refy.clear();
				refy.resize(max_iter - perturbationStartingIter);
			}

			refx[iter - perturbationStartingIter] = mpfr_get_d(Z.real().backend().data(), MPFR_RNDN);
			refy[iter - perturbationStartingIter] = mpfr_get_d(Z.imag().backend().data(), MPFR_RNDN);

		}

		//Z = Z * Z + C;
		mpc_sqr(mpc_temp, Z.backend().data(), MPC_RNDNN);
		mpc_add(mpc_temp, mpc_temp, C.backend().data(), MPC_RNDNN);
		mpc_set(Z.backend().data(), mpc_temp, MPC_RNDNN);

		if(mpc_cmp_abs(Z.backend().data(), mpc4) > 0) {
			perturbationEndIter = iter;
			break;
		}

	}

	if(perturbationEndIter == 0) {
		perturbationEndIter = iter;
	}

	mpc_clear(mpc_temp);



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

			results.shadowValue = std::clamp(t, 0.0f, 1.0f);
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

	// Initial delta to the reference point
	double Dx = x.convert_to<double>();
	double Dy = y.convert_to<double>();

	// Full location of the point
	apfloat Cx = refPointX + x;
	apfloat Cy = refPointY + y;

	//  High precision complex number for the final Series Approximation result
	apcomplex Ecomplex = { 0, 0 };

	// High precision complex number for the Series Approximation power series calculation
	mpc_t Dcomplex;
	mpc_init2(Dcomplex, mpfr_get_prec(x.backend().data()));
	mpc_t temp_Dcomplex;
	mpc_init2(temp_Dcomplex, mpfr_get_prec(x.backend().data()));
	mpc_set_fr_fr(Dcomplex, x.backend().data(), y.backend().data(), MPC_RNDNN);
	mpc_set_ui_ui(temp_Dcomplex, 1, 0, MPC_RNDNN);

	mpc_t temp_mpc;
	mpc_init2(temp_mpc, mpfr_get_prec(x.backend().data()));

	// Final results of the calculations
	fractalData results = {};


	for (int i = 0; i < cf.size(); i++) {
		
		//Ecomplex += cf[i] * pow(Dcomplex, i + 1);

		mpc_mul(temp_Dcomplex, temp_Dcomplex, Dcomplex, MPC_RNDNN);

		mpc_mul(temp_mpc, temp_Dcomplex, cf[i].backend().data(), MPC_RNDNN);

		mpc_add(Ecomplex.backend().data(), Ecomplex.backend().data(), temp_mpc, MPC_RNDNN);



	}

	mpc_clear(temp_mpc);
	mpc_clear(Dcomplex);
	mpc_clear(temp_Dcomplex);

	double derx = 1;
	double dery = 0;
	double derxtemp;

	// doubles for the real and imaginary part of the difference orbit
	double Ex = mpfr_get_d(Ecomplex.real().backend().data(), MPFR_RNDN);
	double Ey = mpfr_get_d(Ecomplex.imag().backend().data(), MPFR_RNDN);

	double xtemp;
	
	// doubles for the reference orbit
	double Zx = 0;
	double Zy = 0;

	// doubles for the full orbit (used in effect calculations and escape checking)
	double Fx = 0;
	double Fy = 0;

	long iter = perturbationStartingIter;

	int escapeR = 1 << 16;

#pragma endregion

#pragma region iteration

	for (/*iter*/; iter < perturbationEndIter; iter++) {

		Zx = refx[iter - perturbationStartingIter];
		Zy = refy[iter - perturbationStartingIter];

		xtemp = 2 * (Zx * Ex - Zy * Ey) + Ex * Ex - Ey * Ey + Dx;
		Ey = 2 * (Zy * Ex + Zx * Ey) + 2 * Ex * Ey + Dy;
		Ex = xtemp;

		Fx = Zx + Ex;
		Fy = Zy + Ey;
		
		derxtemp = 2 * (derx * Fx - dery * Fy) + 1;
		dery = 2 * (derx * Fy + dery * Fx);
		derx = derxtemp;

		if (Fx * Fx + Fy * Fy > escapeR) {
			break;
		}


	}

	results.iterations = iter;

#pragma endregion

#pragma region fancy stuffs

	if (iter < max_iter) {
		results.iterResult = NotInside;

		//Smoothing value calculation

		if ((flags & DisableSmoothing) == 0) {
			
			float log_zn = log(Fx * Fx + Fy * Fy) / 2.0f;
			
			float nu = log(log_zn / log(2.0f)) / log(2.0f);

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

			float t = ux / absu * vx + uy / absu * vy + h2;
			t = t / (1 + h2);

			results.shadowValue = std::clamp(t, 0.0f, 1.0f);
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