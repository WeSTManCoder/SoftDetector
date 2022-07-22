#ifndef _NETMESSAGES_H_
#define _NETMESSAGES_H_

#define NETMSG_TYPE_BITS 			5
#define MAX_PLAYER_NAME_LENGTH		32
#define MAX_CUSTOM_FILES			4		// max 4 files
#define clc_ListenEvents			12
#define MAX_EVENT_BITS			9		// max bits needed for an event index
#define MAX_EVENT_NUMBER		(1<<MAX_EVENT_BITS)

#include <inetchannelinfo.h>
#include <inetmsghandler.h>
#include <inetmessage.h>
#include <tier1/bitbuf.h>
#include <bitvec.h>

#define DECLARE_BASE_MESSAGE( msgtype )						\
	public:													\
		bool			ReadFromBuffer( bf_read &buffer );	\
		bool			WriteToBuffer( bf_write &buffer );	\
		const char		*ToString() const;					\
		int				GetType() const { return msgtype; } \
		const char		*GetName() const { return #msgtype;}\


#define DECLARE_CLC_MESSAGE( name )		\
	DECLARE_BASE_MESSAGE( clc_##name );	\
	IClientMessageHandler *m_pMessageHandler;\
	bool Process() { return m_pMessageHandler->Process##name( this ); }\

class CNetMessage : public INetMessage {
	public:
		CNetMessage() {	m_bReliable = true;
						m_NetChannel = NULL; }

		virtual ~CNetMessage() {};

		virtual int		GetGroup() const { return INetChannelInfo::GENERIC; }
		INetChannel		*GetNetChannel() const { return m_NetChannel; }
			
		virtual void	SetReliable( bool state) {m_bReliable = state;};
		virtual bool	IsReliable() const { return m_bReliable; };
		virtual void    SetNetChannel(INetChannel * netchan) { m_NetChannel = netchan; }	
		virtual bool	Process() { Assert( 0 ); return false; };	// no handler set

	protected:
		bool				m_bReliable;	// true if message should be send reliable
		INetChannel			*m_NetChannel;	// netchannel this message is from/for
};

class CLC_ListenEvents : public CNetMessage {
	DECLARE_CLC_MESSAGE( ListenEvents );

	int	GetGroup() const { return INetChannelInfo::SIGNON; }

public:
	CBitVec<MAX_EVENT_NUMBER> m_EventArray;
};

#endif