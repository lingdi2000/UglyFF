#include "superserver.h"
#include "../include/engine.h"
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

//这个是每个服务器对需要写的代码 。。。。
int ServerTask::verifyConn()
{
	Zebra::logger->debug("ServerTask::verifyConn");
	int retcode = mSocket->recvToBuf_NoPoll();
	if (retcode > 0)
	{
		BYTE pstrCmd[zSocket::MAX_DATASIZE];
		int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
		Zebra::logger->debug("数据长度%d",nCmdLen);
		if (nCmdLen <= 0)
			//这里只是从缓冲取数据包，所以不会出错，没有数据直接返回
			return 0;
		else
		{
			/*for (int i = 0; i < nCmdLen; i++)
			{
			printf("%c", pstrCmd[i]);
			}
			printf("end\n");*/


			//连接建立的时候 客户端首先发过来的一定是 验证求情
			Cmd::Super::t_Startup_Request *ptCmd = (Cmd::Super::t_Startup_Request *)pstrCmd;
			if (Cmd::Super::CMD_STARTUP == ptCmd->cmd
				&& Cmd::Super::PARA_STARTUP_REQUEST == ptCmd->para)
			{
				if (verify(ptCmd->wdServerType, ptCmd->pstrIP))
				{
					Zebra::logger->debug("客户端连接通过验证(%s:%u)", ptCmd->pstrIP, ptCmd->wdServerType);
					return 1;
				}
				else
				{
					Zebra::logger->error("客户端连接验证失败(%s:%u)", ptCmd->pstrIP, ptCmd->wdServerType);
					return -1;
				}
			}
			else
			{
				Zebra::logger->error("客户端连接指令验证失败(%s:%u)", ptCmd->pstrIP, ptCmd->wdServerType);
				return -1;
			}
		}
	}
	//数据不全啊。。。
	return 0;
}

//等待服务器启动完成
int ServerTask::waitSync()
{
	return 1;
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
			Zebra::logger->debug("回复成功 %d", it->first.wdServerID);
			it->second = true;
		}
	}
}
const WORD ServerTask::getID() const
{
	return  0;
}
const WORD ServerTask::getType() const
{
	return  0;
}
const DWORD ServerTask::getOnlineNum() const
{
	return  0;
}
bool ServerTask::checkSequenceTime()
{
	return false;
}



bool ServerTask::verify(WORD wdType, const char *pstrIP)
{
	//用来验证是否  能够连接
	Zebra::logger->debug("验证TASK来自 服务器(%s) 服务器类型(%d) ", pstrIP, wdType);

	return true;
	//返回服务器信息到服务器
	Cmd::Super::t_Startup_Response tCmd;
	tCmd.wdServerID = wdServerID;
	tCmd.wdPort = wdPort;
	strncpy(tCmd.pstrIP, pstrIP, sizeof(tCmd.pstrIP));
	if (!sendCmd(&tCmd, sizeof(tCmd)))
	{

		Zebra::logger->error("向服务器发送指令失败%u(%u)", wdServerID, wdPort);
		return false;
	}

	return true;
}
// 和 verify  一样但是用于 不用返回消息就可以自行启动的服务器
bool ServerTask::verifyTypeOK(const WORD wdType, std::vector<ServerTask *> &sv)
{


	return true;
}
bool ServerTask::processSequence()
{
	return true;
}
bool ServerTask::notifyOther()
{
	return true;
}
bool ServerTask::notifyOther(WORD dstID)
{
	return true;
}
bool ServerTask::notifyMe()
{
	return true;
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
	return nullptr;
}


bool ServerTask::uniqueAdd()
{


	return true;
}


bool ServerTask::uniqueRemove()
{


	return false;
}