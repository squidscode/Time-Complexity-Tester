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

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::microseconds;


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

// ------------------------ PRIVATE ------------------------
void time_complexity::init(){
    total_time = 0;
    dds.clear();
    ratios.clear();
    dratios.clear();
    medians.clear();
    means.clear();
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
                // printf("%lld < %lld\n", get_time - bf, total_budget - total_time);
                end_pid = waitpid(child_pid, status_ptr, WNOHANG);

                // if we successfully exited child, then break.
                if(end_pid == -1){
                    exit(1);
                }else if(end_pid != 0){
                    // cout << "received information!";
                        
                    read(fd[0], &bf, sizeof(long long));
                    read(fd[0], &af, sizeof(long long));
                    // printf("\nPARENT: bf = %lld, af = %lld, duration = %lld\n", bf, af, af - bf);
                    run = false;
                }

                // cout << "Total Time: " << get_time - start_time + total_time << "\n";

                // kill child
                if(get_time - start_time >= total_budget - total_time){
                    // cout << "Attempting to kill! Should print soon!" << endl;
                    kill(child_pid, SIGKILL);
                    total_time += get_time - start_time;
                    run = false;
                    ignore_duration = true;
                }
            }
            // cout << "got to end!\n";
        }
        
        // exit(0);
        if(ignore_duration)
            break;

        duration = af - bf;
        dds.push_back({i, duration});
        total_time += duration;
        count++;
        if(total_budget < total_time)
            break;
        oss << left << "\n(n:" << setw(5) << i << ", Time:" << setw(7) << (double) duration / 1000 << "s)";
    }
    oss << "\n\n";
    oss << "Total time: " << (double) total_time / 1000 << "\n\n";

    int interval = 11;

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
            if(i == 0) rds[i] = vector<rd_t>();
            ratios[i][c] = (long double) it->duration / fs[i].function_base(it->n, st, end);
            ratios[i][c] /= normalize_vals[i];
            rds[i].push_back({it->n, ratios[i][c]});
            oss << setw(20) << setprecision(5) << fixed << ratios[i][c] << setw(2);
        }
        c++;
        oss << "\n";
    }

    // ---------- RATIO DIF TABLE ----------
    dratios = vector<vector<long double>>(num_functions, vector<long double>(count - 1, 0));
    oss << "\nRatio diff Table:\n";
    oss << setw(12) << " ";
    for(int i = 0; i < num_functions; ++i){
        oss << setw(20) << fs[i].name << setw(2);
    }
    oss << "\n";

    c = 0;
    for(int i = 0; i < num_functions; ++i){
        normalize_vals[i] = 1;
        for(int j = 1; j < count; ++j){
            long double pos = abs(ratios[i][j] - ratios[i][j-1]) / jmp;
            if(pos != 0 && !isnan(pos) && !isinf(pos)){
                normalize_vals[i] = pos;
                break;
            }
        }
    }

    for(auto it = ++dds.begin(); it != dds.end(); ++it){
        oss << setw(10) << it->n << setw(2);
        for(int i = 0; i < num_functions; ++i){
            // dratios[i][c] = abs(ratios[i][c + 1] - ratios[i][c]);
            dratios[i][c] = (ratios[i][c + 1] - ratios[i][c]) / jmp;
            dratios[i][c] /= normalize_vals[i];
            oss << setw(20) << setprecision(5) << dratios[i][c] << setw(2);
        }
        c++;
        oss << "\n";
    }

    // ---------- FINDING MODEL ----------
    vector<rd_t> vals[num_functions];
    int max_sz = 0;
    for(int i = 0; i < num_functions; ++i){
        for(int j = 0; j < count - 1; ++j){
            if(isnan(rds[i][j].ratio) || isinf(rds[i][j].ratio) || abs(rds[i][j].ratio) < 0.00000001) continue;
            vals[i].push_back(rds[i][j]);
            max_sz = max_sz < vals[i].size() ? vals[i].size(): max_sz;
        }
    }

    const long double* x[num_functions][max_sz][1];
    const long double* y[num_functions][max_sz];
    for(int i = 0; i < num_functions; ++i){
        for(int j = 0; j < vals[i].size(); ++j){
            x[i][j][0] =  new const long double[1]{(const long double) vals[i][j].n};
            y[i][j] = new const long double[1]{(const long double) vals[i][j].ratio};
        }
    }

    for(int i = 0; i < num_functions; ++i){
        gradient_descent grd(MSE(vals[i].size(), x[i][0], y[i][0], sigmoid), 2, 5000);
        grd.set_guess(new long double[2]{0, 0});
        cout << "Initial guess: " << grd.get_guess()[0] << ", " << grd.get_guess()[1] << "\n";
        grd.run();
        cout << "After: " << grd.get_guess()[0] << ", " << grd.get_guess()[1] << "\n";
    }

    vector<long double> diffvals[num_functions];
    for(int i = 0; i < num_functions; ++i){
        for(int j = 0; j < count - 1; ++j){
            if(isnan(dratios[i][j]) || isinf(dratios[i][j])) continue;
            diffvals[i].push_back(dratios[i][j]);
        }
    }

    for(int i = 0; i < num_functions; ++i){
        sort(diffvals[i].begin(), diffvals[i].end());
    }

    medians = vector<long double>(num_functions);
    means = vector<double>(num_functions);
    for(int i = 0; i < num_functions; ++i){
        if(diffvals[i].size() == 0){
            medians[i] = 0;
            continue;
        }
        medians[i] = diffvals[i][(count - 1)/2];
    }

    for(int i = 0; i < num_functions; ++i){
        if(diffvals[i].size() == 0){
            means[i];
            continue;
        }

        for(auto it = diffvals[i].begin(); it != diffvals[i].end(); ++it){
            if(isnan(*it) || isinf(*it)) printf("ERROR!\n");
            means[i] += (double) *it;
        }
        means[i] /= diffvals[i].size();
    }

    oss << "\nStat Table:\n"; 
    oss << setw(20) << " ";
    for(int i = 0; i < num_functions; ++i){
        oss << setw(20) << fs[i].name << setw(2);
    }
    oss << "\n";
    oss << setw(20) << "diff Medians:" << setw(2);
    for(auto i = 0; i < num_functions; ++i){
        oss << setw(20) << scientific << medians[i] << setw(2);
    }
    oss << "\n";
    oss << setw(20) << "diff Means:" << setw(2);
    for(auto i = 0; i < num_functions; ++i){
        oss << setw(20) << scientific << means[i] << setw(2);
    }
    oss << "\n\n";

    if(verbose){
        cout << oss.str();
    }
}

