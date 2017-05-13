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
#include "DBAccess/RoutingMap.h"
#include "DBAccess/RoutingRestriction.h"

#include <vector>

namespace tse
{

class Routing
{
public:
  Routing()
    : _routingTariff(0),
      _linkNo(0),
      _noofheaders(0),
      _noofRestrictions(0),
      _nooftexts(0),
      _validityInd(0),
      _inhibit(0),
      _directionalInd(0),
      _domRtgvalInd(0),
      _commonPointInd(0),
      _jointRoutingOpt(0),
      _entryExitPointInd(0),
      _unticketedPointInd(0)
  {
  }

  ~Routing()
  {
    std::vector<RoutingMap*>::iterator MapIt;
    for (MapIt = _rmaps.begin(); MapIt != _rmaps.end(); MapIt++)
    { // Nuke 'em!
      delete *MapIt;
    }
    std::vector<RoutingRestriction*>::iterator RestIt;
    for (RestIt = _rests.begin(); RestIt != _rests.end(); RestIt++)
    { // Nuke 'em!
      delete *RestIt;
    }
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  TariffNumber& routingTariff() { return _routingTariff; }
  const TariffNumber& routingTariff() const { return _routingTariff; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  std::string& effDateStr() { return _effDateStr; }
  const std::string& effDateStr() const { return _effDateStr; }

  int& linkNo() { return _linkNo; }
  const int& linkNo() const { return _linkNo; }

  int& noofheaders() { return _noofheaders; }
  const int& noofheaders() const { return _noofheaders; }

  int& noofRestrictions() { return _noofRestrictions; }
  const int& noofRestrictions() const { return _noofRestrictions; }

  int& nooftexts() { return _nooftexts; }
  const int& nooftexts() const { return _nooftexts; }

  Indicator& validityInd() { return _validityInd; }
  const Indicator& validityInd() const { return _validityInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  Indicator& directionalInd() { return _directionalInd; }
  const Indicator& directionalInd() const { return _directionalInd; }

  Indicator& domRtgvalInd() { return _domRtgvalInd; }
  const Indicator& domRtgvalInd() const { return _domRtgvalInd; }

  Indicator& commonPointInd() { return _commonPointInd; }
  const Indicator& commonPointInd() const { return _commonPointInd; }

  Indicator& jointRoutingOpt() { return _jointRoutingOpt; }
  const Indicator& jointRoutingOpt() const { return _jointRoutingOpt; }

  Indicator& entryExitPointInd() { return _entryExitPointInd; }
  const Indicator& entryExitPointInd() const { return _entryExitPointInd; }

  Indicator& unticketedPointInd() { return _unticketedPointInd; }
  const Indicator& unticketedPointInd() const { return _unticketedPointInd; }

  std::vector<tse::RoutingRestriction*>& rests() { return _rests; }
  const std::vector<tse::RoutingRestriction*>& rests() const { return _rests; }

  std::vector<tse::RoutingMap*>& rmaps() { return _rmaps; }
  const std::vector<tse::RoutingMap*>& rmaps() const { return _rmaps; }

  bool operator==(const Routing& rhs) const
  {
    bool eq((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_routingTariff == rhs._routingTariff) && (_routing == rhs._routing) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_expireDate == rhs._expireDate) &&
            (_createDateStr == rhs._createDateStr) && (_effDateStr == rhs._effDateStr) &&
            (_linkNo == rhs._linkNo) && (_noofheaders == rhs._noofheaders) &&
            (_noofRestrictions == rhs._noofRestrictions) && (_nooftexts == rhs._nooftexts) &&
            (_batchci == rhs._batchci) && (_validityInd == rhs._validityInd) &&
            (_inhibit == rhs._inhibit) && (_directionalInd == rhs._directionalInd) &&
            (_domRtgvalInd == rhs._domRtgvalInd) && (_commonPointInd == rhs._commonPointInd) &&
            (_jointRoutingOpt == rhs._jointRoutingOpt) &&
            (_entryExitPointInd == rhs._entryExitPointInd) &&
            (_unticketedPointInd == rhs._unticketedPointInd) &&
            (_rmaps.size() == rhs._rmaps.size()) && (_rests.size() == rhs._rests.size()));

    for (size_t i = 0; (eq && (i < _rmaps.size())); ++i)
    {
      eq = (*(_rmaps[i]) == *(rhs._rmaps[i]));
    }

    for (size_t j = 0; (eq && (j < _rests.size())); ++j)
    {
      eq = (*(_rests[j]) == *(rhs._rests[j]));
    }

    return eq;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(Routing& obj)
  {
    obj._vendor = "ABCD";
    obj._carrier = "EFG";
    obj._routingTariff = 1;
    obj._routing = "HIJK";
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._createDateStr = "aaaaaaaa";
    obj._effDateStr = "bbbbbbbb";
    obj._linkNo = 2;
    obj._noofheaders = 3;
    obj._noofRestrictions = 4;
    obj._nooftexts = 5;
    obj._batchci = "cccccccc";
    obj._validityInd = 'L';
    obj._inhibit = 'M';
    obj._directionalInd = 'N';
    obj._domRtgvalInd = 'O';
    obj._commonPointInd = 'P';
    obj._jointRoutingOpt = 'Q';
    obj._entryExitPointInd = '1';
    obj._unticketedPointInd = '1';

    RoutingMap* rm1 = new RoutingMap;
    RoutingMap* rm2 = new RoutingMap;

    RoutingMap::dummyData(*rm1);
    RoutingMap::dummyData(*rm2);

    obj._rmaps.push_back(rm1);
    obj._rmaps.push_back(rm2);

    RoutingRestriction* rr1 = new RoutingRestriction;
    RoutingRestriction* rr2 = new RoutingRestriction;

    RoutingRestriction::dummyData(*rr1);
    RoutingRestriction::dummyData(*rr2);

    obj._rests.push_back(rr1);
    obj._rests.push_back(rr2);
  }

protected:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _routingTariff;
  RoutingNumber _routing;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _expireDate;
  std::string _createDateStr;
  std::string _effDateStr;
  int _linkNo;
  int _noofheaders;
  int _noofRestrictions;
  int _nooftexts;
  std::string _batchci;
  Indicator _validityInd;
  Indicator _inhibit;
  Indicator _directionalInd;
  Indicator _domRtgvalInd;
  Indicator _commonPointInd;
  Indicator _jointRoutingOpt;
  Indicator _entryExitPointInd;
  Indicator _unticketedPointInd;
  std::vector<tse::RoutingMap*> _rmaps; // routing map
  std::vector<tse::RoutingRestriction*> _rests; // routing restriction};

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_vendor & ptr->_carrier & ptr->_routingTariff & ptr->_routing &
           ptr->_createDate & ptr->_effDate & ptr->_discDate & ptr->_expireDate &
           ptr->_createDateStr & ptr->_effDateStr & ptr->_linkNo & ptr->_noofheaders &
           ptr->_noofRestrictions & ptr->_nooftexts & ptr->_batchci & ptr->_validityInd &
           ptr->_inhibit & ptr->_directionalInd & ptr->_domRtgvalInd & ptr->_commonPointInd &
           ptr->_jointRoutingOpt & ptr->_entryExitPointInd & ptr->_unticketedPointInd &
           ptr->_rmaps & ptr->_rests;
  }

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _routingTariff);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDateStr);
    FLATTENIZE(archive, _effDateStr);
    FLATTENIZE(archive, _linkNo);
    FLATTENIZE(archive, _noofheaders);
    FLATTENIZE(archive, _noofRestrictions);
    FLATTENIZE(archive, _nooftexts);
    FLATTENIZE(archive, _batchci);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _directionalInd);
    FLATTENIZE(archive, _domRtgvalInd);
    FLATTENIZE(archive, _commonPointInd);
    FLATTENIZE(archive, _jointRoutingOpt);
    FLATTENIZE(archive, _entryExitPointInd);
    FLATTENIZE(archive, _unticketedPointInd);
    FLATTENIZE(archive, _rmaps);
    FLATTENIZE(archive, _rests);
  }

protected:
private:
  Routing(const Routing&);
  Routing& operator=(const Routing&);
};
}
