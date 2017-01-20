#include "engine.h"

#include<sys/socket.h>
#include<sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
* ���캯��
* ģ��ƫ�ػ�
* ��̬�����ڴ�Ļ�����,��С������ʱ��չ
*/
template <>
t_BufferCmdQueue::ByteBuffer()
: _maxSize(trunkSize), _offPtr(0), _currPtr(0), _buffer(_maxSize) { }

/**
* ���캯��
* ģ��ƫ�ػ�
* ��̬����Ļ�����,��С������ʱ�ı�
*/
template <>
t_StackCmdQueue::ByteBuffer()
: _maxSize(PACKET_ZIP_BUFFER), _offPtr(0), _currPtr(0) { }
//����0��Ҫ�ٴη��� Ӧ�û�������
int zSocket::sendRawData_NoPoll(const void *pBuffer, const int nSize)
{

	/*fprintf(stderr,"zSocket::sendRawData_NoPoll\n");


	if(((Cmd::stNullUserCmd *)pBuffer)->byCmd == 5 && ((Cmd::stNullUserCmd *)pBuffer)->byParam == 55)
	{
	Cmd::stMapDataMapScreenUserCmd * tt = (Cmd::stMapDataMapScreenUserCmd *)(pBuffer);
	WORD _size = tt->mdih.size;
	fprintf(stderr,"������Ϣ\n");
	}*/
	int retcode = send(sock, (const char*)pBuffer, nSize, 0);
	if (retcode == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		return 0;//should retry ��Ҫ�������ݰ� 
	}
		
	else if (retcode == -1)
		return -1;
	return retcode;
}


zSocket::zSocket(const SOCKET sock, const struct sockaddr_in *addr /* = NULL */, const bool compress /* = false */, bool useEpoll /* = USE_EPOOL */, zTCPTask* pTask /* = NULL */)
:m_dwMySendCount(0), m_dwSendCount(0), m_dwRecvCount(0)
{
	m_bTaskDeleted = false;
	assert(INVALID_SOCKET != sock);

	this->m_bUserEpoll = useEpoll;
	this->sock = sock;
	this->pTask = pTask;

	bzero_(&this->addr, sizeof(struct sockaddr_in));
	if (NULL == addr)
	{
		socklen_t len = sizeof(struct sockaddr_in);
		getpeername(this->sock, (struct sockaddr *)&this->addr, &len);
	}
	else
	{
		bcopy_(&this->addr, addr, sizeof(struct sockaddr_in));
	}

	bzero_(&this->local_addr, sizeof(struct sockaddr_in));
	{
		socklen_t len = sizeof(struct sockaddr_in);
		getsockname(this->sock, (struct sockaddr *)&this->local_addr, &len);
	}

	setNonblock();

	rd_msec = T_RD_MSEC;
	wr_msec = T_WR_MSEC;
	//_rcv_raw_size = 0;
	_current_cmd = 0;

	//ѹ�� ���� ���� ��־����
}

zSocket::~zSocket()
{
	DisConnet();
}

//����Ϣ������е� ����ȡ����ת ͬʱ��Ҫ����
int zSocket::recvToCmd(void *pstrCmd, const int nCmdLen, const bool wait)
{
	if (sock == INVALID_SOCKET)
	{
		return -1;
	}
	if (_rcv_queue.rd_size() > PACKHEADSIZE)
	{
		DWORD nRecordLen = ((PACK_HEAD*)_rcv_queue.rd_buf())->len;
		if (_rcv_queue.rd_size() >= 1)
		{
			int retval = packetUnpack(_rcv_queue.rd_buf(), nRecordLen, (BYTE*)pstrCmd);
			_rcv_queue.rd_flip(nRecordLen + PACKHEADSIZE);
			return retval;
		}
	}

	//��û�н��յ�һ�������İ�
	return 0;
	
}
bool zSocket::sendCmd(const void* pstdCmd, const int nCmdLen, const bool buffer)
{
	if (NULL == pstdCmd || nCmdLen <= 0)
		return false;
	bool retval = false;

	if (buffer)
	{
		t_StackCmdQueue _raw_queue;
		packetAppend(pstdCmd, nCmdLen, _raw_queue);
		tQueue.lock();
		tQueue.put(_raw_queue.rd_buf(),_raw_queue.rd_size());
		tQueue.unlock();

	}
	else
	{
		t_StackCmdQueue _raw_queue;
		packetAppend(pstdCmd,nCmdLen,_raw_queue);

	}

	return retval;
}
bool zSocket::sendCmdNoPack(const void* pstrCmd, const int nCmdLen, const bool buffer)
{

	return 0;
}
int zSocket::Send(const SOCKET socket, const void* pBuffer, const int nLen, int flags)
{

	return 0;
}
//���� tQueue����������е�����
bool zSocket::sync()
{
	Zebra::logger->debug("zSocket::sync");
	//���������
	if (tQueue.rd_ready())
	{
		tQueue.lock();
		int retcode = sendRawData_NoPoll(tQueue.rd_buf(),tQueue.rd_size());
		tQueue.unlock();
		if (retcode > 0)
		{
			//���ͳɹ���Ҫ�ֶ��������飬��
			tQueue.rd_flip(retcode);
		}
		else if (-1 == retcode)
		{
			return false;
		}
	}
	return true;
}

//
void zSocket::force_sync()
{

}



int zSocket::recvToBuf_NoPoll()
{

	return 0;
}
int zSocket::recvToCmd_NoPoll(void *pstrCmd, const int nCmdLen)
{

	return 0;
}

