#include "AllMsg.h"
#include "mapgroup.h"
#include "transformation.h"
#include "Agent.h"
#include "Npc.h"
/*
BOOL CMsgDataArray::Append(int nType, DWORD dwData)
{
	CHECKF(nType >= 0 && nType < _MAX_MSGATRRIB);
	CHECKF(m_unMsgSize + sizeof(DataArrayStruct) < MAX_PACKETSIZE);

	// fill info now
	m_unMsgSize		+= sizeof(DataArrayStruct);
	m_setDataArray[m_dwAttributeNum].ucAttributeType	= nType;
	m_setDataArray[m_dwAttributeNum].dwAttributeData	= dwData;
	m_dwAttributeNum++;

	return true;
}*/

void CMsgDataArray::Process(CUser* pUser)
{
	CHECK(pUser) ;
	IF_NOT( pUser->IsAlive() )
	{
		pUser->SendSysMsg(STR_DIE) ;
		return ;
	}

	switch( m_ucAction )
	{
	case _DATAARRAY_ACTION_UPGRADE_QUALITY: //����װ��Ʒ��	
		{
			this->UpEquipQuality(pUser);
		}
		break;

	case _DATAARRAY_ACTION_UPGRADE_SUPERADDITION: //����װ��׷��	
		{
			this->UpEquipSuperAddition(pUser);
		}
		break;
	case _DATAARRAY_ACTION_UPGRADE_EQUIPLEVEL: //����װ���ȼ�
		{
			this->UpEquipLevel(pUser);
		}
		break;
	case _DATAARRAY_ACTION_EMBEDGEM://��װ��Ƕ�뱦ʯ
		{
			this->EmbedGemToEquip(pUser);
		}
		break;
	case _DATAARRAY_ACTION_SETSYN_DRESS:	//���ð��ɷ�װ
		{
			this->SetSynDress(pUser);
		}
		break;
	default:
		break;

	}

}

int CMsgDataArray::RateSuccForGhostLevel(CItem* pEquipItem)
{
	return 50;
}

