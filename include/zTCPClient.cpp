#include "engine.h"

bool zTCPClient::connect()
{
	Zebra::logger->debug("zTCPClient开始连接服务器");
	int retcode = 0;
	int nSocket = 0;

	struct sockaddr_in addr;
	nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == nSocket)
	{
		Zebra::logger->error("套接口创建失败:%s ", strerror(errno));
		return false;
	}

	//设置套接口发送接收缓冲,并且客户端的必须在connect之前设置
	int window_size = 128 * 1024;
	retcode = setsockopt(nSocket, SOL_SOCKET, SO_RCVBUF, (char*)&window_size, sizeof(window_size));
	if (-1 == retcode)
	{
		::close(nSocket);
		Zebra::logger->error("套接口接收缓冲区大小修改失败:%s ", strerror(errno));
		return false;
	}

	retcode = setsockopt(nSocket, SOL_SOCKET, SO_SNDBUF, (char*)&window_size,sizeof(window_size));
	if (-1 == retcode)
	{
		::close(nSocket);
		Zebra::logger->error("套接口发送缓冲区大小修改失败:%s ", strerror(errno));
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
		Zebra::logger->error("套接口连接服务器(%s:%u)失败:%s ", ip.c_str(),port,strerror(errno));
		return false;
	}

	pSocket = new zSocket(nSocket, &addr, compress);
	if (NULL == pSocket)
	{
		Zebra::logger->error("创建套接口对象zSocket(%s:%u)失败:%s ", ip.c_str(), port, strerror(errno));
		::close(nSocket);
		return false;
	}

	Zebra::logger->info("创建到服务器(%s:%u)的连接成功", ip.c_str(), port);
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
	//需要 加入到epoll中读数据
	while (!isFinal())
	{
		//暂时都用epoll来接收消息
		if (pSocket->m_bUserEpoll)
		{
			int retcode = pSocket->WaitRecv(false);
			if (retcode == -1)
			{
				//socket 出错了
				Zebra::logger->error("zTCPBufferClient WaitRecv 错误");
				break;
			}
			else if (retcode > 0)
			{
				if (!this->ListeningRecv())
				{
					Zebra::logger->error("zTCPBufferClient ListeningRecv 读取错误");
					break;
				}
			}

			retcode = pSocket->WaitSend(false);
			if (retcode == -1)
			{
				Zebra::logger->error("zTCPBufferClient WaitSend 错误");
				break;
			}
			else if (retcode == 1)
			{
				if (!this->ListeningSend())
				{
					Zebra::logger->error("zTCPBufferClient ListeningSend 读取错误");
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
	//取出 cmdBuff 里面的 消息进行处理
	while (true)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = pSocket->recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
		if (nCmdLen <= 0)
		{
			//没有消息发送过来
			break;
		}
		else
		{
			Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd *)pstrCmd;
			if (Cmd::CMD_NULL == pNullCmd->cmd
				&& Cmd::PARA_NULL == pNullCmd->para)
			{
				Zebra::logger->debug("客户端收到测试信号");
				//发送指令失败,退出循环,结束线程
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
