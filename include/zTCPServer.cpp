#ifndef _ZTCPSERVER_H_
#define _ZTCPSERVER_H_

#include "engine.h"
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <string.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

zTCPServer::zTCPServer(const std::string& name)
:name(name),
sock(INVALID_SOCKET)
{
	//�����־
	Zebra::logger->debug("zTCPServer %s ", name.c_str());
}

zTCPServer::~zTCPServer()
{
	if (INVALID_SOCKET != sock)
	{
		close(sock);
		sock = INVALID_SOCKET;
	}
}

bool zTCPServer::Bind(const std::string &name, const WORD port)
{
	struct sockaddr_in addr;

	if (INVALID_SOCKET != sock)
	{
		//�����Ѿ���ʼ��
		return false;
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock)
	{
		return false;
	}

	//�����׽���Ϊ������״̬
	int reuse = 1; //true 
	if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)))
	{
		close(sock);
		sock = INVALID_SOCKET;
		return false;
	}

	//�����׽ӿڷ��ͽ��ջ���,���ҷ������ı�����accept֮ǰ����
	//�����������˽ϴ��ʱ�� ѭ������send recv
	int window_size = 64 * 1024;
	if (-1 == setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&window_size, sizeof(window_size)))
	{
		close(sock);
		sock = INVALID_SOCKET;
		return false;
	}

	if (-1 == setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&window_size, sizeof(window_size)))
	{
		close(sock);
		sock = INVALID_SOCKET;
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	//����һ������,����ֻ��һ��������ʱ��ȽϺ���
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	int ret = bind(sock, (struct sockaddr*)&addr, sizeof(addr));
	if (-1 == ret)
	{
		close(sock);
		sock = INVALID_SOCKET;
		return false;
	}


	ret = listen(sock, MAX_WAITQUEUE);
	if (-1 == ret )
	{
		close(sock);
		sock = INVALID_SOCKET;
		return false;
	}

	addr_ = addr;

	return true;
}

//���ؿͻ��˵�sock
int zTCPServer::Accept(struct sockaddr_in* addr)
{
	socklen_t len = sizeof(struct sockaddr_in);
	bzero_(addr, sizeof(struct sockaddr_in));

	//
	//struct epoll_event accpet_event;


	return accept(sock, (struct sockaddr*)addr, &len);
	
}

SOCKET zTCPServer::getSockFd()
{
	return sock;
}


struct sockaddr_in& zTCPServer::getAddr()
{
	return addr_;
}

const char* zTCPServer::getIP()
{
	return inet_ntoa(addr_.sin_addr);
}

#endif