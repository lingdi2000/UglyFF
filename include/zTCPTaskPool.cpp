#include "engine.h"
#include<sys/epoll.h>
#include<vector>
zTCPTask* g_DeletLog = NULL;

zMutex g_DeleteLogLock;

int zTCPTaskPool::usleep_time = 50000; //循环等待时间

//所谓的外部容器
typedef std::vector<zTCPTask *> zTCPTaskContainer;
typedef zTCPTaskContainer::iterator zTCPTask_IT;

typedef std::vector<struct epoll_event> epollEventContainer;

class zTCPTaskQueue
{
protected:
	virtual void _add(zTCPTask*) = 0;
	DWORD _size;
private:
	zMutex mlock;
	std::vector<zTCPTask *> _queue;

public:
	zTCPTaskQueue() :_size(0)
	{

	}
	virtual ~zTCPTaskQueue(){}
	inline void add(zTCPTask* task)
	{
		mlock.lock();
		_queue.push_back(task);
		_size++;
		mlock.unlock();
	}

	//重新加入？
	inline void check_queue()
	{
		mlock.lock();
		while (!_queue.empty())
		{
			mlock.lock();
			zTCPTask *task = _queue.back();
			_queue.pop_back();
			_add(task); // 派生类实现的_add
		}
		_size = 0;
		mlock.unlock();
	}
};


class zVerifyThread : public zThread, public zTCPTaskQueue
{
private:
	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;

	zTCPTaskContainer::size_type task_count;

	zMutex m_Lock;
	void _add(zTCPTask *task)
	{
		Zebra::logger->debug("zVerifyThread::_add");
		m_Lock.lock();
		
		tasks.push_back(task);
		task_count = tasks.size();
		
		m_Lock.unlock();
		Zebra::logger->error("zVerifyThread::_add_end");
	}

	void remove(zTCPTask_IT &it, int p)
	{
		
	}

public:
	zVerifyThread(zTCPTaskPool* pool, const std::string& name = "zVerifyThread")
		:zThread(name), pool(pool), task_count(0)
	{
		
	}
	~zVerifyThread()
	{
	}

	void run();
};



void zVerifyThread::run()
{
	Zebra::logger->debug("zVerifyThread::run");
	zRTime currentTime;
	zTCPTask_IT it, next;
	epollEventContainer::size_type i;

	//DWORD dwBegin = 0;

	while (!isFinal())
	{
		currentTime.now();
		check_queue();  //把其他线程交过来的 task 添加到 tasks内
		m_Lock.lock();
		if (!tasks.empty())
		{
			for (i = 0, it = tasks.begin(); it != tasks.end();)
			{
				zTCPTask* task = *it;
				if (task->checkVerifyTimeout(currentTime))
				{
					//超过指定时间回收
					it = tasks.erase(it);
					task_count = tasks.size();
					task->resetState();
					pool->addRecycle(task);
				}
				else
				{
					i++;
					it++;
				}
			}
			//超时检查了以后 
			if (!tasks.empty())
			{
				int  i = 0;
				//bool state = false;

				for (i = 0, it = tasks.begin(); it != tasks.end();)
				{
					zTCPTask *task = *it;
					int ret = task->WaitRecv(false);
					if (ret == -1)
					{
						//错误的socket
						//vector的删除用法
						it = tasks.erase(it);
						task_count = tasks.size();
						task->resetState();
						Zebra::logger->error("验证tcptask  WaitRecv 错误的socket");
						pool->addRecycle(task);
					}
					else if (ret > 0)
					{
						// > 0 缓冲区中有数据
						switch (task->verifyConn())
						{
						case 1:
							//验证成功
							it = tasks.erase(it); //验证成功也取出来
							task_count = tasks.size();

							if (task->uniqueAdd())
							{
								//唯一性验证成功,获取下一个状态
								Zebra::logger->debug("客户端唯一性验证成功");
								task->setUnique();
								pool->addSync(task);
							}
							else
							{
								//唯一性验证失败,回收连接任务
								Zebra::logger->debug("客户端唯一性验证失败");
								task->resetState();						
								pool->addRecycle(task);
							}

							break;
						case -1:
							//验证失败 回收
							it = tasks.erase(it);
							task_count = tasks.size();
							task->resetState();
							Zebra::logger->error("验证tcptask 验证错误 错误的socket");
							pool->addRecycle(task);
							break;
						default:
							//超时,下面会处理
							i++;
							it++;
							break;
						}
					}
					else
					{
						// = 0 估计没有数据
						i++;
						it++;
					}
				}


			}
		}
		//这个锁很耗时 啊 
		m_Lock.unlock();

		zThread::msleep(50);
	}
	
	//如果跳出了循环说明线程要结束了
	//把所有等待验证队列中的连接加入到回收队列中,回收这些连接
	if (tasks.size() == 0)
		return;
	for (i = 0, it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		/*remove(it, i);*/
		it = tasks.erase(it);
		task->resetState();
		pool->addRecycle(task);
	}

}


