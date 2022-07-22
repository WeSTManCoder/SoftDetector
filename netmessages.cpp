#include "netmessages.h"
#include <tier1/strtools.h>

static char s_text[1024];

bool CLC_ListenEvents::WriteToBuffer( bf_write &buffer ) {
	buffer.WriteUBitLong( GetType(), NETMSG_TYPE_BITS );

	int count = MAX_EVENT_NUMBER / 32;
	for ( int i = 0; i < count; ++i ) buffer.WriteUBitLong( m_EventArray.GetDWord( i ), 32 );

	return !buffer.IsOverflowed();
}

bool CLC_ListenEvents::ReadFromBuffer( bf_read &buffer ) {
	int count = MAX_EVENT_NUMBER / 32;
	for ( int i = 0; i < count; ++i ) m_EventArray.SetDWord( i, buffer.ReadUBitLong( 32 ) );

	return !buffer.IsOverflowed();
}

const char *CLC_ListenEvents::ToString(void) const {
	int count = 0;

	for ( int i = 0; i<MAX_EVENT_NUMBER; i++ )
		if ( m_EventArray.Get( i ) ) count++;

	Q_snprintf(s_text, sizeof(s_text), "%s: registered events %i", GetName(), count );
	return s_text;
}