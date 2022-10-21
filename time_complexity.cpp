#include "time_complexity.h"
#include "./gradient_descent/gradient_descent.h"
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
#include <cmath>
#include <signal.h>
#include <tuple>
#define get_time duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()

#define MIN_TABLE_VALUES 3
#define GRADIENT_DESCENT_ITERATIONS 100

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::microseconds;

// ------------------------ PRIVATE ------------------------
void time_complexity::init(){
    total_time = 0;
    dds.clear();
    ratios.clear();
    dratios.clear();
    medians.clear();
    means.clear();
    stats.clear();
}


// returns 0 if the function runs within the given time budget,
// 1 if the functions runs more time than the time budget.
int time_complexity::run_func_with_budget(function<void(int)> func, int n, int budget){
    pid_t child_pid;

    if((child_pid = fork()) != 0){  // parent
        // run while the child process is running
        long long start_time = get_time;
        while(waitpid(child_pid, nullptr, WNOHANG) == 0){
            if(get_time - start_time > budget){
                kill(child_pid, SIGKILL);
                return 1;
            }
        }
    }else{                          // child
        func(n);
        exit(0);
    }

    return 0;
}

// semi-open intervals [st, end) 
void time_complexity::complexity_table_generator(function<void(int)> func, int st, int end, int jmp){
    restart:

    init();

    int num_functions = fs.size();

    unsigned long long bf;
    unsigned long long af;
    int duration;
    
    int count = 0;

    ostringstream oss;

    

    for(int i = st; i < end; i += jmp){
        pid_t child_pid;
        bool ignore_duration = false;

        // fork
        child_pid = fork();
        assert(child_pid >= 0);

        if(child_pid == 0){ // child process
            close(fd[0]);

            bf = get_time;
            func(i);
            af = get_time;

            // ie. the duration is 0, we need to increment by 1 otherwise
            // we run for a long time.
            if(af == bf) 
                af++;

            write(fd[1], &bf, sizeof(long long));
            write(fd[1], &af, sizeof(long long));

            exit(0);
        }else{ // parent process
            int status = 1;
            int* status_ptr = &status;
            pid_t end_pid = 0;
            bool run = true;
            
            long long start_time = get_time;

            while(run){
                end_pid = waitpid(child_pid, status_ptr, WNOHANG);
                if(end_pid == -1){
                    exit(1);
                }else if(end_pid != 0){
                    read(fd[0], &bf, sizeof(long long));
                    read(fd[0], &af, sizeof(long long));
                    run = false;
                }

                // kill child
                if(get_time - start_time >= total_budget - total_time){
                    kill(child_pid, SIGKILL);
                    total_time += get_time - start_time;
                    run = false;
                    ignore_duration = true;
                }
            }
        }
        
        if(ignore_duration) break;

        duration = af - bf;
        dds.push_back({i, duration});
        total_time += duration;
        count++;
        if(total_budget < total_time) break;

        // Print test information if verbose is true.
        if(verbose) cout << left << "\n(n:" << setw(5) << i << ", Time:" << setw(7) << (double) duration / 1000 << "s)";
    }

    if(verbose) cout << "\n\nTotal time: " << (double) (total_time + preprocessing_time) / 1000 << "\n\n";

    if(dds.size() < MIN_TABLE_VALUES && jmp == 1){ // we cannot reformat to allow for more data values
        cout << ("Increase total budget.\n. Too few values collected.\n");
        exit(0);
    }else if(dds.size() < MIN_TABLE_VALUES){ // we go back to the start of the function with start and jmp = 1.
        // this occurs when we observe exponential or super-exponential functions
        if(verbose)
            cout << "\n\nRestarting with new interval\n";
        st = 1;
        jmp = 1;
        end = INT_MAX;
        goto restart;
    }

    // ---------- RATIO TABLE ----------
    ratios = vector<vector<long double>>(num_functions, vector<long double>(count, 0));
    vector<rd_t> rds[num_functions];
    oss << "Ratio Table:\n";
    oss << left << setw(12) << " ";
    for(int i = 0; i < num_functions; ++i){
        oss << setw(20) << fs[i].name << setw(2);
    }
    oss << "\n";

    int c = 0;
    auto it_begin = dds.begin();

    double normalize_vals[num_functions];
    for(int i = 0; i < num_functions; ++i){
        normalize_vals[i] = 1;
        for(auto it = dds.begin(); it != dds.end(); ++it){
            long double pos = (long double) it->duration / fs[i].function_base(it->n, st, end);
            if(pos != 0 && !isnan(pos) && !isinf(pos)){
                normalize_vals[i] = pos;
                break;
            }
        }
    }
    for(auto it = dds.begin(); it != dds.end(); ++it){
        oss << setw(10) << it->n << setw(2);
        for(int i = 0; i < num_functions; ++i){
            // if(i == 0) rds[i] = vector<rd_t>();
            ratios[i][c] = (long double) it->duration / fs[i].function_base(it->n, st, end);
            ratios[i][c] /= normalize_vals[i];
            rds[i].push_back({it->n, ratios[i][c]});
            oss << setw(20) << setprecision(5) << fixed << ratios[i][c] << setw(2);
        }
        c++;
        oss << "\n";
    }

    if(verbose){
        cout << oss.str();
    }

    // ---------- FINDING MODEL ----------

    vector<rd_t> vals[num_functions];
    int max_sz = 0;
    for(int i = 0; i < num_functions; ++i){
        int j = 0;
        while(j < count && (rds[i][j].ratio < 0.9 || rds[i][j].ratio > 1.1)) j++; // keep adding one to j until we get to 1.
        for(; j < count; ++j){
            if(isnan(rds[i][j].ratio) || isinf(rds[i][j].ratio)) continue;
            // cout << "Added: " << rds[i][j].n << " " << rds[i][j].ratio << "\n";
            vals[i].push_back(rds[i][j]);
            max_sz = max_sz < vals[i].size() ? vals[i].size(): max_sz;
        }
    }

    const long double* x[num_functions][max_sz];
    long double y[num_functions][max_sz];
    for(int i = 0; i < num_functions; ++i){
        for(int j = 0; j < vals[i].size(); ++j){
            x[i][j] = new long double[1]{(const long double) vals[i][j].n};
            y[i][j] = vals[i][j].ratio;
        }
    }

    for(int i = 0; i < num_functions; ++i){
        int start = vals[i][0].n;
        long double max_b = (long double) vals[i][vals[i].size() - 1].n / 5; // this will be passed in so that b stays within the range of 0 to this value
        function<long double(long double*)> mse = MSE(vals[i].size(), x[i], y[i], 
            [start, max_b](const long double* x, long double* args) -> long double {return convergence_function(x, args, start, max_b);});
        gradient_descent grd(mse, 2, GRADIENT_DESCENT_ITERATIONS);
        grd.set_verbose(false);
        long double first_guess[2] = {vals[i][vals[i].size() - 1].ratio, (long double) 0};
        grd.set_guess(first_guess);
        if(show_gradient) cout << left << setw(15) << fs[i].name << setprecision(5) << "Initial guess: " << setw(14) << grd.get_guess()[0] << ", " << setw(14) << grd.get_guess()[1];
        grd.run();
        if(show_gradient) cout << " After: " << setw(14) << grd.get_guess()[0] << ", " << setw(14) << grd.get_guess()[1];
        long double error = mse(&grd.get_guess()[0]);
        if(show_gradient) cout << right << setw(20) << "Error: " << left << setw(14) << error << "\n";

        // if the error is low enough, we conclude that the ratio converges:
        if(error < convergence_error){
            stats.push_back({fs[i].name, grd.get_guess()[0], error});
        }
    }

    // free
}

