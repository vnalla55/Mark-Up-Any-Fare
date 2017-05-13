#pragma once


#include <map>
#include <vector>

namespace tse
{

class CarrierFlightSeg;

class MockTable986
{
public:
  std::vector<CarrierFlightSeg*> segs;

  static unsigned putTable(MockTable986* table);
  static MockTable986* getTable(unsigned number);

  static std::map<unsigned, MockTable986*> tables;
};

} // namespace tse

