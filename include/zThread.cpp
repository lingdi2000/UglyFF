#include "engine.h"

void* zThread::threadFunc(void *arg)
{
	zThread *thread = (zThread*)arg;
	thread->mlock.lock();
	thread->alive = true;
	thread->mlock.unlock();

	thread->run();
	thread->mlock.lock();
	thread->alive = false;
	thread->mlock.unlock();

	//�������joinable,��Ҫ�����߳���Դ
	if (!thread->isJoinable())
	{
		SAFE_DELETE(thread);
	}
	else
	{
		
		thread->m_hThread = 0;
	}
	return NULL;
}


bool zThread::start()
{

	if (alive)
	{
		Zebra::logger->debug("�߳� %s �Ѿ�������,���ڳ������� ", getThreadName().c_str());
		return true;
	}

	if (0 != pthread_create(&m_hThread, NULL, zThread::threadFunc,(void*)this))
	{
		Zebra::logger->debug("�����߳�ʧ�� %s ԭ��:%s ", getThreadName().c_str(), strerror(errno));
		return false;
	}

	return true;
}

void zThread::join()
{
	//Zebra::logger->debug("zThread::join");
	pthread_join(m_hThread, NULL);
}


zThreadGroup::zThreadGroup() :vts(), rwLock()
{

}

zThreadGroup::~zThreadGroup()
{
	joinAll();
}

void zThreadGroup::joinAll()
{
	zRWLock_scope_wrlock scope_lock(rwLock);
	while (!vts.empty())
	{
		zThread *thread = vts.back();
		vts.pop_back();
		if (thread)
		{
			thread->final();
			thread->join();
			SAFE_DELETE(thread);
		}
	}
}

void zThreadGroup::add(zThread* thread)
{
	zRWLock_scope_wrlock scope_lock(rwLock);
	Container::iterator it = std::find(vts.begin(), vts.end(), thread);
	if (it == vts.end())
		vts.push_back(thread);

}
/**
* \brief ����[]�����,����index�±��ȡ�߳�
* \param index �±���
* \return �߳�
*/
zThread *zThreadGroup::operator[] (const Container::size_type index)
{
	zRWLock_scope_rdlock scope_rdlock(rwLock);

	if (index >= vts.size() || index < 0)
		return NULL;
	else
		return vts[index];
}


zThread *zThreadGroup::getByIndex(const Container::size_type index)
{
	zRWLock_scope_rdlock scope_rdlock(rwLock);
	if (index >= vts.size() || index < 0)
		return NULL;
	else
		return vts[index];
}

/**
* \brief �������е�����Ԫ�ص��ûص�����
* \param cb �ص�����ʵ��
*/
void zThreadGroup::execAll(CallBack &cb)
{
	//���߳�������߳�ִ�� exec
	zRWLock_scope_rdlock scope_rdlock(rwLock);
	for (Container::iterator it = vts.begin(); it != vts.end(); ++it)
	{
		cb.exec(*it);
	}
}





