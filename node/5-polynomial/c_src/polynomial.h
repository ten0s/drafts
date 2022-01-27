#ifndef __POLYNOMIAL_H__
#define __POLYNOMIAL_H__

#include <vector>

class Polynomial {
public:
    Polynomial(double a, double b, double c);

    double at(double x) const;
    std::vector<double> roots() const;

    double getA() const { return a_; }
    double getB() const { return b_; }
    double getC() const { return c_; }
    
    void setA(double a) { a_ = a; }
    void setB(double b) { b_ = b; }
    void setC(double c) { c_ = c; }

private:
    double a_;
    double b_;
    double c_;
};

#endif // __POLYNOMIAL_H__
