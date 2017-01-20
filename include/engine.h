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

//linux ����
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h> // ntohs �Ƚӿ�
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
	����
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
	�ṹ���ڴ���뵥λ��СΪ1,һ�������ʹ�õ���ռ������ڴ���Ǹ�
*/
#pragma pack(1)

/* comment time 2016/12/21 19:17 author:yld
	�����ͷ
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
	�����β��ʽ
*/
struct PACK_LAST
{
	unsigned char Last;
	PACK_LAST()
	{
		//��Ϊ��ͷ�Ѿ����� �������԰�βֻ��������У��һ��
		Last = 0xAA;
	}
};



typedef enum
{
	UNKNOWNSERVER = 0, /** δ֪���������� */
	SUPERSERVER = 1, /** ��������� */
	LOGINSERVER = 10, /** ��½������ */
	RECORDSERVER = 11, /** ���������� */
	BILLSERVER = 12, /** �Ʒѷ����� */
	SESSIONSERVER = 20, /** �Ự������ */
	SCENESSERVER = 21, /** ���������� */
	GATEWAYSERVER = 22, /** ���ط����� */
	MINISERVER = 23    /** С��Ϸ������ */
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
	����ģʽ
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

//�о�̬ ��������Ҫ ��mainǰ��ʼ��
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
	//��Ϸʱ��
	extern volatile QWORD qwGameTime;

	//��־ log4plus�ķ�װ ,����� ��ʽ
	extern zLogger *logger;

	//��ȡȫ�ֱ��� ȫ�ֱ�������̫������,����Ҫ������߳�
	extern zProperties global;
	
	extern log4cplus::Logger log4;
}



using namespace log4cplus;
using namespace log4cplus::helpers;

//������log4cplus ��ʽ����� �ڴ��ڶ��д�����
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
	SharedAppenderPtr append_;// ������ �� ������������������ ����ڵ��豸������������ �ļ� �ն�
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
 ����: stringtok
 ���ܣ��ַ����ָ�
 ������
	container һ������ һ��дvec�ͺ���������ŷָ��õ�������
	in ��Ҫ���ָ���ַ���
	delimiters �ָ��ַ��� Ĭ��  �ո�
	deep ��Ҫ�ֶ��� Ĭ�Ϸֵ����
 ���أ�
 ʱ�䣺12/21/2016 Administrator
 �汾��1.0
 ����: yld
 ��ע��
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
		//i��ʾ���ĸ�λ�ÿ�ʼ����
		i = in.find_first_not_of(delimiters, i);
		if (i == std::string::npos)
			return; //û������ַ���
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
			//substr ���� i �� i + pos -i
			std::string temp = in.substr(i, pos - i);
			container.push_back(temp);
		}
		i = pos + 1;
	}

}

/* comment function 
 ����: GetTimeFromSystemPower()
 ���ܣ����ؿ�������Ŀ���������
 ������
 ���أ����ؿ�������Ŀ���������
 ʱ�䣺1/19/2017 Administrator
 �汾��1.0
 ����: yld
 ��ע��
*/




//�ַ�ת��С��
struct ToLower
{
	char operator()(char c)const
	{
		return tolower(c);
	}
};

/* comment function 
 ����: to_lower
 ���ܣ���string���Сд
 ������string �ַ���
 ���أ�void
 ʱ�䣺12/21/2016 Administrator
 �汾��1.0
 ����: yld
 ��ע��
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
	//��ʲô�õģ�
	void *jpeg_Passport(char *buffer,const int buffer_len,int *size);
	void base64_encrypt(const std::string &input,std::string &output);
	void base64_decrypt(const std::string &input,std::string &output);
*/

//��һ���ַ������ָ�������ַ���, ע����Ҫ�ֶ��ͷ��ڴ棬����������ָ��
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
	ȫ�����ԣ� һ��map������ ���е�ֵ�ͼ������ַ������ؼ��ֲ����ִ�Сд
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
	
	// û�м��� ע�� , һ�� ȫ������ �� ��������ʱ�ʹ������ļ��м������þͺ��ˣ���Ҫ�ڳ�������ʱ ���ã����̲߳���ȫ
	void setProperty(const std::string &key, const std::string value)
	{
		std::string temp_key = key;
		to_upper(temp_key);
		properties[temp_key] = value;
	}

	//����[]
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
	//��Ϊ��Ҫ������ѯ ����ʹ��hash�Ľṹ ,��һ�����Բ���ܶ�
	typedef std::unordered_map<std::string, std::string> property_hashtype;

	property_hashtype properties;

};


