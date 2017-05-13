//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/IndustryFareAppl.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

bool
IndustryFareAppl::ExceptAppl::
operator<(const IndustryFareAppl::ExceptAppl& rhs) const
{
  if (_exceptCarrier != rhs._exceptCarrier)
    return (_exceptCarrier < rhs._exceptCarrier);
  if (_itemNo != rhs._itemNo)
    return (_itemNo < rhs._itemNo);
  if (_fareTariff != rhs._fareTariff)
    return (_fareTariff < rhs._fareTariff);
  if (_vendor != rhs._vendor)
    return (_vendor < rhs._vendor);
  if (_userAppl != rhs._userAppl)
    return (_userAppl < rhs._userAppl);
  if (_yyFareAppl != rhs._yyFareAppl)
    return (_yyFareAppl < rhs._yyFareAppl);
  if (_directionality != rhs._directionality)
    return (_directionality < rhs._directionality);
  if (_loc1 != rhs._loc1)
    return (_loc1 < rhs._loc1);
  if (_loc2 != rhs._loc2)
    return (_loc2 < rhs._loc2);
  if (_globalDir != rhs._globalDir)
    return (_globalDir < rhs._globalDir);
  if (_rule != rhs._rule)
    return (_rule < rhs._rule);
  if (_routing != rhs._routing)
    return (_routing < rhs._routing);
  if (_footNote != rhs._footNote)
    return (_footNote < rhs._footNote);
  if (_cur != rhs._cur)
    return (_cur < rhs._cur);
  if (_fareClass != rhs._fareClass)
    return (_fareClass < rhs._fareClass);
  if (_fareType != rhs._fareType)
    return (_fareType < rhs._fareType);
  if (_owrt != rhs._owrt)
    return (_owrt < rhs._owrt);
  if (_userApplType != rhs._userApplType)
    return (_userApplType < rhs._userApplType);

  return false;
}

bool
IndustryFareAppl::ExceptAppl::
operator==(const IndustryFareAppl::ExceptAppl& rhs) const
{
  return ((_exceptCarrier == rhs._exceptCarrier) && (_itemNo == rhs._itemNo) &&
          (_fareTariff == rhs._fareTariff) && (_vendor == rhs._vendor) &&
          (_userAppl == rhs._userAppl) && (_yyFareAppl == rhs._yyFareAppl) &&
          (_directionality == rhs._directionality) && (_loc1 == rhs._loc1) &&
          (_loc2 == rhs._loc2) && (_globalDir == rhs._globalDir) && (_rule == rhs._rule) &&
          (_routing == rhs._routing) && (_footNote == rhs._footNote) && (_cur == rhs._cur) &&
          (_fareClass == rhs._fareClass) && (_fareType == rhs._fareType) && (_owrt == rhs._owrt) &&
          (_userApplType == rhs._userApplType));
}

void
IndustryFareAppl::ExceptAppl::dummyData(IndustryFareAppl::ExceptAppl& obj)
{
  obj._exceptCarrier = "ABC";
  obj._itemNo = 1;
  obj._fareTariff = 2;
  obj._vendor = "DEFG";
  obj._userAppl = "HIJK";
  obj._yyFareAppl = 'L';
  obj._directionality = 'M';

  LocKey::dummyData(obj._loc1);
  LocKey::dummyData(obj._loc2);

  obj._globalDir = GlobalDirection::US;
  obj._rule = "NOPQ";
  obj._routing = "RSTU";
  obj._footNote = "VW";
  obj._cur = "XYZ";
  obj._fareClass = "abcdefgh";
  obj._fareType = "ijklmnop";
  obj._owrt = 'q';
  obj._userApplType = 'r';
}

void
IndustryFareAppl::ExceptAppl::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _exceptCarrier);
  FLATTENIZE(archive, _itemNo);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _userAppl);
  FLATTENIZE(archive, _yyFareAppl);
  FLATTENIZE(archive, _directionality);
  FLATTENIZE(archive, _loc1);
  FLATTENIZE(archive, _loc2);
  FLATTENIZE(archive, _globalDir);
  FLATTENIZE(archive, _rule);
  FLATTENIZE(archive, _routing);
  FLATTENIZE(archive, _footNote);
  FLATTENIZE(archive, _cur);
  FLATTENIZE(archive, _fareClass);
  FLATTENIZE(archive, _fareType);
  FLATTENIZE(archive, _owrt);
  FLATTENIZE(archive, _userApplType);
}

