#ifndef TEST_H_
#define TEST_H_
#include "../include/engine.h"

class SuperClient :public zTCPBufferClient
{
public:
	bool verified; //是否通过 验证

	SuperClient() :zTCPBufferClient("管理服务器客户端")
	{
		Zebra::logger->error("SuperClient::SuperClient");
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
}

bool SuperClient::msgParse_Startup(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen)
{
	
	
	Zebra::logger->error("SuperClient::msgParse_Startup(%d,%d,%d)",pNullCmd->cmd,pNullCmd->para,nCmdLen);
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
		//if (zSubNetService::subNetServiceInstance()->msgParse_SuperService(pNullCmd, nCmdLen)) return true;
		break;
	}
	Zebra::logger->error("SuperClient::msgParse(%d,%d,%d)", pNullCmd->cmd, pNullCmd->para, nCmdLen);
	return false;


}



#endif