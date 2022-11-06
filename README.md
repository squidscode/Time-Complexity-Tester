# Time Complexity Tester
## Purpose
**Time Complexity Tester** is a customizable time complexity testing program that allows the user to determine and assert the complexity of target functions -- defined as functions with exactly **one argument** with type **int**. 

We can use this class to determine the time complexity of many different types of functions by writing a unary function that takes an integer and *transforming* the input into a field in a larger multi-argument function (while holding all other variables constant).

This program is capable of determining the difference between Big Theta, Θ(f(n)), and Big O, O(f(n)), and the user can write tests for either measure of time complexity.

## Example #1
Let's try to determine the time complexity of the following function:
```
int fibonacci(int n){
  if(n == 0) return 0;
  if(n == 1) return 1;
  return fibonacci(n - 1) + fibonacci(n - 2);
}
```
We would first initialize a time_complexity object, which, for the sake of simplicty, runs for 5 seconds (5000 ms) and begins collecting when the function runs for 10 ms:
```
time_complexity tc(5000, 10);
```
To test the function, ```fibonacci```, we call:
```
tc.compute_complexity("Recursive Fibonacci", fibonacci);
```
The first argument is the name of the test (which will be shown alongside the guess as part of the output).

When we run the code, we get the following output:
```
Possible Big O functions: 
  - O(log n) : (a = 20.38675, error = 0.00000) 
  - O(n log n) : (a = 0.11871, error = 0.00640) 
  - O(n^2) : (a = 0.00728, error = 0.00016) 
  - O(n^3) : (a = -0.00051, error = 0.00000) 
  - O(1.5^n) : (a = 0.00014, error = 0.00000) 
  - O(2^n) : (a = -0.00316, error = 0.00001) 
  - O(n^n) : (a = -0.00131, error = 0.00000) 
[5.014s, n = 3] Recursive Fibonacci                         Guess: Θ(log n)
```
Does our "Recursive Fibonacci" function run in O(log n) time?? Probably not! Our implementation is wildly inefficient! Why, then, has our program guessed the wrong time complexity? 

Take a look at the number of data points our program has collected (indicated as ```n = ...```). We have only ran our fibonacci function **three times**! These results indicate that we need to increase the total amount of time that we run our time complexity tester or we need to decrease the computation budget (when we begin collecting our data points). 

If we choose to increase the total time budget (ie. how long we ran our test), we guarantee a more accurate result. Running the test for 60 seconds gets us:
```
Possible Big O functions: 
  - O(1.5^n) : (a = -0.01169, error = 0.00021) 
  - O(2^n) : (a = -0.01205, error = 0.00022) 
  - O(n^n) : (a = -0.01206, error = 0.00022) 
[60.015s, n = 4] Recursive Fibonacci                        Guess: O(1.5^n)
```
Okay, that's better, but not good enough.

We still haven't increased the number of data points we have collected by a meaningful amount (now we have called the function four times instead of three times). If we also decrease the *computation budget* to 1 ms, we get the following:
```
Possible Big O functions: 
  - O(1.5^n) : (a = -0.01169, error = 0.00021) 
  - O(2^n) : (a = -0.01205, error = 0.00022) 
  - O(n^n) : (a = -0.01206, error = 0.00022) 
[60.005s, n = 4] Recursive Fibonacci                        Guess: O(1.5^n)               
```
The same result!

