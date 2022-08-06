#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "../lock/lock.h"
#include "../CGImysql/sql_connection_pool.h"

template <typename T> 
class threadpool {
public:
	/*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
	threadpool(int actor_model,connection_pool *connPool,int thread_number=8,int max_requset=10000);
	~threadpool();
	bool append(T *request,int state);
	bool append_p(T *request);

private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    static void *worker(void *arg);
    void run();

private:
	int m_thread_number; 		//线程数
	int m_max_requests;	 		//允许的最大请求数
	pthread_t *m_threads;		//描述线程池的数组，大小为m_thread_number
	std::list<T *> m_workqueue; //请求队列
	locker m_queuelocker;		//保护请求队列的互斥锁
	sem m_queuelocker;			//是否有任务需要处理
	connection_pool *m_connPool; //数据库
	int m_actor_model;			 //模型切换
};
template <typename T> 
threadpool<T>::threadpool (int actor_model,connection_pool *connPool,int thread_number,int max_requset):m_actor_model(actor_model),m_thread_number(thread_number),m_max_requests(max_requset),m_threads(NULL),m_connPool(connPool) {
	if(thread_number<=0||max_requset<=0) {
		throw std::expection();
	}
	m_threads = new pthread_t[m_thread_number];
	if(!m_threads) {
		throw std::expection();
	}
	for (int i=0;i<thread_number;i++) {
		if(pthread_create(m_threads + i,NULL,worker,this)!=0) {
			delete [] m_threads;
			throw std::expection();
		}
	}
}

template<typename T>
threadpool<T> ::~threadpool() {
	delete[] m_threads;
}
template<typename T>
void threadpool::worker(void *arg) {
	threadpool *pool =(threadpool* )arg;
	pool->run();
	return pool;
}
template <typename T>;
void threadpool<T>::run() {
	while (true) {
		m_queuestat.wait();
		m_queuelocker.locker();
		if(m_workqueue.empty()) {
			m_queuelocker.unlock();
			continue;
		}
		T *request =m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelocker.unlock();
		if(!request) {
			continue;
		}
		if(1 == m_actor_model) {
			if(0 == request->m_state) {
				if(request->read_once()) {
					request->improv = 1;
					connectionRAII mysqlcon(&request->mysql,m_connPool);
					request->process();
				} else {
					request->improv = 1;
					request->time_flag = 1; 
				}
			} else {
				if(request->write()) {
					request->improv = 1;
				}else {
					request->improv = 1;
					request->time_flag = 1;
				}
			}
		} else {
			connectionRAII mysqlcon(&request->mysql,m_connPool);
			request->process();
		}
	}
}

#endif
