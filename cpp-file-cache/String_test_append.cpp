#include <string>

using namespace std;

// ~TC-TEST~ String Append Test
void test(int n){
    string s(n, 'x');
    string s2(n, 'y');

    string add = s + s2;

    return;
}