
// This file implements the introducer of a peer-to-peer 
// punch-through-NAT demonstration, over UDP. 
// It will respond to registration requests for various 
// peers, and introduce their public IP addresses to each 
// other, so that they can send direct traffic without 
// involving the server.
// For a full description of the technique, see:
// http://www.mindcontrol.org/~hplus/nat-punch.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <assert.h>

#include "nat-reg.h"
#include "nat-util.h"
#include "nat-port.h"


NatGwProtoMsg peers;
time_t lastSeen[ MAX_PEERS ];

void
usage()
{
  fprintf( stderr, "usage: nat-server\n" );
  exit( 1 );
}

void
UpdateOrAllocatePeerAndReply( int sock, NatGwProtoMsg const & msg, struct sockaddr_in const & remote )
{
  IpAndPort iap;
  FromSockAddr( remote, &iap );
  int ixToUse = -1;
  time_t now;
  time( &now );
  time_t best = now;

  // I just use the message I'll reply with to keep track of who's online.
  // I also replace the oldest client, with additional time-out if no refresh 
  // within a minute. This is enough to show the technique of introduction, 
  // but not robust in any way.
  // Note that if nobody sends a message, no time-outs will happen.
  for( int ix = 0; ix < MAX_PEERS; ++ix ) {
    if( !strncmp( peers.regDesc[ ix ].id.name, msg.selfDesc.id.name, PEER_ID_SIZE ) ) {
      // I've seen this guy before!
      if( Equal( peers.regDesc[ ix ].gateway, iap ) ) {
        // He's where he used to be -- shortcut by not re-registering
        ixToUse = ix;
        lastSeen[ ix ] = now;
        goto got_already;
      }
      else {
        // Ooh -- he moved! Re-register in his hold slot.
        ixToUse = ix;
        break;
      }
    }
    if( lastSeen[ ix ] < best ) {
      ixToUse = ix;
      best = lastSeen[ ix ];
    }
    if( lastSeen[ ix ] < (now - 60) && peers.regDesc[ ix ].id.name[ 0 ] ) {
      fprintf( stderr, "Timing out old peer \"%s\".\n", peers.regDesc[ ix ].id.name );
      memset( &peers.regDesc[ ix ], 0, sizeof( peers.regDesc[ ix ] ) );
    }
  }
  assert( ixToUse != -1 );

  // allocate this slot to the new peer
  peers.regDesc[ ixToUse ].id = msg.selfDesc.id;
  peers.regDesc[ ixToUse ].id.name[ PEER_ID_SIZE-1 ] = 0;
  peers.regDesc[ ixToUse ].peer = msg.selfDesc.peer;
  FromSockAddr( remote, &peers.regDesc[ ixToUse ].gateway );
  char a1[ 32 ], a2[ 32 ];
  if( Equal( peers.regDesc[ ixToUse ].peer, peers.regDesc[ ixToUse ].gateway ) ) {
    fprintf( stderr, "Allocating peer \"%s\" index %d (open address).\n", peers.regDesc[ ixToUse ].id.name, ixToUse );
    fprintf( stderr, "%s : %s\n", IpAddr( peers.regDesc[ ixToUse ].peer, a1 ),
        IpAddr( peers.regDesc[ ixToUse ].gateway, a2 ) );
  }
  else {
    fprintf( stderr, "Allocating peer \"%s\" index %d (behind NAT).\n", peers.regDesc[ ixToUse ].id.name, ixToUse );
    fprintf( stderr, "%s : %s\n", IpAddr( peers.regDesc[ ixToUse ].peer, a1 ),
        IpAddr( peers.regDesc[ ixToUse ].gateway, a2 ) );
  }

got_already:
  lastSeen[ ixToUse ] = now;
  DIE_IF_ERR( sendto( sock, &peers, sizeof( peers ), 0, (struct sockaddr *)&remote, sizeof( remote ) ) );
}

int
main( int argc, char * argv[] )
{
  if( argc != 1 ) {
    usage();
  }

  // we will send this guy over and over...
  peers.what = htons( GwMsgRegDesc );

  // open a socket
  protoent * proto = DIE_IF_NULL( getprotobyname( "udp" ) );
  int cliSock = DIE_IF_ERR( socket( PF_INET, SOCK_DGRAM, proto->p_proto ) );

  // bind it locally
  struct sockaddr_in sinLocal;
  memset( &sinLocal, 0, sizeof( sinLocal ) );
  sinLocal.sin_family = AF_INET;
  sinLocal.sin_port = htons( SERVICE_PORT );
  sinLocal.sin_addr.s_addr = INADDR_ANY;
  // I might not necessarily need to bind, or I could look for any unbound port 
  // starting at some range, but for debugging, this makes things more predictable.
  DIE_IF_ERR( bind( cliSock, (struct sockaddr *)&sinLocal, sizeof( sinLocal ) ) );

  // enter the listen loop
  while( true ) {
    NatGwProtoMsg msg;
    struct sockaddr_in remote;
    socklen_t len = sizeof( remote );
    int r = DIE_IF_ERR( recvfrom( cliSock, &msg, sizeof( msg ), 0, (struct sockaddr *)&remote, &len ) );
    if( r != sizeof( msg ) ) {
      fprintf( stderr, "Received malformed packet; size: %d\n", r );
      continue;
    }
    switch( ntohs( msg.what ) ) {
      case GwMsgSelfDesc:
        UpdateOrAllocatePeerAndReply( cliSock, msg, remote );
        break;
      default:
        fprintf( stderr, "Received unexpected message; what code %d\n", ntohs( msg.what ) );
        break;
    }
  }
  return 0;
}

