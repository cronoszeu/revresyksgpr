// �����̲߳���
// �ɽ���, 2001.10.10

//#include "allheads.h"


//	2��CServerSocket���������ڷ����߳��д������������
/*
CServerSocket<key1, key2>::CServerSocket()
{
	m_bufPeerIP[0]	= 0;
	m_sockServer = INVALID_SOCKET;

	m_bSendEnable	= false;
	m_bState		= false;
}
//else*/
template <uint32 key1, uint32 key2>
CServerSocket<key1, key2>::CServerSocket(/*uint32 key1, uint32 key2*/)	// �����м���SOCKETʱ��ʼ��WSAStartup()���ʸô����ص���
{
	m_sockServer	= INVALID_SOCKET;
	m_bufPeerIP[0]	= 0;
	m_tLastRecv		= 0;

//	m_bSendEnable	= false;
	m_bState		= c_stateNone;
}
//*/

template <uint32 key1, uint32 key2>
CServerSocket<key1, key2>::~CServerSocket()
{
	Close();
}

template <uint32 key1, uint32 key2>
bool CServerSocket<key1, key2>::Open(SOCKET sockServer, u_long nClientIP)
{
	if(m_bState)
	{
		LOGWARNING("CServerSocket<key1, key2>::Open()ǰû��CLOSE");
		Close(true);
	}

	if(sockServer == INVALID_SOCKET)
	{
		LOGERROR("CServerSocket<key1, key2>::Open()��������SOCKET����Ч");
		return	false;
	}

	if(m_sockServer != INVALID_SOCKET)
	{
		LOGERROR("CServerSocket<key1, key2>::Open()������ǰһ��SOCKET");
	}

	m_sockServer = sockServer;
#ifdef	ENCRYPT
	m_cEncryptRecv.Init();
	m_cEncryptSend.Init();
#endif
	m_bufPeerIP[0] = 0;
	struct in_addr	in;
	in.S_un.S_addr = nClientIP;
	char * pClientIP = inet_ntoa(in);
	if(pClientIP)
		strcpy(m_bufPeerIP, pClientIP);
	m_tLastRecv		= -clock();			// ��ֵ��ʾ����ʱ��

//	m_bSendEnable	= true;
	m_bState		= c_stateNormal;

//	IncConnectCount();

	return true;
}
/*
bool CServerSocket<key1, key2>::InitSocket(SOCKET sockServer)
{
	if(m_bState)
		return false;

	m_sockServer = sockServer;

	m_bSendEnable	= true;
	m_bState		= true;

	return true;
}
//*/
//*** ����һ����Ϣ��д��pBuf��ָ�Ļ������С�����false: �����رա�����trueʱnLen��0��û������
template <uint32 key1, uint32 key2>
bool CServerSocket<key1, key2>::Recv(char * pBuf, int &nLen)
{
	int		ret;
	u_short	nMessageSize = 0;
	nLen = 0;

	if(!m_bState)
		return false;

	if((ret = recv(m_sockServer, (char *)&nMessageSize, 2, MSG_PEEK)) == 2)	// ����2�ֽڳ���
	{
#ifdef	ENCRYPT
		// ���ܹ���
//		SAVELOG("@@@: δ���ܳ���[%d].", nMessageSize);
		if(key1 || key2)
			m_cEncryptRecv.Encrypt((unsigned char *)&nMessageSize, 2, false);	// false: ���ƶ�����ָ��
//		SAVELOG("@@@: �ѽ��ܳ���[%d].", nMessageSize);
#endif
		// ���յ�2�ֽڵ���Ϣ����
		u_long	nRcvBufLen;
		ioctlsocket(m_sockServer, FIONREAD, &nRcvBufLen);		// ���SOCKET�ڲ���INBUF�ߴ�

		// �����Ϣ����
		if(!(nMessageSize > 0 && nMessageSize <= _MAX_MSGSIZE))
		{
			// ���յ�һ��������Ϣ
			LOGWARNING("���յ�һ�����󳤶�[%d]����Ϣ����ʱRCVBUF����[%d]���ֽڵ����ݡ��ͻ��˵�IP��ַΪ[%s]", 
							nMessageSize, nRcvBufLen, m_bufPeerIP);
/*
			recv(m_sockServer, pBuf, 52, 0);
			pBuf[19] = 0; pBuf[35] = 0; pBuf[51] = 0;
			LOGERROR("���������������¼����������¼��[%s]������[%s]��������[%s]", pBuf+4, pBuf+20, pBuf+36);
*/
			Close(true);
			return false;
		}

		if(nRcvBufLen >= nMessageSize)
		{
			// SOCKET�ڲ����㹻������
			int	nGotBytes	= 0;
			int	nCount		= 0;
			while(nGotBytes < nMessageSize)
			{
				ret = recv(m_sockServer, (char *)pBuf + nGotBytes, nMessageSize - nGotBytes, 0);		// ��������
				if(ret > 0)
				{
					nGotBytes += ret;
				}
				else
				{
					break;
				}

				// ����ѭ��
				if(++nCount > RECVREPEATTIMES)		//?? ���Ա���
				{
					LOGWARNING("ϵͳЧ�������½���recv()�����޷������������ݡ�һ�����ӱ�ǿ�жϿ���");
					Close(true);
					return false;
				}
			}

			// ���յ�����
			if(nGotBytes == nMessageSize)
			{
				// ��� CPU Ч��
				if(nCount >= 2)		//?? 2: ���Բ���
				{
					if(nCount >= 3)		//?? 3: ���Բ���
					{
						LOGMSG("ϵͳЧ�������½���recv()������[%d]�β�ȡ���㹻�����ݡ�", nCount);
					}

#ifdef	LOCKACCEPT
					// ���������̵߳�accept()������5���ڲ���������ҵ�¼��
					if(InterlockedExchange(&s_nLockAcceptTime, clock() + LOCKACCEPTSECOND*CLOCKS_PER_SEC) == 0)
					{
						LOGWARNING("ϵͳ̫��æ������ͣ����������...");
					}
#endif	// LOCKACCEPT
				}

				// ����m_tLastRecv����
				m_tLastRecv = clock();

				// ���ؽ��յ�������
				nLen = nMessageSize;
#ifdef	ENCRYPT
				// ���ܹ���
				if(key1 || key2)
					m_cEncryptRecv.Encrypt((unsigned char *)pBuf, nLen);
//		SAVELOG("@@@: �ѽ��ܳ���[%d][%d].", *(uint16 *)pBuf, nLen);
#endif
#ifdef	DUMP
				char buf[4096] = "";
				char buf3[4];
				for(int i = 0; i < nLen; i++)
				{
					sprintf(buf3, "%02X ", (unsigned char)pBuf[i]);
					strcat(buf, buf3);
					if(i%16 == 15)
						strcat(buf, "\n");
				}
				strcat(buf, "\n");
				LOGMSG("Send: \n%s", buf);
#endif
				return true;
			}
			else if(ret == 0)
			{
#ifdef	ALPHA_X
				LOGMSG("���յ��ͻ��˵Ĺر�SOCKET��Ϣ��");
#endif
				Close();
				return false;	// �ر�
			}
			else if(ret < 0)
			{
				int err = WSAGetLastError();
				if(err == WSAEWOULDBLOCK)
				{
					LOGWARNING("ϵͳЧ�������½���recv()��������������һ�����ӱ�ǿ�жϿ���ϵͳ��[%d]�ֽڣ�", nRcvBufLen);
					LOGWARNING("�������[%d]���ֽڣ���ֻ�ܶ���[%d]���ֽڡ�", nMessageSize, nGotBytes);
//					nLen = 0;		// ������
					return false;
				}
				else
				{
					// ���󣬻��ѹر�
#ifdef	ALPHA_X
					LOGERROR("recv()����[%d]", err);
#endif
					Close(true);
					return false;
				}
			}
			else
			{
				LOGERROR("CServerSocket<key1, key2>::Recv()�����쳣");
				return false;
			}

			// ���󣬻��ѹر�
//			LOGERROR("recv()����");
//			Close(true);
//			return false;
		}
		else	// if(nRcvBufLen >= nMessageSize)
		{
			nLen = 0;		// ������
			return true;
		}
	}
	else if(ret > 0)
	{
		nLen = 0;		// ���ݲ���
		return true;
	}
	else if(ret == 0)
	{
#ifdef	ALPHA_X
		LOGMSG("���յ��ͻ��˵Ĺر�SOCKET��Ϣ��");
#endif
		Close();
		return false;	// �ر�
	}
	else	// if(ret < 0)
	{
		int err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK)
		{
			nLen = 0;		// ������
			return true;
		}
		else
		{
			// ����
#ifdef	ALPHA_X
			LOGERROR("recv()����[%d]", err);
#endif
			Close(true);
			return false;
		}
	}

	// ���󣬻��ѹر�
