//----------------------------------------------------------------------------
//   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#ifndef FLT_TRK_CNTRY_GRP_H
#define FLT_TRK_CNTRY_GRP_H

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class FltTrkCntryGrp
{
public:
  FltTrkCntryGrp() : _flttrkApplInd(' ') {}
  bool operator==(const FltTrkCntryGrp& rhs) const
  {
    return ((_carrier == rhs._carrier) && (_effDate == rhs._effDate) &&
            (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
            (_discDate == rhs._discDate) && (_flttrkApplInd == rhs._flttrkApplInd) &&
            (_nations == rhs._nations));
  }

  static void dummyData(FltTrkCntryGrp& obj)
  {
    obj._carrier = "ABC";
    obj._effDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._flttrkApplInd = 'D';
    obj._nations.push_back("EFG");
    obj._nations.push_back("HIJ");
  }

private:
  CarrierCode _carrier;
  DateTime _effDate;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _discDate;
  Indicator _flttrkApplInd;
  std::vector<NationCode> _nations;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _flttrkApplInd);
    FLATTENIZE(archive, _nations);
  }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& flttrkApplInd() { return _flttrkApplInd; }
  const Indicator& flttrkApplInd() const { return _flttrkApplInd; }

  std::vector<NationCode>& nations() { return _nations; }
  const std::vector<NationCode>& nations() const { return _nations; }
};
}

#endif
