#ifndef GRADIENT_DESCENT
#define GRADIENT_DESCENT

#include <vector>
#include <cstdarg>
#include <functional>

using namespace std;

function<long double(long double*)> MSE(int length_of_data, const long double* x[], const long double y[], 
    function<long double(const long double*, long double*)> func);

class gradient_descent{
private:
    function<long double(long double[])> func;
    int num_args;
    int iterations;
    long double delta;
    bool verbose;
    bool min;
    vector<long double> gradient;
    vector<long double> previous_gradient;
    vector<long double> guess;
    vector<long double> previous_guess;
    void init(function<long double(long double[])> func, int func_argument_count, int iterations);
    void solve_gradient();
    long double solve_learning_rate();

public:
    gradient_descent(function<long double(long double[])> func, int func_argument_count);
    gradient_descent(function<long double(long double[])> func, int func_argument_count, int iterations);
    void set_iterations(int iterations);
    void set_guess(long double* guess);
    void set_delta(long double delta);
    void set_verbose(bool v);
    void set_min_or_max(bool min);
    vector<long double> get_guess();
    vector<long double> run();
};

#endif