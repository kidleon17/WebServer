#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"

using namespace std;

class Log {
public:
	static Log *get_instance {
		static Log instance;
		return &instance;
	}

	static void *flush_log_thread(void *args) {
		Log::get_instance()->async_write_log();
	}

	bool init(const char* file_name,int close_log,int log_buf_size=8192,int split_lines=5000000,int max_queue_size = 0);

	void write_log(int level,const char *format,...);//... 参数占位符,表示可变参数，参数的个数跟类型都不确定

	void flush(void);

private:
	Log();
	virtual ~Log() {
		string single_log;
		//取出一个日志string写入文件
		while (m_log_queue->pop(single_log)) {
			m_mutex.lock();
			fputs(single_log.c_str(),m_fp);
			m_mutex.unlock();
		}
	}
private:
	char dir_name[128];//路径名
	char log_name[128];//log文件名
	int m_split_lines; //日志的最大条数
	int m_log_buf_size; //日志缓冲区的大小
	long long m_count;//日志行数的记录
	int m_today; //记录日志时间
	FILE *m_fp;//打开log指针
	char *m_buf;
	block_queue<string> *m_log_queue;//阻塞队列
	bool m_is_async; //是否同步的标志位
	loocker m_mutex;
	int m_close_log;//关闭日志
};

#define LOG_DEBUG(format,...) if(0==m_close_log) {Log::get_instance()->write_log(0,format,##__VA_ARGS__);Log::get_instance()->flush;}
#define LOG_INFO(format,...) if(0==m_close_log) {Log::get_instance()->write_log(1,format,##__VA_ARGS__);Log::get_instance()->flush;}
#define LOG_WARN(format,...) if(0==m_close_log) {Log::get_instance()->write_log(2,format,##__VA_ARGS__);Log::get_instance()->flush;}
#define LOG_IERROR(format,...) if(0==m_close_log) {Log::get_instance()->write_log(3,format,##__VA_ARGS__);Log::get_instance()->flush;}
#endif