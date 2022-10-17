#ifndef TIME_COMPLEXITY
#define TIME_COMPLEXITY

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <functional>
#include <sstream>
#include <functional>
#include <sys/time.h>
#include <math.h>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <tuple>

using namespace std;

typedef struct functiontype{
    string name;
    function<long double(int, int, int)> function_base;
} ft_t;

typedef struct duration_data{
    int n;
    int duration;
} dd_t;

typedef struct ratio_data{
    int n;
    long double ratio;
} rd_t;

vector<ft_t> default_functions();

class time_complexity{
private:
    int total_budget;
    int computation_budget;
    bool verbose;
    long long total_time;
    vector<ft_t> fs;
    vector<dd_t> dds;
    vector<vector<long double>> ratios;
    vector<vector<long double>> dratios;
    vector<long double> medians;
    vector<double> means;
    int fd[2];
    void init();
    int run_func_with_budget(function<void(int)> func, int n, int budget);
    void complexity_table_generator(function<void(int)> func, int st, int end, int jmp);
    static long double sigmoid(const long double* x, long double* args);
    tuple<int, int, int> find_interval(function<void(int)> func);

public:
    time_complexity(int millisecond_total_budget, int millisecond_computation_budget, bool verbose = false, vector<ft_t> fs=default_functions());
    void compute_complexity(string name, function<void(int)> func, string expected_complexity="");
};

#endif