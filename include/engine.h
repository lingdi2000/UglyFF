#ifndef _ENGINE_H_
#define _ENGINE_H_
#include "macro.h"
#include <assert.h>
//#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <queue>
//#include <xhash>
//#include <ext/hash_map>
#include<unordered_map>
//#include <hash_multimap>
#include <functional>
#include <algorithm>
#include <iostream>

#include <pthread.h>
#include <time.h>
#include <sys/time.h> 

#include<string.h>

//linux 网络
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h> // ntohs 等接口
#include<sys/epoll.h>

#include<netinet/in.h> // sockaddr_in
#include<unistd.h> //close


#include<log4cplus/logger.h>
#include<log4cplus/appender.h>
#include<log4cplus/layout.h>
#include<log4cplus/fileappender.h>
#include<log4cplus/loggingmacros.h> 


#include "csCommon.h"
/* comment time 2016/12/21 19:13 author:yld
	加密
*/
class CEncrypt
{
//	CEncrypt();
//	enum encMethod
//	{
//		ENCDEC_NONE,
//		ENCDEC_DES,
//		ENCDEC_RC5
//	};
//	void random_key_des(ZES_cblock *ret);
//	void set_key_des(const_ZES_cblock *key);
//	void set_key_rc5(const BYTE *data, int nLen, int rounds);
//	int encdec(void *data, DWORD nLen, bool enc);
//
//	void setEncMethod(encMethod method);
//	encMethod getEncMethod() const;
//
//private:
//	void ZES_random_key(ZES_cblock *ret);
//	void ZES_set_key(const_ZES_cblock *key, ZES_key_schedule *schedule);
//	void ZES_encrypt1(ZES_LONG *data, ZES_key_schedule *ks, int enc);
//
//	void RC5_32_set_key(RC5_32_KEY *key, int len, const BYTE *data, int rounds);
//	void RC5_32_encrypt(RC5_32_INT *d, RC5_32_KEY *key);
//	void RC5_32_decrypt(RC5_32_INT *d, RC5_32_KEY *key);
//
//	int encdec_des(BYTE *data, DWORD nLen, bool enc);
//	int encdec_rc5(BYTE *data, DWORD nLen, bool enc);
//
//	ZES_key_schedule key_des;
//	RC5_32_KEY key_rc5;
//	bool haveKey_des;
//	bool haveKey_rc5;
//
//	encMethod method;

};

/* comment time 2016/12/21 19:14 author:yld
	结构体内存对齐单位大小为1,一般情况下使用的是占用最大内存的那个
*/
#pragma pack(1)

/* comment time 2016/12/21 19:17 author:yld
	定义包头
*/
struct PACK_HEAD
{
	unsigned char Header[2];
	unsigned short len;
	PACK_HEAD()
	{
		Header[0] = 0xAA;
		Header[1] = 0xDD;
		len = 0;
	}
};

/* comment time 2016/12/21 19:19 author:yld
	定义包尾格式
*/
struct PACK_LAST
{
	unsigned char Last;
	PACK_LAST()
	{
		//因为包头已经有了 长度所以包尾只是用来简单校验一下
		Last = 0xAA;
	}
};



typedef enum
{
	UNKNOWNSERVER = 0, /** 未知服务器类型 */
	SUPERSERVER = 1, /** 管理服务器 */
	LOGINSERVER = 10, /** 登陆服务器 */
	RECORDSERVER = 11, /** 档案服务器 */
	BILLSERVER = 12, /** 计费服务器 */
	SESSIONSERVER = 20, /** 会话服务器 */
	SCENESSERVER = 21, /** 场景服务器 */
	GATEWAYSERVER = 22, /** 网关服务器 */
	MINISERVER = 23    /** 小游戏服务器 */
}ServerType;

#define PACKHEADLASTSIZE (sizeof(PACK_LAST))
#define PACKHEADSIZE sizeof(PACK_HEAD)
#define PACKLASTSIZE 0

template <class T>
class __mt_alloc
{
	T memPool[2046];
public:
	char *allocate(size_t len){ return (char*)malloc(len); }
	void deallocate(unsigned char* ptr, size_t len)
	{
		free(ptr);
	}
};

/* comment time 2016/12/21 19:25 author:yld
	单例模式
*/
template <typename T>
class SingletonBase
{
protected:
	static T* instance;

public:
	SingletonBase(){}
	virtual ~SingletonBase();
	static T& getInstance()
	{
		assert(instance);
		return *instance;
	}

	static void newInstance()
	{
		SAFE_DELETE(instance);
		instance = new T();
	}

	static void delInstance()
	{
		SAFE_DELETE(instance);
	}
private:
	SingletonBase(const SingletonBase&);
	SingletonBase & operator= (const SingletonBase&);

};

//有静态 变量的需要 在main前初始化
template<typename T> T* SingletonBase<T>::instance = NULL;


using namespace std;
//using namespace stdext;

class zLogger;
class zProperties;
class zCond;
class zMutex;
class zMutex_scope_lock;
class zRWLock;
class zRWLock_scope_rdlock;
class zRWLock_scope_wrlock;
class zThread;
class zThreadGroup;
class zTCPTask;
class zTCPTaskPool;
class zSyncThread;
class zRecycleThread;
class zCheckconnectThread;
class zCheckwaitThread;
class zTCPClientTaskThread;


namespace Zebra
{
	//游戏时间
	extern volatile QWORD qwGameTime;

	//日志 log4plus的封装 ,定义好 格式
	extern zLogger *logger;

	//存取全局变量 全局变量真是太好用了,但是要避免多线程
	extern zProperties global;
	
	extern log4cplus::Logger log4;
}



using namespace log4cplus;
using namespace log4cplus::helpers;

//用来给log4cplus 格式化输出 内存在堆中创建了
//std::string myFormat(const char *format, ...)
//{
//	va_list argptr;
//	va_start(argptr, format);
//	int count = vsnprintf(NULL, 0, format, argptr);
//	va_end(argptr);
//
//	va_start(argptr, format);
//	char* buf = (char*)malloc(count*sizeof(char));
//	vsnprintf(buf, count, format, argptr);
//	va_end(argptr);
//
//	std::string str(buf, count);
//	free(buf);
//	return str;
//}

class zLogger
{
private:
	SharedAppenderPtr append_;// 挂载器 ， 用来将布局器过滤器 与挂在的设备进行连接例如 文件 终端
	Logger _logger;
	std::string name;
public:
	zLogger(const std::string& name, const std::string& logPath);
	~zLogger();
	void debug(const char * pattern, ...);
	void error(const char * pattern, ...);
	void info(const char * pattern, ...);
	void fatal(const char * pattern, ...);
	void warn(const char * pattern, ...);


};




