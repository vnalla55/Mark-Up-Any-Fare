//----------------------------------------------------------------------------
//
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------

#include "DBAccess/TaxReportingRecordInfo.h"

#include "DBAccess/Flattenizable.h"

namespace tse
{

std::ostream&
dumpObject(std::ostream& os, const TaxReportingRecordInfo& obj)
{
  os << "[" << obj._vendor << "|" << obj._nationCode << "|" << obj._taxCarrier << "|"
     << obj._taxCode << "|" << obj._taxType << "|" << obj._createDate << "|" << obj._effDate << "|"
     << obj._discDate << "|" << obj._lockDate << "|" << obj._expireDate << "|"
     << obj._lastModificationDate << "|" << obj._taxCharge << "|" << obj._accountableDocTaxTag
     << "|" << obj._reportingTextTblNo << "|" << obj._interlineAbleTaxTag << "|"
     << obj._refundableTaxTag << "|" << obj._commisionableTaxTag << "|" << obj._vatInd << "|"
     << obj._taxName << "|" << obj._taxTextItemNo << "|" << obj._taxApplicableToTblNo << "|"
     << obj._taxRateTextTblNo << "|" << obj._taxExemptionTextTblNo << "|"
     << obj._taxCollectNrEmmitTblNo << "|" << obj._taxingAuthorityTextTblNo << "|"
     << obj._taxCommentsTextTblNo << "|" << obj._taxSplInstructionsTblNo << "|" << obj._versionNbr;
  os << "]";

  return os;
}

TaxReportingRecordInfo::TaxReportingRecordInfo()
  : _taxCharge(' '),
    _accountableDocTaxTag(' '),
    _reportingTextTblNo(0),
    _interlineAbleTaxTag(' '),
    _refundableTaxTag(' '),
    _commisionableTaxTag(' '),
    _vatInd(' '),
    _taxTextItemNo(0),
    _taxApplicableToTblNo(0),
    _taxRateTextTblNo(0),
    _taxExemptionTextTblNo(0),
    _taxCollectNrEmmitTblNo(0),
    _taxingAuthorityTextTblNo(0),
    _taxCommentsTextTblNo(0),
    _taxSplInstructionsTblNo(0),
    _versionNbr(0)
{
}

std::ostream&
TaxReportingRecordInfo::print(std::ostream& out) const
{
  out << "\nVENDOR:         " << _vendor;
  out << "\nNATION:         " << _nationCode;
  out << "\nTAXCARRIER:     " << _taxCarrier;
  out << "\nTAXCODE:        " << _taxCode;
  out << "\nTAXTYPE:        " << _taxType;
  out << "\nCREATEDATE:     " << _createDate;
  out << "\nEFFDATE:        " << _effDate;
  out << "\nDISCDATE:       " << _discDate;
  out << "\nLOCKDATE:       " << _lockDate;
  out << "\nEXPIREDATE:     " << _expireDate;
  out << "\nLASTMODDATE:    " << _lastModificationDate;
  out << "\nTAXCHARGE:      " << _taxCharge;
  out << "\nACCOUNTABLEDOCTAXTAG:     " << _accountableDocTaxTag;
  out << "\nREPORTINGTEXTTBLNO:       " << _reportingTextTblNo;
  out << "\nINTERLINEABLETAXTAG:      " << _interlineAbleTaxTag;
  out << "\nREFUNDABLETAXTAG:         " << _refundableTaxTag;
  out << "\nCOMMISSIONABLETAXTAG:     " << _commisionableTaxTag;
  out << "\nVATIND:  " << _vatInd;
  out << "\nTAXNAME: " << _taxName;
  out << "\nTAXTEXTITEMNO:            " << _taxTextItemNo;
  out << "\nTAXAPPLICABLETOTBLNO:     " << _taxApplicableToTblNo;
  out << "\nTAXRATETEXTTBLNO:         " << _taxRateTextTblNo;
  out << "\nTAXEXEMPTIONTEXTTBLNO:    " << _taxExemptionTextTblNo;
  out << "\nTAXCOLLECTNREMITTBLNO:    " << _taxCollectNrEmmitTblNo;
  out << "\nTAXINGAUTHORITYTEXTTBLNO: " << _taxingAuthorityTextTblNo;
  out << "\nTAXCOMMENTSTEXTTBLNO:     " << _taxCommentsTextTblNo;
  out << "\nTAXSPLINSTRUCTIONSTBLNO:  " << _taxSplInstructionsTblNo;
  out << "\nVERSIONNBR:               " << _versionNbr;
  out << "\n";
  return out;
}

bool
TaxReportingRecordInfo::
operator<(const TaxReportingRecordInfo& rhs) const
{
  if (_vendor != rhs._vendor)
    return (_vendor < rhs._vendor);
  if (_nationCode != rhs._nationCode)
    return (_nationCode < rhs._nationCode);
  if (_taxCarrier != rhs._taxCarrier)
    return (_taxCarrier < rhs._taxCarrier);
  if (_taxCode != rhs._taxCode)
    return (_taxCode < rhs._taxCode);
  if (_taxType != rhs._taxType)
    return (_taxType < rhs._taxType);
  if (_createDate != rhs._createDate)
    return (_createDate < rhs._createDate);
  if (_effDate != rhs._effDate)
    return (_effDate < rhs._effDate);
  if (_discDate != rhs._discDate)
    return (_discDate < rhs._discDate);
  if (_lockDate != rhs._lockDate)
    return (_lockDate < rhs._lockDate);
  if (_expireDate != rhs._expireDate)
    return (_expireDate < rhs._expireDate);
  if (_lastModificationDate != rhs._lastModificationDate)
    return (_lastModificationDate < rhs._lastModificationDate);
  if (_taxCharge != rhs._taxCharge)
    return (_taxCharge < rhs._taxCharge);
  if (_accountableDocTaxTag != rhs._accountableDocTaxTag)
    return (_accountableDocTaxTag < rhs._accountableDocTaxTag);
  if (_reportingTextTblNo != rhs._reportingTextTblNo)
    return (_reportingTextTblNo < rhs._reportingTextTblNo);
  if (_interlineAbleTaxTag != rhs._interlineAbleTaxTag)
    return (_interlineAbleTaxTag < rhs._interlineAbleTaxTag);
  if (_refundableTaxTag != rhs._refundableTaxTag)
    return (_refundableTaxTag < rhs._refundableTaxTag);
  if (_commisionableTaxTag != rhs._commisionableTaxTag)
    return (_commisionableTaxTag < rhs._commisionableTaxTag);
  if (_vatInd != rhs._vatInd)
    return (_vatInd < rhs._vatInd);
  if (_taxName != rhs._taxName)
    return (_taxName < rhs._taxName);
  if (_taxTextItemNo != rhs._taxTextItemNo)
    return (_taxTextItemNo < rhs._taxTextItemNo);
  if (_taxApplicableToTblNo != rhs._taxApplicableToTblNo)
    return (_taxApplicableToTblNo < rhs._taxApplicableToTblNo);
  if (_taxRateTextTblNo != rhs._taxRateTextTblNo)
    return (_taxRateTextTblNo < rhs._taxRateTextTblNo);
  if (_taxExemptionTextTblNo != rhs._taxExemptionTextTblNo)
    return (_taxExemptionTextTblNo < rhs._taxExemptionTextTblNo);
  if (_taxCollectNrEmmitTblNo != rhs._taxCollectNrEmmitTblNo)
    return (_taxCollectNrEmmitTblNo < rhs._taxCollectNrEmmitTblNo);
  if (_taxingAuthorityTextTblNo != rhs._taxingAuthorityTextTblNo)
    return (_taxingAuthorityTextTblNo < rhs._taxingAuthorityTextTblNo);
  if (_taxCommentsTextTblNo != rhs._taxCommentsTextTblNo)
    return (_taxCommentsTextTblNo < rhs._taxCommentsTextTblNo);
  if (_taxSplInstructionsTblNo != rhs._taxSplInstructionsTblNo)
    return (_taxSplInstructionsTblNo < rhs._taxSplInstructionsTblNo);
  if (_versionNbr != rhs._versionNbr)
    return (_versionNbr < rhs._versionNbr);
  return false;
}

bool
TaxReportingRecordInfo::
operator==(const TaxReportingRecordInfo& rhs) const
{
  return ((_vendor == rhs._vendor) && (_nationCode == rhs._nationCode) &&
          (_taxCarrier == rhs._taxCarrier) && (_taxCode == rhs._taxCode) &&
          (_taxType == rhs._taxType) && (_createDate == rhs._createDate) &&
          (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
          (_lockDate == rhs._lockDate) && (_expireDate == rhs._expireDate) &&
          (_lastModificationDate == rhs._lastModificationDate) && (_taxCharge == rhs._taxCharge) &&
          (_accountableDocTaxTag == rhs._accountableDocTaxTag) &&
          (_reportingTextTblNo == rhs._reportingTextTblNo) &&
          (_interlineAbleTaxTag == rhs._interlineAbleTaxTag) &&
          (_refundableTaxTag == rhs._refundableTaxTag) &&
          (_commisionableTaxTag == rhs._commisionableTaxTag) && (_vatInd == rhs._vatInd) &&
          (_taxName == rhs._taxName) && (_taxTextItemNo == rhs._taxTextItemNo) &&
          (_taxApplicableToTblNo == rhs._taxApplicableToTblNo) &&
          (_taxRateTextTblNo == rhs._taxRateTextTblNo) &&
          (_taxExemptionTextTblNo == rhs._taxExemptionTextTblNo) &&
          (_taxCollectNrEmmitTblNo == rhs._taxCollectNrEmmitTblNo) &&
          (_taxingAuthorityTextTblNo == rhs._taxingAuthorityTextTblNo) &&
          (_taxCommentsTextTblNo == rhs._taxCommentsTextTblNo) &&
          (_taxSplInstructionsTblNo == rhs._taxSplInstructionsTblNo) &&
          (_versionNbr == rhs._versionNbr));
}

void
TaxReportingRecordInfo::dummyData(TaxReportingRecordInfo& obj)
{
  obj._vendor = "aaaa";
  obj._nationCode = "aa";
  obj._taxCarrier = "aaa";
  obj._taxCode = "aa";
  obj._taxType = "aaa";
  obj._createDate = time(nullptr);
  obj._effDate = time(nullptr);
  obj._discDate = time(nullptr);
  obj._lockDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._lastModificationDate = time(nullptr);
  obj._taxCharge = 'a';
  obj._accountableDocTaxTag = 'a';
  obj._reportingTextTblNo = 1;
  obj._interlineAbleTaxTag = 'a';
  obj._refundableTaxTag = 'a';
  obj._commisionableTaxTag = 'a';
  obj._vatInd = 'a';
  obj._taxName = "name";
  obj._taxTextItemNo = 1;
  obj._taxApplicableToTblNo = 1;
  obj._taxRateTextTblNo = 1;
  obj._taxExemptionTextTblNo = 1;
  obj._taxCollectNrEmmitTblNo = 1;
  obj._taxingAuthorityTextTblNo = 1;
  obj._taxCommentsTextTblNo = 1;
  obj._taxSplInstructionsTblNo = 1;
  obj._versionNbr = 1;
}

void
TaxReportingRecordInfo::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _nationCode);
  FLATTENIZE(archive, _taxCarrier);
  FLATTENIZE(archive, _taxCode);
  FLATTENIZE(archive, _taxType);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _lockDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _lastModificationDate);
  FLATTENIZE(archive, _taxCharge);
  FLATTENIZE(archive, _accountableDocTaxTag);
  FLATTENIZE(archive, _reportingTextTblNo);
  FLATTENIZE(archive, _interlineAbleTaxTag);
  FLATTENIZE(archive, _refundableTaxTag);
  FLATTENIZE(archive, _commisionableTaxTag);
  FLATTENIZE(archive, _vatInd);
  FLATTENIZE(archive, _taxName);
  FLATTENIZE(archive, _taxTextItemNo);
  FLATTENIZE(archive, _taxApplicableToTblNo);
  FLATTENIZE(archive, _taxRateTextTblNo);
  FLATTENIZE(archive, _taxExemptionTextTblNo);
  FLATTENIZE(archive, _taxCollectNrEmmitTblNo);
  FLATTENIZE(archive, _taxingAuthorityTextTblNo);
  FLATTENIZE(archive, _taxCommentsTextTblNo);
  FLATTENIZE(archive, _taxSplInstructionsTblNo);
  FLATTENIZE(archive, _versionNbr);
}

} // tse
