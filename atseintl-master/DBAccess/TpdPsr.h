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
#include "DBAccess/TpdPsrViaCxrLoc.h"
#include "DBAccess/TpdPsrViaExcept.h"
#include "DBAccess/TpdPsrViaGeoLoc.h"

#include <vector>

namespace tse
{

class TpdPsr
{
public:
  TpdPsr() = default;
  TpdPsr(const TpdPsr&) = delete;
  TpdPsr& operator=(const TpdPsr&) = delete;

  ~TpdPsr()
  { // Nuke the Kids
    std::vector<TpdPsrViaCxrLoc*>::iterator CLIt;
    for (CLIt = _viaCxrLocs.begin(); CLIt != _viaCxrLocs.end(); CLIt++)
    {
      delete *CLIt;
    }
    std::vector<TpdPsrViaExcept*>::iterator VEIt;
    for (VEIt = _viaExcepts.begin(); VEIt != _viaExcepts.end(); VEIt++)
    {
      delete *VEIt;
    }
    std::vector<TpdPsrViaGeoLoc*>::iterator GLIt;
    for (GLIt = _viaGeoLocs.begin(); GLIt != _viaGeoLocs.end(); GLIt++)
    {
      delete *GLIt;
    }
  }

  Indicator& applInd() { return _applInd; }
  const Indicator& applInd() const { return _applInd; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  Indicator& area1() { return _area1; }
  const Indicator& area1() const { return _area1; }

  Indicator& area2() { return _area2; }
  const Indicator& area2() const { return _area2; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& effTvlDate() { return _effTvlDate; }
  const DateTime& effTvlDate() const { return _effTvlDate; }

  DateTime& discTvlDate() { return _discTvlDate; }
  const DateTime& discTvlDate() const { return _discTvlDate; }

  int& tpmDeduction() { return _tpmDeduction; }
  const int& tpmDeduction() const { return _tpmDeduction; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  std::string& isiCode() { return _isiCode; }
  const std::string& isiCode() const { return _isiCode; }

  Indicator& fareTypeAppl() { return _fareTypeAppl; }
  const Indicator& fareTypeAppl() const { return _fareTypeAppl; }

  Indicator& psrHip() { return _psrHip; }
  const Indicator& psrHip() const { return _psrHip; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& thisCarrierRestr() { return _thisCarrierRestr; }
  const Indicator& thisCarrierRestr() const { return _thisCarrierRestr; }

  Indicator& thruViaLocRestr() { return _thruViaLocRestr; }
  const Indicator& thruViaLocRestr() const { return _thruViaLocRestr; }

  int& stopoverCnt() { return _stopoverCnt; }
  const int& stopoverCnt() const { return _stopoverCnt; }

  Indicator& thruViaMktSameCxr() { return _thruViaMktSameCxr; }
  const Indicator& thruViaMktSameCxr() const { return _thruViaMktSameCxr; }

  Indicator& thruMktCarrierExcept() { return _thruMktCarrierExcept; }
  const Indicator& thruMktCarrierExcept() const { return _thruMktCarrierExcept; }

  Indicator& tpdThruViaMktOnlyInd() { return _tpdThruViaMktOnlyInd; }
  const Indicator& tpdThruViaMktOnlyInd() const { return _tpdThruViaMktOnlyInd; }

  std::vector<CarrierCode>& thruMktCxrs() { return _thruMktCxrs; }
  const std::vector<CarrierCode>& thruMktCxrs() const { return _thruMktCxrs; }

  std::vector<TpdPsrViaCxrLoc*>& viaCxrLocs() { return _viaCxrLocs; }
  const std::vector<TpdPsrViaCxrLoc*>& viaCxrLocs() const { return _viaCxrLocs; }

  std::vector<TpdPsrViaExcept*>& viaExcepts() { return _viaExcepts; }
  const std::vector<TpdPsrViaExcept*>& viaExcepts() const { return _viaExcepts; }

  std::vector<TpdPsrViaGeoLoc*>& viaGeoLocs() { return _viaGeoLocs; }
  const std::vector<TpdPsrViaGeoLoc*>& viaGeoLocs() const { return _viaGeoLocs; }

  bool operator==(const TpdPsr& rhs) const
  {
    bool eq((_applInd == rhs._applInd) && (_carrier == rhs._carrier) && (_area1 == rhs._area1) &&
            (_area2 == rhs._area2) && (_versionDate == rhs._versionDate) &&
            (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
            (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_effTvlDate == rhs._effTvlDate) &&
            (_discTvlDate == rhs._discTvlDate) && (_tpmDeduction == rhs._tpmDeduction) &&
            (_globalDir == rhs._globalDir) && (_isiCode == rhs._isiCode) &&
            (_fareTypeAppl == rhs._fareTypeAppl) && (_psrHip == rhs._psrHip) &&
            (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
            (_thisCarrierRestr == rhs._thisCarrierRestr) &&
            (_thruViaLocRestr == rhs._thruViaLocRestr) && (_stopoverCnt == rhs._stopoverCnt) &&
            (_thruViaMktSameCxr == rhs._thruViaMktSameCxr) &&
            (_thruMktCarrierExcept == rhs._thruMktCarrierExcept) &&
            (_tpdThruViaMktOnlyInd == rhs._tpdThruViaMktOnlyInd) &&
            (_thruMktCxrs == rhs._thruMktCxrs) && (_viaCxrLocs.size() == rhs._viaCxrLocs.size()) &&
            (_viaExcepts.size() == rhs._viaExcepts.size()) &&
            (_viaGeoLocs.size() == rhs._viaGeoLocs.size()));

    for (size_t i = 0; (eq && (i < _viaCxrLocs.size())); ++i)
    {
      eq = (*(_viaCxrLocs[i]) == *(rhs._viaCxrLocs[i]));
    }

    for (size_t j = 0; (eq && (j < _viaExcepts.size())); ++j)
    {
      eq = (*(_viaExcepts[j]) == *(rhs._viaExcepts[j]));
    }

    for (size_t k = 0; (eq && (k < _viaGeoLocs.size())); ++k)
    {
      eq = (*(_viaGeoLocs[k]) == *(rhs._viaGeoLocs[k]));
    }

    return eq;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(TpdPsr& obj)
  {
    obj._applInd = 'A';
    obj._carrier = "BCD";
    obj._area1 = 'E';
    obj._area2 = 'F';
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._effTvlDate = time(nullptr);
    obj._discTvlDate = time(nullptr);
    obj._tpmDeduction = 2;
    obj._globalDir = GlobalDirection::US;
    obj._isiCode = "bbbbbbbb";
    obj._fareTypeAppl = 'G';
    obj._psrHip = 'H';

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._thisCarrierRestr = 'I';
    obj._thruViaLocRestr = 'J';
    obj._stopoverCnt = 3;
    obj._thruViaMktSameCxr = 'K';
    obj._thruMktCarrierExcept = 'L';
    obj._tpdThruViaMktOnlyInd = 'M';

    obj._thruMktCxrs.push_back("NOP");
    obj._thruMktCxrs.push_back("QRS");

    TpdPsrViaCxrLoc* tpvcl1 = new TpdPsrViaCxrLoc;
    TpdPsrViaCxrLoc* tpvcl2 = new TpdPsrViaCxrLoc;

    TpdPsrViaCxrLoc::dummyData(*tpvcl1);
    TpdPsrViaCxrLoc::dummyData(*tpvcl2);

    obj._viaCxrLocs.push_back(tpvcl1);
    obj._viaCxrLocs.push_back(tpvcl2);

    TpdPsrViaExcept* tpve1 = new TpdPsrViaExcept;
    TpdPsrViaExcept* tpve2 = new TpdPsrViaExcept;

    TpdPsrViaExcept::dummyData(*tpve1);
    TpdPsrViaExcept::dummyData(*tpve2);

    obj._viaExcepts.push_back(tpve1);
    obj._viaExcepts.push_back(tpve2);

    TpdPsrViaGeoLoc* tpvgl1 = new TpdPsrViaGeoLoc;
    TpdPsrViaGeoLoc* tpvgl2 = new TpdPsrViaGeoLoc;

    TpdPsrViaGeoLoc::dummyData(*tpvgl1);
    TpdPsrViaGeoLoc::dummyData(*tpvgl2);

    obj._viaGeoLocs.push_back(tpvgl1);
    obj._viaGeoLocs.push_back(tpvgl2);
  }

private:
  Indicator _applInd = ' ';
  CarrierCode _carrier;
  Indicator _area1 = ' ';
  Indicator _area2 = ' ';
  DateTime _versionDate;
  int _seqNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _effTvlDate;
  DateTime _discTvlDate;
  int _tpmDeduction = 0;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  std::string _isiCode;
  Indicator _fareTypeAppl = ' ';
  Indicator _psrHip = ' ';
  LocKey _loc1;
  LocKey _loc2;
  Indicator _thisCarrierRestr = ' ';
  Indicator _thruViaLocRestr = ' ';
  int _stopoverCnt = 0;
  Indicator _thruViaMktSameCxr = ' ';
  Indicator _thruMktCarrierExcept = ' ';
  Indicator _tpdThruViaMktOnlyInd = ' ';
  std::vector<CarrierCode> _thruMktCxrs;
  std::vector<TpdPsrViaCxrLoc*> _viaCxrLocs;
  std::vector<TpdPsrViaExcept*> _viaExcepts;
  std::vector<TpdPsrViaGeoLoc*> _viaGeoLocs;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _applInd);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _area1);
    FLATTENIZE(archive, _area2);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _effTvlDate);
    FLATTENIZE(archive, _discTvlDate);
    FLATTENIZE(archive, _tpmDeduction);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _isiCode);
    FLATTENIZE(archive, _fareTypeAppl);
    FLATTENIZE(archive, _psrHip);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _thisCarrierRestr);
    FLATTENIZE(archive, _thruViaLocRestr);
    FLATTENIZE(archive, _stopoverCnt);
    FLATTENIZE(archive, _thruViaMktSameCxr);
    FLATTENIZE(archive, _thruMktCarrierExcept);
    FLATTENIZE(archive, _tpdThruViaMktOnlyInd);
    FLATTENIZE(archive, _thruMktCxrs);
    FLATTENIZE(archive, _viaCxrLocs);
    FLATTENIZE(archive, _viaExcepts);
    FLATTENIZE(archive, _viaGeoLocs);
  }

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_applInd & ptr->_carrier & ptr->_area1 & ptr->_area2 & ptr->_versionDate &
           ptr->_seqNo & ptr->_createDate & ptr->_expireDate & ptr->_effDate & ptr->_discDate &
           ptr->_effTvlDate & ptr->_discTvlDate & ptr->_tpmDeduction & ptr->_globalDir &
           ptr->_isiCode & ptr->_fareTypeAppl & ptr->_psrHip & ptr->_loc1 & ptr->_loc2 &
           ptr->_thisCarrierRestr & ptr->_thruViaLocRestr & ptr->_stopoverCnt &
           ptr->_thruViaMktSameCxr & ptr->_thruMktCarrierExcept & ptr->_tpdThruViaMktOnlyInd &
           ptr->_thruMktCxrs & ptr->_viaCxrLocs & ptr->_viaExcepts & ptr->_viaGeoLocs;
  }
};
}