The problem is NOT our time complexity tester. Our ```fibonacci``` function simply grows **too rapidly** after we reach 1 ms. If, instead of testing our fibonacci function, we construct the following lambda and call the following test:
```
auto repeat = [](function<void(int)> fn, int a) -> function<void(int)> {
  return [a, &fn](int n) -> void {
    for(int i = 0; i < a; ++i) fn(n);
  };
};

...

time_complexity tc(5000, 10);
tc.compute_complexity("Recursive Fibonacci", repeat(fibonacci, 1000000));
```
We can repeat ```fibonacci``` by calling ```repeat(fibonacci, 1000000)```. This will amplify the part of ```fibonacci``` that is slowed down by ```n```. By passing this in as the input for our time complexity tester, we get:
```
 Possible Big O functions: 
  - O(n^2) : (a = 0.13420, error = 0.00410) 
  - O(n^3) : (a = 0.01678, error = 0.00006) 
  - O(1.5^n) : (a = 0.34504, error = 0.00053) 
  - O(2^n) : (a = 0.03225, error = 0.00029) 
  - O(n^n) : (a = -0.00112, error = 0.00001) 
[5.002s, n = 13] Recursive Fibonacci                        Guess: Θ(1.5^n)
```
Pretty good! For reference, the recursive fibonacci algorithm runs in **O(1.6180^n)** time. Now, if we increase the amount of time we run time complexity tester, we can prune O(n^2) from our possible Big O functions list by running for 60 seconds:
```
 Possible Big O functions: 
  - O(n^3) : (a = 0.02888, error = 0.00049) 
  - O(1.5^n) : (a = 0.38891, error = 0.00516) 
  - O(2^n) : (a = 0.02290, error = 0.00053) 
  - O(n^n) : (a = -0.00035, error = 0.00000) 
[60.002s, n = 18] Recursive Fibonacci                       Guess: Θ(1.5^n)
```
But, removing O(n^3) from the possible Big O functions is very difficult because **n^3** grows faster than **n^1.6180** while **n < 15**, and **n^1.6180** starts to grow faster than **n^3** at **n = 18.03**. In other words, **n^3** and **1.618^n** look awfully alike. After running the tester for 10 minutes, our time complexity test returns the mathematically correct answer:
```
 Possible Big O functions: 
  - O(2^n) : (a = 0.01552, error = 0.00070) 
  - O(n^n) : (a = 0.00008, error = 0.00000) 
[600.003s, n = 23] Recursive Fibonacci                      Guess: O(2^n)
```
*Notice that our result prints O(2^n) rather than Θ(2^n). Our program has identified that our recursive solution is **faster** than O(2^n)!*

## Example #2
Let's do it again!
```
int fibonacci2(int n){
  int fib[n + 1];
  fib[0] = 0;
  fib[1] = 1;
  for(int i = 2; i <= n; ++i){
      fib[i] = fib[i - 1] + fib[i - 2];
  }
  return fib[n];
}
```
When we run:
```
time_complexity tc(5000, 10);
tc.compute_complexity("DP Fibonacci", fibonacci2);
```
we get:
```
 Possible Big O functions: 
  - O(1) : (a = 0.99999, error = 0.00000) 
  - O(log n) : (a = 0.09467, error = 0.00048) 
  - O(sqrt(n)) : (a = 0.02562, error = 0.00054) 
  - O(n) : (a = 0.00122, error = 0.00004) 
  - O(n log n) : (a = 0.00026, error = 0.00001) 
  - O(n^2) : (a = 0.00003, error = 0.00000) 
  - O(n^3) : (a = -0.00001, error = 0.00000) 
  - O(1.5^n) : (a = -0.00000, error = 0.00000) 
  - O(2^n) : (a = -0.00001, error = 0.00000) 
  - O(n^n) : (a = -0.00002, error = 0.00000) 
[5.023s, n = 4999] DP Fibonacci                             Guess: Θ(1)
```
*Interesting!*

Our algorithm definitely runs in O(n) time, but we weren't able to measure any meaningful difference. If we try the trick above and we run:
```
tc.compute_complexity("DP Fibonacci", repeat(fibonacci2, 1000000));
```
we get:
```
 Possible Big O functions: 
  - O(log n) : (a = 0.55425, error = 0.00831) 
  - O(sqrt(n)) : (a = 0.52125, error = 0.00154) 
  - O(n) : (a = 0.10094, error = 0.00115) 
  - O(n log n) : (a = 0.03161, error = 0.00045) 
  - O(n^2) : (a = 0.00802, error = 0.00021) 
  - O(n^3) : (a = 0.00134, error = 0.00002) 
  - O(1.5^n) : (a = 0.00243, error = 0.00011) 
  - O(2^n) : (a = 0.00138, error = 0.00007) 
  - O(n^n) : (a = -0.00013, error = 0.00000) 
[5.012s, n = 57] DP Fibonacci                               Guess: Θ(sqrt(n))
```
Well, our guess changed, and it's closer to the right answer. We could probably increase the accuracy by increasing the time budget. But, lets try to do something else...
```
auto square = [](function<void(int)> fn) -> function<void(int)> {
  return [&fn](int n) -> void {
    for(int i = 0; i < n; ++i) fn(n);
  };
};
```
The function above will square the time complexity of the given function. Now, instead of comparing O(sqrt(n)) and O(n) we are comparing O(n) and O(n^2)! Observing the difference between O(n) and O(n^2) is a lot easier than observing the difference between O(sqrt(n)) and O(n). If we run:
```
tc.compute_complexity("DP Fibonacci", square(repeat(fibonacci2, 1000000)));
```
we get:
```
 Possible Big O functions: 
  - O(n^2) : (a = 0.15353, error = 0.00094) 
  - O(n^3) : (a = 0.02521, error = 0.00040) 
  - O(2^n) : (a = -0.02615, error = 0.00914) 
  - O(n^n) : (a = -0.00740, error = 0.00075) 
[5.011s, n = 14] DP Fibonacci                               Guess: O(n^2)
```
This suggests that our time complexity is O(n), NOT O(sqrt(n)), because this test would return O(n) if our function was O(sqrt(n)).

