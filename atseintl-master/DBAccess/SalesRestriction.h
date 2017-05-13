//----------------------------------------------------------------------------
//   (C) 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/Locale.h"
#include "DBAccess/RuleItemInfo.h"

namespace tse
{

class SalesRestriction : public RuleItemInfo
{
public:
  SalesRestriction()
    : _unavailTag(' '),
      _segCnt(0),
      _countryRest(' '),
      _residentRest(' '),
      _carrierCrsInd(' '),
      _validationInd(' '),
      _carrierSegInd(' '),
      _tvlAgentSaleInd(' '),
      _tvlAgentSelectedInd(' '),
      _fopCashInd(' '),
      _fopCheckInd(' '),
      _fopCreditInd(' '),
      _fopGtrInd(' '),
      _currCntryInd(' '),
      _tktIssMail(' '),
      _tktIssPta(' '),
      _tktIssMech(' '),
      _tktIssSelf(' '),
      _tktIssPtaTkt(' '),
      _tktIssAuto(' '),
      _tktIssSat(' '),
      _tktIssSatOcAto(' '),
      _tktIssElectronic(' '),
      _tktIssSiti(' '),
      _tktIssSoto(' '),
      _tktIssSito(' '),
      _tktIssSoti(' '),
      _familyGrpInd(' '),
      _extendInd(' '),
      _inhibit(' ')
  {
  }

  virtual ~SalesRestriction()
  {
    std::vector<Locale*>::iterator LocIt;
    for (LocIt = _locales.begin(); LocIt != _locales.end(); LocIt++)
    { // Nuke 'em!
      delete *LocIt;
    }
  }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  Indicator& unavailTag() { return _unavailTag; }
  const Indicator& unavailTag() const { return _unavailTag; }

