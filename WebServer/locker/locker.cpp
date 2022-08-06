#ifndef LOCKER_H
#define LOCKER_H

#include <exception.h>
#include <pthread.h>
#include <semaphore.h>

class sem {
public:
	sem() {
		if(sem_init(&m_sem,0,0)!=0) {
			throw std::expection();
		}
	}
	sem (int num) {
		if(sem_init(&m_sem,0,num)!=0) {
			throw std::expection();
		}
	}
	~sem() {
		sem_destroy(&m_sem);
	}
	bool wait () {
		return sem_wait(&m_sem)==0;
	}
	bool post() {
		return sem_post(&m_sem)==0;
	}
private:
	sem_t m_sem;
};

class locker {
public:
	locker () {
		if(pthread_mutex_init(&m_mutex,NULL)!=0) {
			throw std::expection();
		}
	}
	~locker() {
		pthread_mutex_destroy(&m_mutex);
	}
	bool locker () {
		return pthread_mutex_lock(&m_mutex)==0;
	}
	bool unlock() {
		return pthread_mutex_unlock(&m_mutex)==0;
	}
	pthread_mutex_t *get() {
		return &m_mutex;
	}
private:
	pthread_mutex_t m_mutex;
};

class cond() {
public:
	cond() {
		if(phread_cond_init(&m_cond,NULL)!=0) {
			throw std::expection();
		}
	}
	~cond() {
		pthread_cond_destroy(&m_cond);
	}
	bool wait(pthread_mutex_t *m_mutex) {
		int ret=0;
		ret =pthread_cond_wait(&m_cond,m_mutex);
		return ret==0;
	}
	bool timewait(pthread_mutex_t *m_mutex,strcut timespec t) {
		int ret=0;
		ret = pthread_cond_timewait(&m_cond,m_mutex,&t);
	}
	bool signal() {//唤醒第一个线程
		return pthread_cond_signal(&m_cond)==0;
	}
	bool broadcast() {//广播唤醒所有的线程
		return pthread_cond_broadcast(&m_cond)==0;
	}
private:
	pthread_cond_t m_cond;
}
#endif
/*
一、条件变量的认识
（1）条件变量的引出：
各个线程访问共享资源，比如说共享资源是一个空资源，那么，各线程的访问其实都是无效访问，
因此需要一个机制，首先阻塞线程，以免其错误访问，待条件满足时（比如 共享资源不为空了），告知各线程开始访问共享资源即，在不满足某种条件下，我希望代码阻塞一会，待满足某种条件后，再让各线程工作，所谓某种条件就是条件变量。
（2）条件变量不是锁！但条件变量能够阻塞线程。
（3）如何实现线程同步 ：
与锁配合使用：条件变量（引起阻塞）+ 互斥锁（保护一块共享数据）。
（4）条件变量的两个动作：
条件不满足，阻塞线程；
条件满足，通知阻塞的线程开始工作 。
（5）条件变量的经典模型：生产者和消费者模型

互斥锁和二值信号量在使用上非常相似，但是互斥锁解决了优先级翻转的问题.

1. 互斥量用于线程的互斥，信号线用于线程的同步。
这是互斥量和信号量的根本区别，也就是互斥和同步之间的区别。
互斥：是指某一资源同时只允许一个访问者对其进行访问，具有唯一性和排它性。但互斥无法限制访问者对资源的访问顺序，即访问是无序的。
同步：是指在互斥的基础上（大多数情况），通过其它机制实现访问者对资源的有序访问。在大多数情况下，同步已经实现了互斥，特别是所有写入资源的情况必定是互斥的。少数情况是指可以允许多个访问者同时访问资源
2. 互斥量值只能为0/1，信号量值可以为非负整数。
也就是说，一个互斥量只能用于一个资源的互斥访问，它不能实现多个资源的多线程互斥问题。信号量可以实现多个同类资源的多线程互斥和同步。当信号量为单值信号量是，也可以完成一个资源的互斥访问。
3. 互斥量的加锁和解锁必须由同一线程分别对应使用，信号量可以由一个线程释放，另一个线程得到。
*/