class zSyncThread :public zThread, public zTCPTaskQueue
{
private:
	zTCPTaskPool *pool; //所属线程池
	zTCPTaskContainer tasks;  /**< 任务列表 */
	zMutex m_Lock;
	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:
	zSyncThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zSyncThread"))
		: zThread(name), pool(pool)
	{}
	/**
	* \brief 析构函数
	*
	*/
	~zSyncThread() {};

	void run();

};


void zSyncThread::run()
{
	Zebra::logger->debug("zSyncThread::run");
	zTCPTask_IT it;
	//DWORD dwBeginTime = 0;
	while (!isFinal())
	{

		//fprintf(stderr,"zVerifyThread::run\n");

		//if( dwBeginTime != 0 )
		//{
		//	Zebra::logger->debug("zSyncThread循环时间：%d ms", GetTickCount() - dwBeginTime);
		//}

		//dwBeginTime = GetTickCount();

		//fprintf(stderr,"zSyncThread::run\n");
		check_queue();

		m_Lock.lock();
		if (!tasks.empty())
		{
			for (it = tasks.begin(); it != tasks.end();)
			{
				zTCPTask *task = (*it);
				switch (task->waitSync())
				{
				case 1:
					//等待其它线程同步验证成功
					it = tasks.erase(it);
					if (!pool->addOkay(task))
					{
						task->resetState();
						pool->addRecycle(task);
					}
					break;
				case 0:
					it++;
					break;
				case -1:
					//等待其它线程同步验证失败,需要回收连接
					it = tasks.erase(it);
					task->resetState();
					pool->addRecycle(task);
					break;
				}
			}
		}
		m_Lock.unlock();
		zThread::msleep(200);
	}

	Zebra::logger->debug("zSyncThread 跳出run循环 %s", getThreadName().c_str());
	//把所有等待同步验证队列中的连接加入到回收队列中,回收这些连接
	for (it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		it = tasks.erase(it);
		task->resetState();
		pool->addRecycle(task);
	}
}

class zOkayThread : public zThread, public zTCPTaskQueue
{

private:

	Timer  _one_sec_; // 秒定时器
	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< 任务列表 */
	zTCPTaskContainer::size_type task_count;      /**< tasks计数(保证线程安全*/

	//pollfdContainer pfds;

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();

		tasks.push_back(task);
		task_count = tasks.size();

		task->ListeningRecv(false); //这里居然处理消息
		m_Lock.unlock();
	}


	void remove(zTCPTask_IT &it, int p)
	{

	}

public:

	static const zTCPTaskContainer::size_type connPerThread = 512;  /**< 每个线程带的连接数量 */

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	zOkayThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zOkayThread"))
		: zThread(name), _one_sec_(1) ,pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief 析构函数
	*
	*/
	~zOkayThread()
	{
	}

	void run();

	/**
	* \brief 返回连接任务的个数
	* \return 这个线程处理的连接任务数
	*/
	const zTCPTaskContainer::size_type size() const
	{
		return task_count + _size;
	}

};



