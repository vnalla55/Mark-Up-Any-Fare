//----------------------------------------------------------------------------
//  (c) 2013, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//  and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//  or transfer of this software/documentation, in any medium, or incorporation of this
//  software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class TaxReportingRecordInfo
{
  friend inline std::ostream& dumpObject(std::ostream& os, const TaxReportingRecordInfo& obj);
public:
  typedef Code<3> TaxType;
  typedef Indicator TaxCharge;
  typedef Indicator AccountableDocTaxTag;
  typedef int ReportingTextTblNo;
  typedef Indicator InterlineAbleTaxTag;
  typedef Indicator RefundableTaxTag;
  typedef Indicator CommisionableTaxTag;
  typedef Indicator VatInd;
  typedef std::string TaxName;
  typedef int TaxTextItemNo;
  typedef int TaxApplicableToTblNo;
  typedef int TaxRateTextTblNo;
  typedef int TaxExemptionTextTblNo;
  typedef int TaxCollectNrEmmitTblNo;
  typedef int TaxingAuthorityTextTblNo;
  typedef int TaxCommentsTextTblNo;
  typedef int TaxSplInstructionsTblNo;

  TaxReportingRecordInfo();

  std::ostream& print(std::ostream& out) const;

  bool operator<(const TaxReportingRecordInfo& rhs) const;

  bool operator==(const TaxReportingRecordInfo& rhs) const;

  static void dummyData(TaxReportingRecordInfo& obj);

  void flattenize(Flattenizable::Archive& archive);

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  NationCode& nationCode() { return _nationCode; }
  const NationCode& nationCode() const { return _nationCode; }

  CarrierCode& taxCarrier() { return _taxCarrier; }
  const CarrierCode& taxCarrier() const { return _taxCarrier; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  TaxType& taxType() { return _taxType; }
  const TaxType& taxType() const { return _taxType; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lastModificationDate() { return _lastModificationDate; }
  const DateTime& lastModificationDate() const { return _lastModificationDate; }

  TaxCharge& taxCharge() { return _taxCharge; }
  const TaxCharge& taxCharge() const { return _taxCharge; }

  AccountableDocTaxTag& accountableDocTaxTag() { return _accountableDocTaxTag; }
  const AccountableDocTaxTag& accountableDocTaxTag() const { return _accountableDocTaxTag; }

  ReportingTextTblNo& reportingTextTblNo() { return _reportingTextTblNo; }
  const ReportingTextTblNo& reportingTextTblNo() const { return _reportingTextTblNo; }

  InterlineAbleTaxTag& interlineAbleTaxTag() { return _interlineAbleTaxTag; }
  const InterlineAbleTaxTag& interlineAbleTaxTag() const { return _interlineAbleTaxTag; }

  RefundableTaxTag& refundableTaxTag() { return _refundableTaxTag; }
  const RefundableTaxTag& refundableTaxTag() const { return _refundableTaxTag; }

  CommisionableTaxTag& commisionableTaxTag() { return _commisionableTaxTag; }
  const CommisionableTaxTag& commisionableTaxTag() const { return _commisionableTaxTag; }

  VatInd& vatInd() { return _vatInd; }
  const VatInd& vatInd() const { return _vatInd; }

  TaxName& taxName() { return _taxName; }
  const TaxName& taxName() const { return _taxName; }

  TaxTextItemNo& taxTextItemNo() { return _taxTextItemNo; }
  const TaxTextItemNo& taxTextItemNo() const { return _taxTextItemNo; }

  TaxApplicableToTblNo& taxApplicableToTblNo() { return _taxApplicableToTblNo; }
  const TaxApplicableToTblNo& taxApplicableToTblNo() const { return _taxApplicableToTblNo; }

  TaxRateTextTblNo& taxRateTextTblNo() { return _taxRateTextTblNo; }
  const TaxRateTextTblNo& taxRateTextTblNo() const { return _taxRateTextTblNo; }

  TaxExemptionTextTblNo& taxExemptionTextTblNo() { return _taxExemptionTextTblNo; }
  const TaxExemptionTextTblNo& taxExemptionTextTblNo() const { return _taxExemptionTextTblNo; }

  TaxCollectNrEmmitTblNo& taxCollectNrEmmitTblNo() { return _taxCollectNrEmmitTblNo; }
  const TaxCollectNrEmmitTblNo& taxCollectNrEmmitTblNo() const { return _taxCollectNrEmmitTblNo; }

  TaxingAuthorityTextTblNo& taxingAuthorityTextTblNo() { return _taxingAuthorityTextTblNo; }
  const TaxingAuthorityTextTblNo& taxingAuthorityTextTblNo() const
  {
    return _taxingAuthorityTextTblNo;
  }

  TaxCommentsTextTblNo& taxCommentsTextTblNo() { return _taxCommentsTextTblNo; }
  const TaxCommentsTextTblNo& taxCommentsTextTblNo() const { return _taxCommentsTextTblNo; }

  TaxSplInstructionsTblNo& taxSplInstructionsTblNo() { return _taxSplInstructionsTblNo; }
  const TaxSplInstructionsTblNo& taxSplInstructionsTblNo() const
  {
    return _taxSplInstructionsTblNo;
  }

  int& versionNbr() { return _versionNbr; }
  const int& versionNbr() const { return _versionNbr; }

private:
  TaxReportingRecordInfo(const TaxReportingRecordInfo&);
  TaxReportingRecordInfo& operator=(const TaxReportingRecordInfo&);

  VendorCode _vendor;
  NationCode _nationCode;
  CarrierCode _taxCarrier;
  TaxCode _taxCode;
  TaxType _taxType;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _lockDate;
  DateTime _expireDate;
  DateTime _lastModificationDate;
  TaxCharge _taxCharge;
  AccountableDocTaxTag _accountableDocTaxTag;
  ReportingTextTblNo _reportingTextTblNo;
  InterlineAbleTaxTag _interlineAbleTaxTag;
  RefundableTaxTag _refundableTaxTag;
  CommisionableTaxTag _commisionableTaxTag;
  VatInd _vatInd;
  TaxName _taxName;
  TaxTextItemNo _taxTextItemNo;
  TaxApplicableToTblNo _taxApplicableToTblNo;
  TaxRateTextTblNo _taxRateTextTblNo;
  TaxExemptionTextTblNo _taxExemptionTextTblNo;
  TaxCollectNrEmmitTblNo _taxCollectNrEmmitTblNo;
  TaxingAuthorityTextTblNo _taxingAuthorityTextTblNo;
  TaxCommentsTextTblNo _taxCommentsTextTblNo;
  TaxSplInstructionsTblNo _taxSplInstructionsTblNo;
  int _versionNbr;

};

} // tse

