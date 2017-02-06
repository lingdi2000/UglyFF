#include "engine.h"

class SuperClient :public zTCPBufferClient
{
public:
	friend class zSubNetService;
	bool verified; //�Ƿ�ͨ�� ��֤

	SuperClient() :zTCPBufferClient("����������ͻ���")
	{
		Zebra::logger->debug("SuperClient::SuperClient");
	}

	~SuperClient() {};
	void run();
	bool msgParse_Startup(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);
	bool msgParse(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);

};

void SuperClient::run()
{
	zTCPBufferClient::run();

	//�����������Ͽ��Ļ�����ô�ý���ҲӦ�ý���
	zSubNetService::subNetServiceInstance()->Terminate();
}

bool SuperClient::msgParse_Startup(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	using namespace Cmd::Super;
	switch (pNullCmd->para)
	{
	case PARA_STARTUP_RESPONSE:
		{
			t_Startup_Response *ptCmd = (t_Startup_Response *)pNullCmd;

			Zebra::logger->error("PARA_STARTUP_RESPONSE %d,%d",ptCmd->wdServerID,ptCmd->wdPort);
			//�ӷ�����֪�� ���serverID ��ʲôtype ��Ҫʹ��ʲô�˿ڣ�����˿ڳ�ͻ
			zSubNetService::subNetServiceInstance()->setServerInfo(ptCmd);

			verified = true;

			return true;
		}

	}

	Zebra::logger->error("SuperClient::msgParse_Startup(%d,%d,%d)", pNullCmd->cmd, pNullCmd->para, nCmdLen);
	return false;
}

bool SuperClient::msgParse(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	switch (pNullCmd->cmd)
	{
	case Cmd::Super::CMD_STARTUP:
		if (msgParse_Startup(pNullCmd, nCmdLen)) return true;
		break;
	default:
		Zebra::logger->debug("zSubNetServer��Ҫ��������� ");
		if (zSubNetService::subNetServiceInstance()->msgParse_SuperService(pNullCmd, nCmdLen)) return true;
		break;
	}
	Zebra::logger->error("SuperClient::msgParse(%d,%d,%d)", pNullCmd->cmd, pNullCmd->para, nCmdLen);
	return false;


}



bool zSubNetService::msgParse_SuperService(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{

	return true;
}

bool zSubNetService::sendCmdToSuperServer(const void *pstrcmd, const int nCmdLen)
{

	return true;
}
void zSubNetService::setServerInfo(const Cmd::Super::t_Startup_Response *ptCmd)
{

	
}
bool zSubNetService::addServerEntry(const Cmd::Super::ServerEntry &entry)
{

	return true;
}

const Cmd::Super::ServerEntry* zSubNetService::getServerEntryById(const WORD wdServerID)
{

	return NULL;
}
const Cmd::Super::ServerEntry* zSubNetService::getServerEntryByType(const WORD wdServerType)
{

	return NULL;
}
const Cmd::Super::ServerEntry* zSubNetService::getNextServerEntryByType(const WORD wdServerType, const Cmd::Super::ServerEntry **prev)
{

	return NULL;
}

const WORD zSubNetService::getServerID() const
{
	return wdServerID;
}
const WORD zSubNetService::getServerType() const
{

	return wdServerType;
}


zSubNetService::zSubNetService(const std::string name, const WORD wdType) :zNetService(name), superClient(NULL)
{
	Zebra::logger->debug("zSubService::construction");
	
	subNetServiceInst = this;
	bzero(pstrIP,sizeof(pstrIP));
	superClient = new SuperClient();
	wdServerID = 0;
	wdServerType = wdType;
	wdPort = 0;

}
bool zSubNetService::init()
{
	Zebra::logger->debug("zSubNetServer::init");
try_agin:
	//superServer ip port
	while (!superClient->connect("127.0.0.1", 8090))
	{
		Zebra::logger->error("���ӹ��������ʧ�� 2s ����������...");
		sleep(2);
	}

	Zebra::logger->error("���ӹ���������ɹ�...");

	Cmd::Super::t_Startup_Request tCmd;
	tCmd.wdServerType = wdServerType;
	strncpy(tCmd.pstrIP, pstrIP, sizeof(tCmd.pstrIP));
	if (!superClient->sendCmd(&tCmd, sizeof(tCmd)))
	{
		Zebra::logger->error("�������������͵�½ָ��ʧ�ܣ�2�������.....\n");
		sleep(2);
		goto try_agin;
	}

	//���û����֤�ɹ�
	while (!superClient->verified)
	{
		ssize_t retcode = superClient->getZSocket()->RecvData();
		Zebra::logger->debug("���յ������ݳ���%d",retcode);
		if (retcode == -1)
		{
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
			{
				continue;
			}
			else
			{
				Zebra::logger->error("��������� ���ӶϿ� ���ش���");
				return -1;
			}
		}
		else if (retcode == 0)
		{
			Zebra::logger->error("��������� �������ӶϿ� ");
			return -1;
		}
		else
		{
			//�����������׽ӿ��Ƿ����
			int retcode = superClient->getZSocket()->WaitRecv(false);
			if (retcode == -1)
			{
				Zebra::logger->error("��������� ���ӶϿ� ���ش���2");
				return -1;
			}
			else if (retcode > 0)
			{
				//����Ϣ
				BYTE pstrCmd[zSocket::MAX_DATASIZE];
				int nCmdLen = superClient->getZSocket()->recvToCmd(pstrCmd, sizeof(pstrCmd), false);
				if (nCmdLen == -1)
				{
					Zebra::logger->error("��������� ���ӶϿ� ���ش���3");
					return -1;
				}
				else if (nCmdLen > 0)
				{
					if (!superClient->msgParse((Cmd::t_NullCmd *)pstrCmd, nCmdLen))
					{
						Zebra::logger->error("�ӹ���������յ������ָ��(%d:%d)������ʧ�ܣ�\n", ((Cmd::t_NullCmd *)pstrCmd)->cmd, ((Cmd::t_NullCmd *)pstrCmd)->para);
						superClient->getZSocket()->DisConnet();
						return false;
					}
				}
			}
			//�鿴�Ƿ�����Ϣ
		}
	}

	if (!zNetService::init(wdPort))
	{
		return false;
	}


	//���ӳɹ������ú��˷�������Ϣ
	Zebra::logger->info("zSubNetService::init %d,%d,%s:%d", wdServerType, wdServerID, pstrIP, wdPort);
	//start֮ǰ��socket����epoll,��ΪĿǰֻʵ��epollģʽ��
	if (epoll.add(superClient->getZSocket(), EPOLLIN) == -1)
	{
		Zebra::logger->error("supclient����epollsetʧ�� %s",strerror(errno));
		return false;
	}
	superClient->start();

	//������ʵ�ĳ�ʼ������
	return true;

}
bool zSubNetService::validate()
{
	Cmd::Super::t_Startup_OK tCmd;

	Zebra::logger->debug("zSubNetService::validate");
	tCmd.wdServerID = wdServerID;
	return superClient->sendCmd(&tCmd, sizeof(tCmd));
	return true;
}
void zSubNetService::final()
{
	Zebra::logger->debug("zSubNetService::final");
	zNetService::final();

	//�رյ����������������
	superClient->final();
	superClient->join();
	superClient->close();
	
}