/* comment function 
 名称: stringtok
 功能：字符串分割
 参数：
	container 一个容器 一般写vec就好了用来存放分割后得到的数组
	in 需要给分割的字符串
	delimiters 分割字符串 默认  空格
	deep 需要分多少 默认分到最后
 返回：
 时间：12/21/2016 Administrator
 版本：1.0
 作者: yld
 备注：
*/
template <typename Container>
inline void 
stringtok(Container &container, std::string const &in, const char *const delimiters = " ", const int deep = 0)
{
	const std::string::size_type len = in.length();
	std::string::size_type i = 0;
	int count = 0;

	while (i < len)
	{
		//i表示从哪个位置开始查找
		i = in.find_first_not_of(delimiters, i);
		if (i == std::string::npos)
			return; //没有这个字符串
		//
		std::string::size_type pos = in.find_first_of(delimiters,i);
		count++;

		if (pos == std::string::npos || (deep > 0 && count > deep))
		{
			
			container.push_back(in.substr(i));
			return;
		}
		else
		{
			//substr 返回 i 到 i + pos -i
			std::string temp = in.substr(i, pos - i);
			container.push_back(temp);
		}
		i = pos + 1;
	}

}

/* comment function 
 名称: GetTimeFromSystemPower()
 功能：返回开机至今的开机毫米数
 参数：
 返回：返回开机至今的开机毫米数
 时间：1/19/2017 Administrator
 版本：1.0
 作者: yld
 备注：
*/




//字符转成小的
struct ToLower
{
	char operator()(char c)const
	{
		return tolower(c);
	}
};

/* comment function 
 名称: to_lower
 功能：把string变成小写
 参数：string 字符串
 返回：void
 时间：12/21/2016 Administrator
 版本：1.0
 作者: yld
 备注：
*/
inline void to_lower(std::string &s)
{
	std::transform(s.begin(),s.end(),s.begin(), ToLower());
}


struct ToUpper
{
	char operator() (char c)const
	{
		return toupper(c);
	}
};

inline void to_upper(std::string &s)
{
	std::transform(s.begin(), s.end(), s.begin(), ToUpper());
}



inline unsigned long GetTimeFromSystemPower()
{
	/*
	struct timespec {
	time_t tv_sec; // seconds
	long tv_nsec; // nanoseconds
	*/

	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (ts.tv_sec * 1000 + ts.tv_nsec / 1000);
}


/* comment function 12/22/2016 yld
	//干什么用的？
	void *jpeg_Passport(char *buffer,const int buffer_len,int *size);
	void base64_encrypt(const std::string &input,std::string &output);
	void base64_decrypt(const std::string &input,std::string &output);
*/

//将一个字符串，分割成两个字符串, 注意需要手动释放内存，可以用智能指针
template <typename T>
class Parse
{
public:
	T* operator()(const std::string& down, const std::string& separator_down)
	{
		std::string::size_type pos = 0;
		if ((pos = down.find(separator_down)) != std::string::npos)
		{
			std::string first_element = down.substr(0, pos);
			std::string second_element = down.substr(pos, pos + separator_down.length());
			return new T(first_element, second_element);
		}
		return NULL;
	}
};






/* comment function 12/22/2016 yld
	全局属性， 一个map容器， 所有的值和键都是字符串，关键字不区分大小写
*/
class zProperties
{
public:
	const std::string& getProperty(const std::string &key)
	{
		std::string temp_key = key;
		to_upper(temp_key);
		return properties[temp_key];
	}
	
	// 没有加锁 注意 , 一般 全局属性 在 程序运行时就从配置文件中加载设置就好了，不要在程序运行时 设置，多线程不安全
	void setProperty(const std::string &key, const std::string value)
	{
		std::string temp_key = key;
		to_upper(temp_key);
		properties[temp_key] = value;
	}

	//重载[]
	std::string& operator[] (const std::string& key)
	{
		std::string temp_key = key;
		to_upper(temp_key);
		return properties[temp_key];
	}

	void show(std::ostream& out)
	{
		property_hashtype::const_iterator const_it;
		for (const_it = properties.begin(); const_it != properties.end(); const_it++)
		{
			std::cout << const_it->first << " = " << const_it->second << std::endl;
		}
	}

private:
	//因为需要经常查询 所以使用hash的结构 ,且一般属性不会很多
	typedef std::unordered_map<std::string, std::string> property_hashtype;

	property_hashtype properties;

};


/* comment function 12/22/2016 yld
	封装锁的一些操作
*/

class zMutex
{
private:
	pthread_mutex_t mutex_;
	//当前使用锁的线程id
	pid_t holder_;
public:
	zMutex() :holder_(0)
	{
		assert(holder_ == 0);
		//暂时先不设置 attr
		pthread_mutex_init(&mutex_,NULL);
	}
	~zMutex()
	{
		pthread_mutex_destroy(&mutex_);
	}

	inline void lock()
	{
		pthread_mutex_lock(&mutex_);
		holder_ = pthread_self();
	}

	inline void unlock()
	{
		holder_ = 0;
		pthread_mutex_unlock(&mutex_);

	}
	pthread_mutex_t* getPthreadMutex()
	{
		return &mutex_;
	}
};


class zNoncopyable
{
protected:

	/**
	* \brief 缺省构造函数
	*
	*/
	zNoncopyable() {};

	/**
	* \brief 缺省析构函数
	*
	*/
	~zNoncopyable() {};

private:

	/**
	* \brief 拷贝构造函数，没有实现，禁用掉了
	*
	*/
	zNoncopyable(const zNoncopyable&);

	/**
	* \brief 赋值操作符号，没有实现，禁用掉了
	*
	*/
	const zNoncopyable & operator= (const zNoncopyable &);

};




/* comment function 12/22/2016 yld
	在做域内加锁
	{
		metux 应该是对象拥有，这个方法只用在函数里
		zMutex_scope_lock(metux);
		xxx
		xxx
	}
*/
class zMutex_scope_lock :private zNoncopyable
{
private:
	zMutex& metux_;
public:
	explicit zMutex_scope_lock(zMutex& mutex) :metux_(mutex)
	{
		metux_.lock();
	}
	~zMutex_scope_lock()
	{
		metux_.unlock();
	}
};


//条件变量

//读写锁
class zRWLock
{
public:
	/*
	*brief 构造函数，用于创建一个读写锁
	*
	*/

	zRWLock()
	{
		pthread_rwlock_init(&m_rwlock, NULL);
	}

	/**
	* \brief 析构函数，用于销毁一个读写锁
	*
	*/
	~zRWLock()
	{
		pthread_rwlock_destroy(&m_rwlock);
	}

	/**
	* \brief 对读写锁进行读加锁操作
	*
	*/
	inline void rdlock()
	{
		//WaitForSingleObject(m_hMutex, INFINITE);
		pthread_rwlock_rdlock(&m_rwlock);

	};

	/**
	* \brief 对读写锁进行写加锁操作
	*
	*/
	inline void wrlock()
	{
		//WaitForSingleObject(m_hMutex, INFINITE);
		pthread_rwlock_wrlock(&m_rwlock);
	}

	/**
	* \brief 对读写锁进行解锁操作
	*
	*/
	inline void unlock()
	{
		pthread_rwlock_unlock(&m_rwlock);
	}

private:

