//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//  ----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/MileageSurchCxr.h"

namespace tse
{
class MileageSurchExcept
{
public:
  MileageSurchExcept() = default;
  MileageSurchExcept(const MileageSurchExcept&) = delete;
  MileageSurchExcept& operator=(const MileageSurchExcept&) = delete;

  ~MileageSurchExcept()
  { // Nuke the kids!
    std::vector<MileageSurchCxr*>::iterator CxrIt;
    for (CxrIt = _cxrs.begin(); CxrIt != _cxrs.end(); CxrIt++)
    {
      delete *CxrIt;
    }
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& textTblItemNo() { return _textTblItemNo; }
  const int& textTblItemNo() const { return _textTblItemNo; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  RuleNumber& rule() { return _rule; }
  const RuleNumber& rule() const { return _rule; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  TariffCode& ruleTariffCode() { return _ruleTariffCode; }
  const TariffCode& ruleTariffCode() const { return _ruleTariffCode; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  Indicator& mpmSurchExcept() { return _mpmSurchExcept; }
  const Indicator& mpmSurchExcept() const { return _mpmSurchExcept; }

  Indicator& mustViaCxrExcept() { return _mustViaCxrExcept; }
  const Indicator& mustViaCxrExcept() const { return _mustViaCxrExcept; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  LocKey& mustViaLoc() { return _mustViaLoc; }
  const LocKey& mustViaLoc() const { return _mustViaLoc; }

  Indicator& mustOnlyViaInd() { return _mustOnlyViaInd; }
  const Indicator& mustOnlyViaInd() const { return _mustOnlyViaInd; }

  Indicator& mustViaAllInd() { return _mustViaAllInd; }
  const Indicator& mustViaAllInd() const { return _mustViaAllInd; }

  Indicator& noStopoverInd() { return _noStopoverInd; }
  const Indicator& noStopoverInd() const { return _noStopoverInd; }

  LocKey& mustNotViaLoc() { return _mustNotViaLoc; }
  const LocKey& mustNotViaLoc() const { return _mustNotViaLoc; }

  Indicator& psgTypeInfant() { return _psgTypeInfant; }
  const Indicator& psgTypeInfant() const { return _psgTypeInfant; }

  Indicator& psgTypeChild() { return _psgTypeChild; }
  const Indicator& psgTypeChild() const { return _psgTypeChild; }

  std::vector<MileageSurchCxr*>& cxrs() { return _cxrs; }
  const std::vector<MileageSurchCxr*>& cxrs() const { return _cxrs; }

  std::vector<PaxTypeCode>& paxTypes() { return _paxTypes; }
  const std::vector<PaxTypeCode>& paxTypes() const { return _paxTypes; }

  bool operator==(const MileageSurchExcept& rhs) const
  {
    bool eq = ((_vendor == rhs._vendor) && (_textTblItemNo == rhs._textTblItemNo) &&
               (_governingCarrier == rhs._governingCarrier) && (_ruleTariff == rhs._ruleTariff) &&
               (_rule == rhs._rule) && (_createDate == rhs._createDate) &&
               (_expireDate == rhs._expireDate) && (_effDate == rhs._effDate) &&
               (_discDate == rhs._discDate) && (_ruleTariffCode == rhs._ruleTariffCode) &&
               (_fareClass == rhs._fareClass) && (_mpmSurchExcept == rhs._mpmSurchExcept) &&
               (_mustViaCxrExcept == rhs._mustViaCxrExcept) && (_globalDir == rhs._globalDir) &&
               (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) &&
               (_loc2 == rhs._loc2) && (_mustViaLoc == rhs._mustViaLoc) &&
               (_mustOnlyViaInd == rhs._mustOnlyViaInd) && (_mustViaAllInd == rhs._mustViaAllInd) &&
               (_noStopoverInd == rhs._noStopoverInd) && (_mustNotViaLoc == rhs._mustNotViaLoc) &&
               (_psgTypeInfant == rhs._psgTypeInfant) && (_psgTypeChild == rhs._psgTypeChild) &&
               (_cxrs.size() == rhs._cxrs.size()) && (_paxTypes.size() == rhs._paxTypes.size()));

    for (size_t i = 0; (eq && (i < _cxrs.size())); ++i)
    {
      eq = (*(_cxrs[i]) == *(rhs._cxrs[i]));
    }

    for (size_t j = 0; (eq && (j < _paxTypes.size())); ++j)
    {
      eq = (_paxTypes[j] == rhs._paxTypes[j]);
    }

    return eq;
  }

  static void dummyData(MileageSurchExcept& obj)
  {
    MileageSurchCxr* mscxr1 = new MileageSurchCxr();
    MileageSurchCxr* mscxr2 = new MileageSurchCxr();

    mscxr1->carrier() = "ABC";
    mscxr1->carrierGroupSet() = "DEFGHI";
    mscxr2->carrier() = "JKL";
    mscxr2->carrierGroupSet() = "MNOPQR";

    obj.cxrs().push_back(mscxr1);
    obj.cxrs().push_back(mscxr2);

    obj.vendor() = "ABCD";
    obj.textTblItemNo() = 1;
    obj.governingCarrier() = "EFG";
    obj.ruleTariff() = 2;
    obj.rule() = "HIJK";
    obj.createDate() = time(nullptr);
    obj.expireDate() = time(nullptr);
    obj.effDate() = time(nullptr);
    obj.discDate() = time(nullptr);
    obj.ruleTariffCode() = "LMNOPQR";
    obj.fareClass() = "STUVWXYZ";
    obj.mpmSurchExcept() = 'a';
    obj.mustViaCxrExcept() = 'b';
    obj.globalDir() = GlobalDirection::ZZ;
    obj.directionality() = BOTH;
    obj.loc1().loc() = "cdefghij";
    obj.loc1().locType() = 'k';
    obj.loc2().loc() = "lmnopqrs";
    obj.loc2().locType() = 't';
    obj.mustViaLoc().loc() = "uvwxyzAA";
    obj.mustViaLoc().locType() = 'B';
    obj.mustOnlyViaInd() = 'C';
    obj.mustViaAllInd() = 'D';
    obj.noStopoverInd() = 'E';
    obj.mustNotViaLoc().loc() = "FGHIJKLM";
    obj.mustNotViaLoc().locType() = 'N';
    obj.psgTypeInfant() = 'O';
    obj.psgTypeChild() = 'P';

    obj.paxTypes().push_back("QRS");
    obj.paxTypes().push_back("TUV");
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
  VendorCode _vendor;
  int _textTblItemNo = 0;
  CarrierCode _governingCarrier;
  TariffNumber _ruleTariff = 0;
  RuleNumber _rule;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  TariffCode _ruleTariffCode;
  FareClassCode _fareClass;
  Indicator _mpmSurchExcept = ' ';
  Indicator _mustViaCxrExcept = ' ';
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  Directionality _directionality = Directionality::TERMINATE;
  LocKey _loc1;
  LocKey _loc2;
  LocKey _mustViaLoc;
  Indicator _mustOnlyViaInd = ' ';
  Indicator _mustViaAllInd = ' ';
  Indicator _noStopoverInd = ' ';
  LocKey _mustNotViaLoc;
  Indicator _psgTypeInfant = ' ';
  Indicator _psgTypeChild = ' ';
  std::vector<MileageSurchCxr*> _cxrs;
  std::vector<PaxTypeCode> _paxTypes;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _governingCarrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _rule);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _ruleTariffCode);
    FLATTENIZE(archive, _fareClass);
    FLATTENIZE(archive, _mpmSurchExcept);
    FLATTENIZE(archive, _mustViaCxrExcept);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _mustViaLoc);
    FLATTENIZE(archive, _mustOnlyViaInd);
    FLATTENIZE(archive, _mustViaAllInd);
    FLATTENIZE(archive, _noStopoverInd);
    FLATTENIZE(archive, _mustNotViaLoc);
    FLATTENIZE(archive, _psgTypeInfant);
    FLATTENIZE(archive, _psgTypeChild);
    FLATTENIZE(archive, _cxrs);
    FLATTENIZE(archive, _paxTypes);
  }

private:

  template <typename B, typename T>
  static B& convert(B& buffer,
                    T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_textTblItemNo
           & ptr->_governingCarrier
           & ptr->_ruleTariff
           & ptr->_rule
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_ruleTariffCode
           & ptr->_fareClass
           & ptr->_mpmSurchExcept
           & ptr->_mustViaCxrExcept
           & ptr->_globalDir
           & ptr->_directionality
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_mustViaLoc
           & ptr->_mustOnlyViaInd
           & ptr->_mustViaAllInd
           & ptr->_noStopoverInd
           & ptr->_mustNotViaLoc
           & ptr->_psgTypeInfant
           & ptr->_psgTypeChild
           & ptr->_cxrs
           & ptr->_paxTypes;
  }
};
}
