#include "superserver.h"
#include"../include/engine.h"
#include<iostream>

#include <stdarg.h>
#include"../include/log4cplus/logger.h"
#include"../include/log4cplus/appender.h"
#include"../include/log4cplus/layout.h"
#include"../include/log4cplus/fileappender.h"
#include "../include/log4cplus/loggingmacros.h" 
#include "../include/log4cplus/ndc.h"



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
	

	//SuperTimeTick::getInstance().start();
	
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

	struct epoll_event epEvent;
	epEvent.events = EPOLLIN;
	epEvent.data.ptr = tcpTask->getSock();
	if (epEvent.data.ptr == NULL)
	{
		Zebra::logger->error("newTCPTask 新连接socket加入epoll");
		SAFE_DELETE(tcpTask);
		return;
	}
	epoll.add(tcpTask->getSock(), &epEvent);

}

namespace Zebra
{
	volatile QWORD qwGameTime = 0;

	zLogger *logger = NULL;

	zProperties global;

	/**
	* \brief 初始化一些全局变量
	*
	*/
	//static void initGlobal()  __attribute__ ((constructor));

};



int main()
{
	Zebra::logger = new zLogger("SuperServer", "/home/lingdi2000/log/superserver.log");

	SuperService* supservice = SuperService::getInstance();
	supservice->setPort(8090);
	supservice->main();

	return 0;
}