  int& segCnt() { return _segCnt; }
  const int& segCnt() const { return _segCnt; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& earliestResDate() { return _earliestResDate; }
  const DateTime& earliestResDate() const { return _earliestResDate; }

  DateTime& earliestTktDate() { return _earliestTktDate; }
  const DateTime& earliestTktDate() const { return _earliestTktDate; }

  DateTime& latestResDate() { return _latestResDate; }
  const DateTime& latestResDate() const { return _latestResDate; }

  DateTime& latestTktDate() { return _latestTktDate; }
  const DateTime& latestTktDate() const { return _latestTktDate; }

  Indicator& countryRest() { return _countryRest; }
  const Indicator& countryRest() const { return _countryRest; }

  Indicator& residentRest() { return _residentRest; }
  const Indicator& residentRest() const { return _residentRest; }

  Indicator& carrierCrsInd() { return _carrierCrsInd; }
  const Indicator& carrierCrsInd() const { return _carrierCrsInd; }

  CarrierCode& otherCarrier() { return _otherCarrier; }
  const CarrierCode& otherCarrier() const { return _otherCarrier; }

  Indicator& validationInd() { return _validationInd; }
  const Indicator& validationInd() const { return _validationInd; }

  Indicator& carrierSegInd() { return _carrierSegInd; }
  const Indicator& carrierSegInd() const { return _carrierSegInd; }

  Indicator& tvlAgentSaleInd() { return _tvlAgentSaleInd; }
  const Indicator& tvlAgentSaleInd() const { return _tvlAgentSaleInd; }

  Indicator& tvlAgentSelectedInd() { return _tvlAgentSelectedInd; }
  const Indicator& tvlAgentSelectedInd() const { return _tvlAgentSelectedInd; }

  Indicator& fopCashInd() { return _fopCashInd; }
  const Indicator& fopCashInd() const { return _fopCashInd; }

  Indicator& fopCheckInd() { return _fopCheckInd; }
  const Indicator& fopCheckInd() const { return _fopCheckInd; }

  Indicator& fopCreditInd() { return _fopCreditInd; }
  const Indicator& fopCreditInd() const { return _fopCreditInd; }

  Indicator& fopGtrInd() { return _fopGtrInd; }
  const Indicator& fopGtrInd() const { return _fopGtrInd; }

  Indicator& currCntryInd() { return _currCntryInd; }
  const Indicator& currCntryInd() const { return _currCntryInd; }

  CurrencyCode& curr() { return _curr; }
  const CurrencyCode& curr() const { return _curr; }

  Indicator& tktIssMail() { return _tktIssMail; }
  const Indicator& tktIssMail() const { return _tktIssMail; }

  Indicator& tktIssPta() { return _tktIssPta; }
  const Indicator& tktIssPta() const { return _tktIssPta; }

  Indicator& tktIssMech() { return _tktIssMech; }
  const Indicator& tktIssMech() const { return _tktIssMech; }

  Indicator& tktIssSelf() { return _tktIssSelf; }
  const Indicator& tktIssSelf() const { return _tktIssSelf; }

  Indicator& tktIssPtaTkt() { return _tktIssPtaTkt; }
  const Indicator& tktIssPtaTkt() const { return _tktIssPtaTkt; }

  Indicator& tktIssAuto() { return _tktIssAuto; }
  const Indicator& tktIssAuto() const { return _tktIssAuto; }

  Indicator& tktIssSat() { return _tktIssSat; }
  const Indicator& tktIssSat() const { return _tktIssSat; }

  Indicator& tktIssSatOcAto() { return _tktIssSatOcAto; }
  const Indicator& tktIssSatOcAto() const { return _tktIssSatOcAto; }

  Indicator& tktIssElectronic() { return _tktIssElectronic; }
  const Indicator& tktIssElectronic() const { return _tktIssElectronic; }

  Indicator& tktIssSiti() { return _tktIssSiti; }
  const Indicator& tktIssSiti() const { return _tktIssSiti; }

  Indicator& tktIssSoto() { return _tktIssSoto; }
  const Indicator& tktIssSoto() const { return _tktIssSoto; }

  Indicator& tktIssSito() { return _tktIssSito; }
  const Indicator& tktIssSito() const { return _tktIssSito; }

  Indicator& tktIssSoti() { return _tktIssSoti; }
  const Indicator& tktIssSoti() const { return _tktIssSoti; }

  Indicator& familyGrpInd() { return _familyGrpInd; }
  const Indicator& familyGrpInd() const { return _familyGrpInd; }

  Indicator& extendInd() { return _extendInd; }
  const Indicator& extendInd() const { return _extendInd; }

  Indicator& inhibit() { return _inhibit; }
  const Indicator& inhibit() const { return _inhibit; }

  std::vector<Locale*>& locales() { return _locales; }
  const std::vector<Locale*>& locales() const { return _locales; }

  virtual bool operator==(const SalesRestriction& rhs) const
  {
    bool eq((RuleItemInfo::operator==(rhs)) && (_createDate == rhs._createDate) &&
            (_unavailTag == rhs._unavailTag) && (_segCnt == rhs._segCnt) &&
            (_expireDate == rhs._expireDate) && (_earliestResDate == rhs._earliestResDate) &&
            (_earliestTktDate == rhs._earliestTktDate) && (_latestResDate == rhs._latestResDate) &&
            (_latestTktDate == rhs._latestTktDate) && (_countryRest == rhs._countryRest) &&
            (_residentRest == rhs._residentRest) && (_carrierCrsInd == rhs._carrierCrsInd) &&
            (_otherCarrier == rhs._otherCarrier) && (_validationInd == rhs._validationInd) &&
            (_carrierSegInd == rhs._carrierSegInd) && (_tvlAgentSaleInd == rhs._tvlAgentSaleInd) &&
            (_tvlAgentSelectedInd == rhs._tvlAgentSelectedInd) &&
            (_fopCashInd == rhs._fopCashInd) && (_fopCheckInd == rhs._fopCheckInd) &&
            (_fopCreditInd == rhs._fopCreditInd) && (_fopGtrInd == rhs._fopGtrInd) &&
            (_currCntryInd == rhs._currCntryInd) && (_curr == rhs._curr) &&
            (_tktIssMail == rhs._tktIssMail) && (_tktIssPta == rhs._tktIssPta) &&
            (_tktIssMech == rhs._tktIssMech) && (_tktIssSelf == rhs._tktIssSelf) &&
            (_tktIssPtaTkt == rhs._tktIssPtaTkt) && (_tktIssAuto == rhs._tktIssAuto) &&
            (_tktIssSat == rhs._tktIssSat) && (_tktIssSatOcAto == rhs._tktIssSatOcAto) &&
            (_tktIssElectronic == rhs._tktIssElectronic) && (_tktIssSiti == rhs._tktIssSiti) &&
            (_tktIssSoto == rhs._tktIssSoto) && (_tktIssSito == rhs._tktIssSito) &&
            (_tktIssSoti == rhs._tktIssSoti) && (_familyGrpInd == rhs._familyGrpInd) &&
            (_extendInd == rhs._extendInd) && (_inhibit == rhs._inhibit) &&
            (_locales.size() == rhs._locales.size()));

    for (size_t i = 0; (eq && (i < _locales.size())); ++i)
    {
      eq = (*(_locales[i]) == *(rhs._locales[i]));
    }

    return eq;
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(SalesRestriction& obj)
  {
    RuleItemInfo::dummyData(obj);

    obj._createDate = time(nullptr);
    obj._unavailTag = 'E';
    obj._segCnt = 2;
    obj._expireDate = time(nullptr);
    obj._earliestResDate = time(nullptr);
    obj._earliestTktDate = time(nullptr);
    obj._latestResDate = time(nullptr);
    obj._latestTktDate = time(nullptr);
    obj._countryRest = 'F';
    obj._residentRest = 'G';
    obj._carrierCrsInd = 'H';
    obj._otherCarrier = "IJK";
    obj._validationInd = 'L';
    obj._carrierSegInd = 'M';
    obj._tvlAgentSaleInd = 'N';
    obj._tvlAgentSelectedInd = 'O';
    obj._fopCashInd = 'P';
    obj._fopCheckInd = 'Q';
    obj._fopCreditInd = 'R';
    obj._fopGtrInd = 'S';
    obj._currCntryInd = 'T';
    obj._curr = "UVW";
    obj._tktIssMail = 'X';
    obj._tktIssPta = 'Y';
    obj._tktIssMech = 'Z';
    obj._tktIssSelf = 'a';
    obj._tktIssPtaTkt = 'b';
    obj._tktIssAuto = 'c';
    obj._tktIssSat = 'd';
    obj._tktIssSatOcAto = 'e';
    obj._tktIssElectronic = 'f';
    obj._tktIssSiti = 'g';
    obj._tktIssSoto = 'h';
    obj._tktIssSito = 'i';
    obj._tktIssSoti = 'j';
    obj._familyGrpInd = 'k';
    obj._extendInd = 'l';
    obj._inhibit = 'm';

    Locale* l1 = new Locale;
    Locale* l2 = new Locale;

    Locale::dummyData(*l1);
    Locale::dummyData(*l2);

    obj._locales.push_back(l1);
    obj._locales.push_back(l2);
  }

protected:
  DateTime _createDate;
  Indicator _unavailTag;
  int _segCnt;
  DateTime _expireDate;
  DateTime _earliestResDate;
  DateTime _earliestTktDate;
  DateTime _latestResDate;
  DateTime _latestTktDate;
  Indicator _countryRest;
  Indicator _residentRest;
  Indicator _carrierCrsInd;
  CarrierCode _otherCarrier;
  Indicator _validationInd;
  Indicator _carrierSegInd;
  Indicator _tvlAgentSaleInd;
  Indicator _tvlAgentSelectedInd;
  Indicator _fopCashInd;
  Indicator _fopCheckInd;
  Indicator _fopCreditInd;
  Indicator _fopGtrInd;
  Indicator _currCntryInd;
  CurrencyCode _curr;
  Indicator _tktIssMail;
  Indicator _tktIssPta;
  Indicator _tktIssMech;
  Indicator _tktIssSelf;
  Indicator _tktIssPtaTkt;
  Indicator _tktIssAuto;
  Indicator _tktIssSat;
  Indicator _tktIssSatOcAto;
  Indicator _tktIssElectronic;
  Indicator _tktIssSiti;
  Indicator _tktIssSoto;
  Indicator _tktIssSito;
  Indicator _tktIssSoti;
  Indicator _familyGrpInd;
  Indicator _extendInd;
  Indicator _inhibit;
  std::vector<Locale*> _locales;

public:
  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, RuleItemInfo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _unavailTag);
    FLATTENIZE(archive, _segCnt);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _earliestResDate);
    FLATTENIZE(archive, _earliestTktDate);
    FLATTENIZE(archive, _latestResDate);
    FLATTENIZE(archive, _latestTktDate);
    FLATTENIZE(archive, _countryRest);
    FLATTENIZE(archive, _residentRest);
    FLATTENIZE(archive, _carrierCrsInd);
    FLATTENIZE(archive, _otherCarrier);
    FLATTENIZE(archive, _validationInd);
    FLATTENIZE(archive, _carrierSegInd);
    FLATTENIZE(archive, _tvlAgentSaleInd);
    FLATTENIZE(archive, _tvlAgentSelectedInd);
    FLATTENIZE(archive, _fopCashInd);
    FLATTENIZE(archive, _fopCheckInd);
    FLATTENIZE(archive, _fopCreditInd);
    FLATTENIZE(archive, _fopGtrInd);
    FLATTENIZE(archive, _currCntryInd);
    FLATTENIZE(archive, _curr);
    FLATTENIZE(archive, _tktIssMail);
    FLATTENIZE(archive, _tktIssPta);
    FLATTENIZE(archive, _tktIssMech);
    FLATTENIZE(archive, _tktIssSelf);
    FLATTENIZE(archive, _tktIssPtaTkt);
    FLATTENIZE(archive, _tktIssAuto);
    FLATTENIZE(archive, _tktIssSat);
    FLATTENIZE(archive, _tktIssSatOcAto);
    FLATTENIZE(archive, _tktIssElectronic);
    FLATTENIZE(archive, _tktIssSiti);
    FLATTENIZE(archive, _tktIssSoto);
    FLATTENIZE(archive, _tktIssSito);
    FLATTENIZE(archive, _tktIssSoti);
    FLATTENIZE(archive, _familyGrpInd);
    FLATTENIZE(archive, _extendInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _locales);
  }

protected:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    RuleItemInfo::convert(buffer, ptr);
    return buffer & ptr->_createDate & ptr->_unavailTag & ptr->_segCnt & ptr->_expireDate &
           ptr->_earliestResDate & ptr->_earliestTktDate & ptr->_latestResDate &
           ptr->_latestTktDate & ptr->_countryRest & ptr->_residentRest & ptr->_carrierCrsInd &
           ptr->_otherCarrier & ptr->_validationInd & ptr->_carrierSegInd & ptr->_tvlAgentSaleInd &
           ptr->_tvlAgentSelectedInd & ptr->_fopCashInd & ptr->_fopCheckInd & ptr->_fopCreditInd &
           ptr->_fopGtrInd & ptr->_currCntryInd & ptr->_curr & ptr->_tktIssMail & ptr->_tktIssPta &
           ptr->_tktIssMech & ptr->_tktIssSelf & ptr->_tktIssPtaTkt & ptr->_tktIssAuto &
           ptr->_tktIssSat & ptr->_tktIssSatOcAto & ptr->_tktIssElectronic & ptr->_tktIssSiti &
           ptr->_tktIssSoto & ptr->_tktIssSito & ptr->_tktIssSoti & ptr->_familyGrpInd &
           ptr->_extendInd & ptr->_inhibit & ptr->_locales;
  }

private:
  SalesRestriction(const SalesRestriction&);
  SalesRestriction& operator=(const SalesRestriction&);
};
}
