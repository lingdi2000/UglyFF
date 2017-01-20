#include "engine.h"
#include <assert.h>

#include <iostream>

CmdAnalysis zTCPTask::analysis("Task指令接收统计", 600);


bool zTCPTask::sendCmd(const void* pstrCmd, int nCmdLen)
{

	return mSocket->sendCmd(pstrCmd, nCmdLen, buffered);
}

bool zTCPTask::sendCmdNoPack(const void *pstrCmd, int nCmdLen)
{
	//Zebra::logger->debug("zTCPTask::sendCmdNoPack");
	return mSocket->sendCmdNoPack(pstrCmd, nCmdLen, buffered);
}

/* comment function 12/29/2016 yld
	从套接口接受数据，并进行拆包处理,在调用接口之前保证已经对套接口进行轮询
*/
bool zTCPTask::ListeningRecv(bool needRecv)
{
	int retCode = 0;
	if (needRecv){
		retCode = mSocket->recvToBuf_NoPoll();
	}

	if (retCode == -1)
	{
		Zebra::logger->debug("zTCPTask::ListeningRecv -1");
		return false;
	}
	else
	{
		do
		{
			BYTE pstrCmd[zSocket::MAX_DATASIZE];
			//循环内去取所有消息
			int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
			if (nCmdLen <= 0)
			{
				//如果cmd buffer里面没有数据了就返回 跳出循环
				break;
			}
			else
			{
				Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd*)pstrCmd;
				if (Cmd::CMD_NULL == pNullCmd->cmd && Cmd::PARA_NULL == pNullCmd->para)
				{
					//测试而已
					Zebra::logger->debug("服务端收到测试信号");
					clearTick();
				}
				else
				{
					//对消息进行处理
					msgParse(pNullCmd, nCmdLen);
				}

			}

		} while (true);
	}
	return true;
}


bool zTCPTask::ListeningSend()
{
	return mSocket->sync();
}

void zTCPTask::getNextState()
{
	zTCPTask_State old_state = getState();
	switch (old_state)
	{
	case notuse:
		setState(verify);
		break;
	case verify:
		setState(sync);
		break;
	case sync:
		buffered = true;
		addToContainer();
		setState(okay);
		break;
	case okay:
		removeFromContainer();
		setState(recycle);
		break;
	case recycle:
		setState(notuse);
		break;
	}

	Zebra::logger->debug("zTCPTask::getNextState(%s:%u),%s -> %s)", getIP(), getPort(), getStateString(old_state), getStateString(getState()));
}

//重置task状态,会设置成回收
void zTCPTask::resetState()
{
	zTCPTask_State old_state = getState();

	switch (old_state)
	{
	case notuse:
		/*
		* whj
		* 如果sync情况下添加到okay管理器失败会出现okay状态resetState的可能性
		*/
		//case okay:
	case recycle:
		//不可能的
		Zebra::logger->fatal("zTCPTask::resetState:不可能 recycle -> recycle");
		break;
	case verify:
	case sync:
	case okay:
		//TODO 相同的处理方式
		break;
	}

	setState(recycle);
	Zebra::logger->debug("zTCPTask::resetState(%s:%u),%s -> %s)", getIP(), getPort(), getStateString(old_state), getStateString(getState()));

}

void zTCPTask::checkSignal(const zRTime &ct)
{
	//如果需要检测且检测时间间隔到了
	if (ifCheckSignal() && checkInterval(ct))
	{
		if (checkTick())
		{
			//测试信号在指定时间范围内没有返回
			Zebra::logger->error("套接口检查测试信号失败");
			Terminate(zTCPTask::terminate_active);
		}
		else
		{
			//发送测试信号
			Cmd::t_NullCmd tNullCmd;
			//Zebra::logger->debug("服务端发送测试信号");
			if (sendCmd(&tNullCmd, sizeof(tNullCmd)))
				setTick();
		}
	}

}