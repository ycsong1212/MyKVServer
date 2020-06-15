#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include "assert.h"
#include <iostream>

namespace ThreadPool
{
#define MAX_THREAD_NUM 8

	//线程池，可以提交变参函数或lamada表达式的匿名函数执行，可以获取执行返回值
	//֧支持类成员函数，支持类静态成员函数或全局函数，Operator函数等
	class ThreadPool
	{
		typedef std::function<void()> Task;

	private:
		std::vector<std::thread> m_pool;  // 线程池
		std::queue<Task> m_tasks;		  // 任务队列
		std::mutex m_lock;				  // ͬ同步锁
		std::condition_variable m_cv;	  // 条件阻塞
		std::atomic<bool> m_isStoped;	  // 是否关闭提交
		std::atomic<int> m_idleThreadNum; //空闲线程数量
	public:
		ThreadPool(int size = MAX_THREAD_NUM)
		{
			m_isStoped=false;
			size = size > MAX_THREAD_NUM ? MAX_THREAD_NUM : size;
			m_idleThreadNum = size;
			for (int i = 0; i < size; i++)
			{
				//初始化线程数量
				m_pool.emplace_back(&ThreadPool::scheduler, this);
			}
		}
		~ThreadPool()
		{
			Close();
			while (!m_tasks.empty())
			{
				m_tasks.pop();
			}
			m_cv.notify_all(); // 唤醒所有线程执行
			for (std::thread &thread : m_pool)
			{
				if (thread.joinable())
				{
					thread.join(); // 等待任务结束，前提是线程可以执行完
				}
			}
			m_pool.clear();
		}
		// 打开线程池，重启任务提交
		void ReOpen()
		{
			if (m_isStoped)
				m_isStoped.store(false);
			m_cv.notify_all();
		}
		// 关闭线程池，停止提交新任务
		void Close()
		{
			if (!m_isStoped)
				m_isStoped.store(true);
		}
		// 判断线程池是否被关闭
		bool IsClosed() const
		{
			return m_isStoped.load();
		}
		// 获取当前任务队列中的任务数
		int GetTaskSize()
		{
			return m_tasks.size();
		}
		// 获取当前空闲线程数
		int IdleCount()
		{
			return m_idleThreadNum;
		}
		// 提交任务并执行
		// 调用方式为：std::future<returnType> var = threadpool.Submit(...)
		// var.get() 会等待任务执行完，并获取返回值
		// 其中...可以直接用函数名+函数参数代替，例如threadpool.Submit(f, 0, 1)
		// 如果要调用类成员函数，最好用如下方式：
		// threadpool.Submit(std::bind(&Class::Func, &classInstance)) 或
		// threadpool.Submit(std::mem_fn(&Class::Func), &classInstance)
		template <class F, class... Args>
		auto Submit(F &&f, Args &&... args) -> std::future<decltype(f(args...))>
		{
			using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, 函数f的返回值类型
			std::shared_ptr<std::packaged_task<RetType()>> task = std::make_shared<std::packaged_task<RetType()>>(
				std::bind(std::forward<F>(f), std::forward<Args>(args)...));
			std::future<RetType> future = task->get_future();
			// 封装任务并添加到队列
			addTask([task]() {
				(*task)();
			});
			return future;
		}

	private:
		// 消费者
		Task getTask()
		{
			std::unique_lock<std::mutex> lock(m_lock); // unique_lock ��� lock_guard �ĺô��ǣ�������ʱ unlock() �� lock()
			while (m_tasks.empty() && !m_isStoped)
			{
				m_cv.wait(lock);
			} // wait ֱ直到有 task
			if (m_isStoped)
			{
				return Task();
			}
			if(m_tasks.empty())
			{
				return NULL;
			}

			Task task = std::move(m_tasks.front()); // ȡһ�� task
			m_tasks.pop();
			m_cv.notify_one();
			return task;
		}

		// 生产者
		void addTask(Task task)
		{
			std::lock_guard<std::mutex> lock{m_lock}; //对当前快的语句加 lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候unlock()
			m_tasks.push(task);
			m_cv.notify_one(); // 唤醒一个线程执行
			std::cout<<"线程数"<<m_idleThreadNum<<std::endl;
		}

		// 工作线程主循环函数
		void scheduler()
		{
			// std::cout << "bbb  " << m_isStoped << "  " <<  m_isStoped.load() << std::endl;
			while (!m_isStoped.load())
			{
				// 获取一个待执行的task
				Task task(getTask());
				if (task)
				{
					m_idleThreadNum--;
					task();
					m_idleThreadNum++;
				}
			}
		}
	};
} // namespace ThreadPool

#endif