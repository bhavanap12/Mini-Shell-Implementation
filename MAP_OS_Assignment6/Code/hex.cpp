#include <iostream>
#include <sstream>
#include <bitset>
#include <string>
using namespace std;
string binconvert(string s)
{
    stringstream ss;
    ss << hex << s;
    unsigned n;
    ss >> n;
    bitset<8> b(n);
    return b.to_string();
}