/* comment function 12/22/2016 yld
	��װ����һЩ����
*/

class zMutex
{
private:
	pthread_mutex_t mutex_;
	//��ǰʹ�������߳�id
	pid_t holder_;
public:
	zMutex() :holder_(0)
	{
		assert(holder_ == 0);
		//��ʱ�Ȳ����� attr
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
	* \brief ȱʡ���캯��
	*
	*/
	zNoncopyable() {};

	/**
	* \brief ȱʡ��������
	*
	*/
	~zNoncopyable() {};

private:

	/**
	* \brief �������캯����û��ʵ�֣����õ���
	*
	*/
	zNoncopyable(const zNoncopyable&);

	/**
	* \brief ��ֵ�������ţ�û��ʵ�֣����õ���
	*
	*/
	const zNoncopyable & operator= (const zNoncopyable &);

};




/* comment function 12/22/2016 yld
	�������ڼ���
	{
		metux Ӧ���Ƕ���ӵ�У��������ֻ���ں�����
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


//��������

//��д��
class zRWLock
{
public:
	/*
	*brief ���캯�������ڴ���һ����д��
	*
	*/

	zRWLock()
	{
		pthread_rwlock_init(&m_rwlock, NULL);
	}

	/**
	* \brief ������������������һ����д��
	*
	*/
	~zRWLock()
	{
		pthread_rwlock_destroy(&m_rwlock);
	}

	/**
	* \brief �Զ�д�����ж���������
	*
	*/
	inline void rdlock()
	{
		//WaitForSingleObject(m_hMutex, INFINITE);
		pthread_rwlock_rdlock(&m_rwlock);

	};

	/**
	* \brief �Զ�д������д��������
	*
	*/
	inline void wrlock()
	{
		//WaitForSingleObject(m_hMutex, INFINITE);
		pthread_rwlock_wrlock(&m_rwlock);
	}

	/**
	* \brief �Զ�д�����н�������
	*
	*/
	inline void unlock()
	{
		pthread_rwlock_unlock(&m_rwlock);
	}

private:

	//HANDLE m_hMutex;    /**< ϵͳ��д�� */
	pthread_rwlock_t m_rwlock;
};


/**
* \brief wrlock Wrapper
* �����ڸ��Ӻ����ж�д����ʹ��
*/
class zRWLock_scope_wrlock : private zNoncopyable
{

public:

	/**
	* \brief ���캯��
	* ��������wrlock����
	* \param m ��������
	*/
	zRWLock_scope_wrlock(zRWLock &m) : rwlock(m)
	{
		rwlock.wrlock();
	}

	/**
	* \brief ��������
	* ��������unlock����
	*/
	~zRWLock_scope_wrlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief ��������
	*/
	zRWLock &rwlock;

};
/**
* \brief rdlock Wrapper
* �����ڸ��Ӻ����ж�д����ʹ��
*/
class zRWLock_scope_rdlock : private zNoncopyable
{

public:

	/**
	* \brief ���캯��
	* ��������rdlock����
	* \param m ��������
	*/
	zRWLock_scope_rdlock(zRWLock &m) : rwlock(m)
	{
		rwlock.rdlock();
	}

	/**
	* \brief ��������
	* ��������unlock����
	*/
	~zRWLock_scope_rdlock()
	{
		rwlock.unlock();
	}

private:

	/**
	* \brief ��������
	*/
	zRWLock &rwlock;

};

//socket��װ ����ṹ��ò����linux �Ѿ�����
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

