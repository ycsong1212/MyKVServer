#include<string>
#include<iostream>
#include<algorithm>

using namespace std;

string getLastStr(string str)
{
    string res = "";
    int i = str.length() - 1;
    while (str[i] != ' ')
    {
        res.append(1, str[i]);
        i--;
    }
    reverse(res.begin(),res.end());
    return res;
}

int main(int argc,char *argv[])
{
    string s="aiuhdiewgcirwohc owierhhxowec";
    getLastStr(s);
    cout<<s<<endl;
}