	//HANDLE m_hMutex;    /**< 系统读写锁 */
	pthread_rwlock_t m_rwlock;
};


/**
* \brief wrlock Wrapper
* 方便在复杂函数中读写锁的使用
*/
class zRWLock_scope_wrlock : private zNoncopyable
{

public:

	/**
	* \brief 构造函数
	* 对锁进行wrlock操作
	* \param m 锁的引用
	*/
	zRWLock_scope_wrlock(zRWLock &m) : rwlock(m)
	{
		rwlock.wrlock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~zRWLock_scope_wrlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	zRWLock &rwlock;

};
/**
* \brief rdlock Wrapper
* 方便在复杂函数中读写锁的使用
*/
class zRWLock_scope_rdlock : private zNoncopyable
{

public:

	/**
	* \brief 构造函数
	* 对锁进行rdlock操作
	* \param m 锁的引用
	*/
	zRWLock_scope_rdlock(zRWLock &m) : rwlock(m)
	{
		rwlock.rdlock();
	}

	/**
	* \brief 析购函数
	* 对锁进行unlock操作
	*/
	~zRWLock_scope_rdlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief 锁的引用
	*/
	zRWLock &rwlock;

};

//socket封装 这个结构体貌似在linux 已经有了
#ifdef __cplusplus
extern "C"{
#endif
	class zSocket;
	struct pollfdzt
	{
		int fd;
		short events;
		short revents;
		zSocket* pSock;
	};

	extern int poll(struct pollfdzt *fds, unsigned int nfds, int timeout);
	extern int waitRecvAll(struct pollfdzt *fds, unsigned int nfds, int timeout);

#ifdef __cplusplus
}
#endif

//trunk 一般都是只一块的意思 TCP 数据包大小在64k 以内
const DWORD trunkSize = 64 * 1024;
//额，改用 zlib 不好预测 因为同样大小的数据压缩后 大小也不相同
//数据需要压缩后进行传输,但是由于数据压缩后大小无法预测，所以压缩前数据大小 仍然先设置为 trunkSize
//#define unzip_size(zip_size) ((zip_size) * 120 / 100 + 12)
#define unzip_size trunkSize
//64k数据
const DWORD PACKET_ZIP_BUFFER = unzip_size;



/* comment function 12/22/2016 yld
	数据缓冲 用vec 因为内存是连续的
*/
template <typename _type>
class ByteBuffer
{
private:
	DWORD _maxSize;
	DWORD _offPtr; //当前数据的首地址
	DWORD _currPtr; //当前可写入的下标
	_type _buffer; //具体的容器类型

public:
	zMutex m_lock; //锁

	inline void lock()
	{
		m_lock.lock();
	}

	inline void unlock()
	{
		m_lock.unlock();
	}

	ByteBuffer();

	/* comment function 12/22/2016 yld
		向缓冲区填入数据
	*/
	inline void put(const BYTE *buf, const DWORD size)
	{
		//判断,是否有足够内存 不够需要再分配
		//不同的结构方法不一样，集成的类去实现它
		wr_reserve(size);

		if (_maxSize - _currPtr < size)
		{
			//溢出了 需要打个日志
		}
		bcopy_(&_buffer[_currPtr], buf, size);
		_currPtr += size;
		if (_currPtr - _offPtr == 1)
		{
			//干嘛
		}

	}

	/* comment function 12/22/2016 yld
		获取当前buffer可以写入的位置
	*/
	inline BYTE* wr_buf()
	{
		return &_buffer[_currPtr];
	}


	inline void wr_reserve(const DWORD size);


	/* comment function 12/23/2016 yld
		获取已经存入数据的首地址
	*/

	inline BYTE* rd_buf()
	{
		return &_buffer[_offPtr];
	}

	/* comment function 12/23/2016 yld
		判断数据是否有效
	*/
	inline bool rd_ready()
	{
		bool ret = _currPtr > _offPtr;
		return ret;
	}

	/* comment function 12/23/2016 yld
		得到缓冲数据的大小
	*/
	inline DWORD rd_size()
	{
		return _currPtr - _offPtr;
	}

	/* comment function 12/23/2016 yld
		当buffer中的数据使用后，整理buffer下表
		size 最后一次使用数据的长度
	*/
	inline void rd_flip(DWORD size)
	{
		if (size <= 0)
		{
			return;
		}
		//向前移动 最后一次使用的数据长度
		_offPtr += size;
		if (_currPtr > _offPtr)
		{
			DWORD tmp = _currPtr - _offPtr;
			if (_offPtr >= tmp)
			{
				//如果前面剩余的空间 大于现在的有效数据长度 则移动到头部
				memmove(&_buffer[0],&_buffer[_offPtr],tmp);
				_offPtr = 0;
				_currPtr = tmp;
			}
		}
		else
		{
			_offPtr = 0;
			_currPtr = 0;
		}

	}


	/* comment function 12/23/2016 yld
		得到剩余的buffer长度
	*/
	inline DWORD wr_size()
	{
		return  _maxSize - _currPtr;
	}

	/* comment function 12/23/2016 yld
		写入数据的时候 增加偏移量(不会自动整理内存,put 使用过_currPtr += size)
	*/
	inline  void wr_flip(const DWORD size)
	{
		_currPtr += size;
	}

	inline void reset()
	{
		_offPtr = 0;
		_currPtr = 0;
	}

	inline DWORD maxSize() const
	{
		return _maxSize;
	}

};

//动态缓冲区
typedef ByteBuffer<std::vector<BYTE>> t_BufferCmdQueue;

template<>
inline void t_BufferCmdQueue::wr_reserve(const DWORD size)
{
	//可用内存 是否够用
	if (wr_size() < size)
	{
// 保证是trunSize 的一倍以上
#define trunkCount(size) (((size) + trunkSize - 1) / trunkSize)
		_maxSize += (trunkSize * trunkCount(size));
		_buffer.resize(_maxSize);
	}

}

//栈来分配
typedef ByteBuffer<BYTE[PACKET_ZIP_BUFFER]> t_StackCmdQueue;

template <>
inline void t_StackCmdQueue::wr_reserve(const DWORD size)
{
	/*
	if (wr_size() < size)
	{
	//不能动态扩展内存
	assert(false);
	}
	// */
}

//手动调用构造函数，不分配内存 ,但是需要知己提供一个buffer, 那个buffer就是 _Ptr
//new 创建对象做了两件事情， 创建内存， 调用构造函数
//这样的做法是，用同一块内存，可以反复的创建对象和删除对象，减少分派内存的性能消耗
template<class _T1>
inline  void constructInPlace(_T1  *_Ptr)
{
	new (static_cast<void*>(_Ptr)) _T1();
}

/**
* \brief 变长指令的封装，固定大小的缓冲空间
* 在栈空间分配缓冲内存
* \param cmd_type 指令类型
* \param size 缓冲大小
*/
template <typename cmd_type, DWORD size = 64 * 1024>
class CmdBuffer_wrapper
{

public:

