// Copyright 2016 Ismael Jimenez Martinez. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "minimal_leastsq.h"

#include <math.h>
#include <iostream>
#include "assert.h"

// Internal function to calculate the different scalability forms
double fittingCurve(double N, BigO Complexity) {
	if (Complexity == BigO::O_N)
		return N;
	else if (Complexity == BigO::O_N_Squared)
		return pow(N, 2);
	else if (Complexity == BigO::O_N_Cubed)
		return pow(N, 3);
	else if (Complexity == BigO::O_log_N)
		return log2(N);
	else if (Complexity == BigO::O_N_log_N)
		return N * log2(N);

	return 1; // Default value for O_1
}

// Internal function to find the coefficient for the high-order term in the running time, by minimizing the sum of squares of relative error.
//   - N          : Vector containing the size of the benchmark tests.
//   - Time       : Vector containing the times for the benchmark tests.
//   - Complexity : Fitting curve.
// For a deeper explanation on the algorithm logic, look the README file at http://github.com/ismaelJimenez/Minimal-Cpp-Least-Squared-Fit

LeastSq leastSq(const std::vector<int>& N, const std::vector<int>& Time, const BigO Complexity) {
	assert(N.size() == Time.size() && N.size() >= 2);
	assert(Complexity != BigO::O_None &&
		Complexity != BigO::O_Auto);

	double sigmaGN = 0;
	double sigmaGNSquared = 0;
	double sigmaTime = 0;
	double sigmaTimeGN = 0;

	// Calculate least square fitting parameter
	for (size_t i = 0; i < N.size(); ++i) {
		double GNi = fittingCurve(N[i], Complexity);
		sigmaGN += GNi;
		sigmaGNSquared += GNi * GNi;
		sigmaTime += Time[i];
		sigmaTimeGN += Time[i] * GNi;
	}

	LeastSq result;
	result.complexity = Complexity;

	// Calculate complexity. 
	// O_1 is treated as an special case
	if (Complexity != BigO::O_1)
		result.coef = sigmaTimeGN / sigmaGNSquared;
	else
		result.coef = sigmaTime / N.size();

	// Calculate RMS
	double rms = 0;
	for (size_t i = 0; i < N.size(); ++i) {
		double fit = result.coef * fittingCurve(N[i], Complexity);
		rms += pow((Time[i] - fit), 2);
	}

	double mean = sigmaTime / N.size();

	result.rms = sqrt(rms) / mean; // Normalized RMS

	return result;
}

// Find the coefficient for the high-order term in the running time, by minimizing the sum of squares of relative error.
//   - N          : Vector containing the size of the benchmark tests.
//   - Time       : Vector containing the times for the benchmark tests.
//   - Complexity : If different than O_Auto, the fitting curve will stick to this one. If it is O_Auto, it will be calculated 
//                  the best fitting curve.

LeastSq minimalLeastSq(const std::vector<int>& N, const std::vector<int>& Time, const BigO Complexity) {
	assert(N.size() == Time.size() && N.size() >= 2); // Do not compute fitting curve is less than two benchmark runs are given
	assert(Complexity != BigO::O_None);  // Check that complexity is a valid parameter. 

	if(Complexity == BigO::O_Auto) {
		std::vector<BigO> fitCurves = { BigO::O_log_N, BigO::O_N, BigO::O_N_log_N, BigO::O_N_Squared, BigO::O_N_Cubed };

		LeastSq best_fit = leastSq(N, Time, BigO::O_1); // Take O_1 as default best fitting curve

		// Compute all possible fitting curves and stick to the best one
		for (const auto& fit : fitCurves) {
			LeastSq current_fit = leastSq(N, Time, fit);
			if (current_fit.rms < best_fit.rms)
				best_fit = current_fit;
		}

		return best_fit;
	}
	else
		return leastSq(N, Time, Complexity);
}