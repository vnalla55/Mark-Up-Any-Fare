#ifndef __TEST_UTIL_H__
#define __TEST_UTIL_H__

#include "Common/TseCodeTypes.h"

namespace tse
{

class AirSeg;
class Loc;

namespace TestUtil
{
AirSeg*
createAirSeg(int segNumber, const Loc* loc1, const Loc* loc2, const CarrierCode& carrier);
};
}

#endif // __TEST_UTIL_H__