bool
IndustryFareAppl::
operator<(const IndustryFareAppl& rhs) const
{
  if (_selectionType != rhs._selectionType)
    return (_selectionType < rhs._selectionType);
  if (_carrier != rhs._carrier)
    return (_carrier < rhs._carrier);
  if (_seqNo != rhs._seqNo)
    return (_seqNo < rhs._seqNo);
  if (_expireDate != rhs._expireDate)
    return (_expireDate < rhs._expireDate);
  if (_createDate != rhs._createDate)
    return (_createDate < rhs._createDate);
  if (_effDate != rhs._effDate)
    return (_effDate < rhs._effDate);
  if (_discDate != rhs._discDate)
    return (_discDate < rhs._discDate);
  if (_fareTariff != rhs._fareTariff)
    return (_fareTariff < rhs._fareTariff);
  if (_vendor != rhs._vendor)
    return (_vendor < rhs._vendor);
  if (_userAppl != rhs._userAppl)
    return (_userAppl < rhs._userAppl);
  if (_yyFareAppl != rhs._yyFareAppl)
    return (_yyFareAppl < rhs._yyFareAppl);
  if (_directionality != rhs._directionality)
    return (_directionality < rhs._directionality);
  if (_loc1 != rhs._loc1)
    return (_loc1 < rhs._loc1);
  if (_loc2 != rhs._loc2)
    return (_loc2 < rhs._loc2);
  if (_globalDir != rhs._globalDir)
    return (_globalDir < rhs._globalDir);
  if (_rule != rhs._rule)
    return (_rule < rhs._rule);
  if (_routing != rhs._routing)
    return (_routing < rhs._routing);
  if (_footNote != rhs._footNote)
    return (_footNote < rhs._footNote);
  if (_cur != rhs._cur)
    return (_cur < rhs._cur);
  if (_fareClass != rhs._fareClass)
    return (_fareClass < rhs._fareClass);
  if (_fareType != rhs._fareType)
    return (_fareType < rhs._fareType);
  if (_owrt != rhs._owrt)
    return (_owrt < rhs._owrt);
  if (_userApplType != rhs._userApplType)
    return (_userApplType < rhs._userApplType);
  if (_except != rhs._except)
    return (_except < rhs._except);

  return false;
}

bool
IndustryFareAppl::
operator==(const IndustryFareAppl& rhs) const
{
  return ((_selectionType == rhs._selectionType) && (_carrier == rhs._carrier) &&
          (_seqNo == rhs._seqNo) && (_expireDate == rhs._expireDate) &&
          (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
          (_discDate == rhs._discDate) && (_fareTariff == rhs._fareTariff) &&
          (_vendor == rhs._vendor) && (_userAppl == rhs._userAppl) &&
          (_yyFareAppl == rhs._yyFareAppl) && (_directionality == rhs._directionality) &&
          (_loc1 == rhs._loc1) && (_loc2 == rhs._loc2) && (_globalDir == rhs._globalDir) &&
          (_rule == rhs._rule) && (_routing == rhs._routing) && (_footNote == rhs._footNote) &&
          (_cur == rhs._cur) && (_fareClass == rhs._fareClass) && (_fareType == rhs._fareType) &&
          (_owrt == rhs._owrt) && (_userApplType == rhs._userApplType) && (_except == rhs._except));
}

void
IndustryFareAppl::dummyData(IndustryFareAppl& obj)
{
  obj._selectionType = 'A';
  obj._carrier = "BCD";
  obj._seqNo = 1;
  obj._expireDate = time(nullptr);
  obj._createDate = time(nullptr);
  obj._effDate = time(nullptr);
  obj._discDate = time(nullptr);
  obj._fareTariff = 2;
  obj._vendor = "EFGH";
  obj._userAppl = "IJKL";
  obj._yyFareAppl = 'M';
  obj._directionality = 'N';

  LocKey::dummyData(obj._loc1);
  LocKey::dummyData(obj._loc2);

  obj._globalDir = GlobalDirection::US;
  obj._rule = "OPQR";
  obj._routing = "STUV";
  obj._footNote = "WX";
  obj._cur = "YZa";
  obj._fareClass = "bcdefghi";
  obj._fareType = "jklmnopq";
  obj._owrt = 'r';
  obj._userApplType = 's';

  ExceptAppl ea1;
  ExceptAppl ea2;

  ExceptAppl::dummyData(ea1);
  ExceptAppl::dummyData(ea2);

  obj._except.push_back(ea1);
  obj._except.push_back(ea2);
}

void
IndustryFareAppl::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _selectionType);
  FLATTENIZE(archive, _carrier);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _fareTariff);
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _userAppl);
  FLATTENIZE(archive, _yyFareAppl);
  FLATTENIZE(archive, _directionality);
  FLATTENIZE(archive, _loc1);
  FLATTENIZE(archive, _loc2);
  FLATTENIZE(archive, _globalDir);
  FLATTENIZE(archive, _rule);
  FLATTENIZE(archive, _routing);
  FLATTENIZE(archive, _footNote);
  FLATTENIZE(archive, _cur);
  FLATTENIZE(archive, _fareClass);
  FLATTENIZE(archive, _fareType);
  FLATTENIZE(archive, _owrt);
  FLATTENIZE(archive, _userApplType);
  FLATTENIZE(archive, _except);
}

} // tse
