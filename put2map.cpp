#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#define MAXLINE 100

using namespace std;

int main(int argc, char **argv)
{
    int sockfd, n;
    struct sockaddr_in servaddr;

    char buff[8] = {0};
    if (argc != 2)
    {
        printf("usage: ./client <ipaddress>\n");
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

    for (int i = 1; i < 10000000; i++)
    {
        string str_K = to_string(i);
        string str_V = to_string(i * 2 + 1);
        if (i % 500 == 0)
            cout << str_K << "   " << str_V << endl;
        string send_Msg = "put" + to_string(str_K.length() + str_V.length() + 1) + "_" + str_K + ":" + str_V + " ";
        n = send(sockfd, send_Msg.c_str(), send_Msg.length(), 0);
        // cout << n << endl;
        if (n < 0)
        {
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }
        if (n == 0)
            cout << "aoligei";
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
