#include "../../time_complexity.h"
#include <functional>

int fibonacci(int n); // Example #1
int fibonacci2(int n); // Example #2
int fibonacci3(int n); // Example #3

int main(void){
    auto repeat = [](function<void(int)> fn, int times) -> function<void(int)> {
        return [times, &fn](int n) -> void {
            for(int i = 0; i < times; ++i) fn(n);
        };
    };

    auto square = [](function<void(int)> fn) -> function<void(int)> {
        return [&fn](int n) -> void {
            for(int i = 0; i < n; ++i) fn(n);
        };
    };

    time_complexity tc(5000, 10);
    tc.verbose = true;
    // tc.compute_complexity("Recursive Fibonacci", fibonacci);
    // tc.compute_complexity("Recursive Fibonacci", repeat(fibonacci, 1000000));
    tc.compute_complexity("DP Fibonacci", fibonacci2);
    tc.compute_complexity("DP Fibonacci", square(repeat(fibonacci2, 1000000)));
    // tc.compute_complexity("Fast Doubling Fibonacci", repeat(fibonacci3, 1000000));
}

int fibonacci(int n){
    if(n == 0) return 0;
    if(n == 1) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int fibonacci2(int n){
    int fib[n + 1];
    fib[0] = 0;
    fib[1] = 1;
    for(int i = 2; i <= n; ++i){
        fib[i] = fib[i - 1] + fib[i - 2];
    }
    return fib[n];
}

int fibonacci3(int n){
    return 0;
}