*Note that this trick only works if you want to map a function to a faster growing function. We cannot reduce a function that runs in O(n) to one that runs in O(sqrt(n))! This trick can help us separate logarithmic functions from polynomial functions (see below).*

## Example 3
Lets go **all the way**.
```
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
```
The code above uses the fast doubling formula for the fibonacci series to calculate the nth fibonacci number. When we run:
```
tc.compute_complexity("Fast Doubling Fibonacci", fibonacci3);
```
we get:
```
Possible Big O functions: 
  - O(1) : (a = 0.99999, error = 0.00000) 
  - O(log n) : (a = 0.09467, error = 0.00048) 
  - O(sqrt(n)) : (a = 0.02562, error = 0.00054) 
  - O(n) : (a = 0.00122, error = 0.00004) 
  - O(n log n) : (a = 0.00026, error = 0.00001) 
  - O(n^2) : (a = 0.00003, error = 0.00000) 
  - O(n^3) : (a = -0.00001, error = 0.00000) 
  - O(1.5^n) : (a = -0.00000, error = 0.00000) 
  - O(2^n) : (a = -0.00001, error = 0.00000) 
  - O(n^n) : (a = -0.00002, error = 0.00000) 
[5.014s, n = 4999] Fast Doubling Fibonacci                  Guess: Θ(1)
```
This seems familiar. Lets repeat our function a million times.
```
tc.compute_complexity("Fast Doubling Fibonacci", repeat(fibonacci3, 1000000));
```
...results in:
```
Possible Big O functions: 
  - O(1) : (a = 1.40598, error = 0.00239) 
  - O(log n) : (a = 0.25844, error = 0.00148) 
  - O(sqrt(n)) : (a = 0.20374, error = 0.00265) 
  - O(n) : (a = 0.03839, error = 0.00119) 
  - O(n log n) : (a = 0.02670, error = 0.00619) 
  - O(n^2) : (a = 0.00343, error = 0.00010) 
  - O(n^3) : (a = 0.00065, error = 0.00001) 
  - O(1.5^n) : (a = 0.02141, error = 0.00747) 
  - O(2^n) : (a = 0.01052, error = 0.00316) 
  - O(n^n) : (a = -0.00004, error = 0.00000) 
[5.011s, n = 92] Fast Doubling Fibonacci                    Guess: Θ(1)
```
*Weird!*
We can try any of the methods listed above, but, sometimes, we just need to increase our time budget to realize a difference between O(1) and O(log n)
```
Possible Big O functions: 
  - O(log n) : (a = 0.22389, error = 0.00071) 
  - O(sqrt(n)) : (a = 0.15233, error = 0.00435) 
  - O(n) : (a = 0.01915, error = 0.00173) 
  - O(n log n) : (a = 0.00769, error = 0.00196) 
  - O(n^2) : (a = 0.00075, error = 0.00003) 
  - O(n^3) : (a = 0.00011, error = 0.00000) 
  - O(1.5^n) : (a = 0.00017, error = 0.00001) 
  - O(2^n) : (a = 0.00008, error = 0.00000) 
  - O(n^n) : (a = -0.00005, error = 0.00000) 
[30.011s, n = 454] Fast Doubling Fibonacci                  Guess: O(log n)
```
*Note that O(sqrt(n)) and O(log n) grow similarly according to the algorithm. Be careful when using composition functions in order to determine the difference between the two. **The best way to increase the accuracy is to use the repeat composition and by increasing the time budget***.

## Documentation
The time complexity class is constructed via: 
1. ```time_complexity(TIME-BUDGET, COMPUTATION-BUDGET)```
2. ```time_complexity(TIME-BUDGET, COMPUTATION-BUDGET, VECTOR-OF-FUNCTION-TYPES)```. 

