#include "engine.h"

zNetService* zNetService::instance = NULL;

bool zNetService::init(WORD port)
{
	Zebra::logger->debug("zNetService init");
	if (!zService::init())
	{
		Zebra::logger->debug("zService::init() init");
		return false;
	}

	//初始化服务器
	tcpServer = new zTCPServer(serviceName);

	if (NULL == tcpServer)
	{
		Zebra::logger->error("创建TtcpServer 内存不够");
		return false;
	}

	if (!tcpServer->Bind(serviceName, port))
	{
		return false;
	}

	pAcceptThread =  new zAcceptThread(this, serviceName);
	if (pAcceptThread == NULL)
	{
		Zebra::logger->error("创建pAcceptThread失败内存不足");
		return false;
	}
	if (!pAcceptThread->start())
		return false;
	Zebra::logger->debug("zNetService::init bind(%s:%u)", serviceName.c_str(), port);
	return true;
}


bool zNetService::serviceCallback()
{
	
	zRTime currentTime;
	currentTime.now();
	if (_one_sec_(currentTime))
	{
		//zIocp::getInstance().UpdateNetLog();
	}
	usleep(10 * 1000);
	return true;


}

void zNetService::final()
{
	Zebra::logger->info("zNetService::final");

	SAFE_DELETE(tcpServer);
}