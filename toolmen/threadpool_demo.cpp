// ThreadPool.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h"
#include "threadpool.h"
#include <iostream>

struct gfun {
	int operator()(int n) {
		printf("%d  hello, gfun !  %d\n", n, std::this_thread::get_id());
		return 42;
	}
};

class Test
{
public:
	int GetThreadId(std::string a, double b)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		std::thread::id i = std::this_thread::get_id();
		std::cout << "In Test, thread id: " << i << std::endl;
		std::cout << "a: " << a.c_str() << ", b = " << b << std::endl;
		// return i.hash();
		return 0;
	}
};

void fun(int n)
{
    std::cout<<"nononononononono!!!!!!!cnm";
    int res=1;
    for(int i=1;i<=n;i++)
    {
        res*=i;
    }
    std::cout<<"result="<<res<<std::endl;
}

int main()
{
	ThreadPool::ThreadPool worker{ 4 };
	Test t;
	std::cout << "at the beginning: " << std::endl;
	std::cout << "idle threads: " << worker.IdleCount() << std::endl;
	std::cout << "tasks: " << worker.GetTaskSize() << std::endl;
	std::future<void> f1 = worker.Submit(std::bind(&fun, 5));

	std::cout << "after submit 1 task: " << std::endl;
	std::cout << "idle threads: " << worker.IdleCount() << std::endl;
	std::cout << "tasks: " << worker.GetTaskSize() << std::endl;
	std::future<void> f2 = worker.Submit(std::bind(&fun, 7));

	std::cout << "after submit 2 task: " << std::endl;
	std::cout << "idle threads: " << worker.IdleCount() << std::endl;
	std::cout << "tasks: " << worker.GetTaskSize() << std::endl;
	// std::future<int> f3 = worker.Submit(gfun{}, 0);

	// std::cout << "f1 = " << f1.get() << ", f2 = " << f2.get() << ", f3 = " << f3.get() << std::endl;

	std::cout << "after all task: " << std::endl;
	std::cout << "idle threads: " << worker.IdleCount() << std::endl;
	std::cout << "tasks: " << worker.GetTaskSize() << std::endl;
	// system("pause");
	return 0;
}