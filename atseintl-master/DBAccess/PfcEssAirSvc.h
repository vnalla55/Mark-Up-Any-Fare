//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Flattenizable.h"
#include "DBAccess/PfcEssAirSvcProv.h"

namespace tse
{

class PfcEssAirSvc
{
public:
  PfcEssAirSvc() : _segCnt(0), _inhibit(' ') {}

  ~PfcEssAirSvc()
  { // Nuke the kids!
    std::vector<PfcEssAirSvcProv*>::iterator ASPIt;
    for (ASPIt = _asProvs.begin(); ASPIt != _asProvs.end(); ASPIt++)
    {
      delete *ASPIt;
    }
  }

  LocCode& easHubArpt() { return _easHubArpt; }
  const LocCode& easHubArpt() const { return _easHubArpt; }

  LocCode& easArpt() { return _easArpt; }
  const LocCode& easArpt() const { return _easArpt; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<PfcEssAirSvcProv*>& asProvs() { return _asProvs; }
  const std::vector<PfcEssAirSvcProv*>& asProvs() const { return _asProvs; }

  bool operator==(const PfcEssAirSvc& rhs) const
  {
    bool eq = ((_easHubArpt == rhs._easHubArpt) && (_easArpt == rhs._easArpt) &&
               (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
               (_createDate == rhs._createDate) && (_discDate == rhs._discDate) &&
               (_segCnt == rhs._segCnt) && (_vendor == rhs._vendor) && (_inhibit == rhs._inhibit) &&
               (_asProvs.size() == rhs._asProvs.size()));

    for (size_t i = 0; (eq && (i < _asProvs.size())); ++i)
    {
      eq = (*(_asProvs[i]) == *(rhs._asProvs[i]));
    }

    return eq;
  }

  static void dummyData(PfcEssAirSvc& obj)
  {
    obj._easHubArpt = "aaaaaaaa";
    obj._easArpt = "bbbbbbbb";
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._segCnt = 1;
    obj._vendor = "ABCD";
    obj._inhibit = 'E';

    PfcEssAirSvcProv* peasp1 = new PfcEssAirSvcProv;
    PfcEssAirSvcProv* peasp2 = new PfcEssAirSvcProv;

    PfcEssAirSvcProv::dummyData(*peasp1);
    PfcEssAirSvcProv::dummyData(*peasp2);

    obj._asProvs.push_back(peasp1);
    obj._asProvs.push_back(peasp2);
  }

  WBuffer& write(WBuffer& os) const
  {
    return convert(os, this);
  }

  RBuffer& read(RBuffer& is)
  {
    return convert(is, this);
  }

protected:
  LocCode _easHubArpt;
  LocCode _easArpt;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _createDate;
  DateTime _discDate;
  int _segCnt;
  VendorCode _vendor;
  Indicator _inhibit;
  std::vector<PfcEssAirSvcProv*> _asProvs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _easHubArpt);
    FLATTENIZE(archive, _easArpt);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _asProvs);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_easHubArpt
           & ptr->_easArpt
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_createDate
           & ptr->_discDate
           & ptr->_segCnt
           & ptr->_vendor
           & ptr->_inhibit
           & ptr->_asProvs;
  }

  PfcEssAirSvc(const PfcEssAirSvc&);
  PfcEssAirSvc& operator=(const PfcEssAirSvc&);
};
}