//trunk һ�㶼��ֻһ�����˼ TCP ���ݰ���С��64k ����
const DWORD trunkSize = 64 * 1024;
//����� zlib ����Ԥ�� ��Ϊͬ����С������ѹ���� ��СҲ����ͬ
//������Ҫѹ������д���,������������ѹ�����С�޷�Ԥ�⣬����ѹ��ǰ���ݴ�С ��Ȼ������Ϊ trunkSize
//#define unzip_size(zip_size) ((zip_size) * 120 / 100 + 12)
#define unzip_size trunkSize
//64k����
const DWORD PACKET_ZIP_BUFFER = unzip_size;



/* comment function 12/22/2016 yld
	���ݻ��� ��vec ��Ϊ�ڴ���������
*/
template <typename _type>
class ByteBuffer
{
private:
	DWORD _maxSize;
	DWORD _offPtr; //��ǰ���ݵ��׵�ַ
	DWORD _currPtr; //��ǰ��д����±�
	_type _buffer; //�������������

public:
	zMutex m_lock; //��

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
		�򻺳�����������
	*/
	inline void put(const BYTE *buf, const DWORD size)
	{
		//�ж�,�Ƿ����㹻�ڴ� ������Ҫ�ٷ���
		//��ͬ�Ľṹ������һ�������ɵ���ȥʵ����
		wr_reserve(size);

		if (_maxSize - _currPtr < size)
		{
			//����� ��Ҫ�����־
		}
		bcopy_(&_buffer[_currPtr], buf, size);
		_currPtr += size;
		if (_currPtr - _offPtr == 1)
		{
			//����
		}

	}

	/* comment function 12/22/2016 yld
		��ȡ��ǰbuffer����д���λ��
	*/
	inline BYTE* wr_buf()
	{
		return &_buffer[_currPtr];
	}


	inline void wr_reserve(const DWORD size);


	/* comment function 12/23/2016 yld
		��ȡ�Ѿ��������ݵ��׵�ַ
	*/

	inline BYTE* rd_buf()
	{
		return &_buffer[_offPtr];
	}

	/* comment function 12/23/2016 yld
		�ж������Ƿ���Ч
	*/
	inline bool rd_ready()
	{
		bool ret = _currPtr > _offPtr;
		return ret;
	}

	/* comment function 12/23/2016 yld
		�õ��������ݵĴ�С
	*/
	inline DWORD rd_size()
	{
		return _currPtr - _offPtr;
	}

