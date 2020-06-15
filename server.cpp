#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/epoll.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <algorithm>

#include <condition_variable>
#include <functional>
#include <cassert>
#include <memory>
#include "threadpool.h"
#include "helper.h"
// #include <sys/wait.h>

#define MAXLINE 4096
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 500
#define INFTIM 1000

using namespace std;

mutex mtx;
static map<string, string> usrs;
static map<int, string> remains; //存储文件描述符与一次缓冲区读取多出来的字符串

void setnonblocking(int sock);              // 设置非阻塞模式啊
vector<string> split(string str, char del); // 按del字符分割字符串
bool contain_(string str);                  // 检测消息字符串是否包含_
int getDataLen(string str);                 // 获取操作字符串的长度
bool checkLength(string str);               // 插入字符串长度与消息字符串长度是否符合对应关系
bool checkMsg(string str);
bool checkLastMsg(vector<string> msgs); // 检测消息队列的最后一条消息是否完整，若完整正常执行，否则保存不完整的消息到全部变量
string getSubstrOfRecv(string str);     //获取消息字符串中的待操作字符串
void handleAllMsg(string str, int sockfd);
// string handleAllMsg(vector<string> msgs, int sockfd); //处理缓冲区的全部消息
//  void handleAllMsg(vector<string> msgs, int sockfd); //处理缓冲区的全部消息
void str_echo(string str, int sockfd);   //单个消息操作
pair<string, string> splitStr(string s); //建立待插入K-V对
string getLastStr(string str);
// void handleAll(string str, int sockfd);

string getLastStr(string str)
{
    string res = "";
    int i = str.length() - 1;
    while (str[i] != ' ')
    {
        res.append(1, str[i]);
        i--;
    }
    reverse(res.begin(), res.end());
    return res;
}