	typedef cmd_type type;
	DWORD cmd_size;
	DWORD max_size;
	type *cnt;

	CmdBuffer_wrapper() : cmd_size(sizeof(type)), max_size(size)// : cnt(NULL)
	{
		cnt = (type *)buffer;
		constructInPlace(cnt);
	}

private:

	BYTE buffer[size];

};


/* comment function 12/24/2016 yld
	时间 对timeval 封装
*/

class zRTime
{
private:
	QWORD  _msecs; //毫秒

	/* comment function 12/24/2016 yld
		获得当前真实时间
	*/
	QWORD _now()
	{
		//
		//	struct timeval {
		//	time_t       tv_sec;     /* seconds */
		//	suseconds_t   tv_usec; /* microseconds */

		//};
		
		QWORD retval = 0LL;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		retval = tv.tv_sec * 1000;
		retval += tv.tv_usec / 1000;
		return retval;
	}

	/* comment function 12/24/2016 yld
		_msecs 得到当前时间延迟后的时间
	*/
	void nowByDelay(const int delay)
	{
		_msecs = _now();
		addDelay(delay);
	}
public:
	zRTime(const int delay = 0)
	{
		nowByDelay(delay);
	}

	zRTime(const zRTime &rt)
	{
		_msecs = rt._msecs;
	}

	void now()
	{
		_msecs = _now();
	}

	DWORD sec() const
	{
		return _msecs / 1000;
	}

	QWORD msec() const
	{
		return _msecs;
	}

	void addDelay(const int delay)
	{
		_msecs += delay;
	}

	zRTime & operator= (const zRTime &rt)
	{
		_msecs = rt._msecs;
		return *this;
	}

	/**
	* \brief 重构+操作符
	*
	*/
	const zRTime & operator+ (const zRTime &rt)
	{
		_msecs += rt._msecs;
		return *this;
	}

	/**
	* \brief 重构-操作符
	*
	*/
	const zRTime & operator- (const zRTime &rt)
	{
		_msecs -= rt._msecs;
		return *this;
	}

	/**
	* \brief 重构>操作符，比较zRTime结构大小
	*
	*/
	bool operator > (const zRTime &rt) const
	{
		return _msecs > rt._msecs;
	}

	/**
	* \brief 重构>=操作符，比较zRTime结构大小
	*
	*/
	bool operator >= (const zRTime &rt) const
	{
		return _msecs >= rt._msecs;
	}

	/**
	* \brief 重构<操作符，比较zRTime结构大小
	*
	*/
	bool operator < (const zRTime &rt) const
	{
		return _msecs < rt._msecs;
	}

	/**
	* \brief 重构<=操作符，比较zRTime结构大小
	*
	*/
	bool operator <= (const zRTime &rt) const
	{
		return _msecs <= rt._msecs;
	}

	/**
	* \brief 重构==操作符，比较zRTime结构是否相等
	*
	*/
	bool operator == (const zRTime &rt) const
	{
		return _msecs == rt._msecs;
	}


	QWORD elapse(const zRTime &rt)const
	{
		if (rt._msecs > _msecs)
		{
			return (rt._msecs - _msecs);
		}
		else
		{
			return 0LL;
		}
	}


	static std::string & getLocalTZ(std::string &s)
	{
		long tz = 0;
		std::ostringstream so;
		tzset();
		//tz = *__timezone() / 3600;
		//so << _tzname[0];
		so << tz;
		s = so.str();
		return s;
	}
	static void getLocalTime(struct tm & tv1, time_t timValue)
	{
		timValue += 8 * 60 * 60;
		tv1 = *gmtime(&timValue);
	}

};

class zTime
{

public:

	/**
	* \brief 构造函数
	*/
	zTime()
	{
		time(&secs);
		zRTime::getLocalTime(tv, secs);
	}

	/**
	* \brief 拷贝构造函数
	*/
	zTime(const zTime &ct)
	{
		secs = ct.secs;
		zRTime::getLocalTime(tv, secs);
	}

	/**
	* \brief 获取当前时间
	*/
	void now()
	{
		time(&secs);
		zRTime::getLocalTime(tv, secs);
	}

	/**
	* \brief 返回存储的时间
	* \return 时间，秒
	*/
	time_t sec() const
	{
		return secs;
	}

	/**
	* \brief 重载=运算符号
	* \param rt 拷贝的引用
	* \return 自身引用
	*/
	zTime & operator= (const zTime &rt)
	{
		secs = rt.secs;
		return *this;
	}

	/**
	* \brief 重构+操作符
	*/
	const zTime & operator+ (const zTime &rt)
	{
		secs += rt.secs;
		return *this;
	}

	/**
	* \brief 重构-操作符
	*/
	const zTime & operator- (const zTime &rt)
	{
		secs -= rt.secs;
		return *this;
	}

	/**
	* \brief 重构-操作符
	*/
	const zTime & operator-= (const time_t s)
	{
		secs -= s;
		return *this;
	}

	/**
	* \brief 重构>操作符，比较zTime结构大小
	*/
	bool operator > (const zTime &rt) const
	{
		return secs > rt.secs;
	}

	/**
	* \brief 重构>=操作符，比较zTime结构大小
	*/
	bool operator >= (const zTime &rt) const
	{
		return secs >= rt.secs;
	}

	/**
	* \brief 重构<操作符，比较zTime结构大小
	*/
	bool operator < (const zTime &rt) const
	{
		return secs < rt.secs;
	}

	/**
	* \brief 重构<=操作符，比较zTime结构大小
	*/
	bool operator <= (const zTime &rt) const
	{
		return secs <= rt.secs;
	}

	/**
	* \brief 重构==操作符，比较zTime结构是否相等
	*/
	bool operator == (const zTime &rt) const
	{
		return secs == rt.secs;
	}

	/**
	* \brief 计时器消逝的时间，单位秒
	* \param rt 当前时间
	* \return 计时器消逝的时间，单位秒
	*/
	time_t elapse(const zTime &rt) const
	{
		if (rt.secs > secs)
			return (rt.secs - secs);
		else
			return 0;
	}

	/**
	* \brief 计时器消逝的时间，单位秒
	* \return 计时器消逝的时间，单位秒
	*/
	time_t elapse() const
	{
		zTime rt;
		return (rt.secs - secs);
	}

	/**
	* \brief 得到当前分钟，范围0-59点
	*
	* \return
	*/
	int getSec()
	{
		return tv.tm_sec;
	}

	/**
	* \brief 得到当前分钟，范围0-59点
	*
	* \return
	*/
	int getMin()
	{
		return tv.tm_min;
	}

	/**
	* \brief 得到当前小时，范围0-23点
	*
	* \return
	*/
	int getHour()
	{
		return tv.tm_hour;
	}

	/**
	* \brief 得到天数，范围1-31
	*
	* \return
	*/
	int getMDay()
	{
		return tv.tm_mday;
	}

	/**
	* \brief 得到当前星期几，范围1-7
	*
	* \return
	*/
	int getWDay()
	{
		return tv.tm_wday;
	}

