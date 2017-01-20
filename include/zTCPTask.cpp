#include "engine.h"
#include <assert.h>

#include <iostream>

CmdAnalysis zTCPTask::analysis("Taskָ�����ͳ��", 600);


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
	���׽ӿڽ������ݣ������в������,�ڵ��ýӿ�֮ǰ��֤�Ѿ����׽ӿڽ�����ѯ
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
			//ѭ����ȥȡ������Ϣ
			int nCmdLen = mSocket->recvToCmd_NoPoll(pstrCmd, sizeof(pstrCmd));
			if (nCmdLen <= 0)
			{
				//���cmd buffer����û�������˾ͷ��� ����ѭ��
				break;
			}
			else
			{
				Cmd::t_NullCmd *pNullCmd = (Cmd::t_NullCmd*)pstrCmd;
				if (Cmd::CMD_NULL == pNullCmd->cmd && Cmd::PARA_NULL == pNullCmd->para)
				{
					//���Զ���
					Zebra::logger->debug("������յ������ź�");
					clearTick();
				}
				else
				{
					//����Ϣ���д���
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

//����task״̬,�����óɻ���
void zTCPTask::resetState()
{
	zTCPTask_State old_state = getState();

	switch (old_state)
	{
	case notuse:
		/*
		* whj
		* ���sync�������ӵ�okay������ʧ�ܻ����okay״̬resetState�Ŀ�����
		*/
		//case okay:
	case recycle:
		//�����ܵ�
		Zebra::logger->fatal("zTCPTask::resetState:������ recycle -> recycle");
		break;
	case verify:
	case sync:
	case okay:
		//TODO ��ͬ�Ĵ���ʽ
		break;
	}

	setState(recycle);
	Zebra::logger->debug("zTCPTask::resetState(%s:%u),%s -> %s)", getIP(), getPort(), getStateString(old_state), getStateString(getState()));

}

void zTCPTask::checkSignal(const zRTime &ct)
{
	//�����Ҫ����Ҽ��ʱ��������
	if (ifCheckSignal() && checkInterval(ct))
	{
		if (checkTick())
		{
			//�����ź���ָ��ʱ�䷶Χ��û�з���
			Zebra::logger->error("�׽ӿڼ������ź�ʧ��");
			Terminate(zTCPTask::terminate_active);
		}
		else
		{
			//���Ͳ����ź�
			Cmd::t_NullCmd tNullCmd;
			//Zebra::logger->debug("����˷��Ͳ����ź�");
			if (sendCmd(&tNullCmd, sizeof(tNullCmd)))
				setTick();
		}
	}

}