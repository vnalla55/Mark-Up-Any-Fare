#pragma once
//----------------------------------------------------------------------------
// MinFareRuleLevelExcl.h
//
// Copyright Sabre 2004
//
// The copyright to the computer program(s) herein
// is the property of Sabre.
// The program(s) may be used and/or copied only with
// the written permission of Sabre or in accordance
// with the terms and conditions stipulated in the
// agreement/contract under which the program(s)
// have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class MinFareRuleLevelExcl
{
public: // Primary Key Fields
  virtual ~MinFareRuleLevelExcl() = default;

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  int& textTblItemNo() { return _textTblItemNo; }
  const int& textTblItemNo() const { return _textTblItemNo; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  TariffNumber& ruleTariff() { return _ruleTariff; }
  const TariffNumber& ruleTariff() const { return _ruleTariff; }

  DateTime& versionDate() { return _versionDate; }
  const DateTime& versionDate() const { return _versionDate; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  // Non-Key Data
  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  TariffCategory& tariffCat() { return _tariffCat; }
  const TariffCategory& tariffCat() const { return _tariffCat; }

  TariffNumber& routingTariff1() { return _routingTariff1; }
  const TariffNumber& routingTariff1() const { return _routingTariff1; }

  TariffNumber& routingTariff2() { return _routingTariff2; }
  const TariffNumber& routingTariff2() const { return _routingTariff2; }

  TariffCode& ruleTariffCode() { return _ruleTariffCode; }
  const TariffCode& ruleTariffCode() const { return _ruleTariffCode; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Directionality& directionality() { return _directionality; }
  const Directionality& directionality() const { return _directionality; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  GlobalDirection& globalDir() { return _globalDir; }
  const GlobalDirection& globalDir() const { return _globalDir; }

  Indicator& mpmInd() { return _mpmInd; }
  const Indicator& mpmInd() const { return _mpmInd; }

  Indicator& routingInd() { return _routingInd; }
  const Indicator& routingInd() const { return _routingInd; }

  RoutingNumber& routing() { return _routing; }
  const RoutingNumber& routing() const { return _routing; }

  Indicator& hipMinFareAppl() { return _hipMinFareAppl; }
  const Indicator& hipMinFareAppl() const { return _hipMinFareAppl; }

  Indicator& hipFareCompAppl() { return _hipFareCompAppl; }
  const Indicator& hipFareCompAppl() const { return _hipFareCompAppl; }

  Indicator& hipSameGroupAppl() { return _hipSameGroupAppl; }
  const Indicator& hipSameGroupAppl() const { return _hipSameGroupAppl; }

  Indicator& ctmMinFareAppl() { return _ctmMinFareAppl; }
  const Indicator& ctmMinFareAppl() const { return _ctmMinFareAppl; }

  Indicator& ctmFareCompAppl() { return _ctmFareCompAppl; }
  const Indicator& ctmFareCompAppl() const { return _ctmFareCompAppl; }

  Indicator& ctmSameGroupAppl() { return _ctmSameGroupAppl; }
  const Indicator& ctmSameGroupAppl() const { return _ctmSameGroupAppl; }

  Indicator& backhaulMinFareAppl() { return _backhaulMinFareAppl; }
  const Indicator& backhaulMinFareAppl() const { return _backhaulMinFareAppl; }

  Indicator& backhaulFareCompAppl() { return _backhaulFareCompAppl; }
  const Indicator& backhaulFareCompAppl() const { return _backhaulFareCompAppl; }

  Indicator& backhaulSameGroupAppl() { return _backhaulSameGroupAppl; }
  const Indicator& backhaulSameGroupAppl() const { return _backhaulSameGroupAppl; }

  Indicator& dmcMinFareAppl() { return _dmcMinFareAppl; }
  const Indicator& dmcMinFareAppl() const { return _dmcMinFareAppl; }

  Indicator& dmcFareCompAppl() { return _dmcFareCompAppl; }
  const Indicator& dmcFareCompAppl() const { return _dmcFareCompAppl; }

  Indicator& dmcSameGroupAppl() { return _dmcSameGroupAppl; }
  const Indicator& dmcSameGroupAppl() const { return _dmcSameGroupAppl; }

  Indicator& comMinFareAppl() { return _comMinFareAppl; }
  const Indicator& comMinFareAppl() const { return _comMinFareAppl; }

  Indicator& comFareCompAppl() { return _comFareCompAppl; }
  const Indicator& comFareCompAppl() const { return _comFareCompAppl; }

  Indicator& comSameGroupAppl() { return _comSameGroupAppl; }
  const Indicator& comSameGroupAppl() const { return _comSameGroupAppl; }

  Indicator& copMinFareAppl() { return _copMinFareAppl; }
  const Indicator& copMinFareAppl() const { return _copMinFareAppl; }

  Indicator& copFareCompAppl() { return _copFareCompAppl; }
  const Indicator& copFareCompAppl() const { return _copFareCompAppl; }

  Indicator& copSameGroupAppl() { return _copSameGroupAppl; }
  const Indicator& copSameGroupAppl() const { return _copSameGroupAppl; }

  Indicator& cpmMinFareAppl() { return _cpmMinFareAppl; }
  const Indicator& cpmMinFareAppl() const { return _cpmMinFareAppl; }

  Indicator& cpmFareCompAppl() { return _cpmFareCompAppl; }
  const Indicator& cpmFareCompAppl() const { return _cpmFareCompAppl; }

  Indicator& cpmSameGroupAppl() { return _cpmSameGroupAppl; }
  const Indicator& cpmSameGroupAppl() const { return _cpmSameGroupAppl; }

  Indicator& oscFareCompAppl() { return _oscFareCompAppl; }
  const Indicator& oscFareCompAppl() const { return _oscFareCompAppl; }

  Indicator& oscSameGroupAppl() { return _oscSameGroupAppl; }
  const Indicator& oscSameGroupAppl() const { return _oscSameGroupAppl; }

  Indicator& rscFareCompAppl() { return _rscFareCompAppl; }
  const Indicator& rscFareCompAppl() const { return _rscFareCompAppl; }

  Indicator& rscSameGroupAppl() { return _rscSameGroupAppl; }
  const Indicator& rscSameGroupAppl() const { return _rscSameGroupAppl; }

  std::vector<RuleNumber>& rules() { return _rules; }
  const std::vector<RuleNumber>& rules() const { return _rules; }

  std::vector<std::pair<RuleNumber, Footnote>>& ruleFootnotes() { return _ruleFootnotes; }
  const std::vector<std::pair<RuleNumber, Footnote>>& ruleFootnotes() const
  {
    return _ruleFootnotes;
  }

  std::vector<FareClassCode>& fareClasses() { return _fareClasses; }
  const std::vector<FareClassCode>& fareClasses() const { return _fareClasses; }

  std::vector<FareTypeAbbrev>& fareTypes() { return _fareTypes; }
  const std::vector<FareTypeAbbrev>& fareTypes() const { return _fareTypes; }

  std::vector<SetNumber>& fareSet() { return _fareSet; }
  const std::vector<SetNumber>& fareSet() const { return _fareSet; }

  std::vector<TariffNumber>& sameFareGroupTariff() { return _sameFareGroupTariff; }
  const std::vector<TariffNumber>& sameFareGroupTariff() const { return _sameFareGroupTariff; }

  std::vector<RuleNumber>& sameFareGroupRules() { return _sameFareGroupRules; }
  const std::vector<RuleNumber>& sameFareGroupRules() const { return _sameFareGroupRules; }

  std::vector<FareClassCode>& sameFareGroupFareClasses() { return _sameFareGroupFareClasses; }
  const std::vector<FareClassCode>& sameFareGroupFareClasses() const
  {
    return _sameFareGroupFareClasses;
  }

  std::vector<FareTypeAbbrev>& sameFareGroupFareTypes() { return _sameFareGroupFareTypes; }
  const std::vector<FareTypeAbbrev>& sameFareGroupFareTypes() const
  {
    return _sameFareGroupFareTypes;
  }

  bool operator==(const MinFareRuleLevelExcl& rhs) const
  {
    return (
        (_vendor == rhs._vendor) && (_textTblItemNo == rhs._textTblItemNo) &&
        (_governingCarrier == rhs._governingCarrier) && (_ruleTariff == rhs._ruleTariff) &&
        (_versionDate == rhs._versionDate) && (_seqNo == rhs._seqNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
        (_tariffCat == rhs._tariffCat) && (_routingTariff1 == rhs._routingTariff1) &&
        (_routingTariff2 == rhs._routingTariff2) && (_ruleTariffCode == rhs._ruleTariffCode) &&
        (_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
        (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) &&
        (_globalDir == rhs._globalDir) && (_mpmInd == rhs._mpmInd) &&
        (_routingInd == rhs._routingInd) && (_routing == rhs._routing) &&
        (_hipMinFareAppl == rhs._hipMinFareAppl) && (_hipFareCompAppl == rhs._hipFareCompAppl) &&
        (_hipSameGroupAppl == rhs._hipSameGroupAppl) && (_ctmMinFareAppl == rhs._ctmMinFareAppl) &&
        (_ctmFareCompAppl == rhs._ctmFareCompAppl) &&
        (_ctmSameGroupAppl == rhs._ctmSameGroupAppl) &&
        (_backhaulMinFareAppl == rhs._backhaulMinFareAppl) &&
        (_backhaulFareCompAppl == rhs._backhaulFareCompAppl) &&
        (_backhaulSameGroupAppl == rhs._backhaulSameGroupAppl) &&
        (_dmcMinFareAppl == rhs._dmcMinFareAppl) && (_dmcFareCompAppl == rhs._dmcFareCompAppl) &&
        (_dmcSameGroupAppl == rhs._dmcSameGroupAppl) && (_comMinFareAppl == rhs._comMinFareAppl) &&
        (_comFareCompAppl == rhs._comFareCompAppl) &&
        (_comSameGroupAppl == rhs._comSameGroupAppl) && (_copMinFareAppl == rhs._copMinFareAppl) &&
        (_copFareCompAppl == rhs._copFareCompAppl) &&
        (_copSameGroupAppl == rhs._copSameGroupAppl) && (_cpmMinFareAppl == rhs._cpmMinFareAppl) &&
        (_cpmFareCompAppl == rhs._cpmFareCompAppl) &&
        (_cpmSameGroupAppl == rhs._cpmSameGroupAppl) &&
        (_oscFareCompAppl == rhs._oscFareCompAppl) &&
        (_oscSameGroupAppl == rhs._oscSameGroupAppl) &&
        (_rscFareCompAppl == rhs._rscFareCompAppl) &&
        (_rscSameGroupAppl == rhs._rscSameGroupAppl) && (_rules == rhs._rules) &&
        (_ruleFootnotes == rhs._ruleFootnotes) &&
        (_fareClasses == rhs._fareClasses) && (_fareTypes == rhs._fareTypes) &&
        (_fareSet == rhs._fareSet) && (_sameFareGroupTariff == rhs._sameFareGroupTariff) &&
        (_sameFareGroupRules == rhs._sameFareGroupRules) &&
        (_sameFareGroupFareClasses == rhs._sameFareGroupFareClasses) &&
        (_sameFareGroupFareTypes == rhs._sameFareGroupFareTypes));
  }

  static void dummyData(MinFareRuleLevelExcl& obj)
  {
    obj._vendor = "ABCD";
    obj._textTblItemNo = 1;
    obj._governingCarrier = "EFG";
    obj._ruleTariff = 2;
    obj._versionDate = time(nullptr);
    obj._seqNo = 3;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._tariffCat = 4;
    obj._routingTariff1 = 5;
    obj._routingTariff2 = 6;
    obj._ruleTariffCode = "HIJKLMN";
    obj._userApplType = 'O';
    obj._userAppl = "PQRS";
    obj._directionality = BOTH;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._globalDir = GlobalDirection::US;
    obj._mpmInd = 'T';
    obj._routingInd = 'U';
    obj._routing = "VWXY";
    obj._hipMinFareAppl = 'Z';
    obj._hipFareCompAppl = 'a';
    obj._hipSameGroupAppl = 'A';
    obj._ctmMinFareAppl = 'b';
    obj._ctmFareCompAppl = 'c';
    obj._ctmSameGroupAppl = 'C';
    obj._backhaulMinFareAppl = 'd';
    obj._backhaulFareCompAppl = 'e';
    obj._backhaulSameGroupAppl = 'E';
    obj._dmcMinFareAppl = 'f';
    obj._dmcFareCompAppl = 'g';
    obj._dmcSameGroupAppl = 'G';
    obj._comMinFareAppl = 'h';
    obj._comFareCompAppl = 'i';
    obj._comSameGroupAppl = 'I';
    obj._copMinFareAppl = 'j';
    obj._copFareCompAppl = 'k';
    obj._copSameGroupAppl = 'K';
    obj._cpmMinFareAppl = 'l';
    obj._cpmFareCompAppl = 'm';
    obj._cpmSameGroupAppl = 'M';
    obj._oscFareCompAppl = 'n';
    obj._oscSameGroupAppl = 'N';
    obj._rscFareCompAppl = 'o';
    obj._rscSameGroupAppl = 'O';

    obj._rules.push_back("pqrs");
    obj._rules.push_back("tuvw");

    obj._fareClasses.push_back("xyz12345");
    obj._fareClasses.push_back("67890ABC");

    obj._fareTypes.push_back("DEF");
    obj._fareTypes.push_back("GHI");
    obj._fareSet.push_back(7);
    obj._sameFareGroupTariff.push_back(8);
    obj._sameFareGroupRules.push_back("JKLM");
    obj._sameFareGroupFareClasses.push_back("NOPQ");
    obj._sameFareGroupFareTypes.push_back("RSTU");
  }

  virtual WBuffer& write(WBuffer& os) const { return convert(os, this); }

  virtual RBuffer& read(RBuffer& is) { return convert(is, this); }

private:
  // Primary Key Fields
  VendorCode _vendor;
  int _textTblItemNo = 0;
  CarrierCode _governingCarrier;
  TariffNumber _ruleTariff = 0;
  DateTime _versionDate;
  int _seqNo = 0;
  DateTime _createDate;

  // Non-Key Data
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  TariffCategory _tariffCat = 0;
  TariffNumber _routingTariff1 = 0;
  TariffNumber _routingTariff2 = 0;
  TariffCode _ruleTariffCode;
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  Directionality _directionality = Directionality::TERMINATE;
  LocKey _loc1;
  LocKey _loc2;
  GlobalDirection _globalDir = GlobalDirection::NO_DIR;
  Indicator _mpmInd = ' ';
  Indicator _routingInd = ' ';
  RoutingNumber _routing;
  Indicator _hipMinFareAppl = ' ';
  Indicator _hipFareCompAppl = ' ';
  Indicator _hipSameGroupAppl = ' ';
  Indicator _ctmMinFareAppl = ' ';
  Indicator _ctmFareCompAppl = ' ';
  Indicator _ctmSameGroupAppl = ' ';
  Indicator _backhaulMinFareAppl = ' ';
  Indicator _backhaulFareCompAppl = ' ';
  Indicator _backhaulSameGroupAppl = ' ';
  Indicator _dmcMinFareAppl = ' ';
  Indicator _dmcFareCompAppl = ' ';
  Indicator _dmcSameGroupAppl = ' ';
  Indicator _comMinFareAppl = ' ';
  Indicator _comFareCompAppl = ' ';
  Indicator _comSameGroupAppl = ' ';
  Indicator _copMinFareAppl = ' ';
  Indicator _copFareCompAppl = ' ';
  Indicator _copSameGroupAppl = ' ';
  Indicator _cpmMinFareAppl = ' ';
  Indicator _cpmFareCompAppl = ' ';
  Indicator _cpmSameGroupAppl = ' ';
  Indicator _oscFareCompAppl = ' ';
  Indicator _oscSameGroupAppl = ' ';
  Indicator _rscFareCompAppl = ' ';
  Indicator _rscSameGroupAppl = ' ';

  // The kids
  std::vector<RuleNumber> _rules;
  std::vector<std::pair<RuleNumber, Footnote> > _ruleFootnotes;
  std::vector<FareClassCode> _fareClasses;
  std::vector<FareTypeAbbrev> _fareTypes;
  std::vector<SetNumber> _fareSet;
  std::vector<TariffNumber> _sameFareGroupTariff;
  std::vector<RuleNumber> _sameFareGroupRules;
  std::vector<FareClassCode> _sameFareGroupFareClasses;
  std::vector<FareTypeAbbrev> _sameFareGroupFareTypes;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _textTblItemNo);
    FLATTENIZE(archive, _governingCarrier);
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _tariffCat);
    FLATTENIZE(archive, _routingTariff1);
    FLATTENIZE(archive, _routingTariff2);
    FLATTENIZE(archive, _ruleTariffCode);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _directionality);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _globalDir);
    FLATTENIZE(archive, _mpmInd);
    FLATTENIZE(archive, _routingInd);
    FLATTENIZE(archive, _routing);
    FLATTENIZE(archive, _hipMinFareAppl);
    FLATTENIZE(archive, _hipFareCompAppl);
    FLATTENIZE(archive, _hipSameGroupAppl);
    FLATTENIZE(archive, _ctmMinFareAppl);
    FLATTENIZE(archive, _ctmFareCompAppl);
    FLATTENIZE(archive, _ctmSameGroupAppl);
    FLATTENIZE(archive, _backhaulMinFareAppl);
    FLATTENIZE(archive, _backhaulFareCompAppl);
    FLATTENIZE(archive, _backhaulSameGroupAppl);
    FLATTENIZE(archive, _dmcMinFareAppl);
    FLATTENIZE(archive, _dmcFareCompAppl);
    FLATTENIZE(archive, _dmcSameGroupAppl);
    FLATTENIZE(archive, _comMinFareAppl);
    FLATTENIZE(archive, _comFareCompAppl);
    FLATTENIZE(archive, _comSameGroupAppl);
    FLATTENIZE(archive, _copMinFareAppl);
    FLATTENIZE(archive, _copFareCompAppl);
    FLATTENIZE(archive, _copSameGroupAppl);
    FLATTENIZE(archive, _cpmMinFareAppl);
    FLATTENIZE(archive, _cpmFareCompAppl);
    FLATTENIZE(archive, _cpmSameGroupAppl);
    FLATTENIZE(archive, _oscFareCompAppl);
    FLATTENIZE(archive, _oscSameGroupAppl);
    FLATTENIZE(archive, _rscFareCompAppl);
    FLATTENIZE(archive, _rscSameGroupAppl);
    FLATTENIZE(archive, _rules);
    FLATTENIZE(archive, _ruleFootnotes);
    FLATTENIZE(archive, _fareClasses);
    FLATTENIZE(archive, _fareTypes);
    FLATTENIZE(archive, _fareSet);
    FLATTENIZE(archive, _sameFareGroupTariff);
    FLATTENIZE(archive, _sameFareGroupRules);
    FLATTENIZE(archive, _sameFareGroupFareClasses);
    FLATTENIZE(archive, _sameFareGroupFareTypes);
  }

private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_vendor
           & ptr->_textTblItemNo
           & ptr->_governingCarrier
           & ptr->_ruleTariff
           & ptr->_versionDate
           & ptr->_seqNo
           & ptr->_createDate
           & ptr->_expireDate
           & ptr->_effDate
           & ptr->_discDate
           & ptr->_tariffCat
           & ptr->_routingTariff1
           & ptr->_routingTariff2
           & ptr->_ruleTariffCode
           & ptr->_userApplType
           & ptr->_userAppl
           & ptr->_directionality
           & ptr->_loc1
           & ptr->_loc2
           & ptr->_globalDir
           & ptr->_mpmInd
           & ptr->_routingInd
           & ptr->_routing
           & ptr->_hipMinFareAppl
           & ptr->_hipFareCompAppl
           & ptr->_hipSameGroupAppl
           & ptr->_ctmMinFareAppl
           & ptr->_ctmFareCompAppl
           & ptr->_ctmSameGroupAppl
           & ptr->_backhaulMinFareAppl
           & ptr->_backhaulFareCompAppl
           & ptr->_backhaulSameGroupAppl
           & ptr->_dmcMinFareAppl
           & ptr->_dmcFareCompAppl
           & ptr->_dmcSameGroupAppl
           & ptr->_comMinFareAppl
           & ptr->_comFareCompAppl
           & ptr->_comSameGroupAppl
           & ptr->_copMinFareAppl
           & ptr->_copFareCompAppl
           & ptr->_copSameGroupAppl
           & ptr->_cpmMinFareAppl
           & ptr->_cpmFareCompAppl
           & ptr->_cpmSameGroupAppl
           & ptr->_oscFareCompAppl
           & ptr->_oscSameGroupAppl
           & ptr->_rscFareCompAppl
           & ptr->_rscSameGroupAppl
           & ptr->_rules
           & ptr->_ruleFootnotes
           & ptr->_fareClasses
           & ptr->_fareTypes
           & ptr->_fareSet
           & ptr->_sameFareGroupTariff
           & ptr->_sameFareGroupRules
           & ptr->_sameFareGroupFareClasses
           & ptr->_sameFareGroupFareTypes;
  }
};
} // namespace tse

