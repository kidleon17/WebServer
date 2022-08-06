日志:由服务器自动创建，并记录运行状态，错误信息，访问数据的文件。

同步日志，日志写入函数与工作线程串行执行，由于涉及到I/O操作，当单条日志比较大的时候，
同步模式会阻塞整个处理流程，服务器所能处理的并发能力将有所下降，尤其是在峰值的时候，
写日志可能成为系统的瓶颈。

生产者-消费者模型，并发编程中的经典模型。以多线程为例，
为了实现线程间数据同步，生产者线程与消费者线程共享一个缓冲区，
其中生产者线程往缓冲区中push消息，消费者线程从缓冲区中pop消息。
阻塞队列，将生产者-消费者模型进行封装，使用循环数组实现队列，作为两者共享的缓冲区。
异步日志，将所写的日志内容先存入阻塞队列，写线程从阻塞队列中取出内容，写入日志。
单例模式，最简单也是被问到最多的设计模式之一，保证一个类只创建一个实例，
同时提供全局访问的方法。

单例模式
单例模式作为最常用的设计模式之一，保证一个类仅有一个实例，
并提供一个访问它的全局访问点，该实例被所有程序模块共享。

实现思路：私有化它的构造函数，以防止外界创建单例类的对象；
使用类的私有静态指针变量指向类的唯一实例，并用一个公有的静态方法获取该实例。

单例模式有两种实现方法，分别是懒汉和饿汉模式。顾名思义，懒汉模式，
即非常懒，不用的时候不去初始化，所以在第一次被使用时才进行初始化；饿汉模式，
即迫不及待，在程序运行时立即初始化。

//经典的线程安全懒汉模式:
class single{
private:
    //私有静态指针变量指向唯一实例
    static single *p;

    //静态锁，是由于静态函数只能访问静态成员
    static pthread_mutex_t lock;

    //私有化构造函数
    single(){
        pthread_mutex_init(&lock, NULL);
    }
    ~single(){}

public:
    //公有静态方法获取实例
    static single* getinstance();

};

pthread_mutex_t single::lock;

single* single::p = NULL;
single* single::getinstance(){
    if (NULL == p){
        pthread_mutex_lock(&lock);
        if (NULL == p){
            p = new single;
        }
        pthread_mutex_unlock(&lock);
    }
    return p;
}

为什么要用双检测，只检测一次不行吗？
如果只检测一次，在每次调用获取实例的方法时，都需要加锁，这将严重影响程序性能。
双层检测可以有效避免这种情况，仅在第一次创建单例的时候加锁，
其他时候都不再符合NULL == p的情况，直接返回已创建好的实例。

局部静态变量之线程安全懒汉模式:

class single{
private:
    single(){}
    ~single(){}

public:
    static single* getinstance();

};

single* single::getinstance(){
    static single obj;
    return &obj;
}

class single{
private:
   	static pthread_mutex_t lock;
    single(){
        pthread_mutex_init(&lock, NULL);
   }
    ~single(){}

public:
   static single* getinstance();

};
pthread_mutex_t single::lock;
single* single::getinstance(){
    pthread_mutex_lock(&lock);
    static single obj;
    pthread_mutex_unlock(&lock);
   	return &obj;
}

饿汉模式
饿汉模式不需要用锁，就可以实现线程安全。原因在于，在程序运行时就定义了对象，
并对其初始化。之后，不管哪个线程调用成员函数 getinstance()，都只不过是返回一个对象的
指针而已。所以是线程安全的，不需要在获取实例的成员函数中加锁。

class single{
private:
    static single* p;
    single(){}
    ~single(){}

public:
   static single* getinstance();

};
single* single::p = new single();
single* single::getinstance(){
    return p;
}

//测试方法
int main(){

    single *p1 = single::getinstance();
    single *p2 = single::getinstance();

    if (p1 == p2)
        cout << "same" << endl;
    system("pause");
    return 0;
}


饿汉模式虽好，但其存在隐藏的问题，在于非静态对象（函数外的static对象）
在不同编译单元中的初始化顺序是未定义的。如果在初始化完成之前调用 getInstance() 
方法会返回一个未定义的实例。

异步日志
我们考虑设计一个日志队列，这个队列主要是用一个循环数组模拟队列来存储日志，
这里要注意这个队列只是存储我们真正的目的是要写到文件里，所以只是存储并未达到目的。
但是考虑到文件IO操作是比较慢的，所以我们采用的异步IO就是先写到内存里，
然后日志线程自己有空的时候写到文件里。