#include "gradient_descent.h"
#include <vector>
#include <cstdarg>
#include <functional>
#include <cmath>

using namespace std;

// a simple macro to quickly write scope
#define gd(return_type) return_type gradient_descent
#define DEFAULT_ITERATIONS 5
#define DEFAULT_DELTA 0.00001

// ----------- PRIVATE -----------
gd(void)::init(function<long double(long double[])> func, int func_argument_count, int iterations){
    assert(func_argument_count > 0);
    assert(iterations >= 0);
    this->func = func;
    this->num_args = func_argument_count;
    this->iterations = iterations;
    this->delta = DEFAULT_DELTA;
    this->verbose = false;
    this->min = true;
    this->gradient = vector<long double>(num_args);
    this->previous_gradient = vector<long double>(num_args);
    this->guess = vector<long double>(num_args);
    this->previous_guess = vector<long double>(num_args);

    for(int i = 0; i < num_args; ++i){
        guess[i] = 0; // we initialize each element in initial_guess to 0.
        gradient[i] = 0;
    }
}

gd(void)::solve_gradient(){
    long double* args = new long double[num_args];
    
    // initializing the arguments.
    for(int i = 0; i < num_args; ++i){
        args[i] = guess[i];
        if(verbose){
            printf("p_guess[%d] = %Lf, ", i, previous_guess[i]);
            printf("guess[%d] = %Lf, \n", i, guess[i]);
        }
    }

    // solving for f(x) and f(x + dx).
    long double fx = func(args);
    fx = isnan(fx) ? 0 : fx;
    if(verbose)
        printf("fx = %Lf,", fx);
    args[0] += delta;
    long double fxdx = func(args);
    fxdx = isnan(fxdx) ? 0 : fxdx;
    if(verbose)
        printf("fxdx = %Lf, ", fxdx);

    // solving for the first partial derivative
    previous_gradient[0] = gradient[0];
    gradient[0] = (fxdx - fx) / delta;
    gradient[0] = isnan(gradient[0]) ? 0 : gradient[0];
    if(verbose){
        printf("p_gradient[0] = %Lf, ", previous_gradient[0]);
        printf("gradient[0] = %Lf, \n", gradient[0]);
    }

    // solving for the rest of the partials.
    for(int i = 1; i < num_args; ++i){
        args[i - 1] -= delta;
        args[i] += delta;
        fxdx = func(args);
        
        previous_gradient[i] = gradient[i];
        gradient[i] = (fxdx - fx) / delta;
        gradient[i] = isnan(gradient[i]) ? 0 : gradient[i];
        if(verbose){
            printf("p_gradient[%d] = %Lf, ", i, previous_gradient[i]);
            printf("gradient[%d] = %Lf\n", i, gradient[i]);
        }
    }

    delete[] args;
}

// we solve for the learning rate using the Barzilai-Borwein method
gd(long double)::solve_learning_rate(){
    long double tot1 = 0;
    long double tot2 = 0;
    for(int i = 0; i < num_args; ++i){
        if(verbose){
            printf("\n((%Lf - %Lf) * (%Lf - %Lf))^2 = %Lf", guess[i], previous_guess[i],
                gradient[i], previous_gradient[i], pow((guess[i] - previous_guess[i]) * 
                (gradient[i] - previous_gradient[i]), 2));
            printf("\n(%Lf - %Lf)^2 = %Lf\n", gradient[i], previous_gradient[i], 
                pow(gradient[i] - previous_gradient[i], 2));
        }
        tot1 += pow((guess[i] - previous_guess[i]) * (gradient[i] - previous_gradient[i]), 2);
        tot2 += pow(gradient[i] - previous_gradient[i], 2);
    }

    if(tot2 == 0 || isnan(tot1) || isnan(tot2)){
        return 0.0001;
    }

    tot1 = pow(tot1, 0.5);

    return tot1 / tot2;
}

// ----------- PUBLIC -----------
gd()::gradient_descent(function<long double(long double[])> func, int func_argument_count){
    init(func, func_argument_count, DEFAULT_ITERATIONS);
}

gd()::gradient_descent(function<long double(long double[])> func, int func_argument_count, int iterations){
    init(func, func_argument_count, iterations);
}

gd(void)::set_iterations(int iterations){
    this->iterations = iterations;
}

gd(void)::set_guess(long double* guess){
    for(int i = 0; i < num_args; ++i){
        this->guess[i] = guess[i];
    }
}

gd(void)::set_delta(long double delta){
    this->delta = delta;
}

gd(void)::set_verbose(bool v){
    this->verbose = v;
}

gd(void)::set_min_or_max(bool min){
    this->min = min;
}

gd(vector<long double>)::get_guess(){
    vector<long double> g(num_args);
    
    for(int i = 0; i < num_args; ++i){
        g[i] = guess[i];
    }

    return g;
}

gd(vector<long double>)::run(){
    vector<long double> constants(guess);
    long double learning_rate = 0.1;

    for(int i = 0; i < iterations; ++i){
        solve_gradient();
        for(int j = 0; j < num_args; ++j){
            if(i != 0)
                learning_rate = solve_learning_rate();
            if(verbose)
                printf("learning_rate = %Lf\n", learning_rate);
            
            previous_guess[j] = guess[j];
            guess[j] += (min ? -1 : 1) * gradient[j]*learning_rate;
        }
    }

    return get_guess();
}

function<long double(long double*)> MSE(int length_of_data, const long double* x[], const long double y[], 
    function<long double(const long double*, long double*)> func){
    return [length_of_data, x, y, func](long double args[]) -> long double {
        long double sum_of_error_squared = 0;

        int count = 0;
        for(int i = 0; i < length_of_data; ++i){
            if(isnan(pow(y[i] - func(x[i], args), 2))) continue;
            // cout << i << ", " << x[i][0] << " is " << y[i] << ", " << func(x[i], args) << 
            // ", " << pow(y[i] - func(x[i], args), 2) << "\n";
            sum_of_error_squared += pow(y[i] - func(x[i], args), 2);
            count++;
        }

        return sum_of_error_squared / count;
    };
}

#undef gd