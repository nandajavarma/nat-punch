
// This file implements the client (one of many peers) of a 
// peer-to-peer punch-through-NAT demonstration, over UDP. 
// It will periodically try to register itself with a server 
// running on a hard-coded IP address; the server will send 
// a list of all connected peers (including this peer) back. 
// The peer will then attempt to send messages to all the 
// other peers. For full description, see
// http://www.mindcontrol.org/~hplus/nat-punch.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "nat-reg.h"
#include "nat-util.h"
#include "nat-port.h"


PeerId me;

void
usage()
{
  fprintf( stderr, "usage: nat-client id-str\n" );
  exit( 1 );
}

void
RegisterWithIntroducer( SOCKET sock, struct sockaddr_in const * srv )
{
  fprintf( stderr, "Attempting to register with introducer.\n" );
  NatGwProtoMsg msg;
  memset( &msg, 0, sizeof( msg ) );
  msg.what = htons( GwMsgSelfDesc );
  msg.selfDesc.id = me;
  struct sockaddr_in local;
  memset( &local, 0, sizeof( local ) );
  socklen_t len = sizeof( local );
  DIE_IF_ERR( getsockname( sock, (struct sockaddr *)&local, &len ) );
  FromSockAddr( local, &msg.selfDesc.peer );
  DIE_IF_ERR( sendto( sock, (char const *)&msg, sizeof( msg ), 0, (struct sockaddr *)srv, sizeof( *srv ) ) );
}

void
HandlePeerMsg( NatGwProtoMsg const & msg )
{
  PeerId id;
  memcpy( &id, &msg.peerMsg.id, sizeof( id ) );
  id.name[ PEER_ID_SIZE-1 ] = 0;
  fprintf( stderr, "PeerMsg from %s\n", id.name );
}

void
HandleRegDesc( SOCKET sock, NatGwProtoMsg const & msg )
{
  fprintf( stderr, "RegDesc received.\n" );
  // I'm potentially setting myself up for DOS-ing a third party here. Oh, well.
  // Validating that the source of the message was the introducer would be a 
  // small step forward. Using cryptographic authentication is the only way to 
  // really make sure about these things, though.
  for( int i = 0; i < MAX_PEERS; ++i ) {
    if( msg.regDesc[i].id.name[0] && strncmp( msg.regDesc[i].id.name, me.name, PEER_ID_SIZE ) ) {
      char buf[ 32 ];
      fprintf( stderr, "Sending to peer \"%.*s\" at %s.\n", 
          PEER_ID_SIZE, msg.regDesc[i].id.name, IpAddr( msg.regDesc[i].gateway, buf ) );
      NatGwProtoMsg peerMsg;
      peerMsg.what = htons( GwMsgPeerMsg );
      peerMsg.peerMsg.id = me;
      struct sockaddr_in psin;
      ToSockAddr( msg.regDesc[i].gateway, &psin );
      DIE_IF_ERR( sendto( sock, (char const *)&peerMsg, sizeof( peerMsg ), 0, (struct sockaddr *)&psin, sizeof( psin ) ) );
    }
  }
}

void
ReadAndProcessIncomingMessage( SOCKET sock )
{
  NatGwProtoMsg msg;
  memset( &msg, 0, sizeof( msg ) );
  struct sockaddr_in remote;
  memset( &remote, 0, sizeof( remote ) );
  remote.sin_family = AF_INET;
  socklen_t len = sizeof( remote );
  int r = recvfrom( sock, (char *)&msg, sizeof( msg ), 0, (struct sockaddr *)&remote, &len );
#if defined( WIN32 )
  if( (r < 0) && (WSAGetLastError() == WSAECONNRESET) ) {
    r = 0;
  }
#endif
  DIE_IF_ERR( r );
  // wanton waste always sending max-size packets!
  if( r != sizeof( msg ) ) {
    fprintf( stderr, "Received malformed packet; size: %d\n", r );
    return;
  }
  // Currently, I only talk to the peers in response to an introducer message. 
  // In real life, you obviously want a better conversation going between the 
  // peers, and only check back with the introducer once in a blue moon, to 
  // pick up a peer whose public mapping might have changed.
  switch( ntohs( msg.what ) ) {
    case GwMsgRegDesc:
      HandleRegDesc( sock, msg );
      break;
    case GwMsgPeerMsg:
      HandlePeerMsg( msg );
      break;
    default:
      fprintf( stderr, "Received unexpected message; what code: %d\n", ntohs( msg.what ) );
      break;
  }
}

int
main( int argc, char * argv[] )
{
#if defined( WIN32 )
  char * xx[] = {
    "msvc71", "Win32Client", 0
  };
  argv = xx;
  argc = 2;

  WSADATA wsaData;
  DIE_IF_ZERO( (int)!WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) );
#endif
  if( argc != 2 || argv[1][0] == '-' ) {
    usage();
  }

  memset( &me, 0, sizeof( me ) );
  strncpy( me.name, argv[1], 19 );
  fprintf( stderr, "My ID is \"%s\".\n", me.name );

  // open a socket
  protoent * proto = DIE_IF_NULL( port_getprotobyname( "udp" ) );
  SOCKET cliSock = socket( PF_INET, SOCK_DGRAM, proto->p_proto );
  DIE_IF_ERR( (int)cliSock );

  // bind it locally
  struct sockaddr_in sinLocal;
  memset( &sinLocal, 0, sizeof( sinLocal ) );
  sinLocal.sin_family = AF_INET;
  sinLocal.sin_port = htons( CLIENT_PORT );
  sinLocal.sin_addr.s_addr = INADDR_ANY;
  // I might not necessarily need to bind, or I could look for any unbound port 
  // starting at some range, but for debugging, this makes things more predictable.
  DIE_IF_ERR( bind( cliSock, (struct sockaddr *)&sinLocal, sizeof( sinLocal ) ) );

  // find the server
  struct sockaddr_in sinServer;
  IpAndPort iap = { { SERVER_IP }, { SERVICE_PORT >> 8, SERVICE_PORT & 255 } };
  ToSockAddr( iap, &sinServer );

  // set up the state machine
  time_t then = 0;
  time_t now;
  fd_set rdSet;
  struct timeval tv;

  // enter the state machine
  while( true ) {
    FD_ZERO( &rdSet );
    FD_SET( cliSock, &rdSet );
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int out = DIE_IF_ERR( select( (int)cliSock+1, &rdSet, NULL, NULL, &tv ) );
    if( (out > 0) && FD_ISSET( cliSock, &rdSet ) ) {
      // process an incoming message, which is either a registration reply, or a peer-to-peer message
      ReadAndProcessIncomingMessage( cliSock );
    }
    time( &now );
    if( now-then > 4 ) {
      then = now;
      // try registering with the introducer
      RegisterWithIntroducer( cliSock, &sinServer );
    }
  }
  return 0;
}