// Represents a generic converging function. "c" represents the point (c, 1) that f(x) always intersects -- this will be a constant value that depends on
// the start value of n. Since gradient descent requires a long double for each of its arguments, and we want "b" to be in (0, inf), 
// if we call sigmoid(b) with some scale
// f(x) = 2(a - 1)(1 / [1+e^(-(x-c)/(max_b x sigmoid(b)))] - 0.5) + 1
long double time_complexity::convergence_function(const long double* x, long double* args, int c, long double max_b){
    return 2 * (args[0] - 1) * (1 / (1 + exp(-1 * (x[0] - c) * (1 / (max_b * sigmoid(args[1]))))) - 0.5) + 1;
}

// A simple sigmoid function that takes in an x and an a
// f(x) = a / (1 + e^{-x})
long double time_complexity::sigmoid(long double x){
    return 1 / (1 + exp(-1 * x));
}

// find an appropriate given the total_budget and computational_budget
tuple<int, int, int> time_complexity::find_interval(function<void(int)> func){
    int jmp;
    int af, bf, duration1, duration2;
    int max_jmp = 10000000;
    int jmp_factor = 2;
    jmp = jmp_factor;
    int ppbf, ppaf;

    ppbf = get_time;

    while(jmp <= max_jmp){
        // run for the first interval.
        int first_interval = run_func_with_budget(func, jmp, computation_budget);

        if(first_interval){
            ppaf = get_time;
            preprocessing_time = ppaf - ppbf;
            return {1, jmp/jmp_factor * total_budget / computation_budget * 10, jmp/jmp_factor};
        }
            //return {1, jmp/jmp_factor * total_budget / computation_budget * 10, jmp/jmp_factor};

        jmp *= jmp_factor;
    }
    
    // if we get here, the we never get past the computation_budget.
    jmp = jmp_factor;
    // cout << "[jmp overflow] ";

    // Get preprocessing time.
    ppaf = get_time;
    preprocessing_time = ppaf - ppbf;
    return {1, jmp/jmp_factor * total_budget * 10, jmp/jmp_factor};
}


