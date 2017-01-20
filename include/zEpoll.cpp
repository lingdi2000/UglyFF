#include "engine.h"

zEpoll::zEpoll(int _max, int maxevents) :max(_max),
epoll_fd(-1),
epoll_timeout(-1),
epoll_maxevents(maxevents),
backEvents(0)
{



}

zEpoll::~zEpoll()
{
	if (isValid()){
		close(epoll_fd);
	}
	SAFE_DELETE_VEC(backEvents);
	
}

inline bool zEpoll::isValid() const
{
	return epoll_fd > 0;
}

inline void zEpoll::setTimeout(int timeOut)
{
	epoll_timeout = timeOut;
}

inline void zEpoll::setMaxEvents(int maxEvents)
{
	epoll_maxevents = maxEvents;
}

inline const epoll_event* zEpoll::events()const 
{
	return backEvents;
}

int zEpoll::create()
{
	// ����max ����Ϊ0 �ں��Զ�����
	epoll_fd = epoll_create(max);
	if (isValid())
	{
		backEvents = new epoll_event[epoll_maxevents];
	}
	return epoll_fd;
}

int zEpoll::add(zSocket* zSock, epoll_event *event)
{
	if (isValid())
	{
		return epoll_ctl(epoll_fd, ADD, zSock->getSock(), event);
	}
	return -1;
}

int zEpoll::mod(zSocket* zSock, epoll_event *event)
{
	if (isValid())
	{
		return epoll_ctl(epoll_fd, MOD, zSock->getSock(), event);
	}
	return -1;
}

int zEpoll::del(zSocket* zSocket, epoll_event *event)
{
	if (isValid())
	{
		return epoll_ctl(epoll_fd, DEL, zSocket->getSock(), event);
	}
	return -1;
}


int zEpoll::wait()
{
	if (isValid())
	{
		// �¼����ᶪʧ�� ����¼���������¼����� �����´μ�������
		//timeout Ϊ0 ���� epoll_wait ��ͣ�ķ��� ����߳��� �п��ܻ���Ҫ������������
		//������ô��
		return epoll_wait(epoll_fd, backEvents, epoll_maxevents, epoll_timeout);
	}
	//������ϵͳ�Դ��� -1����
	return -2;
}