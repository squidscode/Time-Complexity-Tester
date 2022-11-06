#include <math.h>
#include <cassert>
#include <sstream>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include "../../time_complexity.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::microseconds;

#define sl(x) std::this_thread::sleep_for(std::chrono::milliseconds((int) (x)));

void generic_tester(int n1, int n2, int n3, int n4, int n5, bool b1, bool b2, bool verbose, bool show_gradient);

function<void(int)> scaled_constant(double scale);
function<void(int)> scaled_logarithmic(double scale);
function<void(int)> scaled_sqrt(double scale);
function<void(int)> scaled_linear(double scale);
function<void(int)> scaled_linearxlog(double scale);
function<void(int)> scaled_quadratic(double scale);
function<void(int)> scaled_cubic(double scale);
function<void(int)> scaled_exponential(double scale, double pow);
function<void(int)> scaled_super_exponential(double scale);

void test_constant(int n);
void test_logarithmic(int n);
void test_sqrt(int n);
void test_linear(int n);
void test_linearxlog(int n);
void test_quadratic(int n);
void test_cubic(int n);
void test_exponentialx2(int n);
void test_super_exponential(int n);
void test_exponential(double pow, int n);

void print_help(){
    cout << "\tgeneric_tester [-ghvOT] NUM-1 NUM-2 NUM-3 NUM-4 NUM-5\n\n";
    cout << "FLAGS:\n";
    cout << "\t-g\tshow gradient\n";
    cout << "\t-h\thelp\n";
    cout << "\t-v\tverbose\n";
    cout << "\t-O\tonly run Big-O\n";
    cout << "\t-T\tonly run Big-Theta\n\n";
    cout << "ARGUMENTS:\n";
    cout << "\tNUM-1\tthe millisecond budget for each test\n";
    cout << "\tNUM-2\tthe target milliseconds allowed for one computation\n";
    cout << "\tNUM-3\tthe size of the # of tests\n";
    cout << "\tNUM-4\tthe start value for scale\n";
    cout << "\tNUM-5\tthe step (between each value for scale)\n";
}

int main(int argc, char** argv){
    bool verbose = false; bool show_gradient = false;
    bool b[2];
    b[0] = true; b[1] = true;
    int n[5]; int n_ind = 0; // the numbers
    
    for(int i = 1; i < argc; ++i){
        istringstream iss(argv[i]);

        if(argv[i][0] == '-'){
            int n = strlen(argv[i]);
            for(int j = 1; j < n; ++j){
                switch(argv[i][j]){
                case 'g':
                    show_gradient = true;
                    break;
                case 'h':
                    print_help();
                    exit(0); // exit the program
                case 'v':
                    verbose = true;
                    break;
                case 'O':
                    b[0] = true;
                    b[1] = false;
                    break;
                case 'T':
                    b[0] = false;
                    b[1] = true;
                    break;
                default:
                    print_help();
                    exit(0);
                }
            }
        }else{
            if(n_ind > 4){
                print_help();
                exit(0);
            }

            iss >> n[n_ind];
            n_ind++;

            if(iss.fail()){
                print_help();
                exit(0);
            }
        }
    }

    if(n_ind != 5){ // we need to have 5 numbers in the argument list
        cout << "Invalid # of arguments provided.\n\n";
        print_help();
        exit(0);
    }

    generic_tester(n[0], n[1], n[2], n[3], n[4], b[0], b[1], verbose, show_gradient);
}