//	LOGERROR("recv()����");
//	return false;
}
template <uint32 key1, uint32 key2>
bool CServerSocket<key1, key2>::Send(const Msg* pMsg)	// ����false: �������
{
	return Send((const char*)pMsg,pMsg->GetSize());
}

template <uint32 key1, uint32 key2>
bool CServerSocket<key1, key2>::Send(const char * pBuf, int nLen)	// ����false: �������
{
//		SAVELOG("@@@: �ѽ��ܳ���Send[%d][%d].", *(uint16 *)pBuf, nLen);
	if(m_bState == c_stateNone)
		return false;
	if(m_bState == c_stateShutDown)
		return true;		//?? ���ݶ���

	// ������
	if(!(pBuf && nLen > 0 && nLen <= _MAX_MSGSIZE))
	{
		LOGERROR("CServerSocket<key1, key2>::Send([0x%04X], [%d])�����쳣��", pBuf, nLen);
		Close(true);
		return false;
	}
	int	nMsgLen;
	try{ nMsgLen = *(uint16 *)pBuf;	}catch(...) {
		LOGCATCH("CServerSocket<key1, key2>::Send()ָ�������Ч��һ�����ӱ��Ͽ���");
		Close(true);
		return false;
	}
	if(!(nMsgLen > 0 && nMsgLen <= _MAX_MSGSIZE && nMsgLen == nLen))		// ���ͨѶ��Ϣ������
	{
		LOGERROR("�ں�SEND��һ����Ϣ�ĳ���ֵ[%d]���󣬻�����Ϣ����[%d]������һ�����ӱ��Ͽ���", nMsgLen, nLen);
		LOGERROR("����ϢIDΪ[%d]", *(const uint16 *)(pBuf + 2));
		Close(true);
		return false;
	}
	
#ifdef	PROFILE_X
	InterlockedExchangeAdd(&s_nPackSum, nLen);
	InterlockedIncrement(&s_nPackCount);
#endif	// PROFILE
	
#ifdef	DUMP
	char buf[4096] = "";
	char buf3[4];
	for(int i = 0; i < nLen; i++)
	{
		sprintf(buf3, "%02X ", (unsigned char)pBuf[i]);
		strcat(buf, buf3);
		if(i%16 == 15)
			strcat(buf, "\n");
	}
	strcat(buf, "\n");
	LOGMSG("Send: \n%s", buf);
#endif
#ifdef	ENCRYPT
	// ���ܹ���
	char	buf[_MAX_MSGSIZE];			//??? ���Ż�
	memcpy(buf, pBuf, nLen);
	pBuf = buf;		//? �滻��������ֵ
	if(key1 || key2)
		m_cEncryptSend.Encrypt((unsigned char *)pBuf, nLen);
#endif
	
	// �ظ�д��RECVREPEATTIMES��
	int		nPutBytes = 0;		// �Ѿ������˵ĳ���
	int		nCount = 0;
	int		ret = 0;
	while(nPutBytes < nLen)
	{
		ret = send(m_sockServer, (char *)pBuf + nPutBytes, nLen - nPutBytes, 0);		// ��������
		if(ret > 0)
		{
			nPutBytes += ret;
		}
		else	// ret < 0
		{
			int err = WSAGetLastError();
			if(err != WSAEWOULDBLOCK)
			{
#ifdef	ALPHA_X
				LOGWARNING("����SOCKET��send()����[%d]�����ܿͻ��˹ر���SOCKET", err);
#endif
				Close(true);
				return false;
			}
			
			// �ٸ���һ�λ��ᡣ������
			ret = send(m_sockServer, (char *)pBuf + nPutBytes, nLen - nPutBytes, 0);		// ��������
			if(ret > 0)
			{
				nPutBytes += ret;
				LOGERROR("�״�SEND���ɹ����ٴ�SEND�ɹ���ԭ��δ֪��IP��ַΪ[%s]", m_bufPeerIP);
				// continue;
			}
			else	// ret < 0
			{
				int err = WSAGetLastError();
				if(err == WSAEWOULDBLOCK)
				{
					if(m_tLastRecv > 0)
					{
						if((clock()-m_tLastRecv) / CLOCKS_PER_SEC >= 25)		//? 25�����ϸ�������
							LOGERROR("send()��������ܷ������Ѷ��ߡ�IP��ַΪ[%s]", m_bufPeerIP);
						else
							LOGERROR("send()������������̫����IP��ַΪ[%s]", m_bufPeerIP);
					}
					else if(m_tLastRecv < 0)			// ��ֵ��ʾ����ʱ��
					{
						if((clock()+m_tLastRecv) / CLOCKS_PER_SEC >= 25)		//? 25�����ϸ�������
							LOGERROR("send()��������ܷ������Ѷ��ߡ�IP��ַΪ[%s]��", m_bufPeerIP);
						else
							LOGERROR("send()������������̫����IP��ַΪ[%s]��", m_bufPeerIP);
					}
					else
					{
						LOGERROR("�ڲ��쳣��m_tLastRecv Ϊ 0");
					}
				}	// WSAEWOULDBLOCK
#ifdef	ALPHA_X
				else
					LOGWARNING("����SOCKET��send()����[%d]�����ܿͻ��˹ر���SOCKET.", err);
#endif

				// �ٴ�ERR, �Ͽ�
				Close(true);
				return false;
			}	// ret < 0
		}	// ret < 0

		// ����ѭ��
		if(++nCount > RECVREPEATTIMES)		//?? ���Ա���
		{
			LOGWARNING("ϵͳЧ�������½���send()�����޷�����д�����ݡ�һ�����ӱ�ǿ�жϿ���");
			Close(true);
			return false;
		}
	}

	ASSERT(ret == nLen);

	if(nCount >= 2)		//?? 2: ���Բ���
	{
		LOGMSG("ϵͳЧ�������½���send()������[%d]�βŷ��������ݡ�", nCount);
	}

#ifdef	PROFILE_XXXXX
	// OUTBUF�������
	if(g_aServerShare[nIndex].m_queOutput.Len() > OUTBUFSIZE*4/5)
	{
		LOGWARNING("����[%d]OUTBUFʹ��̫��[%d]���������", nIndex, 
					g_aServerShare[nIndex].m_queOutput.Len());
	}

	// ͳ��OUTBUFʹ�����
#endif

	return true;
}

