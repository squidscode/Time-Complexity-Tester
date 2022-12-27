#include <functional>

auto repeat = [](function<void(int)> f, int n, int times) -> void {
    for(int i = 0; i < times; ++i){
        f(n);
    }
};

int s(int n){
    long long sum = 0;
    for(int i = 0; i < n; ++i){
        sum += i;
    }
    return sum;
}

// ~TC-TEST~ Test #1 O(n)
void fn(int n){
    repeat(s, n, 10000);
}

// ~TC-TEST~ Test #2 O(1)
void fn2(int n){
    repeat(s, 1000, 10000);
}

// ~TC-TEST~ Test #3 O(n^2)
void fn3(int n){
    for(int i = 0; i < n; ++i){
        repeat(s, n, 10000);
    }
}