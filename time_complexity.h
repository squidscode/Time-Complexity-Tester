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
} function_type_t;

typedef struct duration_data{
    int n;
    long long duration;
} dd_t;

typedef struct ratio_data{
    int n;
    long double ratio;
} rd_t;

typedef struct convergence_data{
    string name;
    long double a;
    long double error;
} convergence_data_t;

// f(x) = 2(a - 1)(1 / [1+e^(-(x-c)/(d x sigmoid(b)))] - 0.5) + 1
const string FUNCTION_STR = "2(a - 1)(1 / [1+e^(-(x-c)/(d x sigmoid(b)))] - 0.5) + 1";
typedef struct guess_collection {
    long double a;
    long double b;
    int c;  
    long double d;
    long double error;
} guess_collection_t;

vector<function_type_t> default_functions();

class time_complexity{
private:
    long long total_budget;
    long long computation_budget;
    long long total_time;
    long long preprocessing_time;
    vector<function_type_t> fs;
    vector<dd_t> dds;
    vector<vector<long double>> ratios;
    vector<vector<long double>> dratios;
    vector<long double> medians;
    vector<double> means;
    vector<convergence_data_t> stats;
    string current_test_name;
    int fd[2];
    void init();
    int run_func_with_budget(function<void(int)> func, int n, int budget);
    void complexity_table_generator(function<void(int)> func, int st, int end, int jmp);
    static long double convergence_function(const long double* x, long double* args, int c, long double max_b);
    static long double sigmoid(long double x);
    tuple<int, int, int> find_interval(function<void(int)> func);
    void save_to_file(vector<rd_t> vals[], vector<guess_collection_t> guesses);

public:
    // Where we store the table log information:
    string data_directory{"./data"};
    // Automatically save data to the data directory:
    bool save_data{true};
   // Auto-Interval capabilities:
    bool auto_interval;
    // Different levels of verbose-ness:
    bool verbose;
    bool show_gradient;
    bool show_possible_big_o;
    // default maximum error to indicate convergence.
    long double convergence_error = 0.01; 
    // if a ratio converges to a value below this, we will assume it converges to 0.
    long double zero = 0.3; 
    time_complexity(int millisecond_total_budget, int millisecond_computation_budget=1, vector<function_type_t> fs=default_functions());
    bool compute_complexity(string name, function<void(int)> func, string expected_complexity="");
};

#endif