	/**
	* \brief 得到当前月份，范围1-12
	*
	* \return
	*/
	int getMonth()
	{
		return tv.tm_mon + 1;
	}

	/**
	* \brief 得到当前年份
	*
	* \return
	*/
	int getYear()
	{
		return tv.tm_year + 1900;
	}

private:

	/**
	* \brief 存储时间，单位秒
	*/
	time_t secs;

	/**
	* \brief tm结构，方便访问
	*/
	struct tm tv;


};

#define USE_EPOOL true



typedef struct _DataFlag
{
	enum DataType
	{
		READ_DATA ,
		WRITE_DATA
	};

	SWORD	data_type; //需要操作的数据类型
	bool	is_unfinish; //是否读或者写数据位完成
	DWORD	data_len; //剩余 需要的长度
	DWORD	data_offset;//当前有效数据的下标

}DataFlag;
//0表示socket不可以用了
#define INVALID_SOCKET -2 
class zSocket :private zNoncopyable
{
private:
	SOCKET sock;
	struct sockaddr_in addr;
	struct sockaddr_in local_addr;
	int rd_msec;
	int wr_msec;
	zMutex m_RecvLock; //接收数据所缓冲锁
	t_BufferCmdQueue m_RecvBuffer; //动态接收缓冲区 Epoll


	DWORD		m_dwMySendCount; //通过封装发送的数据总长度
	DWORD		m_dwSendCount;//实际发送总数据长度
	DWORD		m_dwRecvCount;// 接收总数据长度

	t_BufferCmdQueue tQueue; //发送封包缓冲区
	zTCPTask*		pTask; //代表一个连接
	t_BufferCmdQueue _rcv_queue;		//封包接收缓冲区

	
	t_BufferCmdQueue _snd_queue;        /**< 加密缓冲指令队列  先不加密 框架搭起来先*/

	DWORD _current_cmd;
	zMutex mutex;                /**< 锁 */

	zTime last_check_time;            /**< 最后一次检测时间 */

	
	//CEncrypt enc;                /**< 加密方式 */
	//inline bool need_enc() const { return CEncrypt::ENCDEC_NONE != enc.getEncMethod(); }
	
	/**
	* \brief 返回数据包包头最小长度
	* \return 最小长度
	*/
	inline DWORD packetMinSize() const { return PH_LEN; }

	inline DWORD packetSize(const BYTE *in) const { return PH_LEN + ((*((DWORD *)in)) & PACKET_MASK); }

	inline int sendRawData(const void *pBuffer, const int nSize);
	inline bool sendRawDataIM(const void *pBuffer, const int nSize);
	inline int sendRawData_NoPoll(const void *pBuffer, const int nSize);
	inline bool setNonblock();
	inline int waitForRead();
	inline int waitForWrite();
	inline int recvToBuf();

	template<typename buffer_type>
	inline DWORD packetAppend(const void *pData, const DWORD nLen, buffer_type &cmd_queue);

	inline DWORD packetUnpack(BYTE *in, const DWORD nPacketLen, BYTE *out);
public:
	bool		m_bUserEpoll; //是否使用epoll

	DWORD		m_SendSize;
	DWORD		m_LastSize; //单次发送的长度
	DWORD		m_LastSended; //已经发送的数据长度

	//读超时 毫秒数
	static const int T_RD_MSEC = 2100;
	static const int T_WR_MSEC = 2100;

	static const DWORD PH_LEN = sizeof(DWORD);  /**< 数据包包头大小  ??? */ 

	static const DWORD PACKET_ZIP_MIN = 32;            /**< 数据包压缩最小大小 */

	static const DWORD PACKET_ZIP = 0x40000000;        /**< 数据包压缩标志 */
	static const DWORD INCOMPLETE_READ = 0x00000001;        /**< 上次对套接口进行读取操作没有读取完全的标志 */
	static const DWORD INCOMPLETE_WRITE = 0x00000002;        /**< 上次对套接口进行写入操作煤油写入完毕的标志 */


	static const DWORD PACKET_MASK = trunkSize - 1; //最大数据包长度掩码... 掩码 64k - 1 那么就可以有 最低位0 1两个选择了
	static const DWORD MAX_DATABUFFERSIZE = PACKET_MASK; //最大数据包长度 包括头部
	static const DWORD MAX_DATASIZE = (MAX_DATABUFFERSIZE - PH_LEN - PACKHEADLASTSIZE); // 总的进去 头尾
	static const DWORD MAX_USERDATASIZE = (MAX_DATASIZE - 128);        /**< 用户数据包最大长度 */


	zSocket(const SOCKET sock, const struct sockaddr_in *addr = NULL, const bool compress = false, bool useEpoll = USE_EPOOL, zTCPTask* pTask = NULL);
	~zSocket();

	int recvToCmd(void *pstrCmd, const int nCmdLen, const bool wait);
	bool sendCmd(const void* pstdCmd, const int nCmdLen, const bool buffer = false);
	bool sendCmdNoPack(const void* pstrCmd, const int nCmdLen, const bool buffer = false);
	int Send(const SOCKET socket, const void* pBuffer, const int nLen, int flags);
	bool sync();
	void force_sync();


	int recvToBuf_NoPoll();
	int recvToCmd_NoPoll(void *pstrCmd, const int nCmdLen);

	/* comment function 12/24/2016 yld
		获取对等端IP
	*/
	inline const char* getIP() const { return inet_ntoa(addr.sin_addr); }
	inline const DWORD getAddr() const { return addr.sin_addr.s_addr; }

	inline const WORD getPort() const { return ntohs(addr.sin_port); }
	inline const char *getLocalIP() const { return inet_ntoa(local_addr.sin_addr); }
	inline const WORD getLocalPort() const { return ntohs(local_addr.sin_port); }
	inline void setReadTimeout(const int msec) { rd_msec = msec; }
	inline void setWriteTimeout(const int msec) { wr_msec = msec; }

	inline void fillPollFD(struct epoll_event &event_, short events)
	{
		//填充 event
		//The event argument describes the object linked to the file descriptor fd.The struct epoll_event is defined as :

		//typedef union epoll_data {
		//	void        *ptr;
		//	int          fd;
		//	uint32_t     u32;
		//	uint64_t     u64;
		//} epoll_data_t;

		//struct epoll_event {
		//	uint32_t     events;      /* Epoll events */
		//	epoll_data_t data;        /* User data variable */
		//};
	
		event_.events = events;
		event_.data.ptr = this;

	}

	//inline void setEncMethod(CEncrypt::encMethod m) { enc.setEncMethod(m); }
	//inline void set_key_rc5(const BYTE *data, int nLen, int rounds) { enc.set_key_rc5(data, nLen, rounds); }
	//inline void set_key_des(const_ZES_cblock *key) { enc.set_key_des(key); }
	
	inline DWORD getBufferSize() const { return _rcv_queue.maxSize() + _snd_queue.maxSize(); }
	inline zTCPTask*& GetpTask() { return pTask; } // [ranqd] 返回Task指针

