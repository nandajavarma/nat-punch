#if !defined( nat_reg_h )
#define nat_reg_h

#define MAX_PEERS 10
#define PEER_ID_SIZE 20

// grace.speakeasy.net 216,254,0,22
// www.mindcontrol.org 69,17,45,136
#define SERVER_IP 139,162,23,202
#define SERVICE_PORT 11147
#define CLIENT_PORT 11148

struct PeerId {
  char name[ PEER_ID_SIZE ];
};

struct IpAndPort { // in network byte order
  unsigned char ip[ 4 ];
  unsigned char port[ 2 ];
};
void FromSockAddr( struct sockaddr_in const & sin, IpAndPort * iap );
void ToSockAddr( IpAndPort const & iap, struct sockaddr_in * sin );
bool Equal( IpAndPort const & a, IpAndPort const & b );
char const * IpAddr( IpAndPort const & a, char * buf ); // buf must be at least 32 chars

struct NatPeerSelfDesc {
  PeerId id;
  IpAndPort peer;
};

struct NatPeerRegDesc {
  PeerId id;
  IpAndPort peer;
  IpAndPort gateway;
};

struct NatPeerMsg {
  PeerId id;
};

enum NatGwProtoMsgWhat {
  GwMsgNull,
  GwMsgSelfDesc,
  GwMsgRegDesc,
  GwMsgPeerMsg,
};

// The protocol is extremely simplistic, and not robust enough
// for production use (lots of metadata missing, for one).
struct NatGwProtoMsg {
  short what;
  union {
    NatPeerSelfDesc selfDesc;
    NatPeerRegDesc regDesc[ MAX_PEERS ];
    NatPeerMsg peerMsg;
  };
};


#endif  //  nat_reg_h
