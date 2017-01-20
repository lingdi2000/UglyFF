#include "engine.h"

bool zTCPClient::connect()
{
	Zebra::logger->debug("zTCPClient��ʼ���ӷ�����");
	int retcode = 0;
	int nSocket = 0;

	struct sockaddr_in addr;
	nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == nSocket)
	{
		Zebra::logger->error("�׽ӿڴ���ʧ��:%s ", strerror(errno));
		return false;
	}

	//�����׽ӿڷ��ͽ��ջ���,���ҿͻ��˵ı�����connect֮ǰ����
	int window_size = 128 * 1024;
	retcode = setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&window_size, sizeof(window_size));
	if (-1 == retcode)
	{
		::close(nSocket);
		Zebra::logger->error("�׽ӿڽ��ջ�������С�޸�ʧ��:%s ", strerror(errno));
		return false;
	}

	retcode = setsockopt(nSocket, SOL_SOCKET, SO_SNDBUF, (char*)&window_size,sizeof(window_size));
	if (-1 == retcode)
	{
		::close(nSocket);
		Zebra::logger->error("�׽ӿڷ��ͻ�������С�޸�ʧ��:%s ", strerror(errno));
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	addr.sin_port = htons(port);

	retcode = ::connect(nSocket,(struct sockaddr*)&addr,sizeof(addr));
	if (-1 == retcode)
	{
		::close(nSocket);
		Zebra::logger->error("�׽ӿ����ӷ�����(%s:%u)ʧ��:%s ", ip.c_str(),port,strerror(errno));
		return false;
	}

	pSocket = new zSocket(nSocket, &addr, compress);
	if (NULL == pSocket)
	{
		Zebra::logger->error("�����׽ӿڶ���zSocket(%s:%u)ʧ��:%s ", ip.c_str(), port, strerror(errno));
		::close(nSocket);
		return false;
	}

	Zebra::logger->info("������������(%s:%u)�����ӳɹ�", ip.c_str(), port);
	return true;
}

bool zTCPClient::sendCmd(const void *pstrCmd, const int nCmdLen)
{
	if (nCmdLen < 0 || pstrCmd == NULL)
	{
		return false;
	}
	else
	{
		return pSocket->sendCmd(pstrCmd, nCmdLen);
	}

}

zSocket* zTCPClient::getZSocket()
{
	return pSocket;
}

void zTCPClient::run()
{

}

void zTCPBufferClient::run()
{	
	//��Ҫ ���뵽epoll�ж�����
	while (!isFinal())
	{
		//��ʱ����epoll��������Ϣ
		if (pSocket->m_bUserEpoll)
		{
			int retcode = pSocket->WaitRecv(false);
			if (retcode == -1)
			{
				//socket ������
				Zebra::logger->error("zTCPBufferClient WaitRecv ����");
				break;
			}
			else if (retcode > 0)
			{
				if (!this->ListeningRecv())
				{
					Zebra::logger->error("zTCPBufferClient ListeningRecv ��ȡ����");
					break;
				}
			}

			retcode = pSocket->WaitSend(false);
			if (retcode == -1)
			{
				Zebra::logger->error("zTCPBufferClient WaitSend ����");
				break;
			}
			else if (retcode == 1)
			{
				if (!this->ListeningSend())
				{
					Zebra::logger->error("zTCPBufferClient ListeningSend ��ȡ����");
					break;
				}
			}

		}

	}

}
bool zTCPBufferClient::sendCmd(const void *pstrCmd, const int nCmdLen)
{
	Zebra::logger->debug("zTCPBufferClient::sendCmd");
	return zTCPClient::sendCmd(pstrCmd,nCmdLen);
}

bool zTCPBufferClient::ListeningRecv()
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::ListeningRecv");
#endif //_DEBUG
	//int retcode = pSocket->recvToBuf_NoPoll();
	//ȡ�� cmdBuff ����� ��Ϣ���д���
	while (true)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
		if (nCmdLen <= 0)
		{
			//û����Ϣ���͹���
			break;
		}
		else
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)
			{
				Zebra::logger->debug("�ͻ����յ������ź�");
				//����ָ��ʧ��,�˳�ѭ��,�����߳�
				if (!sendCmd(pstrCmd, nCmdLen)) return false;
			}
			else msgParse(pNullCmd, nCmdLen);
		}
	}
	return true;
}
bool zTCPBufferClient::ListeningSend()
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::ListeningSend");
#endif //_DEBUG
	if (pSocket)
	{
		return pSocket->sync();
	}
	else
	{
		return false;
	}
}
void zTCPBufferClient::sync()
{
#ifdef _DEBUG
	Zebra::logger->debug("zTCPBufferClient::sync");
#endif //_DEBUG
	if (pSocket)
		pSocket->force_sync();
}
