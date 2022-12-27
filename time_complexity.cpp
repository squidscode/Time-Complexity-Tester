#include "time_complexity.h"
#include "./gradient_descent/gradient_descent.h"
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <fstream>
#include <functional>
#include <sstream>
#include <functional>
#include <sys/time.h>
#include <math.h>
#include <vector>
#include <unistd.h>
#include <time.h>
#include <cmath>
#include <signal.h>
#include <tuple>
#define get_time duration_cast<nanoseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count()

#define MIN_TABLE_VALUES 3
#define DATA_BEFORE_DOUBLE 500
#define DATA_CAP 10000
#define GRADIENT_DESCENT_ITERATIONS 100
#define min(x,y) (x < y ? x : y)

using namespace std;
using std::chrono::duration_cast;
using std::chrono::nanoseconds;
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
    // cout << n << " " << budget << "\n";
    pid_t child_pid;
    int rv = 0;

    if((child_pid = fork()) != 0){  // parent
        // run while the child process is running
        long long start_time = get_time;
        while(waitpid(child_pid, nullptr, WNOHANG) == 0 && rv == 0){
            if(get_time - start_time > (long long) budget){
                kill(child_pid, SIGKILL);
                rv = 1;
            }
        }
    }else{                          // child
        func(n);
        exit(0);
    }

    return rv;
}

// Generate a unique file name:
string get_file_name(){
    time_t rawtime;
    struct tm* timeinfo;
    char buf[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buf, 80, "%F-%T.json", timeinfo);

    return buf;
}

void time_complexity::save_to_file(vector<rd_t> vals[], vector<guess_collection_t> guesses){
    string dir = data_directory + "/" + current_test_name;
    mkdir(dir.c_str(), 0744);

    ofstream ofs = ofstream();
    ofs.open(dir + "/" + get_file_name()); // create a new file with the given unique name (using current time)

    int num_functions = fs.size();

    // CREATE THE JSON FILE:
    ofs << "{";

    // Write guess:
    ofs << "\"predictions\":{";

    ofs << "\"function string\":\"" << FUNCTION_STR << "\",";
    for(int function_num = 0; function_num < num_functions; ++function_num){ // for each function:
        ofs << "\"" << fs[function_num].name << "\":{";
        
        ofs << "\"guess\":["
            << guesses[function_num].a << ","
            << guesses[function_num].b << ","
            << guesses[function_num].c << ","
            << guesses[function_num].d << "],";
        ofs << "\"error\":" << guesses[function_num].error;

        ofs << "}";
        if(function_num < num_functions - 1) ofs << ",";
    }
    ofs << "},";

    // Write data:
    ofs << "\"data\":{";
    for(int function_num = 0; function_num < num_functions; ++function_num){ // for each function:
        ofs << "\"" << fs[function_num].name << "\":{";

        ofs << "\"x\":[";
        for(int i = 0; i < vals[function_num].size(); ++i){
            ofs << vals[function_num][i].n;
            if(i < vals[function_num].size() - 1) ofs << ",";
        }
        ofs << "],";
        
        ofs << "\"y\":[";
        for(int i = 0; i < vals[function_num].size(); ++i){
            ofs << vals[function_num][i].ratio;
            if(i < vals[function_num].size() - 1) ofs << ",";
        }
        ofs << "]}";

        if(function_num < num_functions - 1) ofs << ",";
    }
    ofs << "}";
    ofs << "}";
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

    
    // Protect against overflow
    for(int i = st; i < end && i > 0; i += jmp){
        pid_t child_pid;
        bool ignore_duration = false;
        long long start_time = get_time;
        long long end_time;

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

            end_time = get_time;
        }
        
        if(ignore_duration) break;

        duration = af - bf;
        dds.push_back({i, duration});
        total_time += (end_time - start_time); // include process startup time
        count++;
        if(total_budget < total_time) break;

        // Double the jump size each time we reach a power of 2
        if(dds.size() % DATA_BEFORE_DOUBLE == 0){
            jmp *= 2;
        }

        // If we reach the data cap, then we will break
        if(dds.size() >= DATA_CAP){
            break;
        }

        // Print test information if verbose is true.
        if(verbose){
            cout << left << "\n(n:" << setw(5) << i << ", Time:" << setw(7) << (double) duration / 1000 << "s)";
        }
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

    vector<guess_collection_t> guesses;
    for(int i = 0; i < num_functions; ++i){
        int start = vals[i][0].n;
        long double max_b = (long double) vals[i][vals[i].size() - 1].n / 5; // this will be passed in so that b stays within the range of 0 to this value
        function<long double(long double*)> mse = MSE(vals[i].size(), x[i], y[i], 
            [start, max_b](const long double* x, long double* args) -> long double {return convergence_function(x, args, start, max_b);});
        
        // GRADIENT DESCENT: Minimize the mean-squared-error of the given function (mse). mse takes 2 arguments.
        gradient_descent grd(mse, 2, GRADIENT_DESCENT_ITERATIONS);
        grd.set_verbose(false);
        long double first_guess[2] = {vals[i][vals[i].size() - 1].ratio, (long double) 0};
        grd.set_guess(first_guess);

        char buf[29];
        sprintf(buf, "(%.5Lf, %.5Lf)", grd.get_guess()[0], grd.get_guess()[1]);
        if(show_gradient) cout << left << setw(15) << fs[i].name << setprecision(5) << "Initial guess: " << setw(30) << buf;
        grd.run();
        sprintf(buf, "(%.5Lf, %.5Lf)", grd.get_guess()[0], grd.get_guess()[1]);
        if(show_gradient) cout << right << setw(20) << " After: " << left << setw(30) << buf;
        long double error = mse(&grd.get_guess()[0]);
        if(show_gradient) cout << right << setw(20) << "Error: " << left << setw(15) << error << "\n";

        guesses.push_back({grd.get_guess()[0], grd.get_guess()[1], start, max_b, error});
        // if the error is low enough, we conclude that the ratio converges:
        if(error < convergence_error){
            stats.push_back({fs[i].name, grd.get_guess()[0], error});
        }
    }

    if(save_data) save_to_file(vals, guesses);

    // Free all allocated space.
    for(int fn = 0; fn < num_functions; ++fn){
        for(int j = 0; j < vals[fn].size(); ++j){
            delete[] x[fn][j];
        }
    } 
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
    int max_jmp = 10000000;
    int jmp_factor = 2;
    jmp = jmp_factor;
    int ppbf, ppaf;

    ppbf = get_time;

    while(jmp <= max_jmp){
        // run for the first interval.
        int first_interval = run_func_with_budget(func, jmp, computation_budget);

        if(first_interval == 1){
            ppaf = get_time;
            preprocessing_time = ppaf - ppbf;
            return {jmp/jmp_factor, INT_MAX, jmp};
        }
            //return {1, jmp/jmp_factor * total_budget / computation_budget * 10, jmp/jmp_factor};

        jmp *= jmp_factor;
    }
    
    // if we get here, the we never get past the computation_budget.

    // Get preprocessing time.
    ppaf = get_time;
    preprocessing_time = ppaf - ppbf;
    jmp = INT_MAX / (total_budget / 10000); // 1000 more possible points than the total_budget.
    return {1, INT_MAX, (jmp <= 0 ? 1 : jmp)};
}


