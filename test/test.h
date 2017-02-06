#ifndef TEST_H_
#define TEST_H_
#include "../include/engine.h"

class SuperClient :public zTCPBufferClient
{
public:
	bool verified; //�Ƿ�ͨ�� ��֤

	SuperClient() :zTCPBufferClient("����������ͻ���")
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

	//�����������Ͽ��Ļ�����ô�ý���ҲӦ�ý���
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
		Zebra::logger->debug("zSubNetServer��Ҫ��������� ");
		//if (zSubNetService::subNetServiceInstance()->msgParse_SuperService(pNullCmd, nCmdLen)) return true;
		break;
	}
	Zebra::logger->error("SuperClient::msgParse(%d,%d,%d)", pNullCmd->cmd, pNullCmd->para, nCmdLen);
	return false;


}



#endif