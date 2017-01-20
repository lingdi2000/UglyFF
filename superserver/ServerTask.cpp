#include "superserver.h"

ServerTask::ServerTask(zTCPTaskPool *tcptaskPool, const SOCKET sock, const struct sockaddr_in *addr) :zTCPTask(tcptaskPool,sock,addr)
{
	wdServerID = 0;
	wdServerType = UNKNOWNSERVER;
	bzero_(pstrIP, sizeof(pstrIP));
	wdPort = 0;
	OnlineNum = 0;
	sequenceOK = false;
	hasNotifyMe = false;
	hasprocessSequence = false;
}




ServerTask::~ServerTask()
{

}

//�����ÿ������������Ҫд�Ĵ��� ��������
int ServerTask::verifyConn()
{
	Zebra::logger->debug("ServerTask::verifyConn");
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
		if (nCmdLen <= 0)
			//����ֻ�Ǵӻ���ȡ���ݰ������Բ������û������ֱ�ӷ���
			return 0;
		else
		{
			//���ӽ�����ʱ�� �ͻ������ȷ�������һ���� ��֤����
			Cmd::Super::t_Startup_Request *ptCmd = (Cmd::Super::t_Startup_Request *)pstrCmd;
			if (Cmd::Super::CMD_STARTUP == ptCmd->cmd
				&& Cmd::Super::PARA_STARTUP_REQUEST == ptCmd->para)
			{
				if (verify(ptCmd->wdServerType, ptCmd->pstrIP))
				{
					Zebra::logger->debug("�ͻ�������ͨ����֤(%s:%u)", ptCmd->pstrIP, ptCmd->wdServerType);
					return 1;
				}
				else
				{
					Zebra::logger->error("�ͻ���������֤ʧ��(%s:%u)", ptCmd->pstrIP, ptCmd->wdServerType);
					return -1;
				}
			}
			else
			{
				Zebra::logger->error("�ͻ�������ָ����֤ʧ��(%s:%u)", ptCmd->pstrIP, ptCmd->wdServerType);
				return -1;
			}
		}
	}

}

//�ȴ��������������
int ServerTask::waitSync()
{

}
int ServerTask::recycleConn()
{
	return 1;
}
void ServerTask::addToContainer()
{

}
void ServerTask::removeFromContainer()
{

}
bool ServerTask::uniqueRemove()
{

}


bool ServerTask::msgParse(const Cmd::t_NullCmd *, const DWORD)
{
	return false;
}


void ServerTask::responseOther(const WORD wdServerID)
{
	Zebra::logger->info("ServerTask::responseOther(%u)", wdServerID);
	for (Container::iterator it = ses.begin(); it != ses.end(); ++it)
	{
		if (it->first.wdServerID == wdServerID)
		{
			Zebra::logger->debug("�ظ��ɹ� %d", it->first.wdServerID);
			it->second = true;
		}
	}
}
const WORD ServerTask::getID() const
{

}
const WORD ServerTask::getType() const
{

}
const DWORD ServerTask::getOnlineNum() const
{

}
bool ServerTask::checkSequenceTime()
{

}



bool ServerTask::verify(WORD wdType, const char *pstrIP)
{
	//������֤�Ƿ�  �ܹ�����
	Zebra::logger->debug("��֤TASK���� ������(%s) ����������(%d) ", pstrIP, wdType);

	
	//���ط�������Ϣ��������
	Cmd::Super::t_Startup_Response tCmd;
	tCmd.wdServerID = wdServerID;
	tCmd.wdPort = wdPort;
	strncpy(tCmd.pstrIP, pstrIP, sizeof(tCmd.pstrIP));
	if (!sendCmd(&tCmd, sizeof(tCmd)))
	{

		Zebra::logger->error("�����������ָ��ʧ��%u(%u)", wdServerID, wdPort);
		return false;
	}

	return true;
}
// �� verify  һ���������� ���÷�����Ϣ�Ϳ������������ķ�����
bool ServerTask::verifyTypeOK(const WORD wdType, std::vector<ServerTask *> &sv)
{


	return true;
}
bool ServerTask::processSequence()
{

}
bool ServerTask::notifyOther()
{

}
bool ServerTask::notifyOther(WORD dstID)
{

}
bool ServerTask::notifyMe()
{

}

bool ServerTask::msgParse_Startup(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{

	return false;
}
bool ServerTask::msgParse_Bill(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	return false;
}
bool ServerTask::msgParse_Gateway(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	return false;
}
bool ServerTask::msgParse_GmTool(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	return false;
}
bool ServerTask::msgParse_CountryOnline(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	return false;
}


const char* ServerTask::GetServerTypeName(const WORD wdServerType)
{

}