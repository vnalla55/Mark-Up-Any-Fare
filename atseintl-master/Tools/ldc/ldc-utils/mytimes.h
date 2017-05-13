#ifndef mytimes__H
#define mytimes__H

#include <stdlib.h>
#include <time.h>

inline time_t toGMT(time_t local)
{
  struct tm localTM;
  localtime_r( &local, &localTM );
  time_t diff = local - timegm( &localTM );
  return local + diff;
}

inline time_t toLocal(time_t gmt)
{
  struct tm gmtTM;
  localtime_r( &gmt, &gmtTM );
  time_t diff = gmt - timegm( &gmtTM );
  return gmt - diff;
}

inline time_t jts2Local(const char * value )
{
  long baseTime = 210866760000000000L;
  long dbtime = atol( value );
  long deltaTime = dbtime - baseTime;
  time_t retval = deltaTime / 1000000;
  return toGMT(retval);
}

inline time_t jts2GMT(const char * value )
{
  long baseTime = 210866760000000000L;
  long dbtime = atol( value );
  long deltaTime = dbtime - baseTime;
  time_t retval = deltaTime / 1000000;
  return retval;
}


#endif
