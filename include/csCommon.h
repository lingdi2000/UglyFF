#ifndef _CS_COMMON_H_
#define _CS_COMMON_H_

namespace Cmd{
	const BYTE CMD_NULL = 0;    /**< 空的指令 */
	const BYTE PARA_NULL = 0;  /**< 空的指令参数 */
	/**
	* \brief 空操作指令，测间试信号和对时指令
	*
	*/
	struct t_NullCmd
	{
		BYTE cmd;          /**< 指令代码 */
		BYTE para;          /**< 指令代码子编号 */
		/**
		* \brief 构造函数
		*
		*/
		t_NullCmd(const BYTE cmd = CMD_NULL, const BYTE para = PARA_NULL) : cmd(cmd), para(para) {};
	};

	//////////////////////////////////////////////////////////////
	// 空指令定义结束
	//////////////////////////////////////////////////////////////


};

/**
* \brief 定义游戏区
* 对游戏进行分类，然后在同种游戏中再分区
*/
struct GameZone_t
{

	operator int() const
	{
		return id;
	}

	union
	{
		/**
		* \brief 唯一编号
		*/
		DWORD id;
		struct
		{
			/**
			* \brief 游戏分区编号
			*/
			WORD zone;
			/**
			* \brief 游戏种类编号
			*/
			WORD game;
		};
	};

	GameZone_t()
	{
		this->game = 0;
		this->zone = 0;
	}
	GameZone_t(const GameZone_t &gameZone)
	{
		this->id = gameZone.id;
	}
	GameZone_t & operator= (const GameZone_t &gameZone)
	{
		this->id = gameZone.id;
		return *this;
	}
	bool operator== (const GameZone_t &gameZone) const
	{
		return this->id == gameZone.id;
	}
};

enum{
	state_none = 0,          /**< 空的状态 */
	state_maintain = 1            /**< 维护中，暂时不允许建立新的连接 */
};


/**
* \brief 定义管理服务器的指令
*/

namespace Cmd
{
	namespace Super
	{
		// 保存一个服务器的信息
		struct ServerEntry
		{


			WORD wdServerID;
			WORD wdServerType;
			char pstrIP[MAX_IP_LENGTH];
			WORD wdPort;
			WORD state;
			ServerEntry()
			{
				wdServerID = 0;
				wdServerType = 0;
				bzero_(pstrIP, sizeof(pstrIP));
				wdPort = 0;
				state = 0;
			}
			ServerEntry(const ServerEntry& se)
			{
				wdServerID = se.wdServerID;
				wdServerType = se.wdServerType;
				strcpy(pstrIP, se.pstrIP);
				wdPort = se.wdPort;
				state = se.state;
			}
			ServerEntry & operator= (const ServerEntry &se)
			{
				wdServerID = se.wdServerID;
				wdServerType = se.wdServerType;
				strcpy(pstrIP, se.pstrIP);
				wdPort = se.wdPort;
				state = se.state;
				return *this;
			}

			operator int() const
			{
				return (int)wdServerID;
			}

			//用于 unordermap 需要重载
			bool operator == (const ServerEntry& p)const
			{
				if (wdServerID != p.wdServerID)
				{
					return false;
				}
				if (wdServerType != p.wdServerType)
				{
					return false;
				}
				if (strcmp(pstrIP, p.pstrIP) != 0)
				{
					return false;
				}
				if (wdPort != p.wdPort)
				{
					return false;
				}
				if (state != p.state)
				{
					return false;
				}
				return true;
			}
		};

		struct ServerEntryHash
		{
			size_t operator()(ServerEntry &se)
			{
				//只通过serverID来判断 hash值
				std::hash<WORD> sh;
				return sh(se.wdServerID);
			}
		};

		struct ServerEntryEqual
		{
			bool operator() (ServerEntry &se, ServerEntry& se2)
			{
				return se == se2;
			}
		};

		const BYTE CMD_STARTUP = 1;
		const BYTE CMD_BILL = 3;
		const BYTE CMD_GATEWAY = 4;
		//const BYTE CMD_GMTOOL  = 5;
		const BYTE CMD_SESSION = 5;
		const BYTE CMD_COUNTRYONLINE = 166;

		const BYTE PARA_STARTUP_REQUEST = 1;
		struct t_Startup_Request : t_NullCmd
		{
			WORD wdServerType;
			char pstrIP[MAX_IP_LENGTH];
			t_Startup_Request()
				: t_NullCmd(CMD_STARTUP, PARA_STARTUP_REQUEST) 
			{
				wdServerType = 0;
				memset(pstrIP,0,sizeof(pstrIP));
			};
		};

		const BYTE PARA_STARTUP_RESPONSE = 2;
		struct t_Startup_Response : t_NullCmd
		{
			WORD wdServerID;
			char pstrIP[MAX_IP_LENGTH];
			WORD wdPort;
			t_Startup_Response()
				: t_NullCmd(CMD_STARTUP, PARA_STARTUP_RESPONSE) {};
		};


		const BYTE PARA_STARTUP_OK = 5;
		struct t_Startup_OK : t_NullCmd
		{
			WORD wdServerID;
			t_Startup_OK()
				: t_NullCmd(CMD_STARTUP, PARA_STARTUP_OK) {};
		};
	};
};

#endif