#include <algorithm>
#include <cmath>
#include "rainfall.h"

bool operator<(const Sample &s1, const Sample &s2) {
	return s1.rainfall < s2.rainfall;
}

double avg_rainfall(Location & loc) {
    double total = 0;
    for (const auto &sample : loc.samples) {
        total += sample.rainfall;
    }
    return total / loc.samples.size();
}

RainResult calc_rain_stats(Location &loc) {
	RainResult result;
	double ss = 0;
	double total = 0;
	
	result.n = loc.samples.size();

	for (const auto &sample : loc.samples) {
        total += sample.rainfall;
  	}
  	result.mean = total / loc.samples.size();
  	
  	for (const auto &sample : loc.samples) {
        ss += pow(sample.rainfall - result.mean, 2);
  	}
  	result.stdev = sqrt(ss/(result.n-1));

  	std::sort(loc.samples.begin(), loc.samples.end());
	if (result.n % 2 == 0) {
		result.median = (loc.samples[result.n / 2 - 1].rainfall + loc.samples[result.n / 2].rainfall) / 2;
	} else {
		result.median = loc.samples[result.n / 2].rainfall;
	}
	return result;
}
