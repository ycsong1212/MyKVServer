
/*
void handleAllMsg(string str, int sockfd)
{
    cout << str << endl;
    vector<string> msgs = split(str, ' ');
    cout << msgs.size() << endl;
    for (int i = 0; i < msgs.size() - 1; i++)
    {
        cout << msgs[i] << "  " << msgs[i].length();
        if (msgs[i] == "exit")
        {
            cout << "client exiting" << endl;
            close(sockfd);
            // return NULL;
            return;
        }
        str_echo(msgs[i], sockfd);
    }
    if (msgs[msgs.size() - 1] == "exit")
    {
        cout << "client exiting" << endl;
        close(sockfd);
        // return NULL;
        return;
    }
    // cout << msgs[msgs.size() - 1] << endl;
    // cout << msgs[msgs.size() - 1].length() << endl;
    if (msgs[msgs.size() - 1].length() <= 5 || getDataLen(msgs[msgs.size() - 1]) != msgs[msgs.size() - 1].length() - 4 - to_string(getDataLen(msgs[msgs.size() - 1])).length())
    {
        cout << "Nooooooooooooooo";
        cout << getDataLen(msgs[msgs.size() - 1]) << "  " << msgs[msgs.size() - 1].length() << "  " << to_string(getDataLen(msgs[msgs.size() - 1])).length() << endl;
        cout << msgs[msgs.size() - 1];
        // return msgs[msgs.size() - 1];
    }

    else
    {
        // cout << "hello,world_前";
        str_echo(msgs[msgs.size() - 1], sockfd);
        // cout << "hello,world_后";
        // return NULL;
    }
}
*/

/*
void handleAll(string str, int sockfd)
{
    vector<string> msgs = split(str, ' ');
    for (int i = 0; i < msgs.size(); i++)
    {
        if (msgs[i] == "exit")
        {
            cout << "client exiting" << endl;
            close(sockfd);
            return;
        }
        str_echo(msgs[i], sockfd);
    }
}
*/

/*
void handleAllMsg(string str, int sockfd)
{
    // thread t_h(str_echo);
    vector<string> msgs = split(str, ' ');
    cout<<msgs.size()<<endl;
    for (int i = 0; i < msgs.size() - 1; i++)
    {
        if (msgs[i] == "exit")
        {
            cout << "client exiting" << endl;
            close(sockfd);
            // return "";
            ss = "";
            return;
        }
        str_echo(msgs[i], sockfd);
    }
    if (msgs[msgs.size() - 1] == "exit")
    {
        cout << "client exiting" << endl;
        close(sockfd);
        // return "";
        ss = "";
        return;
    }
    // cout << msgs[msgs.size() - 1] << endl;

    if (!contain_(msgs[msgs.size() - 1]))
    {
        cout << "Nooooooooooooooo";
        ss = msgs[msgs.size() - 1];
        //return;
        // return msgs[msgs.size() - 1];
    }

    if (checkLength(msgs[msgs.size() - 1]))
    {
        cout << "Nooooooooooooooo";
        ss = msgs[msgs.size() - 1];
        cout << ss << "  " << ss.length() << endl;
        // return;
        // return msgs[msgs.size() - 1];
    }

    else
    {
        // ss = "";
        str_echo(msgs[msgs.size() - 1], sockfd);
        return;
        // return "";
    }

    // sleep(100);
}
*/

/*
string handleAllMsg(vector<string> msgs, int sockfd)
{
    // 处理消息向量中的所有有效消息
    for (int i = 0; i < msgs.size(); i++)
    {
        if (msgs[i] == "exit")
        {
            // 如果收到客户端发送退出信号，断开连接
            cout << "client exiting" << endl;
            close(sockfd);
            ss = "";
            // return;  
        }
        str_echo(msgs[i], sockfd);  //如果不是退出信号，进行单个消息的处理
    }
    return "aaa";
}
*/

/*
    // vector<string> msgs = split(s_R, ' ');  // 划分为消息向量
    for (int i = 0; i < msgs.size(); i++)
        cout << msgs[i] << " ";        
    if (msgs.size() > 0)
    {
        // 检测最后一个消息是否是有效消息，保留不完整部分存放到下一批初始化
        if (!checkLastMsg(msgs))
        {
            cout << "Nooooooooooooooo" << endl;
            ss = msgs[msgs.size() - 1];
            msgs.pop_back();
        }
        // std::future<void> f = worker.Submit(std::bind(&handleAllMsg, msgs, sockfd));  // 在线程池指派线程进行处理函数
        std::future<string> f = worker.Submit(std::bind(&handleAllMsg, msgs, sockfd));  // 在线程池指派线程进行处理函数
        cout<<f.get()<<endl;
    }    
*/