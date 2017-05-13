//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/PfcCoterminal.h"

namespace tse
{

class PfcMultiAirport
{
public:
  PfcMultiAirport() : _segCnt(0), _inhibit(' ') {}

  ~PfcMultiAirport()
  { // Nuke the kids!
    std::vector<PfcCoterminal*>::iterator CTIt;
    for (CTIt = _coterminals.begin(); CTIt != _coterminals.end(); CTIt++)
    {
      delete *CTIt;
    }
  }

  LocKey& loc() { return _loc; }
  const LocKey& loc() const { return _loc; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<PfcCoterminal*>& coterminals() { return _coterminals; }
  const std::vector<PfcCoterminal*>& coterminals() const { return _coterminals; }

  bool operator==(const PfcMultiAirport& rhs) const
  {
    bool eq =
        ((_loc == rhs._loc) && (_createDate == rhs._createDate) &&
         (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
         (_discDate == rhs._discDate) && (_segCnt == rhs._segCnt) && (_vendor == rhs._vendor) &&
         (_inhibit == rhs._inhibit) && (_coterminals.size() == rhs._coterminals.size()));

    for (size_t i = 0; (eq && (i < _coterminals.size())); ++i)
    {
      eq = (*(_coterminals[i]) == *(rhs._coterminals[i]));
    }

    return eq;
  }

  static void dummyData(PfcMultiAirport& obj)
  {
    LocKey::dummyData(obj._loc);

    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._segCnt = 1;
    obj._vendor = "ABCD";
    obj._inhibit = 'E';

    PfcCoterminal* pc1 = new PfcCoterminal;
    PfcCoterminal* pc2 = new PfcCoterminal;

    PfcCoterminal::dummyData(*pc1);
    PfcCoterminal::dummyData(*pc2);

    obj._coterminals.push_back(pc1);
    obj._coterminals.push_back(pc2);
  }

protected:
  LocKey _loc;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  int _segCnt;
  VendorCode _vendor;
  Indicator _inhibit;
  std::vector<PfcCoterminal*> _coterminals;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _loc);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _coterminals);
  }

protected:
private:
  PfcMultiAirport(const PfcMultiAirport&);
  PfcMultiAirport& operator=(const PfcMultiAirport&);
};
}

