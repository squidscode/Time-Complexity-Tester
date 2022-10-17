#ifndef GRADIENT_DESCENT
#define GRADIENT_DESCENT

#include <vector>
#include <cstdarg>
#include <functional>

using namespace std;

function<long double(long double*)> MSE(int length_of_data, const long double* x[], const long double y[], 
    function<long double(const long double*, long double*)> func);

template<class T=long double> class gradient_descent{
private:
    function<T(T[])> func;
    int num_args;
    int iterations;
    T delta;
    bool verbose;
    bool bmin;
    vector<T> gradient;
    vector<T> previous_gradient;
    vector<T> guess;
    vector<T> previous_guess;
    void init(function<T(T[])> func, int func_argument_count, int iterations);
    void solve_gradient();
    T solve_learning_rate();

public:
    gradient_descent(function<T(T[])> func, int func_argument_count);
    gradient_descent(function<T(T[])> func, int func_argument_count, int iterations);
    void set_iterations(int iterations);
    void set_guess(T* guess);
    void set_delta(T delta);
    void set_verbose(bool v);
    void set_min_or_max(bool min);
    vector<T> get_guess();
    vector<T> run();
};

#endif