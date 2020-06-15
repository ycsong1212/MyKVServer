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

#define MAXLINE 4096

using namespace std;

string genPutMsg(string msg)
{
    return "put" + to_string(msg.length()) + "_" + msg + " ";
}

string genGetMsg(string msg)
{
    return "get" + to_string(msg.length()) + "_" + msg + " ";
}

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in servaddr;

    if (argc < 2)
    {
        // printf("usage: ./client <ipaddress>\n");
        cout << "IP address required!" << endl;
        return 0;
    }

    if (argc > 2)
    {
        cout << "too many parameters!" << endl;
        return 0;
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(6666);
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        printf("inet_pton error for %s\n", argv[1]);
        return 0;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        printf("connect error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    while (true)
    {
        char op;
        cout << "请输入您要进行的操作，1表示查询，2表示插入，3表示退出:";
        cin >> op;
        if (op != '1' && op != '2' && op != '3')
        {
            cout << "请输入1，2或3作为您想进行的操作";
            continue;
        }

        if (op == '3')
        {
            cout << "exiting..." << endl;
            char a[6] = "exit ";
            if (n = send(sockfd, a, 5, 0) < 0)
            //if (n = write(sockfd, a, 4) < 0)
            {
                printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                return 0;
            }
            break;
        }

        else if (op == '2')
        {
            string path;
            cout << "Enter the file name you are willing to handle: ";
            cin >> path;
            fstream f_File(path);
            if (f_File.peek() == EOF)
            {
                cout << "file not exist";
            }
            string line;
            while (getline(f_File, line))
            {
                string sendline = genPutMsg(line);
                if (send(sockfd, sendline.c_str(), sendline.length(), 0) < 0)
                // if (write(sockfd, send_Msg.c_str(), send_Msg.length()) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    return 0;
                }
                cout << "input success" << endl;
            }
        }

        else
        {
            char buff[MAXLINE] = {0};
            string path;
            cout << "Enter the file name you are willing to handle: ";
            cin >> path;
            fstream f_File(path);
            if (f_File.peek() == EOF)
            {
                cout << "file not exist";
            }
            string line;
            while (getline(f_File, line))
            {
                bzero(buff, sizeof(buff));
                string sendline = genGetMsg(line);
                if (send(sockfd, sendline.c_str(), sendline.length(), 0) < 0)
                // if (write(sockfd, send_Msg.c_str(), send_Msg.length()) < 0)
                {
                    printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
                    return 0;
                }
                if (recv(sockfd, buff, MAXLINE, 0) < 0)
                {
                    printf("receive msg error: %s(errno: %d)\n", strerror(errno), errno);
                    return 0;
                }
                if (strcmp(buff, "fail") == 0)
                    cout << "No matching" << endl;
                else
                    cout << "The result is " << buff << endl;
            }
        }
    }

    cout << "exiting..." << endl;
    char a[6] = "exit ";
    if (n = send(sockfd, a, 5, 0) < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
    close(sockfd);
    return 0;
}