// ------------------------ PUBLIC ------------------------
// Constructor
time_complexity::time_complexity(int millisecond_total_budget, int millisecond_computation_budget, vector<ft_t> fs){
    this->fs = fs;
    this->total_budget = millisecond_total_budget;
    this->computation_budget = millisecond_computation_budget;
    this->verbose = false;
    this->show_gradient = false;
    this->show_possible_big_o = true;

    // Create pipe
    assert(pipe(fd) != -1);
}

// we need to find the intervals for the omega_test function.
bool time_complexity::compute_complexity(string name, function<void(int)> func, string expected_complexity){
    int st, end, jmp;
    tie(st, end, jmp) = find_interval(func);
    char* s;
    sprintf(s, "Interval: [%d, %d), Jump = %d", st, end, jmp);
    if(verbose) cout << (string) s << "\n";

    // Generate table
    complexity_table_generator(func, st, end, jmp);

    string guess_name = "NOT FOUND";
    if(show_possible_big_o) cout << "Possible Big O functions: \n";
    for(int i = 0; i < stats.size(); ++i){
        if(show_possible_big_o) printf("  - %s : (a = %.5Lf, error = %.5Lf) \n", stats[i].name.c_str(), stats[i].a, stats[i].error);

        // We guess the last function that doesn't converge to zero (there is likely only one function like this).
        if(stats[i].a >= zero){
            guess_name = stats[i].name;
        }
    }

    sprintf(s, "[%.3fs, n = %lu]", (double) (total_time + preprocessing_time) / 1000, dds.size());

    assert(guess_name.length() > 0);
    assert(expected_complexity == "" || expected_complexity.length() > 0);

    if(guess_name != "NOT FOUND") guess_name.erase(guess_name.begin());
    expected_complexity.erase(expected_complexity.begin());
    if(guess_name != "NOT FOUND") guess_name = "\u0398" + guess_name;
    expected_complexity = "\u0398" + expected_complexity;

    cout << setprecision(3) << left << setw(60) << ((string) s + " " + name) << setw(30) << ("Guess: " + guess_name);
    if(expected_complexity != ""){
        cout << setw(20) << ((expected_complexity == guess_name) ? "OK": ("NO -- EXPECTED " + expected_complexity));
    }
    cout << "\n";

    return (expected_complexity == "" || expected_complexity == guess_name);
}


// ------------------------ OTHER FUNCTIONS ------------------------

vector<ft_t> default_functions() {
    vector<ft_t> functions;

    ft_t constant;
    constant.name = "O(1)";
    constant.function_base = [](int n, int st, int end)->long double {return 1;};
    functions.push_back(constant); 

    ft_t logarithmic;
    logarithmic.name = "O(log n)";
    logarithmic.function_base = [](int n, int st, int end)->long double {return log(n);};
    functions.push_back(logarithmic); 

    ft_t sqrt;
    sqrt.name = "O(sqrt(n))";
    sqrt.function_base = [](int n, int st, int end)->long double {return pow(n, 0.5);};
    functions.push_back(sqrt); 

    ft_t linear;
    linear.name = "O(n)";
    linear.function_base = [](int n, int st, int end)->long double {return n;};
    functions.push_back(linear); 

    ft_t linearxlogarithmic;
    linearxlogarithmic.name = "O(n log n)";
    linearxlogarithmic.function_base = [](int n, int st, int end)->long double {return n != 1 ? n * log(n) : 1;};
    functions.push_back(linearxlogarithmic); 

    ft_t quadratic;
    quadratic.name = "O(n^2)";
    quadratic.function_base = [](int n, int st, int end)->long double {return n <= pow(INT_MAX, 0.5) ? n * n: INFINITY;}; // normalized n^2 function
    functions.push_back(quadratic); 

    ft_t cubic;
    cubic.name = "O(n^3)";
    cubic.function_base = [](int n, int st, int end)->long double {
        return n <= pow(INT_MAX, 0.332) ? n * n * n: INFINITY;
    };
    functions.push_back(cubic); 

    ft_t exponentialxhalf;
    exponentialxhalf.name = "O(1.5^n)";
    exponentialxhalf.function_base = [](int n, int st, int end)->long double {
        return n <= log(INT_MAX)/log(1.51) ? pow(1.5, n) : INFINITY;
    };
    functions.push_back(exponentialxhalf); 

    ft_t exponentialx2;
    exponentialx2.name = "O(2^n)";
    exponentialx2.function_base = [](int n, int st, int end)->long double {
        return (long long) n <= log(INT_MAX)/log(2.01) ? pow(2, n) : INFINITY;
    };
    functions.push_back(exponentialx2); 

    ft_t super_exponential;
    super_exponential.name = "O(n^n)";
    super_exponential.function_base = [](int n, int st, int end)->long double {
        if(st > 4) n /= st;
        return (long double) n <= 8 ? pow(n, n)/pow(st, st) : INFINITY;
    };
    functions.push_back(super_exponential); 

    return functions;
}