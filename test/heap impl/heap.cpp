#include <iostream>
#include <iomanip>
#include <functional>
#include <chrono>
#include <sys/time.h>
#include <thread>
#include <ctime>
#include <math.h>
#include <utility>
#include <vector>
#include <map>
#include "time_complexity.cpp"
#define get_time duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::system_clock;

void test(function<bool(int, int)> comp, int);
bool omega_test(function<void(int)>, vector<ft_t>, int, int, int);
void test_push_back(int n);
void test_constantc(int n);
void test_linearc(int n);

// class T must override the < and > operators in order for min_heap to work.
template<class T> class heap{
    private:
        int n;
        int chunk;
        int max_sz;
        T* a;
        function<bool(T, T)> comp;

        void swap(int i, int j){
            T tmp = *(a + j);
            *(a + j) = *(a + i);
            *(a + i) = tmp;
        }


    public:
        heap(int chunk = 100){
            this->n = 0;
            this->chunk = chunk;
            this->max_sz = chunk;
            this->a = (T*) malloc(sizeof(T) * max_sz);
            this->comp = [](T x, T y) -> bool {return x < y;};
        }

        heap(function<bool(T,T)> comp, int chunk = 100){
            this->n = 0;
            this->chunk = chunk;
            this->max_sz = chunk;
            this->a = (T*) malloc(sizeof(T) * max_sz);
            this->comp = comp;
        }

        ~heap(){
            free(a);
        }

        void copy(T* ds, int n){
            for(int i = 0; i < n; ++i){
                push(*(ds + i));
            }
        }

        void push(T var){
            // we need to re-allocate the array!
            if(n >= max_sz){
                a = (T*) realloc(a, sizeof(T) * (max_sz + chunk));
                max_sz += chunk;
            }else{
                *(a + n) = var;
            }

            // now move [n - 1] down
            int j = n;
            int i = (j - 1)/2;
            while(j > 0){
                if(comp(*(a + j), *(a + i))){
                    swap(i, j);
                    j = i;
                    i = (j - 1)/2;
                }else{
                    break;
                }
            }

            // n has now grown.
            n++;
        }

        T pop(){
            if(n == 0){
                return 0;
            }

            T tmp = a[0];
            swap(0, n - 1);

            n--;

            int i = 0;
            int j = 1;
            while(j < n){
                if(comp(*(a + j), *(a + i)) && (j + 1 >= n || comp(*(a + j), *(a + j + 1)) || *(a + j) == *(a + j + 1))){
                    swap(j, i);
                    i = j;
                    j = 2 * i + 1;
                }else if(j + 1 < n && comp(*(a + j + 1), *(a + i))){
                    swap(i, j + 1);
                    i = j + 1;
                    j = 2 * i + 1;
                }else{
                    break;
                }
            }
            return tmp;
        }
        
        int size(){
            return n;
        }

        void print_heap(){
            cout << "heap: ";
            for(int i = 0; i < n; ++i){
                cout << *(a + i) << " ";
            }
            cout << "\n";
        }
};

int main(void){
    test([](int x, int y) -> bool {return x < y;}, 100000);
    test([](int x, int y) -> bool {return x > y;}, 100000);
    function<void(int)> test_func = [](int n)->void{test([](int x, int y) -> bool {return x < y;}, n);};

    time_complexity tc(10000, 100, false);
    tc.compute_complexity("Test Func", test_func);
    tc.compute_complexity("Linear", test_linearc);
    tc.compute_complexity("Push Back", test_push_back);
    tc.compute_complexity("Constant", test_constantc);
}

void test_linearc(int n){
    vector<int> v;

    // cout << "adding random elements: " << endl;
    for(int i = 0; i < n; ++i){
        v.push_back(rand() % 1000000);
    }
}

void test_push_back(int n){
    srand(10);

    function<bool(int,int)> comp = [](int x, int y) -> bool {return x < y;};
    heap<int> mh(comp, 500000);

    // cout << "adding random elements: " << endl;
    for(int i = 0; i < n; ++i){
        mh.push(rand() % 1000000);
    }
}

void test_constantc(int n){
    function<bool(int,int)> comp = [](int x, int y) -> bool {return x > y;};
    heap<int> mh(comp, n);

    // cout << "adding random elements: " << endl;
    for(int i = 0; i < 1000000; ++i){
        mh.push(rand() % 1000000);
    }
}

void test(function<bool(int,int)> comp, int n){
    srand(10);
    int arr[n];
    for(int i = 0; i < n; ++i){
        arr[i] = rand() % 1000000;
    }

    // cout << "init: " << endl;
    heap<int> mh(comp, 500000);
    
    // cout << "copying: " << endl;
    mh.copy(arr, sizeof(arr) / sizeof(int));

    // cout << "adding random elements: " << endl;
    for(int i = 0; i < n; ++i){
        mh.push(rand() % 1000000);
    }

    // cout << "test all elements are ordered: " << endl;
    int i = mh.pop();
    // cout << i << " ";
    while(mh.size() > 0){
        int j = mh.pop();
        // cout << j << " ";
        if(!(comp(i, j) || i == j)) {
            cout << "\nERROR at index " << i << "\n";
        }
        // cout << j << " ";
        i = j;
    }
    // cout << "\n";
}

