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
	//�����˺ܶ����߳� ��֤���� ͬ������ ����������Ϣ ��������
	if (NULL == taskPool || !taskPool->init())
	{
		Zebra::logger->error("�̳߳ش���ʧ��!");
		return false;
	}

	//�����˽������ӵ����߳�
	if (!zNetService::init(wdPort))
	{
		Zebra::logger->error("zNetService init ʧ�� wdPort(%d)", wdPort);
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
		Zebra::logger->error("ServerTask����ʧ��  �ڴ治��");
		close(sock);
		return;
	}
	else if (!taskPool->addVerify(tcpTask))
	{
		
		SAFE_DELETE(tcpTask);
	}
}

