#include "../../time_complexity.h"
#include <functional>
#include <cassert>

void test_correctness(function<int(int)> fib);
int fibonacci(int n); // Example #1
int fibonacci2(int n); // Example #2
int fibonacci3(int n); // Example #3

// --------------------------- TESTING FUNCTIONS ---------------------------
// repeats fn a times.
auto repeat = [](function<void(int)> fn, int a) -> function<void(int)> {
    return [a, &fn](int n) -> void {
        for(int i = 0; i < a; ++i) fn(n);
    };
};

// squares the time complexity of fn.
auto square = [](function<void(int)> fn) -> function<void(int)> {
    return [&fn](int n) -> void {
        for(int i = 0; i < n; ++i) fn(n);
    };
};

auto pow2 = [](function<void(int)> fn) -> function<void(int)> {
    return [&fn](int n) -> void {
        for(int i = 0; i < pow(2, n); ++i) fn(n);
    };
};


int main(void){
    test_correctness(fibonacci);
    test_correctness(fibonacci2);
    test_correctness(fibonacci3);

    time_complexity tc(5000, 1);
    // tc.verbose = true; // uncomment to show verbose output

    // EXAMPLE #1
    // tc.compute_complexity("Recursive Fibonacci", fibonacci);
    tc.compute_complexity("Recursive Fibonacci", repeat(fibonacci, 1000000));

    // EXAMPLE #2
    // tc.compute_complexity("DP Fibonacci", fibonacci2);
    tc.compute_complexity("DP Fibonacci", repeat(fibonacci2, 1000000)); 
    // tc.compute_complexity("DP Fibonacci", square(repeat(fibonacci2, 1000000)));
    // tc.compute_complexity("DP Fibonacci", pow2(fibonacci2));

    // EXAMPLE #3
    // tc.compute_complexity("Fast Doubling Fibonacci", fibonacci3);
    tc.compute_complexity("Fast Doubling Fibonacci", repeat(fibonacci3, 1000000));
    
}

void test_correctness(function<int(int)> fib){
    int ans[14] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233};
    for(int i = 0; i < 14; ++i){
        assert(fib(i) == ans[i]);
    }

    assert(fib(29) == 514229);
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

// SOURCE: https://chunminchang.github.io/blog/post/calculating-fibonacci-numbers-by-fast-doubling
int fibonacci3(int n)
{
  // The position of the highest bit of n.
  // So we need to loop `h` times to get the answer.
  // Example: n = (Dec)50 = (Bin)00110010, then h = 6.
  //                               ^ 6th bit from right side
  unsigned int h = 0;
  for (unsigned int i = n ; i ; ++h, i >>= 1);

  int a = 0; // F(0) = 0
  int b = 1; // F(1) = 1
  // There is only one `1` in the bits of `mask`. The `1`'s position is same as
  // the highest bit of n(mask = 2^(h-1) at first), and it will be shifted right
  // iteratively to do `AND` operation with `n` to check `n_j` is odd or even,
  // where n_j is defined below.
  for (unsigned int mask = 1 << (h - 1) ; mask ; mask >>= 1) { // Run h times!
    // Let j = h-i (looping from i = 1 to i = h), n_j = floor(n / 2^j) = n >> j
    // (n_j = n when j = 0), k = floor(n_j / 2), then a = F(k), b = F(k+1) now.
    int c = a * (2 * b - a); // F(2k) = F(k) * [ 2 * F(k+1) – F(k) ]
    int d = a * a + b * b;   // F(2k+1) = F(k)^2 + F(k+1)^2

    if (mask & n) { // n_j is odd: k = (n_j-1)/2 => n_j = 2k + 1
      a = d;        //   F(n_j) = F(2k + 1)
      b = c + d;    //   F(n_j + 1) = F(2k + 2) = F(2k) + F(2k + 1)
    } else {        // n_j is even: k = n_j/2 => n_j = 2k
      a = c;        //   F(n_j) = F(2k)
      b = d;        //   F(n_j + 1) = F(2k + 1)
    }
  }

  return a;
}
