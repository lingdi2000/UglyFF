#include "../include/engine.h"
#include<iostream>

#include <stdarg.h>
#include"../include/log4cplus/logger.h"
#include"../include/log4cplus/appender.h"
#include"../include/log4cplus/layout.h"
#include"../include/log4cplus/fileappender.h"
#include "../include/log4cplus/loggingmacros.h" 
#include "../include/log4cplus/ndc.h"
using namespace std;

////test 字符串分割
//void teststringtok()
//{
//	std::string str = "hello world hello world";
//	vector<std::string> vecStr;
//
//	stringtok(vecStr, str);
//
//	
//	for (std::vector<string>::iterator i = vecStr.begin(); i != vecStr.end(); ++i)
//        std::cout  << (*i) << "\n";
//
//}



void log4test()
{
	using namespace log4cplus;
	using namespace log4cplus::helpers;
	
	
	SharedAppenderPtr _append(new DailyRollingFileAppender("./log/Test.log", HOURLY, true, 0,false,false));
	Logger _logger = Logger::getInstance("test");
	_logger.addAppender(_append);

	for (int i = 0; i < 5; i++)
	{
		LOG4CPLUS_DEBUG(_logger,"hello world");
		
	}

	//Logger::getRoot().addAppender(append);
	/*Logger _logger = Logger::getRoot();
	Logger test = Logger::getInstance("test");
	Logger subTest = Logger::getInstance("test.subtest");*/
	/*
	for (int i = 0; i < 5; i++)
	{

	LOG4CPLUS_DEBUG(subTest, "Entering  loop #" << i);
	}*/
}

namespace Zebra
{
	volatile QWORD qwGameTime = 0;

	zLogger *logger = NULL;

	zProperties global;

	/**
	* \brief 初始化一些全局变量
	*
	*/
	//static void initGlobal()  __attribute__ ((constructor));

};






int main()
{
	Zebra::logger = new zLogger("ForTest", "/home/lingdi2000/log/testlog.log");
	//Zebra::logger->debug("hello  %s world ", "你好");
	/*std::string test;
	test = Format("hello %s w orld \n  ", "我的");
	printf("%s", test.c_str());*/
	//logger->debug("hello world %s \n", "hahah!");
	//teststringtok();
	
	//Zebra::log4 = log4cplus::Logger::getInstance("TestServer");

	//直接sleep 3秒
	//sleep(3);
	//log4test();

	//动态数组
	std::vector<zSocket*> vecSocks;

	zEpoll testEpoll;
	int epfd = testEpoll.create();
	if (epfd < 0)
	{
		printf("打开epoll失败\n");
		return 0;
	}

	zTCPServer server("TestServer");
	std::string name = "111";
	WORD port = 8090;
	server.Bind("",port);

	
	
	zSocket accpetSock(server.getSockFd(), &server.getAddr());
	struct epoll_event epEvent;
	epEvent.events = EPOLLIN;
	epEvent.data.ptr = (void*)&accpetSock;

	testEpoll.add(&accpetSock,&epEvent);

	int ret = 0;
	while (true)
	{
		printf("进入epoll_wait循环\n");
		ret = testEpoll.wait();
		if (ret == -2)
		{
			printf("epoll 文件描述符错误\n");
			return 0;
		}
		else if (ret == -1)
		{
			//epoll 有错
			printf("epoll_wait错误: %s",strerror(errno));
			return 0;
		}
		else if (ret == 0)
		{
			printf("epoll 循环超时\n");
			continue;
		}
		else if (ret > 0)
		{
			//正常的情况 必须要处理 不然会不停的出发
			//需要判断 是否是listenfd
			for (int index = 0; index < ret; index++)
			{
				if (index == server.getSockFd() && testEpoll[index].events == EPOLLIN)
				{
					int peerSock = 0;
					printf("有新的客户端连接\n");
					struct sockaddr_in peerAddr;
					peerSock = server.Accept(&peerAddr);
					if (peerSock == -1)
					{
						if (errno == EAGAIN || errno == EMFILE || errno == ENFILE)
						{
							//连接数量达到最大了或者需要再次调用
							continue;
						}
						printf("accept出错：%s",strerror(errno));
						testEpoll.add(&accpetSock, &epEvent);
					}

					//创建新的zSocket
					zSocket *newSock = new zSocket(peerSock, &peerAddr);
					vecSocks.push_back(newSock);
					printf("新客户端连接: 客户端IP(%s), 客户端端口(%d)", newSock->getIP(), newSock->getPort());

					//需要重新把 文件描述符加入epoll

				}
				else
				{
					printf("客户端发来了消息");
				}

			}

		}
	}

	//zSocket acceptSock;
	



	printf("程序快结束了\n");
	return 0;
}