void zOkayThread::run()
{
	Zebra::logger->debug("zOkayThread::run");
	zRTime currentTime;
	zTCPTask_IT it, next;
	epollEventContainer::size_type i;

	int time = pool->usleep_time;
	epollEventContainer::iterator iter_r;
	epollEventContainer pfds_r;

	zTCPTaskContainer tasks_r;
	bool check = false; //check是否需要 检查测试信号,以及连接的状态
	//DWORD dwBeginTime = 0;

	while (!isFinal())
	{

		currentTime.now();
		check_queue();
		if (check) 
		{
			m_Lock.lock();
			if (!tasks.empty())
			{
				for (i = 0, it = tasks.begin(); it != tasks.end();)
				{
					zTCPTask *task = *it;
					//检查测试信号指令 检查是否超时，或者发送信号
					task->checkSignal(currentTime);

					if (task->isTerminateWait())
					{
						task->Terminate();
					}
					if (task->isTerminate())
					{
						it = tasks.erase(it);
						task_count = tasks.size();
						// state_sync -> state_okay
						/*
						* whj
						* 先设置状态再添加容器,
						* 否则会导致一个task同时在两个线程中的危险情况
						*/
						task->getNextState();
						pool->addRecycle(task);
					}
					else
					{
						i++;
						it++;
					}

				}
			}

			m_Lock.unlock();
		}
		zThread::usleep_(time);
		time = 0;
		if (check)
		{
			if (time < 0)
			{
				time = 0;
			}
			continue;
		}
		if (time <= 0)
		{
			m_Lock.lock();
			if (!tasks.empty())
			{
				for (i = 0, it = tasks.begin(); it != tasks.end(); it++, i++)
				{
					zTCPTask *task = (*it);
					bool useEpoll = task->isUseEpoll();
					if (useEpoll)
					{
					
						int retcode = task->WaitRecv(false); //这个用来检查套接口是否可用
						if (retcode == -1)
						{
							//套接口出现错误
							Zebra::logger->debug("zOkayThread::run: 套接口异常错误");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if (retcode > 0)
						{
							//套接口准备好读操作
							if (!task->ListeningRecv(true))
							{
								Zebra::logger->debug("zOkayThread::run: 套接口读操作错误");
								task->Terminate(zTCPTask::terminate_active);
							}
						}
						retcode = task->WaitSend(false);
						if (retcode == -1)
						{
							//套接口出现错误
							Zebra::logger->debug("zOkayThread::run: 套接口写操作异常错误");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if (retcode == 1)
						{
							//套接口准备好了写入操作
							if (!task->ListeningSend())
							{
								Zebra::logger->debug("zOkayThread::run: 套接口写操作错误 port = %u", task->getPort());

								task->Terminate(zTCPTask::terminate_active);
							}
						}

					}
				}
			}

			m_Lock.unlock();
			time = pool->usleep_time;
		}
		check = true;
	}
	//跳出循环
	for (i = 0, it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		it = tasks.erase(it);
		//state_sync -> state_okay
		/*
		* whj
		* 先设置状态再添加容器,
		* 否则会导致一个task同时在两个线程中的危险情况
		*/
		task->getNextState();
		pool->addRecycle(task);
	}

}


class zRecycleThread : public zThread, public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< 任务列表 */

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief 构造函数
	* \param pool 所属的连接池
	* \param name 线程名称
	*/
	zRecycleThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zRecycleThread"))
		: zThread(name), pool(pool)
	{}

	/**
	* \brief 析构函数
	*
	*/
	~zRecycleThread() {};

	void run();

};

void zRecycleThread::run()
{
	Zebra::logger->debug("zRecycleThread::run");
	zTCPTask_IT it;
	while (!isFinal())
	{
		
		check_queue();

		int i;
		m_Lock.lock();
		if (!tasks.empty())
		{
			for (i = 0, it = tasks.begin(); it != tasks.end(); i++)
			{
				zTCPTask *task = *it;
				switch (task->recycleConn())
				{
				case 1:
					//回收处理完成可以释放相应的资源
					it = tasks.erase(it);
					if (task->isUnique())
						//如果已经通过了唯一性验证,从全局唯一容器中删除
						task->uniqueRemove();
					task->getNextState();
					//				if( !task->UseIocp() ) // [ranqd] 使用Iocp的连接不在这里回收
					//					g_RecycleLog[task] = 0;
					SAFE_DELETE(task);
					break;
				default:
					//回收超时,下次再处理
					it++;
					break;
				}
			}
		}
		m_Lock.unlock();

		zThread::msleep(200);
	}

	//回收所有的连接

	Zebra::logger->info("zRecycleThread::final 准备结束了");
	for (it = tasks.begin(); it != tasks.end();)
	{
		//回收处理完成可以释放相应的资源
		zTCPTask *task = *it;
		it = tasks.erase(it);
		if (task->isUnique())
			//如果已经通过了唯一性验证,从全局唯一容器中删除
			task->uniqueRemove();
		task->getNextState();
		SAFE_DELETE(task);
	}
}

const int zTCPTaskPool::getSize()
{
	struct MyCallback : zThreadGroup::CallBack
	{
		int size;
		MyCallback() : size(0) {}
		void exec(zThread *e)
		{
			zOkayThread *pOkayThread = (zOkayThread *)e;
			size += pOkayThread->size();
		}
	};

	MyCallback mcb;
	okayThreads.execAll(mcb);
	return mcb.size;

}

bool zTCPTaskPool::addVerify(zTCPTask* task)
{
	Zebra::logger->info("zTCPTaskPool::addverify ");
	static DWORD hashCode = 0;
	// 均匀的分配到不同的 线程组
	zVerifyThread *pVerifyThread = (zVerifyThread *)verifyThreads.getByIndex(hashCode++ % maxVerifyThreads);

	if (pVerifyThread)
	{
		task->getNextState();
		pVerifyThread->add(task);//添加到tcpqueue的容器中
		return true;
	}
	else
	{
		return false;
	}

}


bool zTCPTaskPool::addSync(zTCPTask* task)
{
	if (syncThread)
	{
		task->getNextState();
		syncThread->add(task);
		return true;
	}
	else
	{
		return false;
	}
}

bool zTCPTaskPool::addOkay(zTCPTask* task)
{
	Zebra::logger->debug("zTCPTaskPool::addOkay");
	zOkayThread *pmin = NULL, *nostart = NULL;

	//先找出 处理连接最少的线程 等该线程处理的连接达到最大值，再使用下一个线程
	for (int i = 0; i < maxThreadCount; i++)
	{
		zOkayThread *pOkayThread = (zOkayThread*)okayThreads.getByIndex(i);
		if (pOkayThread)
		{
			if (pOkayThread->isAlive())
			{
				if (NULL == pmin || pmin->size() > pOkayThread->size())
				{
					pmin = pOkayThread;
				}
			}
			else
			{
				//如果线程没有调用过start
				nostart = pOkayThread;
				break;
			}
		}
		else
		{
			//一般不会有这种情况
			return false;
		}
	}

	if (pmin && pmin->size() < zOkayThread::connPerThread)
	{
		task->getNextState();
		pmin->add(task);
		return true;
	}

	if (nostart)
	{
		if (nostart->start())
		{
			Zebra::logger->debug("zTCPTaskPool创建工作线程");
		
			task->getNextState();
			//这个线程同时处理的连接数还没有到达上限
			nostart->add(task);
			return true;
		}
		else
			Zebra::logger->fatal("zTCPTaskPool不能创建工作线程");
	}

	Zebra::logger->fatal("zTCPTaskPool没有找到合适的线程来处理连接");
	return false;
}


bool zTCPTaskPool::addRecycle(zTCPTask *task)
{
	if (recycleThread)
	{
		Zebra::logger->debug("zTCPTaskPool::addRecycle");

		recycleThread->add(task);
		return true;
	}
	else
	{
		return false;
	}
}

#include<ostream>

bool zTCPTaskPool::init()
{
	Zebra::logger->debug("zTCPTaskPool::init");
	//初始化验证线程
	for (int i = 0; i < maxVerifyThreads; i++)
	{
		std::ostringstream name;
		name << "zVerifyThread[" << i << "]";

		zVerifyThread *pVerifyThread = new zVerifyThread(this, name.str());
		if (NULL == pVerifyThread)
		{
			Zebra::logger->error("new 验证线程 %s 内存不够", name.str().c_str());
			return false;
		}
		if (!pVerifyThread->start())
		{
			Zebra::logger->error("创建验证线程 %s start 失败", name.str().c_str());
			return false;
		}

		verifyThreads.add(pVerifyThread);
	}


	syncThread = new zSyncThread(this);
	if (syncThread && !syncThread->start())
	{
		Zebra::logger->error("创建同步线程失败");
		return false;
	}

	//最大线程数等于 可以接受连接的 最大数量 除以 每个 ok线程可以处理的 连接数
	maxThreadCount = (maxConns + zOkayThread::connPerThread - 1) / zOkayThread::connPerThread;
	Zebra::logger->debug("最大TCP连接数%d,每线程TCP连接数%d,线程个数%d", maxConns, zOkayThread::connPerThread, maxThreadCount);
	for (int i = 0; i < maxThreadCount; i++)
	{
		std::ostringstream name;
		name << "zOkayThread[" << i << "]";
		zOkayThread *pOkayThread = new zOkayThread(this, name.str());
		if (NULL == pOkayThread)
		{
			Zebra::logger->error("new 验证线程 %s 内存不够", name.str().c_str());
			return false;
		}
			
		if (!pOkayThread->start())
		{
			Zebra::logger->error("创建验证线程 %s start 失败", name.str().c_str());
			return false;
		}
		okayThreads.add(pOkayThread);
	}

	//创建初始化回收线程池
	recycleThread = new zRecycleThread(this);
	if (recycleThread && !recycleThread->start())
	{
		Zebra::logger->error("创建回收线程 失败");
		return false;
	}

	return true;

}


void zTCPTaskPool::final()
{
	verifyThreads.joinAll();
	if (syncThread)
	{
		syncThread->final();
		syncThread->join();
		SAFE_DELETE(syncThread);
	}

	okayThreads.joinAll();
	if (recycleThread)
	{
		recycleThread->final();
		recycleThread->join();
		SAFE_DELETE(recycleThread);
	}


}