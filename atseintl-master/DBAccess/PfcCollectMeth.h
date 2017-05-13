//----------------------------------------------------------------------------
//       � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//      ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/PfcCollectExcpt.h"

namespace tse
{

class PfcCollectMeth
{
public:
  PfcCollectMeth() : _collectOption(' ') {}

  ~PfcCollectMeth()
  { // Nuke the kids!
    std::vector<PfcCollectExcpt*>::iterator CEIt;
    for (CEIt = _excpts.begin(); CEIt != _excpts.end(); CEIt++)
    {
      delete *CEIt;
    }
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& collectOption() { return _collectOption; }
  const Indicator& collectOption() const { return _collectOption; }

  std::vector<PfcCollectExcpt*>& excpts() { return _excpts; }
  const std::vector<PfcCollectExcpt*>& excpts() const { return _excpts; }

  bool operator==(const PfcCollectMeth& rhs) const
  {
    bool eq = ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
               (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
               (_createDate == rhs._createDate) && (_discDate == rhs._discDate) &&
               (_collectOption == rhs._collectOption) && (_excpts.size() == rhs._excpts.size()));

    for (size_t i = 0; (eq && (i < _excpts.size())); ++i)
    {
      eq = (*(_excpts[i]) == *(rhs._excpts[i]));
    }

    return eq;
  }

  static void dummyData(PfcCollectMeth& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._collectOption = 'H';

    PfcCollectExcpt* pce1 = new PfcCollectExcpt;
    PfcCollectExcpt* pce2 = new PfcCollectExcpt;

    PfcCollectExcpt::dummyData(*pce1);
    PfcCollectExcpt::dummyData(*pce2);

    obj._excpts.push_back(pce1);
    obj._excpts.push_back(pce2);
  }

protected:
  VendorCode _vendor;
  CarrierCode _carrier;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _createDate;
  DateTime _discDate;
  Indicator _collectOption;
  std::vector<PfcCollectExcpt*> _excpts;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _collectOption);
    FLATTENIZE(archive, _excpts);
  }

private:
  PfcCollectMeth(const PfcCollectMeth&);
  PfcCollectMeth& operator=(const PfcCollectMeth&);
};
}

