#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

#include <vector>

namespace tse
{

class MinFareDefaultLogic
{
public:
  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

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

  int& nmlHipTariffCatInd() { return _nmlHipTariffCatInd; }
  const int& nmlHipTariffCatInd() const { return _nmlHipTariffCatInd; }

  int& nmlCtmTariffCatInd() { return _nmlCtmTariffCatInd; }
  const int& nmlCtmTariffCatInd() const { return _nmlCtmTariffCatInd; }

  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Directionality& directionalInd() { return _directionalInd; }
  const Directionality& directionalInd() const { return _directionalInd; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  Indicator& domAppl() { return _domAppl; }
  const Indicator& domAppl() const { return _domAppl; }

  Indicator& domExceptInd() { return _domExceptInd; }
  const Indicator& domExceptInd() const { return _domExceptInd; }

  LocKey& domLoc() { return _domLoc; }
  const LocKey& domLoc() const { return _domLoc; }

  Indicator& nmlFareCompareInd() { return _nmlFareCompareInd; }
  const Indicator& nmlFareCompareInd() const { return _nmlFareCompareInd; }

  Indicator& nmlMpmBeforeRtgInd() { return _nmlMpmBeforeRtgInd; }
  const Indicator& nmlMpmBeforeRtgInd() const { return _nmlMpmBeforeRtgInd; }

  Indicator& nmlRtgBeforeMpmInd() { return _nmlRtgBeforeMpmInd; }
  const Indicator& nmlRtgBeforeMpmInd() const { return _nmlRtgBeforeMpmInd; }

  Indicator& nmlHipRestrCompInd() { return _nmlHipRestrCompInd; }
  const Indicator& nmlHipRestrCompInd() const { return _nmlHipRestrCompInd; }

  Indicator& nmlHipUnrestrCompInd() { return _nmlHipUnrestrCompInd; }
  const Indicator& nmlHipUnrestrCompInd() const { return _nmlHipUnrestrCompInd; }

  Indicator& nmlHipRbdCompInd() { return _nmlHipRbdCompInd; }
  const Indicator& nmlHipRbdCompInd() const { return _nmlHipRbdCompInd; }

  Indicator& nmlHipStopCompInd() { return _nmlHipStopCompInd; }
  const Indicator& nmlHipStopCompInd() const { return _nmlHipStopCompInd; }

  Indicator& nmlHipOrigInd() { return _nmlHipOrigInd; }
  const Indicator& nmlHipOrigInd() const { return _nmlHipOrigInd; }

  Indicator& nmlHipOrigNationInd() { return _nmlHipOrigNationInd; }
  const Indicator& nmlHipOrigNationInd() const { return _nmlHipOrigNationInd; }

  Indicator& nmlHipFromInterInd() { return _nmlHipFromInterInd; }
  const Indicator& nmlHipFromInterInd() const { return _nmlHipFromInterInd; }

  Indicator& nmlHipDestInd() { return _nmlHipDestInd; }
  const Indicator& nmlHipDestInd() const { return _nmlHipDestInd; }

  Indicator& nmlHipDestNationInd() { return _nmlHipDestNationInd; }
  const Indicator& nmlHipDestNationInd() const { return _nmlHipDestNationInd; }

  Indicator& nmlHipToInterInd() { return _nmlHipToInterInd; }
  const Indicator& nmlHipToInterInd() const { return _nmlHipToInterInd; }

  Indicator& nmlHipExemptInterToInter() { return _nmlHipExemptInterToInter; }
  const Indicator& nmlHipExemptInterToInter() const { return _nmlHipExemptInterToInter; }

  Indicator& spclHipTariffCatInd() { return _spclHipTariffCatInd; }
  const Indicator& spclHipTariffCatInd() const { return _spclHipTariffCatInd; }

  Indicator& spclHipRuleTrfInd() { return _spclHipRuleTrfInd; }
  const Indicator& spclHipRuleTrfInd() const { return _spclHipRuleTrfInd; }

  Indicator& spclHipFareClassInd() { return _spclHipFareClassInd; }
  const Indicator& spclHipFareClassInd() const { return _spclHipFareClassInd; }

  Indicator& spclHip1stCharInd() { return _spclHip1stCharInd; }
  const Indicator& spclHip1stCharInd() const { return _spclHip1stCharInd; }

  Indicator& spclHipStopCompInd() { return _spclHipStopCompInd; }
  const Indicator& spclHipStopCompInd() const { return _spclHipStopCompInd; }

  Indicator& spclHipSpclOnlyInd() { return _spclHipSpclOnlyInd; }
  const Indicator& spclHipSpclOnlyInd() const { return _spclHipSpclOnlyInd; }

  LocKey& spclHipLoc() { return _spclHipLoc; }
  const LocKey& spclHipLoc() const { return _spclHipLoc; }

  Indicator& spclHipOrigInd() { return _spclHipOrigInd; }
  const Indicator& spclHipOrigInd() const { return _spclHipOrigInd; }

  Indicator& spclHipOrigNationInd() { return _spclHipOrigNationInd; }
  const Indicator& spclHipOrigNationInd() const { return _spclHipOrigNationInd; }

  Indicator& spclHipFromInterInd() { return _spclHipFromInterInd; }
  const Indicator& spclHipFromInterInd() const { return _spclHipFromInterInd; }

  Indicator& spclHipDestInd() { return _spclHipDestInd; }
  const Indicator& spclHipDestInd() const { return _spclHipDestInd; }

  Indicator& spclHipDestNationInd() { return _spclHipDestNationInd; }
  const Indicator& spclHipDestNationInd() const { return _spclHipDestNationInd; }

  std::string& specialProcessName() { return _specialProcessName; }
  const std::string& specialProcessName() const { return _specialProcessName; }

  Indicator& spclHipToInterInd() { return _spclHipToInterInd; }
  const Indicator& spclHipToInterInd() const { return _spclHipToInterInd; }

  Indicator& spclHipExemptInterToInter() { return _spclHipExemptInterToInter; }
  const Indicator& spclHipExemptInterToInter() const { return _spclHipExemptInterToInter; }

  Indicator& nmlCtmRestrCompInd() { return _nmlCtmRestrCompInd; }
  const Indicator& nmlCtmRestrCompInd() const { return _nmlCtmRestrCompInd; }

  Indicator& nmlCtmUnrestrCompInd() { return _nmlCtmUnrestrCompInd; }
  const Indicator& nmlCtmUnrestrCompInd() const { return _nmlCtmUnrestrCompInd; }

  Indicator& nmlCtmRbdCompInd() { return _nmlCtmRbdCompInd; }
  const Indicator& nmlCtmRbdCompInd() const { return _nmlCtmRbdCompInd; }

  Indicator& nmlCtmStopCompInd() { return _nmlCtmStopCompInd; }
  const Indicator& nmlCtmStopCompInd() const { return _nmlCtmStopCompInd; }

  Indicator& nmlCtmOrigInd() { return _nmlCtmOrigInd; }
  const Indicator& nmlCtmOrigInd() const { return _nmlCtmOrigInd; }

  Indicator& nmlCtmDestNationInd() { return _nmlCtmDestNationInd; }
  const Indicator& nmlCtmDestNationInd() const { return _nmlCtmDestNationInd; }

  Indicator& nmlCtmToInterInd() { return _nmlCtmToInterInd; }
  const Indicator& nmlCtmToInterInd() const { return _nmlCtmToInterInd; }

  Indicator& spclCtmTariffCatInd() { return _spclCtmTariffCatInd; }
  const Indicator& spclCtmTariffCatInd() const { return _spclCtmTariffCatInd; }

  Indicator& spclCtmRuleTrfInd() { return _spclCtmRuleTrfInd; }
  const Indicator& spclCtmRuleTrfInd() const { return _spclCtmRuleTrfInd; }

  Indicator& spclCtmFareClassInd() { return _spclCtmFareClassInd; }
  const Indicator& spclCtmFareClassInd() const { return _spclCtmFareClassInd; }

  Indicator& spclSame1stCharFBInd2() { return _spclSame1stCharFBInd2; }
  const Indicator& spclSame1stCharFBInd2() const { return _spclSame1stCharFBInd2; }

  Indicator& spclCtmStopCompInd() { return _spclCtmStopCompInd; }
  const Indicator& spclCtmStopCompInd() const { return _spclCtmStopCompInd; }

  Indicator& spclCtmMktComp() { return _spclCtmMktComp; }
  const Indicator& spclCtmMktComp() const { return _spclCtmMktComp; }

  Indicator& spclCtmOrigInd() { return _spclCtmOrigInd; }
  const Indicator& spclCtmOrigInd() const { return _spclCtmOrigInd; }

  Indicator& spclCtmDestNationInd() { return _spclCtmDestNationInd; }
  const Indicator& spclCtmDestNationInd() const { return _spclCtmDestNationInd; }

  Indicator& spclCtmToInterInd() { return _spclCtmToInterInd; }
  const Indicator& spclCtmToInterInd() const { return _spclCtmToInterInd; }

  Indicator& cpmExcl() { return _cpmExcl; }
  const Indicator& cpmExcl() const { return _cpmExcl; }

  Indicator& domFareTypeExcept() { return _domFareTypeExcept; }
  const Indicator& domFareTypeExcept() const { return _domFareTypeExcept; }

  Indicator& nmlHipOrigNationTktPt() { return _nmlHipOrigNationTktPt; }
  const Indicator& nmlHipOrigNationTktPt() const { return _nmlHipOrigNationTktPt; }

  Indicator& nmlHipOrigNationStopPt() { return _nmlHipOrigNationStopPt; }
  const Indicator& nmlHipOrigNationStopPt() const { return _nmlHipOrigNationStopPt; }

  Indicator& nmlHipStopoverPt() { return _nmlHipStopoverPt; }
  const Indicator& nmlHipStopoverPt() const { return _nmlHipStopoverPt; }

  Indicator& nmlHipTicketedPt() { return _nmlHipTicketedPt; }
  const Indicator& nmlHipTicketedPt() const { return _nmlHipTicketedPt; }

  Indicator& spclHipOrigNationTktPt() { return _spclHipOrigNationTktPt; }
  const Indicator& spclHipOrigNationTktPt() const { return _spclHipOrigNationTktPt; }

  Indicator& spclHipOrigNationStopPt() { return _spclHipOrigNationStopPt; }
  const Indicator& spclHipOrigNationStopPt() const { return _spclHipOrigNationStopPt; }

  Indicator& spclHipStopoverPt() { return _spclHipStopoverPt; }
  const Indicator& spclHipStopoverPt() const { return _spclHipStopoverPt; }

  Indicator& spclHipTicketedPt() { return _spclHipTicketedPt; }
  const Indicator& spclHipTicketedPt() const { return _spclHipTicketedPt; }

  std::vector<FareTypeAbbrev>& fareTypes() { return _fareTypes; }
  const std::vector<FareTypeAbbrev>& fareTypes() const { return _fareTypes; }

  bool operator==(const MinFareDefaultLogic& rhs) const
  {
    return (
        (_vendor == rhs._vendor) && (_governingCarrier == rhs._governingCarrier) &&
        (_versionDate == rhs._versionDate) && (_seqNo == rhs._seqNo) &&
        (_createDate == rhs._createDate) && (_expireDate == rhs._expireDate) &&
        (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
        (_nmlHipTariffCatInd == rhs._nmlHipTariffCatInd) &&
        (_nmlCtmTariffCatInd == rhs._nmlCtmTariffCatInd) && (_userApplType == rhs._userApplType) &&
        (_userAppl == rhs._userAppl) && (_directionalInd == rhs._directionalInd) &&
        (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_domAppl == rhs._domAppl) &&
        (_domExceptInd == rhs._domExceptInd) && (_domLoc == rhs._domLoc) &&
        (_nmlFareCompareInd == rhs._nmlFareCompareInd) &&
        (_nmlMpmBeforeRtgInd == rhs._nmlMpmBeforeRtgInd) &&
        (_nmlRtgBeforeMpmInd == rhs._nmlRtgBeforeMpmInd) &&
        (_nmlHipRestrCompInd == rhs._nmlHipRestrCompInd) &&
        (_nmlHipUnrestrCompInd == rhs._nmlHipUnrestrCompInd) &&
        (_nmlHipRbdCompInd == rhs._nmlHipRbdCompInd) &&
        (_nmlHipStopCompInd == rhs._nmlHipStopCompInd) && (_nmlHipOrigInd == rhs._nmlHipOrigInd) &&
        (_nmlHipOrigNationInd == rhs._nmlHipOrigNationInd) &&
        (_nmlHipFromInterInd == rhs._nmlHipFromInterInd) &&
        (_nmlHipDestInd == rhs._nmlHipDestInd) &&
        (_nmlHipDestNationInd == rhs._nmlHipDestNationInd) &&
        (_nmlHipToInterInd == rhs._nmlHipToInterInd) &&
        (_nmlHipExemptInterToInter == rhs._nmlHipExemptInterToInter) &&
        (_spclHipTariffCatInd == rhs._spclHipTariffCatInd) &&
        (_spclHipRuleTrfInd == rhs._spclHipRuleTrfInd) &&
        (_spclHipFareClassInd == rhs._spclHipFareClassInd) &&
        (_spclHip1stCharInd == rhs._spclHip1stCharInd) &&
        (_spclHipStopCompInd == rhs._spclHipStopCompInd) &&
        (_spclHipSpclOnlyInd == rhs._spclHipSpclOnlyInd) && (_spclHipLoc == rhs._spclHipLoc) &&
        (_spclHipOrigInd == rhs._spclHipOrigInd) &&
        (_spclHipOrigNationInd == rhs._spclHipOrigNationInd) &&
        (_spclHipFromInterInd == rhs._spclHipFromInterInd) &&
        (_spclHipDestInd == rhs._spclHipDestInd) &&
        (_spclHipDestNationInd == rhs._spclHipDestNationInd) &&
        (_specialProcessName == rhs._specialProcessName) &&
        (_spclHipToInterInd == rhs._spclHipToInterInd) &&
        (_spclHipExemptInterToInter == rhs._spclHipExemptInterToInter) &&
        (_nmlCtmRestrCompInd == rhs._nmlCtmRestrCompInd) &&
        (_nmlCtmUnrestrCompInd == rhs._nmlCtmUnrestrCompInd) &&
        (_nmlCtmRbdCompInd == rhs._nmlCtmRbdCompInd) &&
        (_nmlCtmStopCompInd == rhs._nmlCtmStopCompInd) && (_nmlCtmOrigInd == rhs._nmlCtmOrigInd) &&
        (_nmlCtmDestNationInd == rhs._nmlCtmDestNationInd) &&
        (_nmlCtmToInterInd == rhs._nmlCtmToInterInd) &&
        (_spclCtmTariffCatInd == rhs._spclCtmTariffCatInd) &&
        (_spclCtmRuleTrfInd == rhs._spclCtmRuleTrfInd) &&
        (_spclCtmFareClassInd == rhs._spclCtmFareClassInd) &&
        (_spclSame1stCharFBInd2 == rhs._spclSame1stCharFBInd2) &&
        (_spclCtmStopCompInd == rhs._spclCtmStopCompInd) &&
        (_spclCtmMktComp == rhs._spclCtmMktComp) && (_spclCtmOrigInd == rhs._spclCtmOrigInd) &&
        (_spclCtmDestNationInd == rhs._spclCtmDestNationInd) &&
        (_spclCtmToInterInd == rhs._spclCtmToInterInd) && (_cpmExcl == rhs._cpmExcl) &&
        (_domFareTypeExcept == rhs._domFareTypeExcept) &&
        (_nmlHipOrigNationTktPt == rhs._nmlHipOrigNationTktPt) &&
        (_nmlHipOrigNationStopPt == rhs._nmlHipOrigNationStopPt) &&
        (_nmlHipStopoverPt == rhs._nmlHipStopoverPt) &&
        (_nmlHipTicketedPt == rhs._nmlHipTicketedPt) &&
        (_spclHipOrigNationTktPt == rhs._spclHipOrigNationTktPt) &&
        (_spclHipOrigNationStopPt == rhs._spclHipOrigNationStopPt) &&
        (_spclHipStopoverPt == rhs._spclHipStopoverPt) &&
        (_spclHipTicketedPt == rhs._spclHipTicketedPt) && (_fareTypes == rhs._fareTypes));
  }

  static void dummyData(MinFareDefaultLogic& obj)
  {
    obj._vendor = "ABCD";
    obj._governingCarrier = "EF";
    obj._versionDate = time(nullptr);
    obj._seqNo = 1;
    obj._createDate = time(nullptr);
    obj._expireDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._nmlHipTariffCatInd = 2;
    obj._nmlCtmTariffCatInd = 3;
    obj._userApplType = 'G';
    obj._userAppl = "HIJK";
    obj._directionalInd = TO;

    LocKey::dummyData(obj._loc1);
    LocKey::dummyData(obj._loc2);

    obj._domAppl = 'L';
    obj._domExceptInd = 'M';

    LocKey::dummyData(obj._domLoc);

    obj._nmlFareCompareInd = 'N';
    obj._nmlMpmBeforeRtgInd = 'O';
    obj._nmlRtgBeforeMpmInd = 'P';
    obj._nmlHipRestrCompInd = 'Q';
    obj._nmlHipUnrestrCompInd = 'R';
    obj._nmlHipRbdCompInd = 'S';
    obj._nmlHipStopCompInd = 'T';
    obj._nmlHipOrigInd = 'U';
    obj._nmlHipOrigNationInd = 'V';
    obj._nmlHipFromInterInd = 'W';
    obj._nmlHipDestInd = 'X';
    obj._nmlHipDestNationInd = 'Y';
    obj._nmlHipToInterInd = 'Z';
    obj._nmlHipExemptInterToInter = 'a';
    obj._spclHipTariffCatInd = 'b';
    obj._spclHipRuleTrfInd = 'c';
    obj._spclHipFareClassInd = 'd';
    obj._spclHip1stCharInd = 'e';
    obj._spclHipStopCompInd = 'f';
    obj._spclHipSpclOnlyInd = 'g';

    LocKey::dummyData(obj._spclHipLoc);

    obj._spclHipOrigInd = 'h';
    obj._spclHipOrigNationInd = 'i';
    obj._spclHipFromInterInd = 'j';
    obj._spclHipDestInd = 'k';
    obj._spclHipDestNationInd = 'l';
    obj._specialProcessName = "bbbbbbbb";
    obj._spclHipToInterInd = 'm';
    obj._spclHipExemptInterToInter = 'n';
    obj._nmlCtmRestrCompInd = 'o';
    obj._nmlCtmUnrestrCompInd = 'p';
    obj._nmlCtmRbdCompInd = 'q';
    obj._nmlCtmStopCompInd = 'r';
    obj._nmlCtmOrigInd = 's';
    obj._nmlCtmDestNationInd = 't';
    obj._nmlCtmToInterInd = 'u';
    obj._spclCtmTariffCatInd = 'v';
    obj._spclCtmRuleTrfInd = 'w';
    obj._spclCtmFareClassInd = 'x';
    obj._spclSame1stCharFBInd2 = 'y';
    obj._spclCtmStopCompInd = 'z';
    obj._spclCtmMktComp = '0';
    obj._spclCtmOrigInd = '1';
    obj._spclCtmDestNationInd = '2';
    obj._spclCtmToInterInd = '3';
    obj._cpmExcl = '4';
    obj._domFareTypeExcept = '5';
    obj._nmlHipOrigNationTktPt = '6';
    obj._nmlHipOrigNationStopPt = '7';
    obj._nmlHipStopoverPt = '8';
    obj._nmlHipTicketedPt = '9';
    obj._spclHipOrigNationTktPt = 'A';
    obj._spclHipOrigNationStopPt = 'B';
    obj._spclHipStopoverPt = 'C';
    obj._spclHipTicketedPt = 'D';

    obj._fareTypes.push_back("EFG");
    obj._fareTypes.push_back("HIJ");
  }

private:
  VendorCode _vendor;
  CarrierCode _governingCarrier;
  DateTime _versionDate;
  int _seqNo = 0;
  DateTime _createDate;
  DateTime _expireDate;
  DateTime _effDate;
  DateTime _discDate;
  int _nmlHipTariffCatInd = 0;
  int _nmlCtmTariffCatInd = 0;
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  Directionality _directionalInd = Directionality::TERMINATE;
  LocKey _loc1;
  LocKey _loc2;
  Indicator _domAppl = ' ';
  Indicator _domExceptInd = ' ';
  LocKey _domLoc;
  Indicator _nmlFareCompareInd = ' ';
  Indicator _nmlMpmBeforeRtgInd = ' ';
  Indicator _nmlRtgBeforeMpmInd = ' ';
  Indicator _nmlHipRestrCompInd = ' ';
  Indicator _nmlHipUnrestrCompInd = ' ';
  Indicator _nmlHipRbdCompInd = ' ';
  Indicator _nmlHipStopCompInd = ' ';
  Indicator _nmlHipOrigInd = ' ';
  Indicator _nmlHipOrigNationInd = ' ';
  Indicator _nmlHipFromInterInd = ' ';
  Indicator _nmlHipDestInd = ' ';
  Indicator _nmlHipDestNationInd = ' ';
  Indicator _nmlHipToInterInd = ' ';
  Indicator _nmlHipExemptInterToInter = ' ';
  Indicator _spclHipTariffCatInd = ' ';
  Indicator _spclHipRuleTrfInd = ' ';
  Indicator _spclHipFareClassInd = ' ';
  Indicator _spclHip1stCharInd = ' ';
  Indicator _spclHipStopCompInd = ' ';
  Indicator _spclHipSpclOnlyInd = ' ';
  LocKey _spclHipLoc;
  Indicator _spclHipOrigInd = ' ';
  Indicator _spclHipOrigNationInd = ' ';
  Indicator _spclHipFromInterInd = ' ';
  Indicator _spclHipDestInd = ' ';
  Indicator _spclHipDestNationInd = ' ';
  std::string _specialProcessName;
  Indicator _spclHipToInterInd = ' ';
  Indicator _spclHipExemptInterToInter = ' ';
  Indicator _nmlCtmRestrCompInd = ' ';
  Indicator _nmlCtmUnrestrCompInd = ' ';
  Indicator _nmlCtmRbdCompInd = ' ';
  Indicator _nmlCtmStopCompInd = ' ';
  Indicator _nmlCtmOrigInd = ' ';
  Indicator _nmlCtmDestNationInd = ' ';
  Indicator _nmlCtmToInterInd = ' ';
  Indicator _spclCtmTariffCatInd = ' ';
  Indicator _spclCtmRuleTrfInd = ' ';
  Indicator _spclCtmFareClassInd = ' ';
  Indicator _spclSame1stCharFBInd2 = ' ';
  Indicator _spclCtmStopCompInd = ' ';
  Indicator _spclCtmMktComp = ' ';
  Indicator _spclCtmOrigInd = ' ';
  Indicator _spclCtmDestNationInd = ' ';
  Indicator _spclCtmToInterInd = ' ';
  Indicator _cpmExcl = ' ';
  Indicator _domFareTypeExcept = ' ';
  Indicator _nmlHipOrigNationTktPt = ' ';
  Indicator _nmlHipOrigNationStopPt = ' ';
  Indicator _nmlHipStopoverPt = ' ';
  Indicator _nmlHipTicketedPt = ' ';
  Indicator _spclHipOrigNationTktPt = ' ';
  Indicator _spclHipOrigNationStopPt = ' ';
  Indicator _spclHipStopoverPt = ' ';
  Indicator _spclHipTicketedPt = ' ';
  std::vector<FareTypeAbbrev> _fareTypes;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _governingCarrier);
    FLATTENIZE(archive, _versionDate);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _nmlHipTariffCatInd);
    FLATTENIZE(archive, _nmlCtmTariffCatInd);
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _directionalInd);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _domAppl);
    FLATTENIZE(archive, _domExceptInd);
    FLATTENIZE(archive, _domLoc);
    FLATTENIZE(archive, _nmlFareCompareInd);
    FLATTENIZE(archive, _nmlMpmBeforeRtgInd);
    FLATTENIZE(archive, _nmlRtgBeforeMpmInd);
    FLATTENIZE(archive, _nmlHipRestrCompInd);
    FLATTENIZE(archive, _nmlHipUnrestrCompInd);
    FLATTENIZE(archive, _nmlHipRbdCompInd);
    FLATTENIZE(archive, _nmlHipStopCompInd);
    FLATTENIZE(archive, _nmlHipOrigInd);
    FLATTENIZE(archive, _nmlHipOrigNationInd);
    FLATTENIZE(archive, _nmlHipFromInterInd);
    FLATTENIZE(archive, _nmlHipDestInd);
    FLATTENIZE(archive, _nmlHipDestNationInd);
    FLATTENIZE(archive, _nmlHipToInterInd);
    FLATTENIZE(archive, _nmlHipExemptInterToInter);
    FLATTENIZE(archive, _spclHipTariffCatInd);
    FLATTENIZE(archive, _spclHipRuleTrfInd);
    FLATTENIZE(archive, _spclHipFareClassInd);
    FLATTENIZE(archive, _spclHip1stCharInd);
    FLATTENIZE(archive, _spclHipStopCompInd);
    FLATTENIZE(archive, _spclHipSpclOnlyInd);
    FLATTENIZE(archive, _spclHipLoc);
    FLATTENIZE(archive, _spclHipOrigInd);
    FLATTENIZE(archive, _spclHipOrigNationInd);
    FLATTENIZE(archive, _spclHipFromInterInd);
    FLATTENIZE(archive, _spclHipDestInd);
    FLATTENIZE(archive, _spclHipDestNationInd);
    FLATTENIZE(archive, _specialProcessName);
    FLATTENIZE(archive, _spclHipToInterInd);
    FLATTENIZE(archive, _spclHipExemptInterToInter);
    FLATTENIZE(archive, _nmlCtmRestrCompInd);
    FLATTENIZE(archive, _nmlCtmUnrestrCompInd);
    FLATTENIZE(archive, _nmlCtmRbdCompInd);
    FLATTENIZE(archive, _nmlCtmStopCompInd);
    FLATTENIZE(archive, _nmlCtmOrigInd);
    FLATTENIZE(archive, _nmlCtmDestNationInd);
    FLATTENIZE(archive, _nmlCtmToInterInd);
    FLATTENIZE(archive, _spclCtmTariffCatInd);
    FLATTENIZE(archive, _spclCtmRuleTrfInd);
    FLATTENIZE(archive, _spclCtmFareClassInd);
    FLATTENIZE(archive, _spclSame1stCharFBInd2);
    FLATTENIZE(archive, _spclCtmStopCompInd);
    FLATTENIZE(archive, _spclCtmMktComp);
    FLATTENIZE(archive, _spclCtmOrigInd);
    FLATTENIZE(archive, _spclCtmDestNationInd);
    FLATTENIZE(archive, _spclCtmToInterInd);
    FLATTENIZE(archive, _cpmExcl);
    FLATTENIZE(archive, _domFareTypeExcept);
    FLATTENIZE(archive, _nmlHipOrigNationTktPt);
    FLATTENIZE(archive, _nmlHipOrigNationStopPt);
    FLATTENIZE(archive, _nmlHipStopoverPt);
    FLATTENIZE(archive, _nmlHipTicketedPt);
    FLATTENIZE(archive, _spclHipOrigNationTktPt);
    FLATTENIZE(archive, _spclHipOrigNationStopPt);
    FLATTENIZE(archive, _spclHipStopoverPt);
    FLATTENIZE(archive, _spclHipTicketedPt);
    FLATTENIZE(archive, _fareTypes);
  }
};

} // namespace tse
