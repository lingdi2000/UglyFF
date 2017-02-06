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

	//如果不是joinable,需要回收线程资源
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
		Zebra::logger->debug("线程 %s 已经运行了,还在尝试运行 ", getThreadName().c_str());
		return true;
	}

	if (0 != pthread_create(&m_hThread, NULL, zThread::threadFunc,(void*)this))
	{
		Zebra::logger->debug("创建线程失败 %s 原因:%s ", getThreadName().c_str(), strerror(errno));
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
* \brief 重载[]运算符,按照index下标获取线程
* \param index 下标编号
* \return 线程
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
* \brief 对容器中的所有元素调用回调函数
* \param cb 回调函数实例
*/
void zThreadGroup::execAll(CallBack &cb)
{
	//让线程组里的线程执行 exec
	zRWLock_scope_rdlock scope_rdlock(rwLock);
	for (Container::iterator it = vts.begin(); it != vts.end(); ++it)
	{
		cb.exec(*it);
	}
}