// ------------------------ PUBLIC ------------------------
// Constructor
time_complexity::time_complexity(int millisecond_total_budget, int millisecond_computation_budget, vector<function_type_t> fs){
    this->fs = fs;
    this->total_budget = (long long) millisecond_total_budget * 1000000;
    this->computation_budget = (long long) millisecond_computation_budget * 1000000;

    this->auto_interval = true;
    this->verbose = false;
    this->show_gradient = false;
    this->show_possible_big_o = true;

    // Create pipe
    assert(pipe(fd) != -1);
}

// we need to find the intervals for the omega_test function.
bool time_complexity::compute_complexity(string name, function<void(int)> func, string expected_complexity){
    this->current_test_name = name;

    if(expected_complexity.size() != 0 && expected_complexity[0] != 'T' && expected_complexity[0] != 'O'){
        printf("Invalid time complexity guess: %s\n", expected_complexity.c_str());
        printf("Time complexity guess must start with either \'O\' or \'T\', representing Big-O and Big-Theta tests, respectively.\n");
    }
    
    int st, end, jmp;
    if(this->auto_interval){
        tie(st, end, jmp) = find_interval(func);
    } else {
        preprocessing_time = 0; // since we do not preprocess.
        tie(st, end, jmp) = tuple<int,int,int>{1, INT_MAX, 1}; // a hard cap on the # of tests.
    }

    char s[40];
    sprintf(s, "Interval: [%d, %d), Jump = %d", st, end, jmp);
    if(show_interval) cout << (string) s << "\n";

    // Generate table
    complexity_table_generator(func, st, end, jmp);

    string guess_name = "NOT FOUND";
    if(show_possible_big_o) cout << "Possible Big O functions: \n";
    for(int i = 0; i < stats.size(); ++i){
        if(show_possible_big_o) printf("  - %s : (a = %.5Lf, error = %.5Lf) \n", stats[i].name.c_str(), stats[i].a, stats[i].error);

        // We guess the last function that doesn't converge to zero (there is likely only one function like this).
        if(stats[i].a >= zero){
            guess_name = stats[i].name;
            guess_name.erase(guess_name.begin());
            guess_name = "\u0398" + guess_name;
        }
    }

    if(guess_name == "NOT FOUND" && stats.size() != 0){
        guess_name = stats[0].name; // take the lowest big O.
    }

    sprintf(s, "[%.3fs, n = %lu]", (double) (total_time + preprocessing_time) / 1000000 / 1000, dds.size());

    assert(expected_complexity == "" || expected_complexity.length() > 0);

    cout << setprecision(3) << left << setw(60) << ((string) s + " " + name) << setw(30) << ("Guess: " + guess_name);
    if(expected_complexity != ""){
        if(expected_complexity[0] == 'T'){
            expected_complexity.erase(expected_complexity.begin());
            expected_complexity = "\u0398" + expected_complexity;
            cout << setw(30) << ((expected_complexity == guess_name) ? "OK": ("NO -- EXPECTED " + expected_complexity)) << "\n";
            return expected_complexity == guess_name;
        }else if(expected_complexity[0] == 'O'){
            bool bigO = false;
            for(int i = 0; i < stats.size(); ++i){
                if(expected_complexity == stats[i].name){
                    bigO = true;
                    break;
                }
            }

            // only print "bounded by" message when our guess is also writting in big-O.
            cout << setw(30) << ((bigO) ? ((expected_complexity == guess_name || guess_name[0] != 'O') ? "OK" : "OK [Bounded by: " + expected_complexity + "]") : ("NO -- EXPECTED " + expected_complexity)) << "\n";
            return bigO;
        }
    }
    cout << "\n";

    return (expected_complexity == "" || expected_complexity == guess_name);
}


