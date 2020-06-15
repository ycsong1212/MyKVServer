#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>

using namespace std;

int main()
{
    ofstream fout;
    fout.open("a.txt", ios_base::app);
    // fout.open("b.txt");
    for (long i = 0; i < 10000000; i++)
    {
        fout << to_string(i) << ":" << to_string(i) << endl;
    }
    fout.close();
    return 0;
}