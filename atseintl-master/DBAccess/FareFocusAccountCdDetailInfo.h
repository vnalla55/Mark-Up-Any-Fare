#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{


class FareFocusAccountCdDetailInfo
{
 public:
  FareFocusAccountCdDetailInfo()
    : _accountCdType(' ')
  {
  }

  AccountCode& accountCd() { return _accountCd; }
  const AccountCode& accountCd() const { return _accountCd; }

  Indicator& accountCdType() { return _accountCdType; }
  Indicator accountCdType() const { return _accountCdType; }

  bool operator==(const FareFocusAccountCdDetailInfo& rhs) const
  {
    return _accountCd == rhs._accountCd
           && _accountCdType == rhs._accountCdType;
  }

  static void dummyData(FareFocusAccountCdDetailInfo& obj)
  {
    obj._accountCd = "123456789012";
    obj._accountCdType = 'A';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _accountCd);
    FLATTENIZE(archive, _accountCdType);
  }

 private:
  AccountCode _accountCd;
  Indicator _accountCdType;
};

}// tse

