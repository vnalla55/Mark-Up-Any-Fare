//----------------------------------------------------------------------------
//   2005, Sabre Inc.  All rights reserved.  This software/documentation is
//   the confidential and proprietary product of Sabre Inc. Any unauthorized
//   use, reproduction, or transfer of this software/documentation, in any
//   medium, or incorporation of this software/documentation into any system
//   or publication, is strictly prohibited
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{
class FareDisplayPref
{
public:
  Indicator& userApplType() { return _userApplType; }
  const Indicator& userApplType() const { return _userApplType; }

  UserApplCode& userAppl() { return _userAppl; }
  const UserApplCode& userAppl() const { return _userAppl; }

  Indicator& pseudoCityType() { return _pseudoCityType; }
  const Indicator& pseudoCityType() const { return _pseudoCityType; }

  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  TJRGroup& ssgGroupNo() { return _ssgGroupNo; }
  const TJRGroup& ssgGroupNo() const { return _ssgGroupNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  int& memoNo() { return _memoNo; }
  const int& memoNo() const { return _memoNo; }

  std::string& creatorId() { return _creatorId; }
  const std::string& creatorId() const { return _creatorId; }

  std::string& creatorBusinessUnit() { return _creatorBusinessUnit; }
  const std::string& creatorBusinessUnit() const { return _creatorBusinessUnit; }

  TemplateID& singleCxrTemplateId() { return _singleCxrTemplateId; }
  const TemplateID& singleCxrTemplateId() const { return _singleCxrTemplateId; }

  TemplateID& multiCxrTemplateId() { return _multiCxrTemplateId; }
  const TemplateID& multiCxrTemplateId() const { return _multiCxrTemplateId; }

  Indicator& showRoutings() { return _showRoutings; }
  const Indicator& showRoutings() const { return _showRoutings; }

  Indicator& doubleForRoundTrip() { return _doubleForRoundTrip; }
  const Indicator& doubleForRoundTrip() const { return _doubleForRoundTrip; }

  Indicator& displayHalfRoundTrip() { return _displayHalfRoundTrip; }
  const Indicator& displayHalfRoundTrip() const { return _displayHalfRoundTrip; }

  Indicator& sameFareBasisSameLine() { return _sameFareBasisSameLine; }
  const Indicator& sameFareBasisSameLine() const { return _sameFareBasisSameLine; }

  Indicator& returnDateValidation() { return _returnDateValidation; }
  const Indicator& returnDateValidation() const { return _returnDateValidation; }

  Indicator& noFutureSalesDate() { return _noFutureSalesDate; }
  const Indicator& noFutureSalesDate() const { return _noFutureSalesDate; }

  Indicator& singleCarrierSvcSched() { return _singleCarrierSvcSched; }
  const Indicator& singleCarrierSvcSched() const { return _singleCarrierSvcSched; }

  Indicator& multiCarrierSvcSched() { return _multiCarrierSvcSched; }
  const Indicator& multiCarrierSvcSched() const { return _multiCarrierSvcSched; }

  TemplateID& taxTemplateId() { return _taxTemplateId; }
  const TemplateID& taxTemplateId() const { return _taxTemplateId; }

  TemplateID& addOnTemplateId() { return _addOnTemplateId; }
  const TemplateID& addOnTemplateId() const { return _addOnTemplateId; }

  Indicator& journeyInd() { return _journeyInd; }
  const Indicator& journeyInd() const { return _journeyInd; }

  Indicator& validateLocaleForPublFares() { return _validateLocaleForPublFares; }
  const Indicator& validateLocaleForPublFares() const { return _validateLocaleForPublFares; }

  Indicator& applyDOWvalidationToOWFares() { return _applyDOWvalidationToOWFares; }
  const Indicator& applyDOWvalidationToOWFares() const { return _applyDOWvalidationToOWFares; }

  bool operator==(const FareDisplayPref& rhs) const
  {
    return ((_userApplType == rhs._userApplType) && (_userAppl == rhs._userAppl) &&
            (_pseudoCityType == rhs._pseudoCityType) && (_pseudoCity == rhs._pseudoCity) &&
            (_ssgGroupNo == rhs._ssgGroupNo) && (_createDate == rhs._createDate) &&
            (_lockDate == rhs._lockDate) && (_memoNo == rhs._memoNo) &&
            (_creatorId == rhs._creatorId) && (_creatorBusinessUnit == rhs._creatorBusinessUnit) &&
            (_singleCxrTemplateId == rhs._singleCxrTemplateId) &&
            (_multiCxrTemplateId == rhs._multiCxrTemplateId) &&
            (_showRoutings == rhs._showRoutings) &&
            (_doubleForRoundTrip == rhs._doubleForRoundTrip) &&
            (_displayHalfRoundTrip == rhs._displayHalfRoundTrip) &&
            (_sameFareBasisSameLine == rhs._sameFareBasisSameLine) &&
            (_returnDateValidation == rhs._returnDateValidation) &&
            (_noFutureSalesDate == rhs._noFutureSalesDate) &&
            (_singleCarrierSvcSched == rhs._singleCarrierSvcSched) &&
            (_multiCarrierSvcSched == rhs._multiCarrierSvcSched) &&
            (_taxTemplateId == rhs._taxTemplateId) && (_addOnTemplateId == rhs._addOnTemplateId) &&
            (_journeyInd == rhs._journeyInd) &&
            (_validateLocaleForPublFares == rhs._validateLocaleForPublFares) &&
            (_applyDOWvalidationToOWFares == rhs._applyDOWvalidationToOWFares));
  }

  static void dummyData(FareDisplayPref& obj)
  {
    obj._userApplType = 'A';
    obj._userAppl = "BCDE";
    obj._pseudoCityType = 'F';
    obj._pseudoCity = "GHIJK";
    obj._ssgGroupNo = 1;
    obj._createDate = time(nullptr);
    obj._lockDate = time(nullptr);
    obj._memoNo = 2;
    obj._creatorId = "aaaaaaaa";
    obj._creatorBusinessUnit = "bbbbbbbb";
    obj._singleCxrTemplateId = "LM";
    obj._multiCxrTemplateId = "NO";
    obj._showRoutings = 'P';
    obj._doubleForRoundTrip = 'Q';
    obj._displayHalfRoundTrip = 'R';
    obj._sameFareBasisSameLine = 'S';
    obj._returnDateValidation = 'T';
    obj._noFutureSalesDate = 'U';
    obj._singleCarrierSvcSched = 'V';
    obj._multiCarrierSvcSched = 'W';
    obj._taxTemplateId = "XY";
    obj._addOnTemplateId = "Za";
    obj._journeyInd = 'b';
    obj._validateLocaleForPublFares = 'c';
    obj._applyDOWvalidationToOWFares = 'd';
  }

private:
  Indicator _userApplType = ' ';
  UserApplCode _userAppl;
  Indicator _pseudoCityType = ' ';
  PseudoCityCode _pseudoCity;
  TJRGroup _ssgGroupNo = 0;
  DateTime _createDate;
  DateTime _lockDate;
  int _memoNo = 0;
  std::string _creatorId;
  std::string _creatorBusinessUnit;
  TemplateID _singleCxrTemplateId;
  TemplateID _multiCxrTemplateId;
  Indicator _showRoutings = ' ';
  Indicator _doubleForRoundTrip = ' ';
  Indicator _displayHalfRoundTrip = ' ';
  Indicator _sameFareBasisSameLine = ' ';
  Indicator _returnDateValidation = ' ';
  Indicator _noFutureSalesDate = ' ';
  Indicator _singleCarrierSvcSched = ' ';
  Indicator _multiCarrierSvcSched = ' ';
  TemplateID _taxTemplateId;
  TemplateID _addOnTemplateId;
  Indicator _journeyInd = ' ';
  Indicator _validateLocaleForPublFares = ' ';
  Indicator _applyDOWvalidationToOWFares = 'N';

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _userApplType);
    FLATTENIZE(archive, _userAppl);
    FLATTENIZE(archive, _pseudoCityType);
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _ssgGroupNo);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _lockDate);
    FLATTENIZE(archive, _memoNo);
    FLATTENIZE(archive, _creatorId);
    FLATTENIZE(archive, _creatorBusinessUnit);
    FLATTENIZE(archive, _singleCxrTemplateId);
    FLATTENIZE(archive, _multiCxrTemplateId);
    FLATTENIZE(archive, _showRoutings);
    FLATTENIZE(archive, _doubleForRoundTrip);
    FLATTENIZE(archive, _displayHalfRoundTrip);
    FLATTENIZE(archive, _sameFareBasisSameLine);
    FLATTENIZE(archive, _returnDateValidation);
    FLATTENIZE(archive, _noFutureSalesDate);
    FLATTENIZE(archive, _singleCarrierSvcSched);
    FLATTENIZE(archive, _multiCarrierSvcSched);
    FLATTENIZE(archive, _taxTemplateId);
    FLATTENIZE(archive, _addOnTemplateId);
    FLATTENIZE(archive, _journeyInd);
    FLATTENIZE(archive, _validateLocaleForPublFares);
    FLATTENIZE(archive, _applyDOWvalidationToOWFares);
  }
};
}