void setnonblocking(int sock)
{
    // 设置非阻塞
    int opts;
    opts = fcntl(sock, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(sock,GETFL)");
        exit(1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, opts) < 0)
    {
        perror("fcntl(sock,SETFL,opts)");
        exit(1);
    }
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

bool contain_(string str)
{
    // 检验字符串是否包含_
    for (int i = 0; i < str.length(); i++)
    {
        if (str[i] == '_')
            return true;
    }
    return false;
}

int getDataLen(string str)
{
    //在有效消息中获取带操作部分的长度
    string tmp = "";
    for (int i = 3; str[i] != '_'; i++)
    {
        tmp += str[i];
    }
    return atoi(tmp.c_str());
}

bool checkLength(string str)
{
    //检查待处理字符串长度与消息字符串长度是否符合对应关系
    return getDataLen(str) == str.length() - 4 - to_string(getDataLen(str)).length();
}

bool checkMsg(string str)
{
    // 检测是否是有效消息
    if (str == "exit")
        return true;
    if (!contain_(str))
        return false;
    if (!checkLength(str))
        return false;
    return true;
}

bool checkLastMsg(vector<string> msgs)
{
    // 检测消息向量的最后一条消息是否是可处理的消息
    if (msgs.size() <= 0)
        return false;
    return checkMsg(msgs[msgs.size() - 1]);
}

string getSubstrOfRecv(string str)
{
    //获取服务器需要处理的字符串部分
    return str.substr(4 + to_string(getDataLen(str)).length());
}

void handleAllMsg(string str, int sockfd)
{
    // 处理缓冲区的全部可处理的消息，若中间消息有误跳过并写入日志，最后一条更新到remains向量中
    vector<string> msgs = split(str, ' ');
    // cout << str << endl;
    if (msgs.size() > 0)
    {
        for (int i = 0; i < msgs.size(); i++)
        {
            // cout << msgs[i] << "  " << msgs[i].length();
            if (msgs[i] == "exit")
            {
                cout << "client exiting" << endl;
                close(sockfd);
                // unique_lock<mutex> lck(mtx2);
                remains.erase(sockfd);
                return;
            }
            str_echo(msgs[i], sockfd);
        }
    }
}

void str_echo(string str, int sockfd)
{
    // 处理单个消息的方法
    if (str.substr(0, 3) == "get")
    {
        //如果是get指令，获取get到的内容对应的值，若不存在发送错误消息
        // cout << str << endl;
        cout << "The result is :" << usrs[getSubstrOfRecv(str)] << endl;
        if (usrs[getSubstrOfRecv(str)].length() == 0)
        {
            // 如果没有value，发送fail
            if (send(sockfd, "fail", 4, 0) < 0)
            {
                printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                return;
            }
        }
        else
        {
            //如果有，发送对应的value到指定客户端
            if (send(sockfd, usrs[getSubstrOfRecv(str)].c_str(), strlen(usrs[getSubstrOfRecv(str)].c_str()), 0) < 0)
            {
                printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                return;
            }
        }
    }
    else if (str.substr(0, 3) == "put")
    {
        // 将待插入内容插入map中
        unique_lock<mutex> lck(mtx);
        usrs.insert(splitStr(getSubstrOfRecv(str)));
        // cout << "insert success!" << endl;
    }
    else
    {
        // 如果是无效消息，跳过，写入日志
        cout << "hello,world不gp" << endl;
        return;
    }
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

int main(int argc, char **argv)
{
    ThreadPool::ThreadPool worker{20}; // 产生一个包含8个线程的线程池
    //初始化往usrs插入一些值
    usrs.insert(pair<string, string>("song", "excellent"));
    usrs.insert(pair<string, string>("hu", "sucks"));
    usrs.insert(pair<string, string>("gay", "ahahahahjjjuyyihf"));

    int listenfd;
    int epfd, nfds;
    struct sockaddr_in servaddr;
    struct epoll_event ev, events[20];
    // ssize_t n;

    epfd = epoll_create(256);
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    ev.data.fd = listenfd;
    ev.events = EPOLLIN;
    // ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    if (listen(listenfd, 10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("======waiting for client's request======\n");

    struct sockaddr_in clientaddr;
    socklen_t clilen = sizeof(struct sockaddr);

    while (1)
    {
        // struct sockaddr_in client_addr;
        // socklen_t length = sizeof(client_addr);
        nfds = epoll_wait(epfd, events, 20, 500);
        for (int i = 0; i < nfds; ++i)
        {
            if (events[i].data.fd == listenfd)
            {
                // 如果没有记录，添加到事件数组中
                int connfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);
                if (connfd < 0)
                {
                    perror("connfd<0");
                    exit(1);
                }
                setnonblocking(connfd); // 设置为非阻塞
                remains[connfd] = "";   //向remains数组添加连接与消息遗留字符串的键值对
                char *str = inet_ntoa(clientaddr.sin_addr);
                cout << "accapt a connection from " << str << endl;

                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                //ev.events=EPOLLIN;

                epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev); //注册新的epoll信息
                // cout << connfd << endl;
            }

            else if (events[i].events & EPOLLIN)
            {
                char recvBuf[MAXLINE] = {0}; // 一个事件建立一个缓冲区
                int sockfd = events[i].data.fd;
                ssize_t n;
                if (sockfd < 0)
                    continue;
                // 将不完整的字符串部分粘贴到缓冲区
                for (int j = 0; j < remains[sockfd].length(); j++)
                    recvBuf[j] = remains[sockfd][j];

                cout << "EPOLLIN" << endl;
                n = recv(sockfd, recvBuf + remains[sockfd].length(), MAXLINE - 1 - remains[sockfd].length(), 0);
                if (n < 0) // 接收client发送的消息
                {
                    if (errno == ECONNRESET)
                    {
                        close(sockfd);
                        events[i].data.fd = -1;
                    }
                    else
                        cout << "recvBuf error" << endl;
                }

                else if (n == 0)
                {
                    close(sockfd);
                    events[i].data.fd = -1;
                }

                string s_R; // 将缓冲区内容转化为字符串
                for (int k = 0; k < strlen(recvBuf); k++)
                    s_R.append(1, recvBuf[k]);

                string tmp = getLastStr(s_R);
                if (tmp.length() > 0)
                {
                    remains[sockfd] = tmp;
                }
                else
                {
                    remains[sockfd] = "";
                }

                future<void> f = worker.Submit(std::bind(&handleAllMsg, s_R.substr(0, s_R.length() - tmp.length() - 1), sockfd)); //分配线程处理任务

                // ev.data.fd=sockfd;
                // ev.events=EPOLLOUT|EPOLLET;
                // epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }

            /*
            else if (events[i].events & EPOLLOUT)
            {
                sockfd = events[i].data.fd;
                // write(sockfd, recvBuf, n);
                cout << "write!!" << endl;
                //ev.data.fd=sockfd;
                //ev.events=EPOLLIN|EPOLLET;
                //epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);
            }
            */
        }
    }
    cout << "closed";
    close(listenfd);
    close(epfd);

    return 0;
}