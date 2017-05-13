//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/PfcCxrExcpt.h"

namespace tse
{

class PfcPFC
{
public:
  PfcPFC()
    : _pfcAmt1(0),
      _pfcAirTaxExcp(' '),
      _pfcCharterExcp(' '),
      _freqFlyerInd(' '),
      _segCnt(0),
      _inhibit(' ')
  {
  }

  ~PfcPFC()
  { // Nuke the kids!
    std::vector<PfcCxrExcpt*>::iterator CEIt;
    for (CEIt = _cxrExcpts.begin(); CEIt != _cxrExcpts.end(); CEIt++)
    {
      delete *CEIt;
    }
  }

  LocCode& pfcAirport() { return _pfcAirport; }
  const LocCode& pfcAirport() const { return _pfcAirport; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  MoneyAmount& pfcAmt1() { return _pfcAmt1; }
  const MoneyAmount& pfcAmt1() const { return _pfcAmt1; }

  Indicator& pfcAirTaxExcp() { return _pfcAirTaxExcp; }
  const Indicator& pfcAirTaxExcp() const { return _pfcAirTaxExcp; }

  Indicator& pfcCharterExcp() { return _pfcCharterExcp; }
  const Indicator& pfcCharterExcp() const { return _pfcCharterExcp; }

  Indicator& freqFlyerInd() { return _freqFlyerInd; }
  const Indicator& freqFlyerInd() const { return _freqFlyerInd; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  CurrencyCode& pfcCur1() { return _pfcCur1; }
  const CurrencyCode& pfcCur1() const { return _pfcCur1; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<PfcCxrExcpt*>& cxrExcpts() { return _cxrExcpts; }
  const std::vector<PfcCxrExcpt*>& cxrExcpts() const { return _cxrExcpts; }

  bool operator==(const PfcPFC& rhs) const
  {
    bool eq = ((_pfcAirport == rhs._pfcAirport) && (_effDate == rhs._effDate) &&
               (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
               (_discDate == rhs._discDate) && (_pfcAmt1 == rhs._pfcAmt1) &&
               (_pfcAirTaxExcp == rhs._pfcAirTaxExcp) && (_pfcCharterExcp == rhs._pfcCharterExcp) &&
               (_freqFlyerInd == rhs._freqFlyerInd) && (_segCnt == rhs._segCnt) &&
               (_pfcCur1 == rhs._pfcCur1) && (_vendor == rhs._vendor) &&
               (_inhibit == rhs._inhibit) && (_cxrExcpts.size() == rhs._cxrExcpts.size()));

    for (size_t i = 0; (eq && (i < _cxrExcpts.size())); ++i)
    {
      eq = (*(_cxrExcpts[i]) == *(rhs._cxrExcpts[i]));
    }

    return eq;
  }

  static void dummyData(PfcPFC& obj)
  {
    obj._pfcAirport = "aaaaaaaa";
    obj._effDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._pfcAmt1 = 1.11;
    obj._pfcAirTaxExcp = 'A';
    obj._pfcCharterExcp = 'B';
    obj._freqFlyerInd = 'C';
    obj._segCnt = 2;
    obj._pfcCur1 = "DEF";
    obj._vendor = "GHIJ";
    obj._inhibit = 'K';

    PfcCxrExcpt* pce1 = new PfcCxrExcpt;
    PfcCxrExcpt* pce2 = new PfcCxrExcpt;

    PfcCxrExcpt::dummyData(*pce1);
    PfcCxrExcpt::dummyData(*pce2);

    obj._cxrExcpts.push_back(pce1);
    obj._cxrExcpts.push_back(pce2);
  }

protected:
  LocCode _pfcAirport;
  DateTime _effDate;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _discDate;
  MoneyAmount _pfcAmt1;
  Indicator _pfcAirTaxExcp;
  Indicator _pfcCharterExcp;
  Indicator _freqFlyerInd;
  int _segCnt;
  CurrencyCode _pfcCur1;
  VendorCode _vendor;
  Indicator _inhibit;
  std::vector<PfcCxrExcpt*> _cxrExcpts;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pfcAirport);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _pfcAmt1);
    FLATTENIZE(archive, _pfcAirTaxExcp);
    FLATTENIZE(archive, _pfcCharterExcp);
    FLATTENIZE(archive, _freqFlyerInd);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _pfcCur1);
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _cxrExcpts);
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
private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_pfcAirport
           & ptr->_effDate
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_discDate
           & ptr->_pfcAmt1
           & ptr->_pfcAirTaxExcp
           & ptr->_pfcCharterExcp
           & ptr->_freqFlyerInd
           & ptr->_segCnt
           & ptr->_pfcCur1
           & ptr->_vendor
           & ptr->_inhibit
           & ptr->_cxrExcpts;
  }

  PfcPFC(const PfcPFC&);
  PfcPFC& operator=(const PfcPFC&);
};
}

