#include "superserver.h"
#include"../include/engine.h"

void ServerManager::addServer(ServerTask *task)
{
	Zebra::logger->debug("ServerManager::addServer()");
	if (task)
	{
		mutex.lock();
		container.push_front(task); //在前面插入 因为会有很多的task所以使用list 但是为什么不用 map 或者set呢
		mutex.unlock();
	}
}
void ServerManager::removeServer(ServerTask *task)
{
	Zebra::logger->debug("ServerManager::removeServer()");
	if (task)
	{
		mutex.lock();
		container.remove(task);
		mutex.unlock();
	}
}

//返回一个指定ServerID的task
ServerTask* ServerManager::getServer(WORD wdServerID)
{
	Zebra::logger->info("ServerManager::getServer(serverid=%u)", wdServerID);
	Containter_const_iterator it;
	ServerTask *retval = NULL;

	mutex.lock();
	for (it = container.begin(); it != container.end(); it++)
	{
		if ((*it)->getID() == wdServerID)
		{
			retval = *it;
			break;
		}
	}
	mutex.unlock();

	return retval;
}
bool ServerManager::uniqueAdd(ServerTask *task)
{
	Zebra::logger->info("ServerManager::uniqueVerify id=%u", task->getID());
	ServerTaskHashmap_const_iterator it;
	mutex.lock();

	it = taskUniqueContainer.find(task->getID());
	if (it != taskUniqueContainer.end())
	{
		Zebra::logger->error("ServerManager::uniqueAdd");
		mutex.unlock();
		return false;
	}
	taskUniqueContainer.insert(ServerTaskHashmap_pair(task->getID(), task));
	mutex.unlock();
	return true;
}
//wdServerID代表的服务器是否已经连接
bool ServerManager::uniqueVerify(const WORD wdServerID)
{
	Zebra::logger->info("ServerManager::uniqueVerify id=%u", wdServerID);
	ServerTaskHashmap_const_iterator it;
	mutex.lock();
	it = taskUniqueContainer.find(wdServerID);
	if (it != taskUniqueContainer.end())
	{
		Zebra::logger->error("ServerManager::uniqueAdd");
		mutex.unlock();
		return false;
	}
	
	mutex.unlock();
	return true;
}
bool ServerManager::uniqueRemove(ServerTask *task)
{
	Zebra::logger->info("ServerManager::uniqueRemove id=%u", task->getID());
	ServerTaskHashmap_iterator it;
	mutex.lock();
	it = taskUniqueContainer.find(task->getID());
	if (it != taskUniqueContainer.end())
	{
		taskUniqueContainer.erase(it);
	}
	else
		Zebra::logger->warn("ServerManager::uniqueRemove");
	mutex.unlock();
	return true;
}
bool ServerManager::broadcast(const void *pstrCmd, int nCmdLen)
{
	bool retval = true;

	mutex.lock();
	for (Containter_const_iterator it = container.begin(); it != container.end(); it++)
	{
		if (!(*it)->sendCmd(pstrCmd, nCmdLen))
			retval = false;
	}
	mutex.unlock();

	return retval;
}
//将消息发送到指定的服务器
bool ServerManager::broadcastByID(const WORD wdServerID, const void *pstrCmd, int nCmdLen)
{
	Zebra::logger->info("ServerManager::broadcastByID(wdServerID=%u)", wdServerID);
	bool retval = false;

	mutex.lock();
	for (Containter_const_iterator it = container.begin(); it != container.end(); it++)
	{
		if ((*it)->getID() == wdServerID)
		{
			retval = (*it)->sendCmd(pstrCmd, nCmdLen);
			break;
		}
	}
	mutex.unlock();

	return retval;
}
//将消息发送到指定服务器类型的服务器上
bool ServerManager::broadcastByType(const WORD wdType, const void *pstrCmd, int nCmdLen)
{
	bool retval = true;

	mutex.lock();
	for (Containter_const_iterator it = container.begin(); it != container.end(); it++)
	{
		if ((*it)->getType() == wdType
			&& !(*it)->sendCmd(pstrCmd, nCmdLen))
			retval = false;
	}
	mutex.unlock();

	return retval;
}
const DWORD ServerManager::caculateOnlineNum()
{
	DWORD retval = 0;

	mutex.lock();
	for (Containter_const_iterator it = container.begin(); it != container.end(); it++)
	{
		if ((*it)->getType() == GATEWAYSERVER) //网关保持的连接就是客户端的连接
			retval += (*it)->getOnlineNum();
	}
	mutex.unlock();

	return retval;
}
void ServerManager::responseOther(const WORD srcID, const WORD wdServerID)
{
	Zebra::logger->info("ServerManager::responseOther(srcid=%u,wdServerID=%u)", srcID, wdServerID);
	ServerTaskHashmap_const_iterator it;
	mutex.lock();
	it = taskUniqueContainer.find(srcID);//taskUniqueContainer.find(srcID);
	if (it != taskUniqueContainer.end())
	{
		if (it->second)
			it->second->responseOther(wdServerID);
	}
	else
	{
		Zebra::logger->info("ServerManager::responseOther find srcid=%u", srcID);
	}
	mutex.unlock();
}