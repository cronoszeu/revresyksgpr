#pragma once

#include <time.h>
#include <winsock2.h>
#include "I_mydb.h"
#include "NetMsg.h"
#include "I_Shell.h"
#include "T_Singlemap.h"
#include "ClientSocket.h"
#include "SynWorldManager.h"
#include "AutoPtr.h"

#include "protocol.h"
//using namespace world_kernel;
#include "UserList.h"
#include "MapList.h"
#include "ConnectUser.h"
#include "T_SingleMap.h"

#include "Msg.h"

#define NEW_ROLE_STR		"NEW_ROLE"			// ����������
const int	MAX_GM_ID			= 1000;			// С��1000��ACCOUNT_ID����¼��������������

typedef	ISingleMap<CConnectUser>	IConnectSet;
typedef	CSingleMap<CConnectUser>	CConnectSet;

class ISynMessage
{
public:
	virtual bool	CreateSyndicate		(const CreateSyndicateInfo* pInfo)				PURE_VIRTUAL_FUNCTION_0
	virtual bool	DestroySyndicate	(OBJID idSyn, OBJID idTarget = ID_NONE)			PURE_VIRTUAL_FUNCTION_0
	virtual bool	CombineSyndicate	(OBJID idSyn, OBJID idTarget)					PURE_VIRTUAL_FUNCTION_0
	virtual bool	ChangeSyndicate		(const SynFuncInfo0* pFuncInfo)					PURE_VIRTUAL_FUNCTION_0
};

class CWorldKernel : IWorld, ISocket, ISynMessage
{
public:
	CWorldKernel() {m_setConnectClient=m_setConnectAccount=0;m_nState=0;}		// ��������
	virtual ~CWorldKernel() {}		// ��������

public:
	IWorld*	GetInterface()				{ return (IWorld*)this; }
	ISocket*	GetSocketInterface()	{ return (ISocket*)this; }
	CUserList*	QueryUserList()			{ ASSERT(m_pUserList); return m_pUserList; }
	CMapList*	QueryMapList()			{ ASSERT(m_pMapList); return m_pMapList; }
	ISynMessage*	QuerySynMessage()	{ return (ISynMessage*)this; }
	CSynWorldManager*	QuerySynManager()	{ ASSERT(m_pSynManager); return m_pSynManager; }

protected: // IWorld
	virtual bool	Create(IMessagePort* pPort);
	virtual bool	ProcessMsg(OBJID idPacket, void* buf, int nType, int nFrom);
	virtual bool	OnTimer(time_t tCurr);
	virtual bool	Release();

protected: // ISocket
	//virtual bool SendMsg			(CNetMsg* pNetMsg);
	virtual bool CloseSocket		(SOCKET_ID idSocket);			// ֱ�ӹر�socket

public: // send
	virtual bool SendClientMsg		(SOCKET_ID idSocket, Msg* pMsg);
	virtual bool SendNpcMsg			(OBJID idNpc, Msg* pMsg);
	virtual bool PrintText			(LPCTSTR szText);
	virtual bool ChangeCode			(SOCKET_ID idSocket, DWORD dwData);

public: // account server
	bool IsAccountLoginOK()						{ return m_nState == STATE_NORMAL; }
	bool SendFee(OBJID idAccount, int nType);
	bool SendQueryFee(OBJID idAccount, DWORD dwData, int nTime, int ucType);
	void ProcessAccountNetMsg();
	void ProcessLogin(const MsgAccountLogin& msgLogin);
	void ProcessFee(const MsgFee& msgFee);
	void ProcessQueryFee(const MsgQueryFee& msgQueryFee);
	bool ProcessConnect(const MsgConnect& msgConnect);			// return true: pass barrier
	bool ClientConnect(OBJID idAccount, DWORD dwData, LPCTSTR szInfo, SOCKET_ID idSocket);			// return true: pass barrier

protected:
	bool ProcessMyMsg		(SOCKET_ID idSocket, OBJID idNpc, Msg* pMsg, int nTrans=0);
	PROCESS_ID	GetProcessID() { return m_idProcess; }

public:
	IMessagePort*	GetMsgPort()							{ return m_pMsgPort; }
	IDatabase*		GetDatabase()							{ return m_pDb; }
	SOCKET_ID		GetNpcSocketID()						{ return m_idNpcSocket; }
	void			SetNpcSocketID(SOCKET_ID idNpcSocket)	{ m_idNpcSocket = idNpcSocket; }
	bool			LoadConfig(IDatabase* pDb);

public: // MessageBoard ---------------------
	bool SendMapGroupMsgForChatRoom(int nMapGroup,SOCKET_ID idSocket,Msg* pMsg);
	CMessageBoard*	QueryTradeMsgBd()						{ CHECKF(m_ptrTradeMsgBd); return m_ptrTradeMsgBd; }
	CMessageBoard*	QueryFriendMsgBd()						{ CHECKF(m_ptrFriendMsgBd); return m_ptrFriendMsgBd; }
	CMessageBoard*	QueryTeamMsgBd()						{ CHECKF(m_ptrTeamMsgBd); return m_ptrTeamMsgBd; }
	CMessageBoard*	QueryOtherMsgBd()						{ CHECKF(m_ptrOtherMsgBd); return m_ptrOtherMsgBd; }
	CMessageBoard*	QuerySystemMsgBd()						{ CHECKF(m_ptrSystemMsgBd); return m_ptrSystemMsgBd; }
protected:
	typedef	CAutoPtr<CMessageBoard>	MSGBD_PTR;
	MSGBD_PTR		m_ptrTradeMsgBd;
	MSGBD_PTR		m_ptrFriendMsgBd;
	MSGBD_PTR		m_ptrTeamMsgBd;
	MSGBD_PTR		m_ptrOtherMsgBd;
	MSGBD_PTR		m_ptrSystemMsgBd;

protected: // Syndicate
	virtual bool	CreateSyndicate		(const CreateSyndicateInfo* pInfo);
	virtual bool	DestroySyndicate	(OBJID idSyn, OBJID idTarget = ID_NONE);
	virtual bool	CombineSyndicate	(OBJID idSyn, OBJID idTarget);
	virtual bool	ChangeSyndicate		(const SynFuncInfo0* pFuncInfo);

protected:
	PROCESS_ID		m_idProcess;
	IDatabase*		m_pDb;
	IMessagePort*	m_pMsgPort;
	CUserList*		m_pUserList;
	CMapList*		m_pMapList;
	CSynWorldManager*	m_pSynManager;

	SOCKET_ID		m_idNpcSocket;

protected: // account server
	CClientSocket	m_sock;
	IConnectSet*		m_setConnectClient;
	IConnectSet*		m_setConnectAccount;
	OBJID		m_idServer;			// ��Ϸ��������ID
	enum { STATE_NONE, STATE_CONNECTOK, STATE_ACCOUNTOK, STATE_NORMAL };
	int			m_nState;
	NAMESTR		m_szServer;
	NAMESTR		m_szAccount;
	NAMESTR		m_szPassword;

protected: // static this
	static CWorldKernel*	m_pWorld;
	friend CWorldKernel* GameWorld();
};

inline CWorldKernel* GameWorld()		{ ASSERT(CWorldKernel::m_pWorld); return CWorldKernel::m_pWorld; }
inline CUserList*	UserList()			{ return GameWorld()->QueryUserList(); }
inline CMapList*	MapList()			{ return GameWorld()->QueryMapList(); }
inline CSynWorldManager*	SynWorldManager()	{ return GameWorld()->QuerySynManager(); }

#define	g_UserList	(*UserList())