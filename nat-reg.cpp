
#include <string.h>
#include <stdio.h>

#include "nat-port.h"
#include "nat-reg.h"

#include <assert.h>

void FromSockAddr( struct sockaddr_in const & sin, IpAndPort * iap )
{
  memset( iap, 0, sizeof( *iap ) );
  assert( sizeof( unsigned int ) == 4 );
  assert( sizeof( unsigned short ) == 2 );
  *(unsigned int *)iap->ip = (unsigned int)sin.sin_addr.s_addr;
  *(unsigned short *)iap->port = (unsigned short)sin.sin_port;
}


void ToSockAddr( IpAndPort const & iap, struct sockaddr_in * sin )
{
  memset( sin, 0, sizeof( *sin ) );
  assert( sizeof( unsigned int ) == 4 );
  assert( sizeof( unsigned short ) == 2 );
  sin->sin_addr.s_addr = *(unsigned int *)iap.ip;
  sin->sin_port = *(unsigned short *)iap.port;
  sin->sin_family = AF_INET;
}

bool Equal( IpAndPort const & a, IpAndPort const & b )
{
  return !memcmp( &a, &b, sizeof( a ) );
}

char const * IpAddr( IpAndPort const & a, char * buf )
{
  int port = ((int)a.port[0]<<8) + a.port[1];
  sprintf( buf, "%d.%d.%d.%d:%d", a.ip[0], a.ip[1], a.ip[2], a.ip[3], port );
  return buf;
}