	DWORD RecvByte(DWORD size); // [ranqd] 请求读取单个字节
	DWORD RecvData(DWORD dwNum = 0); // [ranqd] 通过Iocp收取数据

	int SendData(DWORD dwNum); // [ranqd] 通过Iocp发送数据

	int WaitRecv(bool bWait, unsigned int timeout = 0);  // [ranqd] 等待数据接收

	int WaitSend(bool bWait, unsigned int timeout = 0); // [ranqd] 等待数据发送


	void DisConnet()
	{
		shutdown(sock, 0x02);//读写都关闭 ,优雅关闭 , close 不会等待缓冲区中的数据发送或者接受完
		close(sock);
		sock = INVALID_SOCKET;
	}
	zMutex m_Lock;


	bool m_bTaskDeleted;   // task是否释放

	bool m_bDeleted;       // 释放标志

	bool SafeDelete() // [ranqd] 安全释放本类
	{
		m_Lock.lock();
			
		if (!m_bTaskDeleted)
		{
			DisConnet();
			m_Lock.unlock();
			return false;
		}
		if (m_bDeleted) //已经释放过了
		{
			m_Lock.unlock();
			return false;
		}
		m_bDeleted = true;
		m_Lock.unlock();
		
		return true;
	}

	SOCKET getSock()
	{
		return sock;
	}

	bool isValid()
	{
		if (sock == INVALID_SOCKET)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

};

//这里现在没有加密 只是存进去一个头部而已
template<typename buffer_type>
inline DWORD zSocket::packetAppend(const void *pData, const DWORD nLen, buffer_type &cmd_queue)
{
	t_StackCmdQueue t_cmd_queue;
	//亚索

	//加密

	PACK_HEAD head;
	head.len = nLen;
	cmd_queue.put((BYTE*)&head,sizeof(head));
	cmd_queue.put((BYTE*)pData, nLen);

	return nLen;
}




class zThread :private zNoncopyable
{
private:
	std::string threadName;//线程名 
	zMutex mlock;
	volatile bool alive;
	volatile bool complete;
	pthread_t m_hThread;
	bool  joinable;

public:
	zThread(const std::string &name = std::string("zThread"), const bool joinable = true)
		: threadName(name), alive(false), complete(false), joinable(joinable) {
		m_hThread = 0;
	}

	virtual ~zThread()
	{
		if (0 != m_hThread)
		{
			//线程释放- -需要干嘛
		}
	}

	static void sleep(const long sec)
	{
		sleep(sec);
	}

	static void msleep(const long msec)
	{
		usleep(msec * 1000);
	}

	static void usleep_(const long usec)
	{
		usleep(usec);
	}

	const bool isJoinable()const
	{
		return joinable;
	}

	const bool isAlive()
	{
		return alive;
	}

	bool start();
	void join();

	static void*  threadFunc(void *arg);
	void final()
	{
		complete = true;
	}
	const bool isFinal() const
	{
		return complete;
	}
	
	virtual void run() = 0;
	const std::string &getThreadName() const
	{
		return threadName;
	}
};


class zProcessor
{
public:
	virtual bool msgParse(const Cmd::t_NullCmd *, const DWORD) = 0;
};
class Timer
{
public:
	Timer(const float how_long, const int delay = 0) : _long((int)(how_long * 1000)), _timer(delay * 1000)
	{

	}
	Timer(const float how_long, const zRTime cur) : _long((int)(how_long * 1000)), _timer(cur)
	{
		_timer.addDelay(_long);
	}
	void next(const zRTime &cur)
	{
		_timer = cur;
		_timer.addDelay(_long);
	}
	bool operator() (const zRTime& current)
	{
		if (_timer <= current) {
			_timer = current;
			_timer.addDelay(_long);
			return true;
		}

		return false;
	}
private:
	int _long;
	zRTime _timer;
};

/* comment function 12/25/2016 yld
	TCP服务器封装
*/

class zTCPServer :private zNoncopyable
{
private:
	std::string name;            /**< 服务器名称 */
	SOCKET sock;                /**< 套接口 */
	static const int T_MSEC = 2100;      /**< 轮询超时，毫秒 */
	static const int MAX_WAITQUEUE = 2000;  /**< 最大等待队列 */
	sockaddr_in addr_;
public:

	zTCPServer(const std::string &name);
	~zTCPServer();
	bool Bind(const std::string &name,const WORD port);
	int Accept(struct sockaddr_in *addr);
	SOCKET getSockFd();
	struct sockaddr_in& getAddr();

};

/**
* \brief 指令流量分析
*/
struct CmdAnalysis
{
	CmdAnalysis(const char *disc, DWORD time_secs) :_log_timer(time_secs)
	{
		bzero_(_disc, sizeof(disc));
		strncpy(_disc, disc, sizeof(_disc)-1);
		bzero_(_data, sizeof(_data));
		_switch = false;
	}
	struct
	{
		DWORD num;
		DWORD size;
	}_data[256][256];
	zMutex _mutex;
	Timer _log_timer;
	char _disc[256];
	bool _switch;//开关
	void add(const BYTE &cmd, const BYTE &para, const DWORD &size)
	{
		if (!_switch)
		{
			return;
		}
		_mutex.lock();
		_data[cmd][para].num++;
		_data[cmd][para].size += size;
		zRTime ct;
		if (_log_timer(ct))
		{
			for (int i = 0; i < 256; i++)
			{
				for (int j = 0; j < 256; j++)
				{
					if (_data[i][j].num)
					{
						//Zebra::logger->debug("%s:%d,%d,%d,%d", _disc, i, j, _data[i][j].num, _data[i][j].size);}
					}
				}
			}
			bzero_(_data, sizeof(_data));
		}
		_mutex.unlock();
	}
};

//线程池的最小工作单位zTCPTask
class zTCPTask :public zProcessor, private zNoncopyable
{
public:
	/**
	* \brief 连接断开方式
	*
	*/
	enum TerminateMethod
	{
		terminate_no,              /**< 没有结束任务 */
		terminate_active,            /**< 客户端主动断开连接，主要是由于服务器端检测到套接口关闭或者套接口异常 */
		terminate_passive,            /**< 服务器端主动断开连接 */
	};

	enum zTCPTask_State
	{
		notuse = 0,            /**< 连接关闭状态 */
		verify = 1,            /**< 连接验证状态 */
		sync = 2,            /**< 等待来自其它服务器的验证信息同步 */
		okay = 3,            /**< 连接处理阶段，验证通过了，进入主循环 */
		recycle = 4              /**< 连接退出状态，回收 */
	};


	bool buffered;                  /**< 发送指令是否缓冲 */
	//	zSocket mSocket;                /**< 底层套接口 */
	zSocket* mSocket;              // [ranqd] 修改为指针

	zTCPTask_State state;              /**< 连接状态 */

private:

