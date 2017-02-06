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
	//��ʼ�� epoll �̳߳�
	epoll.create();
	pEpollThread = new zEpollRecvThread(this, &epoll, serviceName);
	if (pEpollThread == NULL)
	{
		Zebra::logger->error("Epoll �̴߳���ʧ��");
		return false;
	}

	if (!pEpollThread->start())
	{
		Zebra::logger->error("Epoll �̴߳���ʧ��");
		return false;
	}

	//�ȴ� epoll wait
	::usleep(200);



	//��ʼ��������
	tcpServer = new zTCPServer(serviceName);

	if (NULL == tcpServer)
	{
		Zebra::logger->error("����TtcpServer �ڴ治��");
		return false;
	}

	if (!tcpServer->Bind(serviceName, port))
	{
		Zebra::logger->error("TCPServer Bind ʧ��");	
		return false;
	}

	pAcceptThread =  new zAcceptThread(this, serviceName);
	if (pAcceptThread == NULL)
	{
		Zebra::logger->error("����pAcceptThreadʧ���ڴ治��");
		return false;
	}
	if (!pAcceptThread->start())
		return false;
	Zebra::logger->debug("zNetService::init bind(%s:%u) ", tcpServer->getIP(), port);
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


std::string zNetService::getServerName()
{
	return serviceName;
}