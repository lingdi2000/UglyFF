#include "../include/engine.h"

class SuperService :public zNetService
{
private:
	GameZone_t gameZone; //��Ϸ�����


	//��Ϸ������
	std::string zoneName;

	WORD wdServerID; //���������
	WORD wdServerType;	//���������ͣ�����ʵ����ʱ��ȷ�� gateserver sessionserver
	char pstrIP[MAX_IP_LENGTH];		//����������ip��ַ
	WORD wdPort;					//�����������˿�
	static SuperService *instance;

	zTCPTaskPool *taskPool;
private:
	SuperService() :zNetService("���������")
	{
		wdServerID = 1;
		wdServerType = SUPERSERVER;
		bzero_(pstrIP, sizeof(pstrIP));
		wdPort = 0;
		taskPool = NULL;
	}

	bool init();
	void newTCPTask(const SOCKET sock, const struct sockaddr_in *addr);
	void final();

	bool getServerInfo();
public:
	~SuperService()
	{
		instance = NULL;

		//�ر��̳߳�
		if (taskPool)
		{
			taskPool->final();
			SAFE_DELETE(taskPool);
		}
	}

	const int getPoolSize() const
	{
		if (taskPool)
			return taskPool->getSize();
		else
			return 0;
	}

	static SuperService &getInstance()
	{
		if (NULL == instance)
		{
			instance = new SuperService();
		}
		return *instance;
	}

	static void delInstance()
	{
		SAFE_DELETE(instance);
	}

	void reloadConfig();



	const GameZone_t &getZoneID() const
	{
		return gameZone;
	}
	void setZoneID(const GameZone_t &gameZone)
	{
		this->gameZone = gameZone;
	}
	const std::string &getZoneName() const
	{
		return zoneName;
	}
	void setZoneName(const char *zoneName)
	{
		this->zoneName = zoneName;
	}
	const WORD getID() const
	{
		return wdServerID;
	}
	const WORD getType() const
	{
		return wdServerType;
	}
	const char *getIP() const
	{
		return pstrIP;
	}

	const WORD getPort() const
	{
		return wdPort;
	}
};

class SuperTimeTick :public zThread
{
public:

	~SuperTimeTick() {};

	static SuperTimeTick &getInstance()
	{
		if (NULL == instance)
			instance = new SuperTimeTick();

		return *instance;
	}

	/**
	* \brief �ͷ����Ψһʵ��
	*
	*/
	static void delInstance()
	{
		SAFE_DELETE(instance);
	}

	void run()
	{

	}

private:

	static zRTime currentTime;
	static SuperTimeTick *instance;

	zRTime startTime;
	QWORD qwStartGameTime;

	SuperTimeTick() : zThread("TimeTick"), startTime()
	{
		qwStartGameTime = 0;
	}

	bool readTime()
	{

	}
	bool saveTime()
	{

	}
};

class ServerTask : public zTCPTask
{
public:
	ServerTask(zTCPTaskPool *tcptaskPool, const SOCKET sock, const struct sockaddr_in *addr);
	virtual ~ServerTask();
	int verifyConn();
	int waitSync();
	int recycleConn();
	void addToContainer();
	void removeFromContainer();
	bool uniqueAdd();
	bool uniqueRemove();
	bool msgParse(const Cmd::t_NullCmd *, const DWORD);

	void responseOther(const WORD wdServerID);
	const WORD getID() const;
	const WORD getType() const;
	const DWORD getOnlineNum() const;
	bool checkSequenceTime();
private:
	WORD wdServerID;          /**< ��������ţ�һ����Ψһ�� */
	WORD wdServerType;          /**< ���������ͣ�������ʵ����ʱ���Ѿ�ȷ�� */
	char pstrIP[MAX_IP_LENGTH];      /**< ������������ַ */
	WORD wdPort;            /**< �����������˿� */

	DWORD      OnlineNum;      /**< ��������ͳ�� */

	zTime lastSequenceTime;        /**< ���һ�δ�������˳���ʱ�� */
	bool sequenceOK;          /**< �Ƿ��Ѿ��������������˳�� */
	bool hasNotifyMe;
	bool hasprocessSequence;


	bool verify(WORD wdType, const char *pstrIP);
	bool verifyTypeOK(const WORD wdType, std::vector<ServerTask *> &sv);
	bool processSequence();
	bool notifyOther();
	bool notifyOther(WORD dstID);
	bool notifyMe();

	bool msgParse_Startup(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);
	bool msgParse_Bill(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);
	bool msgParse_Gateway(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);
	bool msgParse_GmTool(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);
	bool msgParse_CountryOnline(const Cmd::t_NullCmd *pNullCmd, const DWORD nCmdLen);


	const char* GetServerTypeName(const WORD wdServerType);

	typedef std::unordered_map <Cmd::Super::ServerEntry, bool> Container;
	Container ses;
};


class ServerManager : zNoncopyable
{
private:

	typedef std::list<ServerTask *> Container;
	typedef Container::iterator Container_iterator;
	typedef Container::const_iterator Containter_const_iterator;

	typedef std::unordered_map<WORD, ServerTask *> ServerTaskHashmap;
	typedef ServerTaskHashmap::iterator ServerTaskHashmap_iterator;
	typedef ServerTaskHashmap::const_iterator ServerTaskHashmap_const_iterator;
	typedef ServerTaskHashmap::value_type ServerTaskHashmap_pair;

	
	zMutex mutex;
	Container container;	
	ServerTaskHashmap taskUniqueContainer;
	static ServerManager *instance;
	ServerManager() {};

public:

	~ServerManager() {};

	static ServerManager &getInstance()
	{
		if (NULL == instance)
			instance = new ServerManager();

		return *instance;
	}

	static void delInstance()
	{
		SAFE_DELETE(instance);
	}

	void addServer(ServerTask *task);
	void removeServer(ServerTask *task);
	ServerTask *getServer(WORD wdServerID);
	bool uniqueAdd(ServerTask *task);
	bool uniqueVerify(const WORD wdServerID);
	bool uniqueRemove(ServerTask *task);
	bool broadcast(const void *pstrCmd, int nCmdLen);
	bool broadcastByID(const WORD wdServerID, const void *pstrCmd, int nCmdLen);
	bool broadcastByType(const WORD wdType, const void *pstrCmd, int nCmdLen);
	const DWORD caculateOnlineNum();
	void responseOther(const WORD srcID, const WORD wdServerID);

};