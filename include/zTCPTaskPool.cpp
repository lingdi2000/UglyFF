#include "engine.h"
#include<sys/epoll.h>
#include<vector>
zTCPTask* g_DeletLog = NULL;

zMutex g_DeleteLogLock;

int zTCPTaskPool::usleep_time = 50000; //ѭ���ȴ�ʱ��

//��ν���ⲿ����
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

	//���¼��룿
	inline void check_queue()
	{
		mlock.lock();
		while (!_queue.empty())
		{
			mlock.lock();
			zTCPTask *task = _queue.back();
			_queue.pop_back();
			_add(task); // ������ʵ�ֵ�_add
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
		check_queue();  //�������߳̽������� task ��ӵ� tasks��
		m_Lock.lock();
		if (!tasks.empty())
		{
			for (i = 0, it = tasks.begin(); it != tasks.end();)
			{
				zTCPTask* task = *it;
				if (task->checkVerifyTimeout(currentTime))
				{
					//����ָ��ʱ�����
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
			//��ʱ������Ժ� 
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
						//�����socket
						//vector��ɾ���÷�
						it = tasks.erase(it);
						task_count = tasks.size();
						task->resetState();
						Zebra::logger->error("��֤tcptask  WaitRecv �����socket");
						pool->addRecycle(task);
					}
					else if (ret > 0)
					{
						// > 0 ��������������
						switch (task->verifyConn())
						{
						case 1:
							//��֤�ɹ�
							it = tasks.erase(it); //��֤�ɹ�Ҳȡ����
							task_count = tasks.size();

							if (task->uniqueAdd())
							{
								//Ψһ����֤�ɹ�,��ȡ��һ��״̬
								Zebra::logger->debug("�ͻ���Ψһ����֤�ɹ�");
								task->setUnique();
								pool->addSync(task);
							}
							else
							{
								//Ψһ����֤ʧ��,������������
								Zebra::logger->debug("�ͻ���Ψһ����֤ʧ��");
								task->resetState();						
								pool->addRecycle(task);
							}

							break;
						case -1:
							//��֤ʧ�� ����
							it = tasks.erase(it);
							task_count = tasks.size();
							task->resetState();
							Zebra::logger->error("��֤tcptask ��֤���� �����socket");
							pool->addRecycle(task);
							break;
						default:
							//��ʱ,����ᴦ��
							i++;
							it++;
							break;
						}
					}
					else
					{
						// = 0 ����û������
						i++;
						it++;
					}
				}


			}
		}
		//������ܺ�ʱ �� 
		m_Lock.unlock();

		zThread::msleep(50);
	}
	
	//���������ѭ��˵���߳�Ҫ������
	//�����еȴ���֤�����е����Ӽ��뵽���ն�����,������Щ����
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
	zTCPTaskPool *pool; //�����̳߳�
	zTCPTaskContainer tasks;  /**< �����б� */
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
	* \brief ��������
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
		//	Zebra::logger->debug("zSyncThreadѭ��ʱ�䣺%d ms", GetTickCount() - dwBeginTime);
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
					//�ȴ������߳�ͬ����֤�ɹ�
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
					//�ȴ������߳�ͬ����֤ʧ��,��Ҫ��������
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

	Zebra::logger->debug("zSyncThread ����runѭ�� %s", getThreadName().c_str());
	//�����еȴ�ͬ����֤�����е����Ӽ��뵽���ն�����,������Щ����
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

	Timer  _one_sec_; // �붨ʱ��
	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< �����б� */
	zTCPTaskContainer::size_type task_count;      /**< tasks����(��֤�̰߳�ȫ*/

	//pollfdContainer pfds;

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();

		tasks.push_back(task);
		task_count = tasks.size();

		task->ListeningRecv(false); //�����Ȼ������Ϣ
		m_Lock.unlock();
	}


	void remove(zTCPTask_IT &it, int p)
	{

	}

public:

	static const zTCPTaskContainer::size_type connPerThread = 512;  /**< ÿ���̴߳����������� */

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zOkayThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zOkayThread"))
		: zThread(name), _one_sec_(1) ,pool(pool)
	{
		task_count = 0;
	}

	/**
	* \brief ��������
	*
	*/
	~zOkayThread()
	{
	}

	void run();

	/**
	* \brief ������������ĸ���
	* \return ����̴߳��������������
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
	bool check = false; //check�Ƿ���Ҫ �������ź�,�Լ����ӵ�״̬
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
					//�������ź�ָ�� ����Ƿ�ʱ�����߷����ź�
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
						* ������״̬���������,
						* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
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
					
						int retcode = task->WaitRecv(false); //�����������׽ӿ��Ƿ����
						if (retcode == -1)
						{
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("zOkayThread::run: �׽ӿ��쳣����");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if (retcode > 0)
						{
							//�׽ӿ�׼���ö�����
							if (!task->ListeningRecv(true))
							{
								Zebra::logger->debug("zOkayThread::run: �׽ӿڶ���������");
								task->Terminate(zTCPTask::terminate_active);
							}
						}
						retcode = task->WaitSend(false);
						if (retcode == -1)
						{
							//�׽ӿڳ��ִ���
							Zebra::logger->debug("zOkayThread::run: �׽ӿ�д�����쳣����");
							task->Terminate(zTCPTask::terminate_active);
						}
						else if (retcode == 1)
						{
							//�׽ӿ�׼������д�����
							if (!task->ListeningSend())
							{
								Zebra::logger->debug("zOkayThread::run: �׽ӿ�д�������� port = %u", task->getPort());

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
	//����ѭ��
	for (i = 0, it = tasks.begin(); it != tasks.end();)
	{
		zTCPTask *task = *it;
		it = tasks.erase(it);
		//state_sync -> state_okay
		/*
		* whj
		* ������״̬���������,
		* ����ᵼ��һ��taskͬʱ�������߳��е�Σ�����
		*/
		task->getNextState();
		pool->addRecycle(task);
	}

}


