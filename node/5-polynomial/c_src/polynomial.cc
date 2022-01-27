#include <cmath>
#include "polynomial.h"

Polynomial::Polynomial(double a, double b, double c) : a_(a), b_(b), c_(c) {}

double Polynomial::at(double x) {
    return x * x * a_ + x * b_ + c_;
}

std::vector<double> Polynomial::roots() {
    std::vector<double> roots;
    
    double desc = b_ * b_ - (4 * a_ * c_);
    if (desc >= 0) {
        double r = (-b_ + sqrt(desc)) / (2 * a_);
        roots.push_back(r);
        
        if (desc > 0) {
            r = (-b_ - sqrt(desc)) / (2 * a_);
            roots.push_back(r);
        }
    }

    return roots;
}
