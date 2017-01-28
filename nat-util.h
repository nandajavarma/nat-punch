
#if !defined( nat_util_h )
#define nat_util_h

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define DIE_IF_NULL(x) \
  die_if_null( x, #x, __FILE__, __LINE__ )
#define DIE_IF_ERR(x) \
  die_if_y<int>( x, -1, #x, __FILE__, __LINE__ )
#define DIE_IF_ZERO(x) \
  die_if_y( x, 0, #x, __FILE__, __LINE__ )

template< class T > T die_if_y( T x, T y, char const * str, char const * file, int line )
{
  if( x == y ) {
#if defined( WIN32 )
    fprintf( stderr, "WSAGetLastError(): %d\n", WSAGetLastError() );
#endif
    perror( "perror(): " );
    fprintf( stderr, "%s:%d: %s is bad!\n", file, line, str );
    abort();
  }
  return x;
}

template< class T > T die_if_null( T x, char const * str, char const * file, int line )
{
  if( x == NULL ) {
    perror( "perror(): " );
    fprintf( stderr, "%s:%d: %s is NULL!\n", file, line, str );
    abort();
  }
  return x;
}


#endif  //  nat_util_h

