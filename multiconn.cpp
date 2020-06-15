#include <iostream>
#include <sstream>
#include <fstream>
#include <ostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <vector>
#include <thread>
// #include <boost/thread/thread.hpp>
// #include <boost/bind.hpp>

#define MAXLINE 4096
#define THREADNUD 10

using namespace std;

string genPutMsg(string msg)
{
    return "put" + to_string(msg.length()) + "_" + msg + " ";
}

string genGetMsg(string msg)
{
    return "get" + to_string(msg.length()) + "_" + msg + " ";
}

vector<string> split(string str, char del)
{
    // 按del字符将缓冲区字符串划分为消息向量
    stringstream ss(str);
    string tok;
    vector<string> ret; //作为消息向量结果返回

    while (getline(ss, tok, del))
    {
        if (tok > "")
            ret.push_back(tok);
    }
    return ret;
}

pair<string, string> splitStr(string s)
{
    // 分离put操作的key-value
    vector<string> str_K_V = split(s, ':');
    pair<string, string> res;
    res.first = str_K_V[0];
    res.second = str_K_V[1];
    return res;
}

inline int mySocket()
{
    int conndfd;
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(6666);
    conndfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    connect(conndfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    return conndfd;
}

void packFunc()
{
    char buf[MAXLINE];
    int n;

    vector<int> conndfd(20);
    for (int i = 0; i < 20; i++)
    {
        conndfd[i] = mySocket();
    }
    // cout << "connection done";
    int count = 0;
    while (fgets(buf, MAXLINE, stdin))
    {
        string str = buf;
        // cout << str << endl;
        // getKeyValue(str, key, value);
        string msg = genPutMsg(buf);
        bzero(buf, MAXLINE);
        for (int i = 0; i < msg.length(); i++)
        {
            buf[i] = msg[i];
        }
        // strcpy(KV_.KEY, (char *)key.data());
        // strcpy(KV_.VALUE, (char *)value.data());
        // TLV_EncodeCat(&KV_, buf);

        write(conndfd[count], buf, sizeof(buf) - 1);

        bzero(buf, MAXLINE);
        /*
        while ((n = read(conndfd[count], buf, MAXLINE) > 0))
        {
            fputs(buf, stdout);
            break;
        }
        if (++count == 20)
        {
            count = 0;
        }
        bzero(buf, MAXLINE);
        */
    }
    
    // close(conndfd);
}

int main(int argc, char *argv[])
{
    thread threadFunc[THREADNUD];
    for (int i = 0; i < THREADNUD; i++)
    {
        threadFunc[i] = thread(bind(packFunc));
    }

    for (int i = 0; i < THREADNUD; i++)
    {
        threadFunc[i].join();
    }

    cout << "done";
    //boost::thread threadFunc_1(boost::bind(packFunc));
    //threadFunc_1.join();

    return 0;
}