// ------------------------ OTHER FUNCTIONS ------------------------

vector<function_type_t> default_functions() {
    vector<function_type_t> functions;

    function_type_t constant;
    constant.name = "O(1)";
    constant.function_base = [](int n, int st, int end)->long double {return 1;};
    functions.push_back(constant); 

    function_type_t logarithmic;
    logarithmic.name = "O(log n)";
    logarithmic.function_base = [](int n, int st, int end)->long double {return log(n);};
    functions.push_back(logarithmic); 

    function_type_t sqrt;
    sqrt.name = "O(sqrt(n))";
    sqrt.function_base = [](int n, int st, int end)->long double {return pow(n, 0.5);};
    functions.push_back(sqrt); 

    function_type_t linear;
    linear.name = "O(n)";
    linear.function_base = [](int n, int st, int end)->long double {return n;};
    functions.push_back(linear); 

    function_type_t linearxlogarithmic;
    linearxlogarithmic.name = "O(n log n)";
    linearxlogarithmic.function_base = [](int n, int st, int end)->long double {return n != 1 ? n * log(n) : 1;};
    functions.push_back(linearxlogarithmic); 

    function_type_t quadratic;
    quadratic.name = "O(n^2)";
    quadratic.function_base = [](int n, int st, int end)->long double {return n <= pow(INT_MAX, 0.5) ? n * n: INFINITY;}; // normalized n^2 function
    functions.push_back(quadratic); 

    function_type_t cubic;
    cubic.name = "O(n^3)";
    cubic.function_base = [](int n, int st, int end)->long double {
        return n <= pow(INT_MAX, 0.332) ? n * n * n: INFINITY;
    };
    functions.push_back(cubic); 

    function_type_t exponentialxhalf;
    exponentialxhalf.name = "O(1.5^n)";
    exponentialxhalf.function_base = [](int n, int st, int end)->long double {
        return n <= log(INT_MAX)/log(1.51) ? pow(1.5, n) : INFINITY;
    };
    functions.push_back(exponentialxhalf); 

    function_type_t exponentialx2;
    exponentialx2.name = "O(2^n)";
    exponentialx2.function_base = [](int n, int st, int end)->long double {
        return (long long) n <= log(INT_MAX)/log(2.01) ? pow(2, n) : INFINITY;
    };
    functions.push_back(exponentialx2); 

    function_type_t super_exponential;
    super_exponential.name = "O(n^n)";
    super_exponential.function_base = [](int n, int st, int end)->long double {
        if(st > 4) n /= st;
        return (long double) n <= 8 ? pow(n, n)/pow(st, st) : INFINITY;
    };
    functions.push_back(super_exponential); 

    return functions;
}