class zRecycleThread : public zThread, public zTCPTaskQueue
{

private:

	zTCPTaskPool *pool;
	zTCPTaskContainer tasks;  /**< �����б� */

	zMutex m_Lock;

	void _add(zTCPTask *task)
	{
		m_Lock.lock();
		tasks.push_back(task);
		m_Lock.unlock();
	}

public:

	/**
	* \brief ���캯��
	* \param pool ���������ӳ�
	* \param name �߳�����
	*/
	zRecycleThread(
		zTCPTaskPool *pool,
		const std::string &name = std::string("zRecycleThread"))
		: zThread(name), pool(pool)
	{}

	/**
	* \brief ��������
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
					//���մ�����ɿ����ͷ���Ӧ����Դ
					it = tasks.erase(it);
					if (task->isUnique())
						//����Ѿ�ͨ����Ψһ����֤,��ȫ��Ψһ������ɾ��
						task->uniqueRemove();
					task->getNextState();
					//				if( !task->UseIocp() ) // [ranqd] ʹ��Iocp�����Ӳ����������
					//					g_RecycleLog[task] = 0;
					SAFE_DELETE(task);
					break;
				default:
					//���ճ�ʱ,�´��ٴ���
					it++;
					break;
				}
			}
		}
		m_Lock.unlock();

		zThread::msleep(200);
	}

	//�������е�����

	Zebra::logger->info("zRecycleThread::final ׼��������");
	for (it = tasks.begin(); it != tasks.end();)
	{
		//���մ�����ɿ����ͷ���Ӧ����Դ
		zTCPTask *task = *it;
		it = tasks.erase(it);
		if (task->isUnique())
			//����Ѿ�ͨ����Ψһ����֤,��ȫ��Ψһ������ɾ��
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
	// ���ȵķ��䵽��ͬ�� �߳���
	zVerifyThread *pVerifyThread = (zVerifyThread *)verifyThreads.getByIndex(hashCode++ % maxVerifyThreads);

	if (pVerifyThread)
	{
		task->getNextState();
		pVerifyThread->add(task);//��ӵ�tcpqueue��������
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

	//���ҳ� �����������ٵ��߳� �ȸ��̴߳�������Ӵﵽ���ֵ����ʹ����һ���߳�
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
				//����߳�û�е��ù�start
				nostart = pOkayThread;
				break;
			}
		}
		else
		{
			//һ�㲻�����������
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
			Zebra::logger->debug("zTCPTaskPool���������߳�");
		
			task->getNextState();
			//����߳�ͬʱ�������������û�е�������
			nostart->add(task);
			return true;
		}
		else
			Zebra::logger->fatal("zTCPTaskPool���ܴ��������߳�");
	}

	Zebra::logger->fatal("zTCPTaskPoolû���ҵ����ʵ��߳�����������");
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
	//��ʼ����֤�߳�
	for (int i = 0; i < maxVerifyThreads; i++)
	{
		std::ostringstream name;
		name << "zVerifyThread[" << i << "]";

		zVerifyThread *pVerifyThread = new zVerifyThread(this, name.str());
		if (NULL == pVerifyThread)
		{
			Zebra::logger->error("new ��֤�߳� %s �ڴ治��", name.str().c_str());
			return false;
		}
		if (!pVerifyThread->start())
		{
			Zebra::logger->error("������֤�߳� %s start ʧ��", name.str().c_str());
			return false;
		}

		verifyThreads.add(pVerifyThread);
	}


	syncThread = new zSyncThread(this);
	if (syncThread && !syncThread->start())
	{
		Zebra::logger->error("����ͬ���߳�ʧ��");
		return false;
	}

	//����߳������� ���Խ������ӵ� ������� ���� ÿ�� ok�߳̿��Դ���� ������
	maxThreadCount = (maxConns + zOkayThread::connPerThread - 1) / zOkayThread::connPerThread;
	Zebra::logger->debug("���TCP������%d,ÿ�߳�TCP������%d,�̸߳���%d", maxConns, zOkayThread::connPerThread, maxThreadCount);
	for (int i = 0; i < maxThreadCount; i++)
	{
		std::ostringstream name;
		name << "zOkayThread[" << i << "]";
		zOkayThread *pOkayThread = new zOkayThread(this, name.str());
		if (NULL == pOkayThread)
		{
			Zebra::logger->error("new ��֤�߳� %s �ڴ治��", name.str().c_str());
			return false;
		}
			
		if (!pOkayThread->start())
		{
			Zebra::logger->error("������֤�߳� %s start ʧ��", name.str().c_str());
			return false;
		}
		okayThreads.add(pOkayThread);
	}

	//������ʼ�������̳߳�
	recycleThread = new zRecycleThread(this);
	if (recycleThread && !recycleThread->start())
	{
		Zebra::logger->error("���������߳� ʧ��");
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