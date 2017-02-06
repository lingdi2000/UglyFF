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
* ¹¹Ôìº¯Êı
* Ä£°åÆ«ÌØ»¯
* ¶¯Ì¬·ÖÅäÄÚ´æµÄ»º³åÇø,´óĞ¡¿ÉÒÔËæÊ±À©Õ¹
*/
template <>
t_BufferCmdQueue::ByteBuffer()
: _maxSize(trunkSize), _offPtr(0), _currPtr(0), _buffer(_maxSize) { }

/**
* ¹¹Ôìº¯Êı
* Ä£°åÆ«ÌØ»¯
* ¾²Ì¬Êı×éµÄ»º³åÇø,´óĞ¡²»ÄÜËæÊ±¸Ä±ä
*/
template <>
t_StackCmdQueue::ByteBuffer()
: _maxSize(PACKET_ZIP_BUFFER), _offPtr(0), _currPtr(0) { }
//·µ»Ø0ĞèÒªÔÙ´Î·¢ËÍ Ó¦¸Ã»º´æÏÂÀ´
int zSocket::sendRawData_NoPoll(const void *pBuffer, const int nSize)
{

	/*fprintf(stderr,"zSocket::sendRawData_NoPoll\n");


	if(((Cmd::stNullUserCmd *)pBuffer)->byCmd == 5 && ((Cmd::stNullUserCmd *)pBuffer)->byParam == 55)
	{
	Cmd::stMapDataMapScreenUserCmd * tt = (Cmd::stMapDataMapScreenUserCmd *)(pBuffer);
	WORD _size = tt->mdih.size;
	fprintf(stderr,"ÎÊÌâÏûÏ¢\n");
	}*/
	int retcode = send(sock, (const char*)pBuffer, nSize, 0);
		
	if (retcode == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
	{
		return 0;//should retry ĞèÒª»º´æÊı¾İ°¡ 
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

	setNonblock(); //ÉèÖÃÎª·Ç×èÈû Ó¦¸ÃÊÖ¶¯ÉèÖÃ

	rd_msec = T_RD_MSEC;
	wr_msec = T_WR_MSEC;
	//_rcv_raw_size = 0;
	_current_cmd = 0;

	//Ñ¹Ëõ »òÕß ¼ÓÃÜ ±êÖ¾ÉèÖÃ
}

zSocket::~zSocket()
{
	DisConnet();
}

//½«ÏûÏ¢»º³å¶ÓÁĞµÄ Êı¾İÈ¡³öÀ´×ª Í¬Ê±»¹Òª½â·â°ü
int zSocket::recvToCmd(void *pstrCmd, const int nCmdLen, const bool wait)
{
	//Õâ¸ö²»ĞèÒª¼ì²é socketÊÇ·ñÓĞ ÒòÎªÖ»ĞèÒªÖªµÀÊı¾İÊÇ·ñÓĞÒ»¸öÍêÕûµÄ°ü¾Í¿ÉÒÔÁË
	if (sock == INVALID_SOCKET)
	{
		return -1;
	}
	if (_rcv_queue.rd_size() > PACKHEADSIZE)
	{
		DWORD nRecordLen = ((PACK_HEAD*)_rcv_queue.rd_buf())->len;
		Zebra::logger->debug("È¡³ö·â°ü ·â°üÄÚÈİ³¤¶È %d",nRecordLen);

		if (_rcv_queue.rd_size() > nRecordLen)
		{
			int retval = packetUnpack(_rcv_queue.rd_buf(), nRecordLen, (BYTE*)pstrCmd);
			_rcv_queue.lock(); //ÄÚ´æÒÆ¶¯»¹ÊÇÍ¦ºÄÊ±µÄ
			_rcv_queue.rd_flip(nRecordLen + PACKHEADSIZE);
			_rcv_queue.unlock();

			return retval;
		}
	}

	//»¹Ã»ÓĞ½ÓÊÕµ½Ò»¸öÍêÕûµÄ°ü
	return 0;
	
}
bool zSocket::sendCmd(const void* pstdCmd, const int nCmdLen, const bool buffer)
{
	if (NULL == pstdCmd || nCmdLen <= 0)
		return false;
	bool retval = true;

	if (buffer)
	{
		t_StackCmdQueue _raw_queue;
		packetAppend(pstdCmd, nCmdLen, _raw_queue);
		tQueue.lock();
		tQueue.put(_raw_queue.rd_buf(),_raw_queue.rd_size());
		tQueue.unlock();
		
	}
	else if (tQueue.rd_size() > 0)
	{
		//Èç¹û»º³åÇøÖĞÓĞÊı¾İ ËµÃ÷Ç°ÃæµÄÊı¾İ»¹Ã»ÓĞ·¢ËÍ,ËùÒÔĞÂµÄÏûÏ¢Òª¼ÓÈë»º³åÇø
		t_StackCmdQueue _raw_queue;
		packetAppend(pstdCmd, nCmdLen, _raw_queue);
		tQueue.lock();
		tQueue.put(_raw_queue.rd_buf(), _raw_queue.rd_size());
		tQueue.unlock();
		
	}
	else
	{
		t_StackCmdQueue _raw_queue;
		packetAppend(pstdCmd,nCmdLen,_raw_queue);
		//·¢ËÍÊı¾İ
		Zebra::logger->debug("·¢ËÍÊı¾İĞèÒª·¢ËÍµÄ³¤¶È%d",_raw_queue.rd_size());
		int retcode = send(sock, _raw_queue.rd_buf(), _raw_queue.rd_size(),0);
		if (-1 == retcode)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
			{
				//·¢ËÍ»º³åÇøÊı¾İÒÑ¾­ÂúÁË  ĞèÒª»º´æ
				tQueue.lock();
				tQueue.put(_raw_queue.rd_buf(), _raw_queue.rd_size());
				tQueue.unlock();
			}
			else
			{
				//Á¬½Ó³ö´íÁË
				DisConnet();
				return false;
			}
		}
		//Õâ¸öÇ½×©Ä¿Ç°Ã»ÓĞÎÊÌâ£¬ÒòÎª·¢ËÍ»º³åÇø´óĞ¡ÊÇ64k
		else if ((unsigned int)retcode < _raw_queue.rd_size())
		{
			//Êı¾İ²¢Ã»ÓĞÍêÈ«±»·¢ËÍ³öÈ¥
			Zebra::logger->debug("·¢ËÍÊı¾İĞèÒª·¢ËÍµÄ³¤¶È%d,Êµ¼Ê%d", _raw_queue.rd_size(), retcode);

			_raw_queue.rd_flip(retcode);
			tQueue.lock();
			tQueue.put(_raw_queue.rd_buf(), _raw_queue.rd_size());
			tQueue.unlock();
		}
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
//·¢ËÍ tQueue·â°ü»º³åÇøÖĞµÄÊı¾İ
bool zSocket::sync()
{
	Zebra::logger->debug("zSocket::sync");
	//Èç¹ûÓĞÊı¾İ
	if (tQueue.rd_ready())
	{
		tQueue.lock();
		int retcode = sendRawData_NoPoll(tQueue.rd_buf(),tQueue.rd_size());
		tQueue.unlock();
		if (retcode > 0)
		{
			//·¢ËÍ³É¹¦ĞèÒªÊÖ¶¯µ÷ÕûÊı×é£¬·³
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


//epoll Ä£Ê½ÏÂ ¼ì²éÊÇ·ñÓĞÊı¾İ  Ò»°ãÄ£Ê½ÏÂÓ¦¸Ã×èÈû
int zSocket::recvToBuf_NoPoll()
{
	int retcode = 0;

	if (m_bUserEpoll)
	{
		retcode = _rcv_queue.rd_size();
	}
	else
	{
		//·Ç×èÈûÄ£Ê½ ĞèÒª½ÓÊÕÊı¾İ
	}
	return retcode;
}

//²»ÊÊÓÃepoll À´½ÓÊÕÊı¾İ
int zSocket::recvToCmd_NoPoll(void *pstrCmd, const int nCmdLen)
{
	return recvToCmd(pstrCmd,nCmdLen,false);
}

//¶ÁÈ¡ÏûÏ¢Êı¾İ ·ÅÈëm_RecvBufferÖĞ
ssize_t zSocket::RecvByte(DWORD size)
{
	ssize_t ret = 0;
	m_RecvBuffer.wr_reserve(size);

	ret = recv(sock,m_RecvBuffer.wr_buf(),size,0);
	
	if (ret != -1)
	{
		m_RecvBuffer.wr_flip(ret);
	}
	return ret;

}


ssize_t zSocket::RecvData(DWORD dwNum)
{
	//Ì×½Ó×Ö½ÓÊÜÊı¾İ
	static PACK_HEAD head;
	unsigned int nLen = 0;
	ssize_t ret = 0;
	m_RecvLock.lock();
	//Èç¹û½ÓÊÜ»º³åÇøÖĞµÄÊı¾İ ÓĞÒ»¸öÍ·²¿µÄ´óĞ¡
	if (m_RecvBuffer.rd_size() == PACKHEADSIZE)
	{
		//¼ìÑéÊÇ·ñÊÇÍ·²¿
		if ((m_RecvBuffer.rd_buf()[0]) != head.Header[0] || m_RecvBuffer.rd_buf()[1] != head.Header[1])
		{
			Zebra::logger->error("¼ì²éµ½½ÓÊÕÊı¾İµÄÍ·²¿³ö´í: (%s)(%u)",getIP(),getPort());
			DisConnet();
			ret = -1;
			goto ret;
		}
		else
		{
			nLen = ((PACK_HEAD*)m_RecvBuffer.rd_buf())->len;
			Zebra::logger->debug("ÊÕµ½ÕıÈ·µÄ°üÍ·, ĞèÒª½ÓÊÕµÄÄÚÈİ³¤¶ÈÎª %d",nLen);
			ret = RecvByte(nLen);
			Zebra::logger->debug("½ÓÊÕµ½µÄÊµ¼Ê³¤¶ÈÎª%d",ret);

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
		Zebra::logger->debug("Êı¾İÍ·²¿½ÓÊÕÍêÕû nLen %d header %d",nLen,PACKHEADSIZE);
		if (nLen > MAX_DATASIZE)
		{
			m_RecvBuffer.reset();//bufferÀïµÄÏûÏ¢Çå³ı
			ret = RecvByte(PACKHEADSIZE); //ÖØĞÂ¶ÁÈ¡Í·²¿
			goto ret;
		}
		else
		{
			Zebra::logger->debug("ÏûÏ¢»¹²»ÍêÕû %s(%d)", getIP(), getPort());
			ret = RecvByte(PACKHEADSIZE + nLen - m_RecvBuffer.rd_size());
			goto ret;
		}
	}



ret:
	m_RecvLock.unlock();

	//ÕâÀïÓ¦¸Ã¼ì²é½ÓÊÕµ½µÄÊı¾İÊÇ·ñÍêÕû
	//Zebra::logger->debug("buffer size= %d  header size = %d nlen =%d", m_RecvBuffer.rd_size(),PACKHEADSIZE, nLen);
	if (m_RecvBuffer.rd_size() >PACKHEADSIZE &&  m_RecvBuffer.rd_size() == (nLen + PACKHEADSIZE))
	{
		//µÃµ½ÁËÒ»¸öÍêÕûµÄÏûÏ¢
		Zebra::logger->debug("µÃµ½Ò»¸öÍêÕûµÄÏûÏ¢ %s(%d)", getIP(), getPort());
		_rcv_queue.lock();
		_rcv_queue.put(m_RecvBuffer.rd_buf(), nLen + PACKHEADSIZE);
		_rcv_queue.unlock();
		m_RecvBuffer.rd_flip(nLen + PACKHEADSIZE);

		//ret = RecvByte(PACKHEADSIZE); //ÖØĞÂ¶ÁÈ¡Í·²¿ 
		goto ret;
	}
	return ret;
}
int zSocket::SendData(DWORD dwNum)
{
	return  0;
}
//¼ì²é ÍêÕûÏûÏ¢bufferÊÇ·ñÓĞÏûÏ¢
//bWait ÊÇ·ñĞèÒªµÈ´ıÊı¾İ·¢ËÍÍê³É
//timeout ³¬Ê±Ê±³¤ µ¥Î»sec
//return -1 Á¬½Ó¶Ï¿ª -2µÈ´ı³¬Ê±  >0 Êı¾İ¤¶È
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
		//²»ÓÃµÈ´ı
		if (sock == INVALID_SOCKET)
		{
			return -1;
		}

		return _rcv_queue.rd_size();
	}

	return  0;
}

//·µ»Ø1 ÒÑ¾­×¼±¸ºÃ·¢ËÍÊı¾İ£¬»º³åÇøÃ»ÓĞÊı¾İ 0Êı¾İ·¢ËÍ»¹Ã»ÓĞÍê³É -1 sock¶Ï¿ª
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
		//²»ÓÃµÈ´ı
		if (sock == INVALID_SOCKET)
		{
			return -1;
		}

		return tQueue.rd_size();
	}
	//ÄªÃûÆäÃî£¬ÕâÀï¸ù±¾²»¿ÉÄÜ×ßµ½ ±àÒëÆ÷¾¯¸æ
	return  -1;
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
	int  flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	return  true;
}

bool zSocket::setBlock()
{
	int  flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags&~O_NONBLOCK);
	return 0;
}

int zSocket::waitForRead()
{
	return  0;
}
int zSocket::waitForWrite()
{
	return  0;
}
//½ÓÊÜÊı¾İµ½»º³åÇø

int zSocket::recvToBuf()
{
	return  0;
}


DWORD zSocket::packetUnpack(BYTE *in, const DWORD nRecordLen, BYTE *out)
{
	//¼õÈ¥Í·²¿³¤¶È
	DWORD nRecvCmdLen = nRecordLen;
	Zebra::logger->debug("½â·â°ü³¤¶È %d", nRecvCmdLen);
	in += PACKHEADSIZE;
	//ÏÈ²»½âÑ¹ ºÍ ½âÃÜ
	memcpy(out, in, nRecvCmdLen);
	return nRecvCmdLen;
}
