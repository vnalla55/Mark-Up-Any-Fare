#ifndef _ATAE_TEST_MOCK_OBJECTS_H_
#define _ATAE_TEST_MOCK_OBJECTS_H_

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"

namespace tse
{
class CarrierSwapUtilMock
{
public:
  CarrierCode getCarrierCode(const CarrierCode& carrier) const
  {
    return carrier;
  }
};

class CurrentTimeMock
{
  DateTime _currentTime;
public:
  DateTime getCurrentTime() const
  {
    return _currentTime;
  }
  void setCurrentTime(const DateTime& currentTime)
  {
    _currentTime = currentTime;
  }
};

}

#endif //_ATAE_TEST_MOCK_OBJECTS_H_