//	�ر�д�������Ա��öԷ������ر�SOCKET��
template <uint32 key1, uint32 key2>
bool	CServerSocket<key1, key2>::ShutDown()
{
	if(!m_bState)
		return false;

//	m_bSendEnable = false;
	m_bState = c_stateShutDown;
	if(shutdown(m_sockServer, 1))		// 1: SD_SEND
	{
		int err = WSAGetLastError();
		LOGWARNING("����SOCKET��shutdown()����[%d]�����ܿͻ��˹ر���SOCKET", err);
		Close(true);
		return false;
	}
	else
	{
		return true;	// �����ر�
	}
}

//	���ظ����á�ͨ�����ã����������ر�SOCKET��
template <uint32 key1, uint32 key2>
void	CServerSocket<key1, key2>::Close(bool bLinger0 /*= false*/)
{
	if(m_bState)
	{
		if(bLinger0)
		{
			//	�޸�SOCKET���Գ������ر���
			struct linger ling = { 1, 0 };
			setsockopt(m_sockServer, SOL_SOCKET, SO_LINGER, (char *)&ling, sizeof(ling));
		}

		m_bState = c_stateNone;
//		m_bSendEnable = false;
		closesocket(m_sockServer);
		InterlockedDecrement(&s_nSocketCount);
		m_sockServer	= INVALID_SOCKET;
		m_bufPeerIP[0]	= 0;

//		DecConnectCount();
	}
}

template <uint32 key1, uint32 key2>
char *	CServerSocket<key1, key2>::GetPeerIP()	// ����NULLΪ�����ɸ�ΪNEWʱ��ACCEPT����nClientIP???
{
	if(!m_bState)
		return NULL;

	if(strlen(m_bufPeerIP))
	{
		return m_bufPeerIP;
	}
	else
	{
		// ȡ�Է�IP��������
		LOGERROR("������GetPeerIP()�е���Ч���롣���޴���)");
		sockaddr_in	inAddr;
		memset(&inAddr, 0, sizeof(inAddr));
		int		nLen = sizeof(inAddr);
		if(getpeername(m_sockServer, (sockaddr *)&inAddr, &nLen))
		{
			int err = WSAGetLastError();
			LOGERROR("�����SOCKET��getpeername()����[%d].", err);
			Close(true);
			return NULL;
		}
		char *	pAddr = inet_ntoa(inAddr.sin_addr);
		if(pAddr)
		{
			strcpy(m_bufPeerIP, pAddr);
			return m_bufPeerIP;
		}
		else
		{
			LOGERROR("�����SOCKET��inet_ntoa()����.");
			Close(true);
			return NULL;
		}
	}
}