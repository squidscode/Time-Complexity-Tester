#include <math.h>
#include <chrono>
#include <thread>
#include "../time_complexity.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::microseconds;

#define sl(x) std::this_thread::sleep_for(std::chrono::milliseconds((int) (x)));

function<void(int)> scaled_constant(double scale);
function<void(int)> scaled_logarithmic(double scale);
function<void(int)> scaled_sqrt(double scale);
function<void(int)> scaled_linear(double scale);
function<void(int)> scaled_linearxlog(double scale);
function<void(int)> scaled_quadratic(double scale);
function<void(int)> scaled_cubic(double scale);
function<void(int)> scaled_exponential(double scale, double pow);
function<void(int)> scaled_super_exponential(double scale);

void test_constant(int n);
void test_logarithmic(int n);
void test_sqrt(int n);
void test_linear(int n);
void test_linearxlog(int n);
void test_quadratic(int n);
void test_cubic(int n);
void test_exponentialx2(int n);
void test_super_exponential(int n);
void test_exponential(double pow, int n);

int main(void){
    time_complexity tc(5000, 100, false);
    unsigned int s;
    srand(s);
    for(int i = 0; i < 20; ++i){
        double scale = rand() % 200 + 1;
        scale /= (rand() % 400 + 200);
        // double scale = 1;
        cout << "Scale: " << scale << "\n";
        tc.compute_complexity("Constant", scaled_constant(scale), "O(1)");
        tc.compute_complexity("Log", scaled_logarithmic(scale), "O(log n)");
        tc.compute_complexity("Sqrt", scaled_sqrt(scale), "O(sqrt(n))");
        tc.compute_complexity("Linear", scaled_linear(scale), "O(n)");
        tc.compute_complexity("Linearxlog", scaled_linearxlog(scale), "O(n log n)");
        tc.compute_complexity("Quadratic", scaled_quadratic(scale), "O(n^2)");
        tc.compute_complexity("Cubic", scaled_cubic(scale), "O(n^3)");
        tc.compute_complexity("Exp", scaled_exponential(scale, 1.51), "O(1.5^n)");
        tc.compute_complexity("Exp", scaled_exponential(scale, 2.01), "O(2^n)");
        tc.compute_complexity("S_Exp", scaled_super_exponential(scale), "O(n^n)");
        cout << "\n";
    }

    // time_complexity tc(3000, 20, false);
    // tc.compute_complexity("Constant Function", test_constant, "O(1)");
    // tc.compute_complexity("Logarithmic Function", test_logarithmic, "O(log n)");
    // tc.compute_complexity("SQRT Function", test_sqrt, "O(sqrt(n))");
    // tc.compute_complexity("Linear Function", test_linear, "O(n)");
    // tc.compute_complexity("Linear x Logarithmic Function", test_linearxlog, "O(n log n)");
    // tc.compute_complexity("Quadratic", test_quadratic, "O(n^2)");
    // tc.compute_complexity("Cubic", test_cubic, "O(n^3)");
    // tc.compute_complexity("1.5^n Function", [](int n) -> void {return test_exponential(1.5, n);}, "O(1.5^n)");
    // tc.compute_complexity("1.7^n Function", [](int n) -> void {return test_exponential(1.7, n);}, "O(1.5^n)");
    // tc.compute_complexity("1.9^n Function", [](int n) -> void {return test_exponential(1.9, n);}, "O(1.5^n)");
    // tc.compute_complexity("2^n Function", [](int n) -> void {return test_exponential(2.001, n);}, "O(2^n)");
    // tc.compute_complexity("2^n Function", test_exponentialx2, "O(2^n)");
    // tc.compute_complexity("3^n Function", [](int n) -> void {return test_exponential(3, n);}, "O(2^n)");
    // tc.compute_complexity("n^n Function", test_super_exponential, "O(n^n)");
}


function<void(int)> scaled_constant(double scale){
    return [scale](int n) -> void{sl(10);};
}

function<void(int)> scaled_logarithmic(double scale){
    return [scale](int n) -> void{sl(scale * log(n));};
}

function<void(int)> scaled_sqrt(double scale){
    return [scale](int n) -> void{sl(scale * sqrt(n));};
}

function<void(int)> scaled_linear(double scale){
    return [scale](int n) -> void{sl(scale * n);};
}

function<void(int)> scaled_linearxlog(double scale){
    return [scale](int n) -> void{sl(scale * n * log(n));};
}

function<void(int)> scaled_quadratic(double scale){
    return [scale](int n) -> void{sl(scale * pow(n, 2));};
}

function<void(int)> scaled_cubic(double scale){
    return [scale](int n) -> void{sl(scale * pow(n, 3));};
}

function<void(int)> scaled_exponential(double scale, double power){
    return [scale, power](int n) -> void{sl(scale * pow(power, n));};
}

function<void(int)> scaled_super_exponential(double scale){
    return [scale](int n) -> void{sl(scale * pow(n, n));};
}





void test_constant(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void test_logarithmic(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(10 * log(n))));
}

void test_sqrt(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(10 * sqrt(n))));
}

void test_linear(int n){
    std::this_thread::sleep_for(std::chrono::microseconds(1000 * n));
}

void test_linearxlog(int n){
    std::this_thread::sleep_for(std::chrono::microseconds((int)(100 * n * log(n))));
}

void test_quadratic(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * n * n));
}

void test_cubic(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds(n * n * n));
}

void test_exponentialx2(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(pow(2, n))));
}

void test_exponential(double power, int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(pow(power, n))));
}


void test_super_exponential(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(pow(n, n))));
}

