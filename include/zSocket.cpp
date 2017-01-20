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
* 构造函数
* 模板偏特化
* 动态分配内存的缓冲区,大小可以随时扩展
*/
template <>
t_BufferCmdQueue::ByteBuffer()
: _maxSize(trunkSize), _offPtr(0), _currPtr(0), _buffer(_maxSize) { }

/**
* 构造函数
* 模板偏特化
* 静态数组的缓冲区,大小不能随时改变
*/
template <>
t_StackCmdQueue::ByteBuffer()
: _maxSize(PACKET_ZIP_BUFFER), _offPtr(0), _currPtr(0) { }
//返回0需要再次发送 应该缓存下来
int zSocket::sendRawData_NoPoll(const void *pBuffer, const int nSize)
{

	/*fprintf(stderr,"zSocket::sendRawData_NoPoll\n");


	if(((Cmd::stNullUserCmd *)pBuffer)->byCmd == 5 && ((Cmd::stNullUserCmd *)pBuffer)->byParam == 55)
	{
	Cmd::stMapDataMapScreenUserCmd * tt = (Cmd::stMapDataMapScreenUserCmd *)(pBuffer);
	WORD _size = tt->mdih.size;
	fprintf(stderr,"问题消息\n");
	}*/
	int retcode = send(sock, (const char*)pBuffer, nSize, 0);
	if (retcode == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		return 0;//should retry 需要缓存数据啊 
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

	//压缩 或者 加密 标志设置
}

zSocket::~zSocket()
{
	DisConnet();
}

//将消息缓冲队列的 数据取出来转 同时还要解封包
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

	//还没有接收到一个完整的包
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
//发送 tQueue封包缓冲区中的数据
bool zSocket::sync()
{
	Zebra::logger->debug("zSocket::sync");
	//如果有数据
	if (tQueue.rd_ready())
	{
		tQueue.lock();
		int retcode = sendRawData_NoPoll(tQueue.rd_buf(),tQueue.rd_size());
		tQueue.unlock();
		if (retcode > 0)
		{
			//发送成功需要手动调整数组，烦
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

//读取消息数据 放入m_RecvBuffer中
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
	//套接字接受数据
	static PACK_HEAD head;
	unsigned int nLen = 0;
	DWORD ret = 0;
	m_RecvLock.lock();
	//如果接受缓冲区中的数据 有一个头部的大小
	if (m_RecvBuffer.rd_size() == PACKHEADSIZE)
	{
		//检验是否是头部
		if ((m_RecvBuffer.rd_buf()[0]) != head.Header[0] || m_RecvBuffer.rd_buf()[1] != head.Header[1])
		{
			Zebra::logger->error("检查到接收数据的头部出错: (%s)(%u)",getIP(),getPort());
			DisConnet();
			ret = -1;
			goto ret;
		}
		else
		{
			Zebra::logger->debug("收到正确的包头");
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
			m_RecvBuffer.reset();//buffer里的消息清除
			ret = RecvByte(PACKHEADSIZE); //重新读取头部
			goto ret;
		}
		else if (m_RecvBuffer.rd_size() == nLen + PACKHEADSIZE)
		{
			//得到了一个完整的消息
			Zebra::logger->debug("得到一个完整的消息 %s(%d)",getIP(), getPort());
			_rcv_queue.lock();
			_rcv_queue.put(m_RecvBuffer.rd_buf(), nLen + PACKHEADSIZE);
			_rcv_queue.unlock();
			m_RecvBuffer.rd_flip(nLen + PACKHEADSIZE);
			ret = RecvByte(PACKHEADSIZE); //重新读取头部
			goto ret;
		}
		else
		{
			Zebra::logger->debug("消息还不完整 %s(%d)", getIP(), getPort());
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
//检查 完整消息buffer是否有消息
//bWait 是否需要等待数据发送完成
//timeout 超时时长 单位sec
//return -1 连接断开 -2等待超时  >0 数据长度
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
		//不用等待
		if (sock == INVALID_SOCKET)
		{
			return -1;
		}

		return _rcv_queue.rd_size();
	}

	return  0;
}

//返回1 已经准备好发送数据，缓冲区没有数据 0数据发送还没有完成 -1 sock断开
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
		//不用等待
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
//接受数据到缓冲区

int zSocket::recvToBuf()
{
	return  0;
}


DWORD zSocket::packetUnpack(BYTE *in, const DWORD nPacketLen, BYTE *out)
{
	//减去头部长度
	DWORD nRecvCmdLen = nPacketLen - PACKHEADSIZE;


	//先不解压 和 解密
	memcpy(&in[PACKHEADSIZE],out, nRecvCmdLen);
	return nRecvCmdLen;
}