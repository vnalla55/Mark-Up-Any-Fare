#pragma once

#include "Common/TseBoostStringTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

#include <vector>

namespace tse
{

class MinFareFareTypeGrpSeg
{
public:
  MinFareFareTypeGrpSeg() : _setNo(0), _orderNo(0), _grpSetNo(0), _grpType(0) {}
  FareTypeAbbrev& fareType() { return _fareType; }
  const FareTypeAbbrev& fareType() const { return _fareType; }

  int& setNo() { return _setNo; }
  const int& setNo() const { return _setNo; }

  int& orderNo() { return _orderNo; }
  const int& orderNo() const { return _orderNo; }

  int32_t& grpSetNo() { return _grpSetNo; }
  const int32_t& grpSetNo() const { return _grpSetNo; }

  int32_t& grpType() { return _grpType; }
  const int32_t& grpType() const { return _grpType; }

  bool operator==(const MinFareFareTypeGrpSeg& rhs) const
  {
    return ((_fareType == rhs._fareType) && (_setNo == rhs._setNo) && (_orderNo == rhs._orderNo) &&
            (_grpSetNo == rhs._grpSetNo) && (_grpType == rhs._grpType));
  }

  static void dummyData(MinFareFareTypeGrpSeg& obj)
  {
    obj._fareType = "ABC";
    obj._setNo = 1;
    obj._orderNo = 2;
    obj._grpSetNo = 3;
    obj._grpType = 4;
  }

protected:
  FareTypeAbbrev _fareType;
  int _setNo;
  int _orderNo;
  int32_t _grpSetNo;
  int32_t _grpType;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _setNo);
    FLATTENIZE(archive, _orderNo);
    FLATTENIZE(archive, _grpSetNo);
    FLATTENIZE(archive, _grpType);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_fareType
           & ptr->_setNo
           & ptr->_orderNo
           & ptr->_grpSetNo
           & ptr->_grpType;
  }
};

} // namespace tse

