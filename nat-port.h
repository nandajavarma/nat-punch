
#if !defined( nat_port_h )
#define nat_port_h

#if defined( WIN32 )

 #include <Winsock2.h>
 #include <Ws2tcpip.h>
 #include <assert.h>
 #define PF_INET AF_INET

inline struct protoent * port_getprotobyname( char const * p )
{
  static protoent ret;
  assert( !strcmp( p, "udp" ) );
  ret.p_aliases = 0;
  ret.p_name = "udp";
  ret.p_proto = IPPROTO_UDP;
  return &ret;
} 

#else

 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <sys/select.h>
 #include <sys/time.h>

 #define SOCKET int
 #define port_getprotobyname getprotobyname

#endif

#endif  //  nat_port_h

