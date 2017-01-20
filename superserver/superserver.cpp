#include "superserver.h"

SuperService* SuperService::instance = NULL;

bool SuperService::init()
{
	Zebra::logger->info("SuperService::init");

	int state = state_none;

	if (false)
	{
		state = state_maintain;
	}
	taskPool = new zTCPTaskPool(2048,state);
	//创建了很多子线程 验证连接 同步连接 处理连接消息 回收连接
	if (NULL == taskPool || !taskPool->init())
	{
		Zebra::logger->error("线程池创建失败!");
		return false;
	}

	//创建了接受连接的子线程
	if (!zNetService::init(wdPort))
	{
		Zebra::logger->error("zNetService init 失败 wdPort(%d)", wdPort);
		return false;
	}

	SuperTimeTick::getInstance().start();
	
	return true;

}

void SuperService::newTCPTask(const SOCKET sock, const struct sockaddr_in *addr)
{
	ServerTask *tcpTask = new ServerTask(taskPool,sock, addr);
	if (tcpTask == NULL)
	{
		Zebra::logger->error("ServerTask创建失败  内存不足");
		close(sock);
		return;
	}
	else if (!taskPool->addVerify(tcpTask))
	{
		
		SAFE_DELETE(tcpTask);
	}
}

