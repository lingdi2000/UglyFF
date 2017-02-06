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
	//打个日志
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
		//可能已经初始化
		return false;
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (-1 == sock)
	{
		return false;
	}

	//设置套接字为可重用状态
	int reuse = 1; //true 
	if (-1 == setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)))
	{
		close(sock);
		sock = INVALID_SOCKET;
		return false;
	}

	//设置套接口发送接收缓冲,并且服务器的必须在accept之前设置
	//避免在数据了较大的时候 循环调用send recv
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
	//任意一张网卡,电脑只有一张网卡的时候比较合适
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

//返回客户端的sock
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