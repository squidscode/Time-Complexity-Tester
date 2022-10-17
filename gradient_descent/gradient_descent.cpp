#include "gradient_descent.h"
#include <vector>
#include <cstdarg>
#include <functional>
#include <cmath>

using namespace std;

// a simple macro to quickly write scope
#define gd(return_type) template<class T> return_type gradient_descent<T>
#define DEFAULT_ITERATIONS 5
#define DEFAULT_DELTA 0.00001

// ----------- PRIVATE -----------
gd(void)::init(function<T(T[])> func, int func_argument_count, int iterations){
    assert(func_argument_count > 0);
    assert(iterations >= 0);
    this->func = func;
    this->num_args = func_argument_count;
    this->iterations = iterations;
    this->delta = DEFAULT_DELTA;
    this->verbose = false;
    this->bmin = true;
    this->gradient = vector<T>(num_args);
    this->previous_gradient = vector<T>(num_args);
    this->guess = vector<T>(num_args);
    this->previous_guess = vector<T>(num_args);

    for(int i = 0; i < num_args; ++i){
        guess[i] = 0; // we initialize each element in initial_guess to 0.
        gradient[i] = 0;
    }
}

gd(void)::solve_gradient(){
    T* args = new T[num_args];
    
    // initializing the arguments.
    for(int i = 0; i < num_args; ++i){
        args[i] = guess[i];
        if(verbose){
            printf("previous_guess[%d] = %Lf, ", i, previous_guess[i]);
            printf("guess[%d] = %Lf, ", i, guess[i]);
        }
    }

    // solving for f(x) and f(x + dx).
    T fx = func(args);
    if(verbose)
        printf("fx = %Lf,", fx);
    args[0] += delta;
    T fxdx = func(args);
    if(verbose)
        printf("fxdx = %Lf, ", fxdx);

    // solving for the first partial derivative
    previous_gradient[0] = gradient[0];
    gradient[0] = (fxdx - fx) / delta;
    if(verbose){
        printf("previous_gradient[0] = %Lf, ", previous_gradient[0]);
        printf("gradient[0] = %Lf, ", gradient[0]);
    }

    // solving for the rest of the partials.
    for(int i = 1; i < num_args; ++i){
        args[i - 1] -= delta;
        args[i] += delta;
        fxdx = func(args);
        
        previous_gradient[i] = gradient[i];
        gradient[i] = (fxdx - fx) / delta;
        if(verbose){
            printf("previous_gradient[%d] = %Lf, ", i, previous_gradient[i]);
            printf("gradient[%d] = %Lf\n", i, gradient[i]);
        }
    }

    delete[] args;
}

// we solve for the learning rate using the Barzilai-Borwein method
gd(T)::solve_learning_rate(){
    T tot1 = 0;
    T tot2 = 0;
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

    if(tot2 == 0){
        return 0.00001;
    }

    tot1 = pow(tot1, 0.5);

    return tot1 / tot2;
}

// ----------- PUBLIC -----------
gd()::gradient_descent(function<T(T[])> func, int func_argument_count){
    init(func, func_argument_count, DEFAULT_ITERATIONS);
}

gd()::gradient_descent(function<T(T[])> func, int func_argument_count, int iterations){
    init(func, func_argument_count, iterations);
}

gd(void)::set_iterations(int iterations){
    this->iterations = iterations;
}

gd(void)::set_guess(T* guess){
    for(int i = 0; i < num_args; ++i){
        this->guess[i] = guess[i];
    }
}

gd(void)::set_delta(T delta){
    this->delta = delta;
}

gd(void)::set_verbose(bool v){
    this->verbose = v;
}

gd(void)::set_min_or_max(bool min){
    this->min = min;
}

gd(vector<T>)::get_guess(){
    vector<T> g(num_args);
    
    for(int i = 0; i < num_args; ++i){
        g[i] = guess[i];
    }

    return g;
}

gd(vector<T>)::run(){
    vector<T> constants(guess);
    T learning_rate = 0.1;

    for(int i = 0; i < iterations; ++i){
        solve_gradient();
        for(int j = 0; j < num_args; ++j){
            if(i != 0)
                learning_rate = solve_learning_rate();
            if(verbose)
                printf("learning_rate = %Lf\n", learning_rate);
            
            previous_guess[j] = guess[j];
            guess[j] += (bmin ? -1 : 1) * gradient[j]*learning_rate;
        }
    }

    return get_guess();
}

function<long double(long double*)> MSE(int length_of_data, const long double* x[], const long double y[], 
    function<long double(const long double*, long double*)> func){
    return [length_of_data, x, y, func](long double args[]) -> long double {
        long double sum_of_error_squared = 0;

        for(int i = 0; i < length_of_data; ++i){
            sum_of_error_squared += pow(y[i] - func(x[i], args), 2);
        }

        return sum_of_error_squared;
    };
}

#undef gd