	/* comment function 12/23/2016 yld
		��buffer�е�����ʹ�ú�����buffer�±�
		size ���һ��ʹ�����ݵĳ���
	*/
	inline void rd_flip(DWORD size)
	{
		if (size <= 0)
		{
			return;
		}
		//��ǰ�ƶ� ���һ��ʹ�õ����ݳ���
		_offPtr += size;
		if (_currPtr > _offPtr)
		{
			DWORD tmp = _currPtr - _offPtr;
			if (_offPtr >= tmp)
			{
				//���ǰ��ʣ��Ŀռ� �������ڵ���Ч���ݳ��� ���ƶ���ͷ��
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
		�õ�ʣ���buffer����
	*/
	inline DWORD wr_size()
	{
		return  _maxSize - _currPtr;
	}

	/* comment function 12/23/2016 yld
		д�����ݵ�ʱ�� ����ƫ����(�����Զ������ڴ�,put ʹ�ù�_currPtr += size)
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

//��̬������
typedef ByteBuffer<std::vector<BYTE>> t_BufferCmdQueue;

template<>
inline void t_BufferCmdQueue::wr_reserve(const DWORD size)
{
	//�����ڴ� �Ƿ���
	if (wr_size() < size)
	{
// ��֤��trunSize ��һ������
#define trunkCount(size) (((size) + trunkSize - 1) / trunkSize)
		_maxSize += (trunkSize * trunkCount(size));
		_buffer.resize(_maxSize);
	}

}

//ջ������
typedef ByteBuffer<BYTE[PACKET_ZIP_BUFFER]> t_StackCmdQueue;

template <>
inline void t_StackCmdQueue::wr_reserve(const DWORD size)
{
	/*
	if (wr_size() < size)
	{
	//���ܶ�̬��չ�ڴ�
	assert(false);
	}
	// */
}

//�ֶ����ù��캯�����������ڴ� ,������Ҫ֪���ṩһ��buffer, �Ǹ�buffer���� _Ptr
//new �������������������飬 �����ڴ棬 ���ù��캯��
//�����������ǣ���ͬһ���ڴ棬���Է����Ĵ��������ɾ�����󣬼��ٷ����ڴ����������
template<class _T1>
inline  void constructInPlace(_T1  *_Ptr)
{
	new (static_cast<void*>(_Ptr)) _T1();
}

/**
* \brief �䳤ָ��ķ�װ���̶���С�Ļ���ռ�
* ��ջ�ռ���仺���ڴ�
* \param cmd_type ָ������
* \param size �����С
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
	ʱ�� ��timeval ��װ
*/

class zRTime
{
private:
	QWORD  _msecs; //����

	/* comment function 12/24/2016 yld
		��õ�ǰ��ʵʱ��
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
		_msecs �õ���ǰʱ���ӳٺ��ʱ��
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
	* \brief �ع�+������
	*
	*/
	const zRTime & operator+ (const zRTime &rt)
	{
		_msecs += rt._msecs;
		return *this;
	}

	/**
	* \brief �ع�-������
	*
	*/
	const zRTime & operator- (const zRTime &rt)
	{
		_msecs -= rt._msecs;
		return *this;
	}

	/**
	* \brief �ع�>���������Ƚ�zRTime�ṹ��С
	*
	*/
	bool operator > (const zRTime &rt) const
	{
		return _msecs > rt._msecs;
	}

	/**
	* \brief �ع�>=���������Ƚ�zRTime�ṹ��С
	*
	*/
	bool operator >= (const zRTime &rt) const
	{
		return _msecs >= rt._msecs;
	}

	/**
	* \brief �ع�<���������Ƚ�zRTime�ṹ��С
	*
	*/
	bool operator < (const zRTime &rt) const
	{
		return _msecs < rt._msecs;
	}

	/**
	* \brief �ع�<=���������Ƚ�zRTime�ṹ��С
	*
	*/
	bool operator <= (const zRTime &rt) const
	{
		return _msecs <= rt._msecs;
	}

	/**
	* \brief �ع�==���������Ƚ�zRTime�ṹ�Ƿ����
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
	* \brief ���캯��
	*/
	zTime()
	{
		time(&secs);
		zRTime::getLocalTime(tv, secs);
	}

	/**
	* \brief �������캯��
	*/
	zTime(const zTime &ct)
	{
		secs = ct.secs;
		zRTime::getLocalTime(tv, secs);
	}

	/**
	* \brief ��ȡ��ǰʱ��
	*/
	void now()
	{
		time(&secs);
		zRTime::getLocalTime(tv, secs);
	}

	/**
	* \brief ���ش洢��ʱ��
	* \return ʱ�䣬��
	*/
	time_t sec() const
	{
		return secs;
	}

	/**
	* \brief ����=�������
	* \param rt ����������
	* \return ��������
	*/
	zTime & operator= (const zTime &rt)
	{
		secs = rt.secs;
		return *this;
	}

	/**
	* \brief �ع�+������
	*/
	const zTime & operator+ (const zTime &rt)
	{
		secs += rt.secs;
		return *this;
	}

	/**
	* \brief �ع�-������
	*/
	const zTime & operator- (const zTime &rt)
	{
		secs -= rt.secs;
		return *this;
	}

	/**
	* \brief �ع�-������
	*/
	const zTime & operator-= (const time_t s)
	{
		secs -= s;
		return *this;
	}

	/**
	* \brief �ع�>���������Ƚ�zTime�ṹ��С
	*/
	bool operator > (const zTime &rt) const
	{
		return secs > rt.secs;
	}

	/**
	* \brief �ع�>=���������Ƚ�zTime�ṹ��С
	*/
	bool operator >= (const zTime &rt) const
	{
		return secs >= rt.secs;
	}

	/**
	* \brief �ع�<���������Ƚ�zTime�ṹ��С
	*/
	bool operator < (const zTime &rt) const
	{
		return secs < rt.secs;
	}

	/**
	* \brief �ع�<=���������Ƚ�zTime�ṹ��С
	*/
	bool operator <= (const zTime &rt) const
	{
		return secs <= rt.secs;
	}

	/**
	* \brief �ع�==���������Ƚ�zTime�ṹ�Ƿ����
	*/
	bool operator == (const zTime &rt) const
	{
		return secs == rt.secs;
	}

	/**
	* \brief ��ʱ�����ŵ�ʱ�䣬��λ��
	* \param rt ��ǰʱ��
	* \return ��ʱ�����ŵ�ʱ�䣬��λ��
	*/
	time_t elapse(const zTime &rt) const
	{
		if (rt.secs > secs)
			return (rt.secs - secs);
		else
			return 0;
	}

	/**
	* \brief ��ʱ�����ŵ�ʱ�䣬��λ��
	* \return ��ʱ�����ŵ�ʱ�䣬��λ��
	*/
	time_t elapse() const
	{
		zTime rt;
		return (rt.secs - secs);
	}

	/**
	* \brief �õ���ǰ���ӣ���Χ0-59��
	*
	* \return
	*/
	int getSec()
	{
		return tv.tm_sec;
	}

	/**
	* \brief �õ���ǰ���ӣ���Χ0-59��
	*
	* \return
	*/
	int getMin()
	{
		return tv.tm_min;
	}

	/**
	* \brief �õ���ǰСʱ����Χ0-23��
	*
	* \return
	*/
	int getHour()
	{
		return tv.tm_hour;
	}

	/**
	* \brief �õ���������Χ1-31
	*
	* \return
	*/
	int getMDay()
	{
		return tv.tm_mday;
	}

	/**
	* \brief �õ���ǰ���ڼ�����Χ1-7
	*
	* \return
	*/
	int getWDay()
	{
		return tv.tm_wday;
	}

	/**
	* \brief �õ���ǰ�·ݣ���Χ1-12
	*
	* \return
	*/
	int getMonth()
	{
		return tv.tm_mon + 1;
	}

	/**
	* \brief �õ���ǰ���
	*
	* \return
	*/
	int getYear()
	{
		return tv.tm_year + 1900;
	}

private:

	/**
	* \brief �洢ʱ�䣬��λ��
	*/
	time_t secs;

	/**
	* \brief tm�ṹ���������
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

	SWORD	data_type; //��Ҫ��������������
	bool	is_unfinish; //�Ƿ������д����λ���
	DWORD	data_len; //ʣ�� ��Ҫ�ĳ���
	DWORD	data_offset;//��ǰ��Ч���ݵ��±�

}DataFlag;
//0��ʾsocket����������
#define INVALID_SOCKET -2 
class zSocket :private zNoncopyable
{
private:
	SOCKET sock;
	struct sockaddr_in addr;
	struct sockaddr_in local_addr;
	int rd_msec;
	int wr_msec;
	zMutex m_RecvLock; //����������������
	t_BufferCmdQueue m_RecvBuffer; //��̬���ջ����� Epoll


	DWORD		m_dwMySendCount; //ͨ����װ���͵������ܳ���
	DWORD		m_dwSendCount;//ʵ�ʷ��������ݳ���
	DWORD		m_dwRecvCount;// ���������ݳ���

	t_BufferCmdQueue tQueue; //���ͷ��������
	zTCPTask*		pTask; //����һ������
	t_BufferCmdQueue _rcv_queue;		//������ջ�����

	
	t_BufferCmdQueue _snd_queue;        /**< ���ܻ���ָ�����  �Ȳ����� ��ܴ�������*/

	DWORD _current_cmd;
	zMutex mutex;                /**< �� */

	zTime last_check_time;            /**< ���һ�μ��ʱ�� */

	
	//CEncrypt enc;                /**< ���ܷ�ʽ */
	//inline bool need_enc() const { return CEncrypt::ENCDEC_NONE != enc.getEncMethod(); }
	
	/**
	* \brief �������ݰ���ͷ��С����
	* \return ��С����
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
	bool		m_bUserEpoll; //�Ƿ�ʹ��epoll

	DWORD		m_SendSize;
	DWORD		m_LastSize; //���η��͵ĳ���
	DWORD		m_LastSended; //�Ѿ����͵����ݳ���

	//����ʱ ������
	static const int T_RD_MSEC = 2100;
	static const int T_WR_MSEC = 2100;

	static const DWORD PH_LEN = sizeof(DWORD);  /**< ���ݰ���ͷ��С  ??? */ 

	static const DWORD PACKET_ZIP_MIN = 32;            /**< ���ݰ�ѹ����С��С */

	static const DWORD PACKET_ZIP = 0x40000000;        /**< ���ݰ�ѹ����־ */
	static const DWORD INCOMPLETE_READ = 0x00000001;        /**< �ϴζ��׽ӿڽ��ж�ȡ����û�ж�ȡ��ȫ�ı�־ */
	static const DWORD INCOMPLETE_WRITE = 0x00000002;        /**< �ϴζ��׽ӿڽ���д�����ú��д����ϵı�־ */


	static const DWORD PACKET_MASK = trunkSize - 1; //������ݰ���������... ���� 64k - 1 ��ô�Ϳ����� ���λ0 1����ѡ����
	static const DWORD MAX_DATABUFFERSIZE = PACKET_MASK; //������ݰ����� ����ͷ��
	static const DWORD MAX_DATASIZE = (MAX_DATABUFFERSIZE - PH_LEN - PACKHEADLASTSIZE); // �ܵĽ�ȥ ͷβ
	static const DWORD MAX_USERDATASIZE = (MAX_DATASIZE - 128);        /**< �û����ݰ���󳤶� */


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
		��ȡ�Եȶ�IP
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
		//��� event
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
	inline zTCPTask*& GetpTask() { return pTask; } // [ranqd] ����Taskָ��

	DWORD RecvByte(DWORD size); // [ranqd] �����ȡ�����ֽ�
	DWORD RecvData(DWORD dwNum = 0); // [ranqd] ͨ��Iocp��ȡ����

	int SendData(DWORD dwNum); // [ranqd] ͨ��Iocp��������

	int WaitRecv(bool bWait, unsigned int timeout = 0);  // [ranqd] �ȴ����ݽ���

	int WaitSend(bool bWait, unsigned int timeout = 0); // [ranqd] �ȴ����ݷ���


	void DisConnet()
	{
		shutdown(sock, 0x02);//��д���ر� ,���Źر� , close ����ȴ��������е����ݷ��ͻ��߽�����
		close(sock);
		sock = INVALID_SOCKET;
	}
	zMutex m_Lock;


	bool m_bTaskDeleted;   // task�Ƿ��ͷ�

	bool m_bDeleted;       // �ͷű�־

	bool SafeDelete() // [ranqd] ��ȫ�ͷű���
	{
		m_Lock.lock();
			
		if (!m_bTaskDeleted)
		{
			DisConnet();
			m_Lock.unlock();
			return false;
		}
		if (m_bDeleted) //�Ѿ��ͷŹ���
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

//��������û�м��� ֻ�Ǵ��ȥһ��ͷ������
template<typename buffer_type>
inline DWORD zSocket::packetAppend(const void *pData, const DWORD nLen, buffer_type &cmd_queue)
{
	t_StackCmdQueue t_cmd_queue;
	//����

	//����

	PACK_HEAD head;
	head.len = nLen;
	cmd_queue.put((BYTE*)&head,sizeof(head));
	cmd_queue.put((BYTE*)pData, nLen);

	return nLen;
}




class zThread :private zNoncopyable
{
private:
	std::string threadName;//�߳��� 
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
			//�߳��ͷ�- -��Ҫ����
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
	TCP��������װ
*/

class zTCPServer :private zNoncopyable
{
private:
	std::string name;            /**< ���������� */
	SOCKET sock;                /**< �׽ӿ� */
	static const int T_MSEC = 2100;      /**< ��ѯ��ʱ������ */
	static const int MAX_WAITQUEUE = 2000;  /**< ���ȴ����� */
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
* \brief ָ����������
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
	bool _switch;//����
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

//�̳߳ص���С������λzTCPTask
class zTCPTask :public zProcessor, private zNoncopyable
{
public:
	/**
	* \brief ���ӶϿ���ʽ
	*
	*/
	enum TerminateMethod
	{
		terminate_no,              /**< û�н������� */
		terminate_active,            /**< �ͻ��������Ͽ����ӣ���Ҫ�����ڷ������˼�⵽�׽ӿڹرջ����׽ӿ��쳣 */
		terminate_passive,            /**< �������������Ͽ����� */
	};

	enum zTCPTask_State
	{
		notuse = 0,            /**< ���ӹر�״̬ */
		verify = 1,            /**< ������֤״̬ */
		sync = 2,            /**< �ȴ�������������������֤��Ϣͬ�� */
		okay = 3,            /**< ���Ӵ���׶Σ���֤ͨ���ˣ�������ѭ�� */
		recycle = 4              /**< �����˳�״̬������ */
	};


	bool buffered;                  /**< ����ָ���Ƿ񻺳� */
	//	zSocket mSocket;                /**< �ײ��׽ӿ� */
	zSocket* mSocket;              // [ranqd] �޸�Ϊָ��

	zTCPTask_State state;              /**< ����״̬ */

private:

	zTCPTaskPool *pool;                /**< ���������ĳ� */
	TerminateMethod terminate;            /**< �Ƿ�������� */
	bool terminate_wait;              /**< �����߳����õȴ��Ͽ�����״̬,��pool�߳����öϿ�����״̬ */
	bool fdsradd;                  /**< ���¼���ӱ�־ */
	zRTime lifeTime;                /**< ���Ӵ���ʱ���¼ */

	bool uniqueVerified;              /**< �Ƿ�ͨ����Ψһ����֤ */
	const bool _checkSignal;            /**< �Ƿ�����·����ź� */
	Timer _ten_min;
	bool tick;


public:
	static CmdAnalysis analysis;
	/**
	* \brief ���캯�������ڴ���һ������
	*
	*
	* \param pool �������ӳ�ָ��
	* \param sock �׽ӿ�
	* \param addr ��ַ
	* \param compress �ײ����ݴ����Ƿ�֧��ѹ��
	* \param checkSignal �Ƿ���������·�����ź�
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
			Zebra::logger->error("new zSocketʱ�ڴ治�㣡");
			
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

	//���epoll
	void fillPollFD(struct epoll_event& event_, WORD events)
	{
		mSocket->fillPollFD(event_, events);
	}

	bool checkVerifyTimeout(const zRTime &ct, const QWORD interval = 5000)const
	{
		return (lifeTime.elapse(ct) > interval);
	}

	//�Ƿ��Ѿ�������¼���־
	bool isFdsrAdd()
	{
		return fdsradd;
	}

	//��������¼���־
	bool fdsrAdd()
	{
		fdsradd = true;
		return fdsradd;
	}

	virtual int verifyConn()
	{
		return 1;
	}
	// 1�ɹ� 0��Ҫ�ȴ� -1ʧ�� �Ͽ�����
	virtual int waitSync()
	{
		return 1;
	}

	//�Ƿ���ճɹ� ��Ҫɾ���йظ�TCP���������Դ
	virtual int recycleConn()
	{
		return 1;
	}

	/**
	* \brief һ������������֤�Ȳ�������Ժ���Ҫ��ӵ�ȫ��������
	*
	* ���ȫ���������ⲿ����
	*
	*/
	virtual void addToContainer() {}

	/**
	* \brief ���������˳���ʱ����Ҫ��ȫ��������ɾ��
	*
	* ���ȫ���������ⲿ����
	*
	*/
	virtual void removeFromContainer() {}
	
	/**
	* \brief ��ӵ��ⲿ���������������Ҫ��֤������ӵ�Ψһ��
	*
	* \return ����Ƿ�ɹ�
	*/
	virtual bool uniqueAdd()
	{
		return true;
	}

	/**
	* \brief ���ⲿ����ɾ�������������Ҫ��֤������ӵ�Ψһ��
	*
	* \return ɾ���Ƿ�ɹ�
	*/
	virtual bool uniqueRemove()
	{
		return true;
	}

	/**
	* \brief ����Ψһ����֤ͨ�����
	*
	*/
	void setUnique()
	{
		uniqueVerified = true;
	}

	/**
	* \brief �ж��Ƿ��Ѿ�ͨ����Ψһ����֤
	*
	* \return �Ƿ��Ѿ�ͨ����Ψһ�Ա��
	*/
	bool isUnique() const
	{
		return uniqueVerified;
	}

	/**
	* \brief �ж��Ƿ������߳�����Ϊ�ȴ��Ͽ�����״̬
	*
	* \return true or false
	*/
	bool isTerminateWait()
	{
		return terminate_wait;
	}


	/**
	* \brief �ж��Ƿ������߳�����Ϊ�ȴ��Ͽ�����״̬
	*
	* \return true or false
	*/
	void TerminateWait()
	{
		terminate_wait = true;
	}

	/**
	* \brief �ж��Ƿ���Ҫ�ر�����
	*
	* \return true or false
	*/
	bool isTerminate() const
	{
		return terminate_no != terminate;
	}

	/**
	* \brief ��Ҫ�����Ͽ��ͻ��˵�����
	*
	* \param method ���ӶϿ���ʽ
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
	* \brief ��ȡ��������ǰ״̬
	* \return ״̬
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
	* \brief ���״̬���ַ�������
	*
	*
	* \param state ״̬
	* \return ����״̬���ַ�������
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
	* \brief �������ӵ�IP��ַ
	* \return ���ӵ�IP��ַ
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

	//�յ������ź�
	void clearTick()
	{
		tick = false;
	}
	
	//���������ź�
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
	* \brief �������źŷ��ͼ��
	*
	* \return ����Ƿ�ɹ�
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

	typedef std::vector<zThread *> Container; //��������

	zThreadGroup();
	~zThreadGroup();
	void add(zThread* thread);
	zThread *getByIndex(const Container::size_type index);
	zThread *operator[] (const Container::size_type index);

	void joinAll();
	void execAll(CallBack &cb);

private:
	Container vts;	//�߳�����
	zRWLock rwLock;	//��д��

};


class zTCPTaskPool : private zNoncopyable
{
private:
	const int maxConns; //�̳߳ز��д�������������
	static const int maxVerifyThreads = 4;
	zThreadGroup verifyThreads;		//��֤�߳̿����ж��
	zSyncThread *syncThread;	//�ȴ�ͬ���߳�ֻ��һ��
	static const int minThreadCount = 1;//�̳߳���ͬʱ�����������̵߳�������
	int maxThreadCount;		//������
	zThreadGroup okayThreads;		//�������߳�

	zRecycleThread* recycleThread;	//���ӻ����߳�
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
	static zService *serviceInst;    /**< ���Ψһʵ��ָ�룬���������࣬��ʼ��Ϊ��ָ�� */

	std::string name;          /**< �������� */
	bool terminate;            /**< ���������� */

public:
	Timer  _one_sec_; // �붨ʱ��

	virtual ~zService(){};
	//����HUP�źź���
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
	* \brief ȷ�Ϸ�������ʼ���ɹ��������������ص�����
	*
	* \return ȷ���Ƿ�ɹ�
	*/
	virtual bool validate()
	{
		return true;
	}

	/**
	* \brief �����������ص���������Ҫ���ڼ�������˿ڣ��������false���������򣬷���true����ִ�з���
	*
	* \return �ص��Ƿ�ɹ�
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


// [ranqd] ���������߳���
class zAcceptThread : public zThread
{
public:
	zAcceptThread(zNetService* p, const std::string &name) : zThread(name)
	{
		pService = p;
	}

	zNetService* pService;

	void run()         // [ranqd] ���������̺߳���
	{
		while (!isFinal())
		{
			//Zebra::logger->debug("���������߳̽�����");
			struct sockaddr_in addr;
			if (pService->tcpServer != NULL)
			{
				int retcode = pService->tcpServer->Accept(&addr);
				if (retcode >= 0)
				{
					//�������ӳɹ�����������
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
	const bool compress; //�Ƿ�ѹ��

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
