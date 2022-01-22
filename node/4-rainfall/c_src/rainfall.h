#ifndef __RAINFALL_H__
#define __RAINFALL_H__

#include <string>
#include <vector>

using namespace std;

class Sample {
public:
    Sample() {
        date = "";
        rainfall = 0;
    }

    Sample(string d, double r) {
        date = d;
        rainfall = r;
    }

    string date;
    double rainfall;
};

bool operator<(const Sample &s1, const Sample &s2);

class Location {
public:
    double longitude;
    double latitude;
    vector<Sample> samples;
};

class RainResult {
public:
    float median;
    float mean;
    float stdev;
    int n;
};

double avg_rainfall(Location & loc);
RainResult calc_rain_stats(Location &loc);

#endif
