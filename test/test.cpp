#include "../include/engine.h"
#include<iostream>

#include <stdarg.h>
#include"../include/log4cplus/logger.h"
#include"../include/log4cplus/appender.h"
#include"../include/log4cplus/layout.h"
#include"../include/log4cplus/fileappender.h"
#include "../include/log4cplus/loggingmacros.h" 
#include "../include/log4cplus/ndc.h"

#include "test.h"
using namespace std;

////test �ַ����ָ�
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
	* \brief ��ʼ��һЩȫ�ֱ���
	*
	*/
	//static void initGlobal()  __attribute__ ((constructor));

};

int epolltest()
{
	//��̬����
	std::vector<zSocket*> vecSocks;

	zEpoll testEpoll;
	int epfd = testEpoll.create();
	if (epfd < 0)
	{
		printf("��epollʧ��\n");
		return 0;
	}

	zTCPServer server("TestServer");
	std::string name = "111";
	WORD port = 8090;
	server.Bind("", port);



	zSocket accpetSock(server.getSockFd(), &server.getAddr());
	struct epoll_event epEvent;
	epEvent.events = EPOLLIN;
	epEvent.data.ptr = (void*)&accpetSock;

	testEpoll.add(&accpetSock, &epEvent);

	int ret = 0;
	while (true)
	{
		printf("����epoll_waitѭ��\n");
		ret = testEpoll.wait();
		if (ret == -2)
		{
			printf("epoll �ļ�����������\n");
			return 0;
		}
		else if (ret == -1)
		{
			//epoll �д�
			printf("epoll_wait����: %s", strerror(errno));
			return 0;
		}
		else if (ret == 0)
		{
			printf("epoll ѭ����ʱ\n");
			continue;
		}
		else if (ret > 0)
		{
			//��������� ����Ҫ���� ��Ȼ�᲻ͣ�ĳ���
			//��Ҫ�ж� �Ƿ���listenfd
			for (int index = 0; index < ret; index++)
			{
				if (index == server.getSockFd() && testEpoll[index].events == EPOLLIN)
				{
					int peerSock = 0;
					printf("���µĿͻ�������\n");
					struct sockaddr_in peerAddr;
					peerSock = server.Accept(&peerAddr);
					if (peerSock == -1)
					{
						if (errno == EAGAIN || errno == EMFILE || errno == ENFILE)
						{
							//���������ﵽ����˻�����Ҫ�ٴε���
							continue;
						}
						printf("accept����%s", strerror(errno));
						testEpoll.add(&accpetSock, &epEvent);
					}

					//�����µ�zSocket
					zSocket *newSock = new zSocket(peerSock, &peerAddr);
					vecSocks.push_back(newSock);
					printf("�¿ͻ�������: �ͻ���IP(%s), �ͻ��˶˿�(%d)", newSock->getIP(), newSock->getPort());

					//��Ҫ���°� �ļ�����������epoll

				}
				else
				{
					printf("�ͻ��˷�������Ϣ");
				}

			}

		}
	}
}

class base
{
public:
	virtual void say()
	{
		cout << "I am base\n";
	}

	void main()
	{
		say();
	}
};


class dog :public base
{
public:
	void say()
	{
		cout << "I am dog\n";
	}

};




#define TEST_SERVER 30

int main()
{
	Zebra::logger = new zLogger("ForTest", "/home/lingdi2000/log/testlog.log");
	//Zebra::logger->debug("hello  %s world ", "���");
	/*std::string test;
	test = Format("hello %s w orld \n  ", "�ҵ�");
	printf("%s", test.c_str());*/
	//logger->debug("hello world %s \n", "hahah!");
	//teststringtok();
	
	//Zebra::log4 = log4cplus::Logger::getInstance("TestServer");

	//ֱ��sleep 3��
	//sleep(3);
	//log4test();

	SuperClient *superClient = new SuperClient;

	if (!superClient->connect("127.0.0.1", 8090))
	{
		Zebra::logger->error("���ӹ��������ʧ��");
		return -1;
	}

	Cmd::Super::t_Startup_Request tCmd;
	tCmd.wdServerType = TEST_SERVER; //
	strncpy(tCmd.pstrIP, "127.0.0.1", sizeof("127.0.0.1"));
	Zebra::logger->debug("���͵�������Ϣ����%d", sizeof(tCmd));
	Zebra::logger->error("�ͻ�������ָ����֤ʧ��(%s:%u)", tCmd.pstrIP, tCmd.wdServerType);

	if (!superClient->sendCmd(&tCmd, sizeof(tCmd)))
	{
		Zebra::logger->error("�������������͵�½ָ��ʧ�ܣ�2�������.....");
		return false;
	}
	
	//���û����֤�ɹ�
	while (!superClient->verified)
	{
		ssize_t retcode = superClient->getZSocket()->RecvData();
		if (retcode == -1)
		{
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
			{
				continue;
			}
			else
			{
				Zebra::logger->error("��������� ���ӶϿ� ���ش���");
				return -1;
			}
		}
		else if (retcode == 0)
		{
			Zebra::logger->error("��������� �������ӶϿ� ");
			return -1;
		}
		else
		{
			//�����������׽ӿ��Ƿ����
			int retcode = superClient->getZSocket()->WaitRecv(false); 
			if (retcode == -1)
			{
				Zebra::logger->error("��������� ���ӶϿ� ���ش���2");
				return -1;
			}
			else if (retcode > 0)
			{
				//����Ϣ
				BYTE pstrCmd[zSocket::MAX_DATASIZE];
				int nCmdLen = superClient->getZSocket()->recvToCmd(pstrCmd, sizeof(pstrCmd), false);
				if (nCmdLen == -1)
				{
					Zebra::logger->error("��������� ���ӶϿ� ���ش���3");
					return -1;
				}
				else if (nCmdLen > 0)
				{
					if (!superClient->msgParse((Cmd::t_NullCmd *)pstrCmd, nCmdLen))
					{
						Zebra::logger->error("�ӹ���������յ������ָ��(%d:%d)������ʧ�ܣ�\n", ((Cmd::t_NullCmd *)pstrCmd)->cmd, ((Cmd::t_NullCmd *)pstrCmd)->para);
						superClient->getZSocket()->DisConnet();
						return false;
					}
				}


			}
			//�鿴�Ƿ�����Ϣ
		}
	}
	

	printf("����������\n");
	return 0;
}