void generic_tester(int n1, int n2, int n3, int n4, int n5, bool b1, bool b2, bool verbose, bool show_gradient){
    time_complexity tc(n1, n2);
    tc.verbose = verbose;
    tc.show_gradient = show_gradient;
    tc.show_possible_big_o = true; // default

    int size = n3;
    double val = n4;
    double step = n5;
    double scales[size];
    for(int i = 0; i < size; ++i, val += step){
        scales[i] = val;
    }

    if(b1){
        cout << "Testing Big O:\n\n"; 
        for(int i = 0; i < size; ++i){
            double scale = scales[i];
            string db = to_string(scale);
            cout << "Scale: " << scale << "\n";
            tc.compute_complexity("Constant [" + db + "]", scaled_constant(scale), "O(1)");
            tc.compute_complexity("Log [" + db + "]", scaled_logarithmic(scale), "O(log n)");
            tc.compute_complexity("Sqrt [" + db + "]", scaled_sqrt(scale), "O(sqrt(n))");
            tc.compute_complexity("Linear [" + db + "]", scaled_linear(scale), "O(n)");
            tc.compute_complexity("Linearxlog [" + db + "]", scaled_linearxlog(scale), "O(n log n)");
            tc.compute_complexity("Quadratic [" + db + "]", scaled_quadratic(scale), "O(n^2)");
            tc.compute_complexity("Cubic [" + db + "]", scaled_cubic(scale), "O(n^3)");
            tc.compute_complexity("Exp [" + db + "]", scaled_exponential(scale, 1.51), "O(1.5^n)");
            tc.compute_complexity("Exp [" + db + "]", scaled_exponential(scale, 2.01), "O(2^n)");
            tc.compute_complexity("S_Exp [" + db + "]", scaled_super_exponential(scale), "O(n^n)");
            cout << "\n";
        }
        if(b2) cout << "\n\n";
    }
    
    if(b2){
        cout << "Testing Big Theta:\n\n";
        for(int i = 0; i < size; ++i){
            double scale = scales[i];
            string db = to_string(scale);
            cout << "Scale: " << scale << "\n";
            tc.compute_complexity("Constant [" + db + "]", scaled_constant(scale), "T(1)");
            tc.compute_complexity("Log [" + db + "]", scaled_logarithmic(scale), "T(log n)");
            tc.compute_complexity("Sqrt [" + db + "]", scaled_sqrt(scale), "T(sqrt(n))");
            tc.compute_complexity("Linear [" + db + "]", scaled_linear(scale), "T(n)");
            tc.compute_complexity("Linearxlog [" + db + "]", scaled_linearxlog(scale), "T(n log n)");
            tc.compute_complexity("Quadratic [" + db + "]", scaled_quadratic(scale), "T(n^2)");
            tc.compute_complexity("Cubic [" + db + "]", scaled_cubic(scale), "T(n^3)");
            tc.compute_complexity("Exp [" + db + "]", scaled_exponential(scale, 1.51), "T(1.5^n)");
            tc.compute_complexity("Exp [" + db + "]", scaled_exponential(scale, 2.01), "T(2^n)");
            tc.compute_complexity("S_Exp [" + db + "]", scaled_super_exponential(scale), "T(n^n)");
            cout << "\n";
        }
    }
    
}

function<void(int)> scaled_constant(double scale){
    return [scale](int n) -> void{sl(scale*10);};
}

function<void(int)> scaled_logarithmic(double scale){
    return [scale](int n) -> void{sl(scale * log(n));};
}

function<void(int)> scaled_sqrt(double scale){
    return [scale](int n) -> void{sl(scale * sqrt(n));};
}

function<void(int)> scaled_linear(double scale){
    return [scale](int n) -> void{sl(scale * n);};
}

function<void(int)> scaled_linearxlog(double scale){
    return [scale](int n) -> void{sl(scale * n * log(n));};
}

function<void(int)> scaled_quadratic(double scale){
    return [scale](int n) -> void{sl(scale * pow(n, 2));};
}

function<void(int)> scaled_cubic(double scale){
    return [scale](int n) -> void{sl(scale * pow(n, 3));};
}

function<void(int)> scaled_exponential(double scale, double power){
    return [scale, power](int n) -> void{sl(scale * pow(power, n));};
}

function<void(int)> scaled_super_exponential(double scale){
    return [scale](int n) -> void{sl(scale * pow(n, n));};
}





void test_constant(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void test_logarithmic(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(10 * log(n))));
}

void test_sqrt(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(10 * sqrt(n))));
}

void test_linear(int n){
    std::this_thread::sleep_for(std::chrono::microseconds(1000 * n));
}

void test_linearxlog(int n){
    std::this_thread::sleep_for(std::chrono::microseconds((int)(100 * n * log(n))));
}

void test_quadratic(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * n * n));
}

void test_cubic(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds(n * n * n));
}

void test_exponentialx2(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(pow(2, n))));
}

void test_exponential(double power, int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(pow(power, n))));
}


void test_super_exponential(int n){
    std::this_thread::sleep_for(std::chrono::milliseconds((int)(pow(n, n))));
}