// Represents the function: f(x) = (a / (1 + e^{-(x - b)}) + 1)
long double time_complexity::sigmoid(const long double* x, long double* args){
    return (args[1] / (1 + exp(-1 * (x[0] - args[0]))) + 1);
}

// find an appropriate given the total_budget and computational_budget
tuple<int, int, int> time_complexity::find_interval(function<void(int)> func){
    int jmp;
    int af, bf, duration1, duration2;
    int max_jmp = 10000000;
    int jmp_factor = 2;
    jmp = jmp_factor;

    while(jmp <= max_jmp){
        // run for the first interval.
        int first_interval = run_func_with_budget(func, jmp, computation_budget);

        if(first_interval) 
            return {1, jmp/jmp_factor * total_budget / computation_budget * 10, jmp/jmp_factor};
            //return {1, jmp/jmp_factor * total_budget / computation_budget * 10, jmp/jmp_factor};

        jmp *= jmp_factor;
    }
    
    // if we get here, the we never get past the computation_budget.
    jmp = jmp_factor;
    // cout << "[jmp overflow] ";
    return {1, jmp/jmp_factor * total_budget * 10, jmp/jmp_factor};
}


// ------------------------ PUBLIC ------------------------
// Constructor
time_complexity::time_complexity(int millisecond_total_budget, int millisecond_computation_budget, bool verbose, vector<ft_t> fs){
    this->fs = fs;
    this->total_budget = millisecond_total_budget;
    this->computation_budget = millisecond_computation_budget;
    this->verbose = verbose;

    // Create pipe
    assert(pipe(fd) != -1);
}

// we need to find the intervals for the omega_test function.
void time_complexity::compute_complexity(string name, function<void(int)> func, string expected_complexity){
    int st, end, jmp;
    tie(st, end, jmp) = find_interval(func);
    char* s;
    sprintf(s, "Interval: [%d, %d), Jump = %d", st, end, jmp);
    if(verbose)
        cout << (string) s << "\n";
    complexity_table_generator(func, st, end, jmp);
    long double val;
    int i;
    for(i = 1; i < fs.size(); ++i){
        val = ratios[i][ratios[i].size() - 1];
        // cout << fs[i].name << " " << val << "\n";
        // arbitrary cutoff of 0.8.
        if(val < 0.8){
            break;
        }  
    }
    i--;
    sprintf(s, "[%.3fs]", (double) total_time / 1000);
    cout << setprecision(3) << left << setw(40) << ((string) s + " " + name) << setw(30) << ("Guess: " + fs[i].name);
    if(expected_complexity != ""){
        cout << setw(20) << ((expected_complexity == fs[i].name) ? "OK": ("NO -- EXPECTED " + expected_complexity));
    }
    cout << "\n";
}