//��ȡ��Ϣ���� ����m_RecvBuffer��
DWORD zSocket::RecvByte(DWORD size)
{
	int ret = 0;
	m_RecvBuffer.wr_reserve(size);

	ret = recv(sock,m_RecvBuffer.wr_buf(),size,0);
	if (ret != -1)
	{
		m_RecvBuffer.wr_flip(ret);
	}
	return ret;

}


DWORD zSocket::RecvData(DWORD dwNum)
{
	//�׽��ֽ�������
	static PACK_HEAD head;
	unsigned int nLen = 0;
	DWORD ret = 0;
	m_RecvLock.lock();
	//������ܻ������е����� ��һ��ͷ���Ĵ�С
	if (m_RecvBuffer.rd_size() == PACKHEADSIZE)
	{
		//�����Ƿ���ͷ��
		if ((m_RecvBuffer.rd_buf()[0]) != head.Header[0] || m_RecvBuffer.rd_buf()[1] != head.Header[1])
		{
			Zebra::logger->error("��鵽�������ݵ�ͷ������: (%s)(%u)",getIP(),getPort());
			DisConnet();
			ret = -1;
			goto ret;
		}
		else
		{
			Zebra::logger->debug("�յ���ȷ�İ�ͷ");
			nLen = ((PACK_HEAD*)m_RecvBuffer.rd_buf())->len;
			ret = RecvByte(nLen);
			goto ret;
		}
	}
	else if (m_RecvBuffer.rd_size() < PACKHEADSIZE)
	{
		ret = RecvByte(PACKHEADSIZE - m_RecvBuffer.rd_size());
		goto ret;
	}
	else
	{
		nLen = ((PACK_HEAD*)m_RecvBuffer.rd_buf())->len;
		if (nLen > MAX_DATASIZE)
		{
			m_RecvBuffer.reset();//buffer�����Ϣ���
			ret = RecvByte(PACKHEADSIZE); //���¶�ȡͷ��
			goto ret;
		}
		else if (m_RecvBuffer.rd_size() == nLen + PACKHEADSIZE)
		{
			//�õ���һ����������Ϣ
			Zebra::logger->debug("�õ�һ����������Ϣ %s(%d)",getIP(), getPort());
			_rcv_queue.lock();
			_rcv_queue.put(m_RecvBuffer.rd_buf(), nLen + PACKHEADSIZE);
			_rcv_queue.unlock();
			m_RecvBuffer.rd_flip(nLen + PACKHEADSIZE);
			ret = RecvByte(PACKHEADSIZE); //���¶�ȡͷ��
			goto ret;
		}
		else
		{
			Zebra::logger->debug("��Ϣ�������� %s(%d)", getIP(), getPort());
			ret = RecvByte(PACKHEADSIZE + nLen - m_RecvBuffer.rd_size());
			goto ret;
		}
	}



ret:
	m_RecvLock.unlock();
	return ret;
}
int zSocket::SendData(DWORD dwNum)
{
	return  0;
}
//��� ������Ϣbuffer�Ƿ�����Ϣ
//bWait �Ƿ���Ҫ�ȴ����ݷ������
//timeout ��ʱʱ�� ��λsec
//return -1 ���ӶϿ� -2�ȴ���ʱ  >0 ���ݳ���
int zSocket::WaitRecv(bool bWait, unsigned int timeout )
{
	if (bWait)
	{
		DWORD EnterTime = GetTimeFromSystemPower();
		do 
		{
			if (_rcv_queue.rd_size() > 0)
			{
				return _rcv_queue.rd_size();
			}
			if (sock == INVALID_SOCKET)
			{
				return -1;
			}
			::usleep(20 * 1000);

		} while (timeout == 0 || GetTimeFromSystemPower() - EnterTime < timeout * 1000);
	}
	else
	{
		//���õȴ�
		if (sock == INVALID_SOCKET)
		{
			return -1;
		}

		return _rcv_queue.rd_size();
	}

	return  0;
}

//����1 �Ѿ�׼���÷������ݣ�������û������ 0���ݷ��ͻ�û����� -1 sock�Ͽ�
int zSocket::WaitSend(bool bWait,unsigned int timeout)
{
	if (bWait)
	{
		DWORD EnterTime = GetTimeFromSystemPower();
		do 
		{
			if (tQueue.rd_size() == 0)
			{
				return 1;
			}
			if (sock == INVALID_SOCKET)
			{
				return -1;
			}
			::usleep(20 * 1000);

		} while (timeout <= 0 || GetTimeFromSystemPower() - EnterTime < timeout * 1000);
	}
	else
	{
		//���õȴ�
		if (sock == INVALID_SOCKET)
		{
			return -1;
		}

		return tQueue.rd_size() == 0;
	}
	return -1;
}

int zSocket::sendRawData(const void *pBuffer, const int nSize)
{
	return  0;
}
bool zSocket::sendRawDataIM(const void *pBuffer, const int nSize)
{
	return  0;
}
bool zSocket::setNonblock()
{
	return  0;
}
int zSocket::waitForRead()
{
	return  0;
}
int zSocket::waitForWrite()
{
	return  0;
}
//�������ݵ�������

int zSocket::recvToBuf()
{
	return  0;
}


DWORD zSocket::packetUnpack(BYTE *in, const DWORD nPacketLen, BYTE *out)
{
	//��ȥͷ������
	DWORD nRecvCmdLen = nPacketLen - PACKHEADSIZE;


	//�Ȳ���ѹ �� ����
	memcpy(&in[PACKHEADSIZE],out, nRecvCmdLen);
	return nRecvCmdLen;
}