	zTCPTaskPool *pool;                /**< 任务所属的池 */
	TerminateMethod terminate;            /**< 是否结束任务 */
	bool terminate_wait;              /**< 其它线程设置等待断开连接状态,由pool线程设置断开连接状态 */
	bool fdsradd;                  /**< 读事件添加标志 */
	zRTime lifeTime;                /**< 连接创建时间记录 */

	bool uniqueVerified;              /**< 是否通过了唯一性验证 */
	const bool _checkSignal;            /**< 是否发送链路检测信号 */
	Timer _ten_min;
	bool tick;


public:
	static CmdAnalysis analysis;
	/**
	* \brief 构造函数，用于创建一个对象
	*
	*
	* \param pool 所属连接池指针
	* \param sock 套接口
	* \param addr 地址
	* \param compress 底层数据传输是否支持压缩
	* \param checkSignal 是否发送网络链路测试信号
	*/
	zTCPTask(
		zTCPTaskPool *pool,
		const SOCKET sock,
		const struct sockaddr_in *addr = NULL,
		const bool compress = false,
		const bool checkSignal = true,
		const bool useEpoll = USE_EPOOL) :pool(pool), lifeTime(), _checkSignal(checkSignal), _ten_min(600), tick(false)
	{
		terminate = terminate_no;
		terminate_wait = false;
		fdsradd = false;
		buffered = false;
		state = notuse;
		mSocket = NULL;
		mSocket = new zSocket(sock, addr, compress, useEpoll, this);
		if (mSocket == NULL)
		{
			Zebra::logger->error("new zSocket时内存不足！");
			
		}
	}

	virtual ~zTCPTask()
	{
		if (mSocket != NULL)
		{
			if (mSocket->SafeDelete())
				delete mSocket;
			mSocket = NULL;
		}
	}

	//填充epoll
	void fillPollFD(struct epoll_event& event_, WORD events)
	{
		mSocket->fillPollFD(event_, events);
	}

	bool checkVerifyTimeout(const zRTime &ct, const QWORD interval = 5000)const
	{
		return (lifeTime.elapse(ct) > interval);
	}

	//是否已经加入读事件标志
	bool isFdsrAdd()
	{
		return fdsradd;
	}

	//设置入读事件标志
	bool fdsrAdd()
	{
		fdsradd = true;
		return fdsradd;
	}

	virtual int verifyConn()
	{
		return 1;
	}
	// 1成功 0需要等待 -1失败 断开连接
	virtual int waitSync()
	{
		return 1;
	}

	//是否回收成功 需要删除有关该TCP连接相关资源
	virtual int recycleConn()
	{
		return 1;
	}

	/**
	* \brief 一个连接任务验证等步骤完成以后，需要添加到全局容器中
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void addToContainer() {}

	/**
	* \brief 连接任务退出的时候，需要从全局容器中删除
	*
	* 这个全局容器是外部容器
	*
	*/
	virtual void removeFromContainer() {}
	
	/**
	* \brief 添加到外部容器，这个容器需要保证这个连接的唯一性
	*
	* \return 添加是否成功
	*/
	virtual bool uniqueAdd()
	{
		return true;
	}

	/**
	* \brief 从外部容器删除，这个容器需要保证这个连接的唯一性
	*
	* \return 删除是否成功
	*/
	virtual bool uniqueRemove()
	{
		return true;
	}

	/**
	* \brief 设置唯一性验证通过标记
	*
	*/
	void setUnique()
	{
		uniqueVerified = true;
	}

	/**
	* \brief 判断是否已经通过了唯一性验证
	*
	* \return 是否已经通过了唯一性标记
	*/
	bool isUnique() const
	{
		return uniqueVerified;
	}

	/**
	* \brief 判断是否被其它线程设置为等待断开连接状态
	*
	* \return true or false
	*/
	bool isTerminateWait()
	{
		return terminate_wait;
	}


	/**
	* \brief 判断是否被其它线程设置为等待断开连接状态
	*
	* \return true or false
	*/
	void TerminateWait()
	{
		terminate_wait = true;
	}

	/**
	* \brief 判断是否需要关闭连接
	*
	* \return true or false
	*/
	bool isTerminate() const
	{
		return terminate_no != terminate;
	}

	/**
	* \brief 需要主动断开客户端的连接
	*
	* \param method 连接断开方式
	*/
	virtual void Terminate(const TerminateMethod method = terminate_passive)
	{
		terminate = method;
	}

	virtual bool sendCmd(const void *, int);
	bool sendCmdNoPack(const void *, int);
	virtual bool ListeningRecv(bool);
	virtual bool ListeningSend();

	/**
	* \brief 获取连接任务当前状态
	* \return 状态
	*/
	const zTCPTask_State getState() const
	{
		return state;
	}

	void setState(const zTCPTask_State state)
	{
		this->state = state;
	}
	void getNextState();
	void resetState();

	/**
	* \brief 获得状态的字符串描述
	*
	*
	* \param state 状态
	* \return 返回状态的字符串描述
	*/
	const char *getStateString(const zTCPTask_State state) const
	{
		const char *retval = NULL;

		switch (state)
		{
		case notuse:
			retval = "notuse";
			break;
		case verify:
			retval = "verify";
			break;
		case sync:
			retval = "sync";
			break;
		case okay:
			retval = "okay";
			break;
		case recycle:
			retval = "recycle";
			break;
		default:
			retval = "none";
			break;
		}

		return retval;
	}

	/**
	* \brief 返回连接的IP地址
	* \return 连接的IP地址
	*/
	const char *getIP() const
	{
		return mSocket->getIP();
	}
	const DWORD getAddr() const
	{
		return mSocket->getAddr();
	}

	const WORD getPort()
	{
		return mSocket->getPort();
	}

	int WaitRecv(bool bWait = false, int timeout = 0)
	{
		return mSocket->WaitRecv(bWait, timeout);
		//return 0;
	}

	int WaitSend(bool bWait = false, int timeout = 0)
	{
		return mSocket->WaitSend(bWait, timeout);
		
	}

	bool isUseEpoll()
	{
		return mSocket->m_bUserEpoll;
	}

	//收到测试信号
	void clearTick()
	{
		tick = false;
	}
	
	//发出测试信号
	void setTick()
	{
		tick = true;
	}

	bool checkTick()
	{
		return tick;
	}

	void checkSignal(const zRTime &ct);
	const bool ifCheckSignal() const
	{
		return _checkSignal;
	}

	/**
	* \brief 检测测试信号发送间隔
	*
	* \return 检测是否成功
	*/
	bool checkInterval(const zRTime &ct)
	{
		return _ten_min(ct);
	}
};


class zThreadGroup :private zNoncopyable
{
public:
	struct CallBack
	{
		virtual void exec(zThread *e) = 0;
		virtual ~CallBack();
	};

	typedef std::vector<zThread *> Container; //容器类型

	zThreadGroup();
	~zThreadGroup();
	void add(zThread* thread);
	zThread *getByIndex(const Container::size_type index);
	zThread *operator[] (const Container::size_type index);

