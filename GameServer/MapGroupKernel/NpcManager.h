
#pragma once

#include "define.h"
#include "protocol.h"
#include "AiNpc.h"
#include "GameObjSet.h"
#include "T_MyQueue.h"


typedef	IGameObjSet<CNpc>			INpcSet;
typedef	CGameObjSet<CNpc>			CNpcSet;

class INpcManager
{
public:
	virtual ~INpcManager()	{}
	virtual void	Release()																			PURE_VIRTUAL_FUNCTION
	virtual bool	DelAllMonster()																		PURE_VIRTUAL_FUNCTION_0
	virtual bool	CreateMonster(const ST_CREATENEWNPC* pInfo)											PURE_VIRTUAL_FUNCTION_0
	virtual bool	CreateSynPet(const ST_CREATENEWNPC* pInfo, LPCTSTR pszName=NULL)					PURE_VIRTUAL_FUNCTION_0
	virtual CMonster*	CreateCallPet(CUser* pUser, OBJID idType, int x, int y)								PURE_VIRTUAL_FUNCTION_0
	// �������� -- zlong 2004-02-05
	virtual CMonster*	CreateEudemon(CUser* pUser, CItem* pItem, int x, int y)							PURE_VIRTUAL_FUNCTION_0
};

class CNpcManager : INpcManager
{
public:
	CNpcManager(PROCESS_ID idProcess);
	virtual ~CNpcManager();
	INpcManager*	GetInterface()			{ return (INpcManager*)this; }
	bool		Create();

public:
	virtual void	Release()				{ delete this; }
	virtual bool	DelAllMonster();
	virtual bool	CreateMonster(const ST_CREATENEWNPC* pInfo);
	virtual bool	CreateSynPet(const ST_CREATENEWNPC* pInfo, LPCTSTR pszName=NULL);
	virtual CMonster*	CreateCallPet(CUser* pUser, OBJID idType, int x, int y);
	// �������� -- zlong 2004-02-05
	virtual CMonster*	CreateEudemon(CUser* pUser, CItem* pItem, int x, int y);

protected: // ctrl
	PROCESS_ID			m_idProcess;

public:
	MYHEAP_DECLARATION(s_heap)
};


