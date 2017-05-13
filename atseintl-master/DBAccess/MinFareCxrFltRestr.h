#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class MinFareCxrFltRestr
{
public:
  MinFareCxrFltRestr() : _flight1(0), _flight2(0) {}
  virtual ~MinFareCxrFltRestr() {}

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  FlightNumber& flight1() { return _flight1; }
  const FlightNumber& flight1() const { return _flight1; }

  FlightNumber& flight2() { return _flight2; }
  const FlightNumber& flight2() const { return _flight2; }

  bool operator==(const MinFareCxrFltRestr& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_flight1 == rhs._flight1) && (_flight2 == rhs._flight2));
  }

  static void dummyData(MinFareCxrFltRestr& obj)
  {
    obj._carrier = "ABC";
    obj._flight1 = 1234;
    obj._flight2 = 5678;
  }

  virtual WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  CarrierCode _carrier;
  FlightNumber _flight1;
  FlightNumber _flight2;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _flight1);
    FLATTENIZE(archive, _flight2);
  }

private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_carrier
           & ptr->_flight1
           & ptr->_flight2;
  }
};
} // namespace tse