	void joinAll();
	void execAll(CallBack &cb);

private:
	Container vts;	//线程数组
	zRWLock rwLock;	//读写锁

};


class zTCPTaskPool : private zNoncopyable
{
private:
	const int maxConns; //线程池并行处理的最大连接数
	static const int maxVerifyThreads = 4;
	zThreadGroup verifyThreads;		//验证线程可以有多个
	zSyncThread *syncThread;	//等待同步线程只有一个
	static const int minThreadCount = 1;//线程池中同时存在主处理线程的最大个数
	int maxThreadCount;		//最大个数
	zThreadGroup okayThreads;		//处理主线程

	zRecycleThread* recycleThread;	//连接回收线程
	int state;
public:
	static int usleep_time;
public:
	explicit zTCPTaskPool(const int maxConns, const int state, const int us = 50000) :maxConns(maxConns), state(state)
	{
		setUsleepTime(us);
		syncThread = NULL;
		recycleThread = NULL;
		maxThreadCount = minThreadCount;
	}

	~zTCPTaskPool()
	{
		final();
	}

	void final();
	bool init();
	static void setUsleepTime(int time)
	{
		usleep_time = time;
	}

	bool addRecycle(zTCPTask* task);
	bool addVerify(zTCPTask* task);
	bool addSync(zTCPTask* task);
	bool addOkay(zTCPTask* task);

	const int getSize();
	void clearState(const int state)
	{
		this->state &= ~state;
	}

	const int getState() const
	{
		return state;
	}

	void setState(const int state)
	{
		this->state |= state;
	}
};

class zEpoll
{

private:
	bool isValid() const;
	int max;
	int epoll_fd;
	int epoll_timeout;
	int epoll_maxevents;
	epoll_event *backEvents;

public:
	enum EPOLL_OP{
		ADD = EPOLL_CTL_ADD,
		MOD = EPOLL_CTL_MOD,
		DEL = EPOLL_CTL_DEL
	};

	zEpoll(int _max = 30 , int maxevents = 20);
	~zEpoll();
	int create();
	int add(zSocket* zSock, epoll_event *event);
	int mod(zSocket* zSock, epoll_event *event);
	int del(zSocket* zSock, epoll_event *event);
	void setMaxEvents(int maxEvents);
	void setTimeout(int timeOut);
	int wait();
	const epoll_event* events() const;
	const epoll_event& operator[](int index)
	{
		return backEvents[index];
	}
	int getEpollFd()const
	{
		return epoll_fd;
	}
};


class zEpollRecvThread : public zThread
{

};





class zService :private zNoncopyable
{
private:
	static zService *serviceInst;    /**< 类的唯一实例指针，包括派生类，初始化为空指针 */

	std::string name;          /**< 服务名称 */
	bool terminate;            /**< 服务结束标记 */

public:
	Timer  _one_sec_; // 秒定时器

	virtual ~zService(){};
	//处理HUP信号函数
	virtual void reloadConfig()
	{

	}

	bool isTerminate() const
	{
		return terminate;
	}

	void Terminate()
	{
		terminate = true;
	}

	void main();

	static zService *serviceInstance()
	{
		return serviceInst;
	}

	zProperties env;

protected:
	zService(const std::string &name) : name(name), _one_sec_(1)
	{
		serviceInst = this; 

		terminate = false;
	}


	virtual bool init();
	/**
	* \brief 确认服务器初始化成功，即将进入主回调函数
	*
	* \return 确认是否成功
	*/
	virtual bool validate()
	{
		return true;
	}

	/**
	* \brief 服务程序的主回调函数，主要用于监听服务端口，如果返回false将结束程序，返回true继续执行服务
	*
	* \return 回调是否成功
	*/
	virtual bool serviceCallback() = 0;

	virtual void final() = 0;
};

class zAcceptThread;

class zNetService :public zService
{
private:
	static zNetService *instance;
	std::string serviceName;

	zAcceptThread *pAcceptThread;

	

public:
	zTCPServer *tcpServer;

public:
	virtual ~zNetService() { instance = NULL; }
	virtual void newTCPTask(const SOCKET sock, const struct sockaddr_in *addr) = 0;
	virtual const int getPoolSize() const
	{
		return 0;
	}
	virtual const int getPoolState() const
	{
		return 0;
	}
	zNetService(const std::string &name):zService(name)
	{
		instance = this;

		serviceName = name;
		tcpServer = NULL;
	}
	bool init(WORD port);
	bool serviceCallback();
	void final();
};


// [ranqd] 接收连接线程类
class zAcceptThread : public zThread
{
public:
	zAcceptThread(zNetService* p, const std::string &name) : zThread(name)
	{
		pService = p;
	}

	zNetService* pService;

	void run()         // [ranqd] 接收连接线程函数
	{
		while (!isFinal())
		{
			//Zebra::logger->debug("接收连接线程建立！");
			struct sockaddr_in addr;
			if (pService->tcpServer != NULL)
			{
				int retcode = pService->tcpServer->Accept(&addr);
				if (retcode >= 0)
				{
					//接收连接成功，处理连接
					pService->newTCPTask(retcode, &addr);
				}
			}
		}
	}
};



class zTCPClient: public zThread, public zProcessor
{
protected:
	std::string ip;
	WORD port;
	zSocket *pSocket;
	const bool compress; //是否压缩

public:
	zTCPClient(const std::string name, const std::string ip = "127.0.0.1",
		const WORD port = 8090,
		const bool compress = false)
		:zThread(name), ip(ip), port(port), pSocket(NULL), compress(compress)
	{
		  
	}

	~zTCPClient()
	{
		close();
	}

	virtual void close()
	{
		if (pSocket != NULL)
		{
			if (pSocket->SafeDelete())
				delete pSocket;
			pSocket = NULL;
		}
	}

	bool connect();

	virtual bool sendCmd(const void *pstrCmd, const int nCmdLen);
	void setIP(const char *ip)
	{
		this->ip = ip;
	}
	const char *getIP() const
	{
		return ip.c_str();
	}
	void setPort(const WORD port)
	{
		this->port = port;
	}
	const WORD getPort() const
	{
		return port;
	}
	virtual void run();

	zSocket* getZSocket();

};

class zTCPBufferClient :public zTCPClient
{
private:
	int usleep_time;
	volatile bool _buffered;

	bool ListeningRecv();
	bool ListeningSend();
	void sync();

public:
	zTCPBufferClient(
		const std::string &name,
		const std::string &ip = "127.0.0.1",
		const WORD port = 8090,
		const bool compress = false,
		const int usleep_time = 50000)
		:zTCPClient(name, ip, port, compress), usleep_time(usleep_time), _buffered(false)
	{

	}

	void close()
	{
		sync();
		zTCPClient::close();
	}

	void run();
	bool sendCmd(const void *pstrCmd,const int nCmdLen);
	void setUsleepTime(const int utime)
	{
		usleep_time = utime;
	}
};


#define _ENGINE_H_
#endif // !_ENGING_H_
