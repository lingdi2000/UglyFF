#include "engine.h"

class SuperClient :public zTCPBufferClient
{
public:
	friend class zSubNetService;
	bool verified; //是否通过 验证

	SuperClient() :zTCPBufferClient("管理服务器客户端")
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

	//与管理服务器断开的话，那么该进程也应该结束
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
			//从服务器知道 这个serverID 是什么type 需要使用什么端口，避免端口冲突
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
		Zebra::logger->debug("zSubNetServer需要处理的任务 ");
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
		Zebra::logger->error("连接管理服务器失败 2s 后重新连接...");
		sleep(2);
	}

	Zebra::logger->error("连接管理服务器成功...");

	Cmd::Super::t_Startup_Request tCmd;
	tCmd.wdServerType = wdServerType;
	strncpy(tCmd.pstrIP, pstrIP, sizeof(tCmd.pstrIP));
	if (!superClient->sendCmd(&tCmd, sizeof(tCmd)))
	{
		Zebra::logger->error("向管理服务器发送登陆指令失败，2秒后重试.....\n");
		sleep(2);
		goto try_agin;
	}

	//如果没有验证成功
	while (!superClient->verified)
	{
		ssize_t retcode = superClient->getZSocket()->RecvData();
		Zebra::logger->debug("接收到的数据长度%d",retcode);
		if (retcode == -1)
		{
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
			{
				continue;
			}
			else
			{
				Zebra::logger->error("管理服务器 连接断开 严重错误");
				return -1;
			}
		}
		else if (retcode == 0)
		{
			Zebra::logger->error("管理服务器 主动连接断开 ");
			return -1;
		}
		else
		{
			//这个用来检查套接口是否可用
			int retcode = superClient->getZSocket()->WaitRecv(false);
			if (retcode == -1)
			{
				Zebra::logger->error("管理服务器 连接断开 严重错误2");
				return -1;
			}
			else if (retcode > 0)
			{
				//有消息
				BYTE pstrCmd[zSocket::MAX_DATASIZE];
				int nCmdLen = superClient->getZSocket()->recvToCmd(pstrCmd, sizeof(pstrCmd), false);
				if (nCmdLen == -1)
				{
					Zebra::logger->error("管理服务器 连接断开 严重错误3");
					return -1;
				}
				else if (nCmdLen > 0)
				{
					if (!superClient->msgParse((Cmd::t_NullCmd *)pstrCmd, nCmdLen))
					{
						Zebra::logger->error("从管理服务器收到错误的指令(%d:%d)，启动失败！\n", ((Cmd::t_NullCmd *)pstrCmd)->cmd, ((Cmd::t_NullCmd *)pstrCmd)->para);
						superClient->getZSocket()->DisConnet();
						return false;
					}
				}
			}
			//查看是否有消息
		}
	}

	if (!zNetService::init(wdPort))
	{
		return false;
	}


	//连接成功且设置好了服务器信息
	Zebra::logger->info("zSubNetService::init %d,%d,%s:%d", wdServerType, wdServerID, pstrIP, wdPort);
	//start之前把socket加入epoll,因为目前只实现epoll模式的
	if (epoll.add(superClient->getZSocket(), EPOLLIN) == -1)
	{
		Zebra::logger->error("supclient加入epollset失败 %s",strerror(errno));
		return false;
	}
	superClient->start();

	//调用真实的初始化函数
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

	//关闭到管理服务器的连接
	superClient->final();
	superClient->join();
	superClient->close();
	
}