/*
ͨ����꾧ʯ���������������Ʒ�ʣ�ÿ���ȼ�����ĳɹ��ʲ�һ����
1--2��2--3��3--4��4--5�������ɹ��ʶ���100%��
��ͨ������0--1����30%�ɹ�
�����ϣ�1--2����12%�ɹ�
��������2--3����6%�ɹ�
��������3--4����4%�ɹ�

*/
int CMsgDataArray::RateSuccForQuality(CItem* pEquipItem)
{
	int iQuality = pEquipItem->GetQuality();
	
	if (iQuality==0)	return 30;
	else if(iQuality==1)	return 12;
	else if(iQuality==2)	return 6;
	else if(iQuality==3)	return 4;
	
	return 0;
}
//���������ĵȼ�
//������ƷTYPE�İ�λ��ʮλʵ���Ͼ��������ĵȼ����ȼ�Ϊ0--22��һ��������22�Ρ�
bool CMsgDataArray::UpEquipLevel(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForUpLevel())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//�Ƿ�Ϊװ��������
	{
		//��鼶��
		int nLevel = pEquipItem->GetLevel();
		if (nLevel>=22) {
			return false;
		}
		
		int nRateSucc =  RateSuccForEquipLevel(pEquipItem);
		if( RandGet(100) < nRateSucc ) //succ
		{
			IF_NOT(pEquipItem->UpLevel())
			{
				pUser->SendSysMsg(STR_UPEQUIPLEVEL_FAIL_NOTYPE);			
				return false;
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 
			//֪ͨ�ͻ���
			CMsgItemInfo msg;			
			if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);				
			pUser->SendSysMsg(STR_UPEQUIPLEVEL_SUCC);				
			return true;
		}
		else
		{
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 				
			pUser->SendSysMsg(STR_UPEQUIPLEVEL_FAIL);
			return false;
		}
	}
	DEBUG_CATCH("bool CMsgDataArray::UpEquipLevel()")
	return false;
}
//��������ʱ�ĳɹ���
/*
�����ࣨ������������������������ָ��Ь�ӵȣ���
������ƷTYPE�İ�λ��ʮλʵ���Ͼ��������ĵȼ����ȼ�Ϊ0--22��һ��������22�Ρ����������ʱ��itemtype�����Ҳ�����Ӧ�����ݣ����ʾ���������ˣ����磺450205�����Ҫ����������Ϊ450215������Ϊ���ݿ���û��450215�����ݣ����Ա�ʾ�����������������ˡ���
�����ɹ��ʣ�
���ȼ�<4ʱ�������ɹ���Ϊ100%
���ȼ�<7ʱ�������ɹ���Ϊ��35%
���ȼ�<10ʱ�������ɹ���Ϊ��20%
���ȼ�<13ʱ�������ɹ���Ϊ��10%
���ȼ�<16ʱ�������ɹ���Ϊ��7%
���ȼ�<19ʱ�������ɹ���Ϊ��4%
���ȼ�>=19ʱ�������ɹ���Ϊ��2%

װ���ࣨ�����·���ñ�ӺͶ��ƣ���
������ƷTYPE��ʮλ��ʾ�ȼ����ȼ�Ϊ0--9��һ��������9�Ρ����������ʱ��itemtype�����Ҳ�����Ӧ�����ݣ����ʾ���������ˡ�
���ȼ�<2ʱ�������ɹ���100%
���ȼ�<4ʱ�������ɹ���Ϊ��35%
���ȼ�<6ʱ�������ɹ���Ϊ��20%
���ȼ�<7ʱ�������ɹ���Ϊ��10%
���ȼ�<8ʱ�������ɹ���Ϊ��7%
���ȼ�<9ʱ�������ɹ���Ϊ��4%
*/
int CMsgDataArray::RateSuccForEquipLevel(CItem* pEquipItem)
{
	IF_NOT(pEquipItem)
		return 0;

	int nLevel = pEquipItem->GetLevel();
	if (pEquipItem->IsShield() || pEquipItem->IsArmor() || pEquipItem->IsHelmet())
	{
		if (nLevel>=0 && nLevel < 2)		 	    return 100;
		else if(nLevel>=2 && nLevel < 4)			return 35;
		else if(nLevel>=4 && nLevel < 6)			return 20;
		else if(nLevel>=6 && nLevel < 7)			return 10;
		else if(nLevel>=7 && nLevel < 8)			return 7;
		else if(nLevel>=8 && nLevel < 9)			return 4;		
	}
	else
	{
		if (nLevel>=0 && nLevel < 4)		 	    return 100;
		else if(nLevel>=4 && nLevel < 7)			return 35;
		else if(nLevel>=7 && nLevel < 10)			return 20;
		else if(nLevel>=10 && nLevel < 13)			return 10;
		else if(nLevel>=13 && nLevel < 16)			return 7;
		else if(nLevel>=16 && nLevel < 19)			return 4;
		else if(nLevel>=19&&nLevel<22) 			return 2;
	}	
	return 0;
}