The first constructor takes in the ```TIME-BUDGET```, which is the **maximum amount of milliseconds allocated to a single call to compute_complexity**. Note that the accuracy for the time complexity tester is roughly correlated to the number of data points the tester collects, and the ```TIME-BUDGET``` is correlated to the number of data points collected (because the longer the test runs, the more calls to the unary function we can make without timing out). Therefore, **if you want to increase the accuracy of the time complexity tester, increase the amount of time you run the time_complexity tester**.

The constructor also takes in the ```COMPUTATION-BUDGET```. This value needs to be fine-tuned to the types of functions passed into the time complexity tester. The value describes the **number of milliseconds that the unary functions need to run for before we begin collecting data**. As a general rule of thumb, this value can be set to 100, and the time complexity tester will accurately determine the growth rate of the function; however, if the unary function's scale for n (ie. how much the value of f(n) matters for the runtime of the program) is **very low**, you might need to increase this value OR you might need modify the function we feed into the program (ie. feed a unary function that calls the target function 1,000 times). 

If you wish to determine whether a function grows at a non-traditional growth rate, one that is not:
1. Constant O(1)
2. Logarithmic O(log n)
3. Square root O(sqrt(n))
4. Linear O(n)
5. Linear times Logarithmic O(n log n)
6. Quadratic O(n^2)
7. Cubic O(n^3)
8. Exponential (base 1.5) O(1.5^n)
9. Exponential (base 2) O(2^n)
10. Self Exponentiation O(n^n)
Then you need to call the second constructor and pass in a vector of **ordered** ```function_type_t``` into the ```VECTOR-OF-FUNCTION-TYPES``` field. The order of the functions matter because, when we cannot find a function that exactly fits the target function (ie. we cannot find **big-theta** of the target function), then we will guess the first function type that grows faster than our target function for our **big-O** estimate.

The time complexity function can be called in two different ways:
1. ```tc.compute_complexity(NAME, UNARY-FUNCTION)```
2. ```tc.compute_complexity(NAME, UNARY-FUNCTION, PREDICTED-TIME-COMPLEXITY)```

After creating an instance of the time_complexity class, we can begin testing the time complexity of target functions

## Sample Code and Output
```
Possible Big O functions: 
  - O(log n) : (a = 10.25229, error = 2.37424) 
  - O(sqrt(n)) : (a = 0.69903, error = 0.05438) 
  - O(n) : (a = 0.00075, error = 0.00000) 
  - O(n log n) : (a = 0.00005, error = 0.00000) 
  - O(n^2) : (a = -0.00001, error = 0.00000) 
  - O(n^3) : (a = -0.00001, error = 0.00000) 
  - O(1.5^n) : (a = -0.00001, error = 0.00000) 
  - O(2^n) : (a = -0.00001, error = 0.00000) 
  - O(n^n) : (a = -0.00001, error = 0.00000) 
[10.275s, n = 14] heap.push_back(decreasing)                Guess: Θ(sqrt(n))            
Possible Big O functions: 
  - O(1) : (a = 0.99999, error = 0.00000) 
  - O(log n) : (a = 0.08559, error = 0.00027) 
  - O(sqrt(n)) : (a = 0.01896, error = 0.00028) 
  - O(n) : (a = 0.00065, error = 0.00003) 
  - O(n log n) : (a = 0.00013, error = 0.00000) 
  - O(n^2) : (a = 0.00001, error = 0.00000) 
  - O(n^3) : (a = -0.00001, error = 0.00000) 
  - O(1.5^n) : (a = -0.00001, error = 0.00000) 
  - O(2^n) : (a = -0.00001, error = 0.00000) 
  - O(n^n) : (a = -0.00002, error = 0.00000) 
[10.307s, n = 9999] heap.push_back(increasing)              Guess: Θ(1)                  
Possible Big O functions: 
  - O(1) : (a = 1.39390, error = 0.00056) 
  - O(log n) : (a = 0.17961, error = 0.00162) 
  - O(sqrt(n)) : (a = 0.19463, error = 0.00157) 
  - O(n) : (a = 0.03863, error = 0.00090) 
  - O(n log n) : (a = 0.01193, error = 0.00138) 
  - O(n^2) : (a = 0.00505, error = 0.00014) 
  - O(n^3) : (a = 0.00346, error = 0.00042) 
  - O(1.5^n) : (a = -0.00106, error = 0.00043) 
  - O(2^n) : (a = -0.00030, error = 0.00008) 
  - O(n^n) : (a = 0.00499, error = 0.00155) 
[10.101s, n = 86] Constant # of heap.push_back              Guess: Θ(1)                  OK                  
```