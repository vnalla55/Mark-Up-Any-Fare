#ifndef LOC_GENERATOR_H
#define LOC_GENERATOR_H

#include <utility>
#include "DBAccess/Loc.h"
#include "test/include/TestMemHandle.h"

//#include <cppunit/TestFixture.h>
//#include <cppunit/TestSuite.h>
//#include <cppunit/extensions/HelperMacros.h>

namespace tse
{

class LocGenerator
{
public:
  Loc* createLoc(char lathem,
                 LatLong latdeg,
                 LatLong latmin,
                 LatLong latsec,
                 char lnghem,
                 LatLong lngdeg,
                 LatLong lngmin,
                 LatLong lngsec,
                 const LocCode& loc)
  {
    Loc* location = _memHandle.create<Loc>();
    location->lathem() = lathem;
    location->latdeg() = latdeg;
    location->latmin() = latmin;
    location->latsec() = latsec;
    location->lnghem() = lnghem;
    location->lngdeg() = lngdeg;
    location->lngmin() = lngmin;
    location->lngsec() = lngsec;
    location->loc() = loc;
    return location;
  }

  std::pair<Loc*, Loc*> distance118SameHem()
  {
    return std::make_pair(createLoc('N', 70, 0, 0, 'W', 70, 0, 0, "ABC"),
                          createLoc('N', 70, 0, 0, 'W', 75, 0, 0, "DEF"));
  }

  std::pair<Loc*, Loc*> distance2641SameLat()
  {
    return std::make_pair(createLoc('N', 70, 0, 0, 'W', 70, 0, 0, "ABC"),
                          createLoc('N', 70, 0, 0, 'E', 75, 0, 0, "DEF"));
  }

  std::pair<Loc*, Loc*> distance9657SameLng()
  {
    return std::make_pair(createLoc('N', 70, 0, 0, 'W', 70, 0, 0, "ABC"),
                          createLoc('S', 70, 0, 0, 'W', 75, 0, 0, "DEF"));
  }

  std::pair<Loc*, Loc*> distance11613DiffHem()
  {
    return std::make_pair(createLoc('N', 70, 0, 0, 'W', 70, 0, 0, "ABC"),
                          createLoc('S', 70, 0, 0, 'E', 75, 0, 0, "DEF"));
  }

private:
  TestMemHandle _memHandle;
};
}
#endif // LOC_GENERATOR_H