bool CMsgDataArray::UpEquipQuality(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForUpQuality())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//�Ƿ�Ϊװ��������
	{
		//���Ʒ��
		int nQuality = pEquipItem->GetQuality();
		if (nQuality>=9) {
			return false;
		}
		
		//����Ʒ��GEM
		int iRateSucc = RateSuccForQuality(pEquipItem);		
		
		if (iRateSucc==0) {//�Ѿ�����õ�Ʒ��
			pUser->SendSysMsg(STR_QUALITY_UPGRADE_MAXLEVEL);
			return false;
		}
		if( RandGet(100) < iRateSucc ) //succ
		{
			IF_NOT(pEquipItem->UpQuality())
			{
				pUser->SendSysMsg(STR_ITEM_CHANGETYPE_FAILED);
				return false;
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 
			//֪ͨ�ͻ���
			CMsgItemInfo msg;			
			if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);
			pUser->SendSysMsg(STR_QUALITY_UPGRADE_SUCCESS);
			return true;	
		}
		else 
		{  
			//ʧ������
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 
			pUser->SendSysMsg(STR_QUALITY_UPGRADE_FAILED);
			return false;
		}		
	}	
	DEBUG_CATCH("bool CMsgDataArray::UpEquipQuality()")
	return false;
	
}
/*
2��ħ��ȼ���
ͨ��ħ�꾧ʯ��ħ�꾧ʯtype��1037150��ע��������ħ��ȼ���ÿ�������ɹ���Ϊ50%������ɹ�����ħ��ȼ�+1�����ʧ�ܣ���ħ��ȼ�-1�����Ŀǰħ��ȼ�Ϊ0���򲻻ήħ��ȼ�����ħ��ȼ�Ҳ��0--9����

*/
bool CMsgDataArray::UpEquipSuperAddition(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForUpSuperAddition())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//�Ƿ�Ϊװ��������
	{
		//���׷������
		int nSuperAddition = pEquipItem->GetSuperAddition();
		if (nSuperAddition>=9) {
			return false;
		}
		//����׷��
		int iRateSucc = RateSuccForGhostLevel(pEquipItem);		
		if( RandGet(100) < iRateSucc ) //succ
		{
			IF_NOT(pEquipItem->UpSuperAddition())
			{
				pUser->SendSysMsg(STR_GHOSTLEVEL_FAILED_NOTYPE);
				return false;
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 
			//֪ͨ�ͻ���
			CMsgItemInfo msg;			
			if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
				pUser->SendMsg(&msg);
			pUser->SendSysMsg(STR_GHOSTLEVEL_SUCCESS);			
			return true;
		}
		else 
		{  //ʧ��,����׷������
			if(pEquipItem->DecSuperAddition())
			{
				//֪ͨ�ͻ���
				CMsgItemInfo msg;			
				if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
					pUser->SendMsg(&msg);
			}
			pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 
			pUser->SendSysMsg(STR_GHOSTLEVEL_FAILED);
			return false;
		}		
	}
	DEBUG_CATCH("bool CMsgDataArray::UpEquipSuperAddition()")
	return false;

}
//��װ����Ƕ�뱦ʯ
bool CMsgDataArray::EmbedGemToEquip(CUser* pUser)
{
	DEBUG_TRY

	OBJID idEquipment, idGem;

	IF_NOT(m_uLong.GetSize()==2) 
		return false;
	
	idEquipment = m_uLong[0] ;
	idGem = m_uLong[1] ;

	CItem* pEquipItem = pUser->GetItem(idEquipment) ;
	CItem* pGem = pUser->GetItem(idGem) ;

	IF_NOT(pGem && pEquipItem && pGem->IsGemForEmbedEquip())
		return false;
	
	if( pEquipItem->IsEquipment() || pEquipItem->IsWeapon() )//�Ƿ�Ϊװ��������
	{
		//�õ�ħ��ȼ���ս��ȼ�
		int nSuperAdditionLevel = pEquipItem->GetSuperAddition();
		int nWarGhostLevel = pEquipItem->GetWarGhostLevel();

		if(nSuperAdditionLevel < MAX_LEVEL_SUPERADDITION || nWarGhostLevel < MAX_LEVEL_SUPERADDITION )
		{
			pUser->SendSysMsg(STR_EMBED_FAIL);
			return false;
		}

		DWORD dwGemType = pGem->GetInt(ITEMDATA_TYPE);		
		pUser->EraseItem(idGem,SYNCHRO_TRUE) ; //֪ͨ�Է�����ʯɾ�� 
		pEquipItem->SetInt(ITEMDATA_GEMTYPE,dwGemType);
		CMsgItemInfo msg;			
		if (msg.Create(pEquipItem,ITEMINFO_UPDATE))
			pUser->SendMsg(&msg);		
		pUser->SendSysMsg(STR_EMBED_SUCC);

		return true;
	}
	DEBUG_CATCH("bool CMsgDataArray::EmbedGemToEquip(...)")
	return false;
}

void CMsgDataArray::SetSynDress(CUser* pUser)
{
	IF_NOT(m_uLong.GetSize()==4) 
		return ;
	OBJID idNpc = m_uLong[0] ;
	CNpc* pNpc; 
	if(!pUser->GetMap()->QueryObj(pUser->GetPosX(), pUser->GetPosY(), OBJ_NPC, idNpc, IPP_OF(pNpc)))
		return ;
	if(!pNpc->IsTaskNpc())
		return;
	
	int nLevel1 = m_uLong[1]+1 ;
	int nLevel2 = m_uLong[2]+1 ;
	int nLevel3 = m_uLong[3]+1 ;
	int nLevel4 = m_uLong[4]+1 ;
//	char szData1[255];
//	sprintf(szData1,"%d%d%d%d",nLevel1,nLevel2,nLevel3,nLevel4);
//	int nLevel = atoi(szData1);
	
	int nLevel	= nLevel1*1000 + nLevel2*100 + nLevel3*10 + nLevel4;

	pNpc->SetData(SYNDRESS_SETTING,nLevel,true);//���ð��ɷ�װ
	
	pUser->SendSysMsg(STR_SYN_SETDRESS_SUCC);		
	
}