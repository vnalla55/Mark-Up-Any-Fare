//-------------------------------------------------------------------------------
//  CalcTotals.h
//
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TaxResponse.h"
#include "FareCalc/FcMessage.h"
#include "FareCalc/FcTaxInfo.h"
#include "FareCalc/FormattedFareCalcLine.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"

#include <map>
#include <set>
#include <vector>

namespace tse
{
class MileageTypeData;
class DifferentialData;
class FareCalcConfig;
class FarePath;
class PaxType;
class PaxTypeFare;
class PfcItem;
class PricingUnit;
class SurchargeData;

struct FareBreakPointInfo
{
  MoneyAmount fareAmount = 0;
  std::string fareBasisCode;

  // Net Remit Retrieved Fare Info
  MoneyAmount netPubFareAmount = 0;
  std::string netPubFbc;
};

struct SurchargeInfo
{
  int16_t count = 0;
  MoneyAmount total = 0;
  std::string fcDispStr;
  std::set<CurrencyCode> pubCurrency;
};

struct AdjustedSellingDiffInfo
{
  std::string description;
  std::string typeCode;
  std::string amount;

  AdjustedSellingDiffInfo(const std::string& d, const std::string& t, const std::string& a) :
    description(d), typeCode(t), amount(a) {}
};

struct CalcTotals
{
public:
  const FarePath* farePath = nullptr;
  const FareCalcConfig* fcConfig = nullptr;
  CalcTotals* netRemitCalcTotals = nullptr;
  CalcTotals* netCalcTotals = nullptr;
  CalcTotals* adjustedCalcTotal = nullptr;
  PaxTypeCode truePaxType;
  PaxTypeCode requestedPaxType;
  bool mixedPaxType = false;
  bool negFareUsed = false;
  bool privateFareUsed = false;
  bool allPaxTypeInSabreGroup = false;

  std::string fclROE;
  std::string fclBSR;
  std::string fclZP;
  std::string fclXT;
  std::string fclXF;
  std::string fareCalculationLine;
  std::string netRemitFareCalcLine;
  std::string lastTicketDay;
  std::string lastTicketTime;
  bool simultaneousResTkt = false;

  std::vector<std::string> warningMsgs;

  const std::string& getFormattedFareCalcLine() { return formattedFareCalcLine.value(); }

  FormattedFareCalcLine formattedFareCalcLine;

  // Fare Component (FareUsage) summary information
  struct FareCompInfo
  {
    bool fareType = false; // false = normal, true = special fare

    MoneyAmount fareAmount = 0;
    CurrencyNoDec fareNoDec = 0;
    CurrencyCode fareCurrencyCode;
    MoneyAmount pfcAmount = 0;

    bool isDiscounted = false;
    MoneyAmount discountPercentage = 0;
    std::string discountCode;

    VendorCode vendor;
    FareClassCode fareBasisCode;
    GlobalDirection globalDirection = GlobalDirection::XX;
    CabinType fareCabin;
  };

  struct FarePathInfo
  {
    // FarePath totals:
    MoneyAmount nucFareAmount = 0;
    CurrencyNoDec fareNoDec = 0;
    CurrencyCode fareCurrencyCode;

    MoneyAmount fareAmount = 0;
    MoneyAmount pfcAmount = 0;

    MoneyAmount commissionAmount = 0;
    MoneyAmount commissionPercent = 0;

    std::string IATASalesCode;

    PaxTypeFare::PaxTypeFareCPFailedStatus cpFailedStatus;

    // TODO: turn this into a vector with the FU stored in the direction of Itin travel,
    // that would simplify a lot of code:
    std::map<const FareUsage*, FareCompInfo> fareCompInfo;
  } farePathInfo;

  // Fare usage
  bool useNUC = false;
  bool fareType = false; // false = normal and true = special fare
  bool fclToLong = false;
  MoneyAmount pfcAmount = 0;
  MoneyAmount fareAmount = 0;
  CurrencyCode fareCurrencyCode;

  // Converted amounts
  MoneyAmount convertedBaseFare = 0;
  CurrencyNoDec convertedBaseFareNoDec = 0;
  CurrencyCode convertedBaseFareCurrencyCode;
  MoneyAmount equivFareAmount = 0;
  CurrencyNoDec equivNoDec = 0;
  CurrencyCode equivCurrencyCode;
  CurrencyCode interCurrencyCode;
  MoneyAmount intermediateAmount = 0;
  CurrencyNoDec calcCurrencyNoDec = 0;
  Money effectiveDeviation{INVALID_CURRENCYCODE};
  // NoDec (tempDec) setting used to display various amount on FCL line:
  int16_t fclNoDec = 0;

  // Exchange rate
  ExchRate roeRate = 0;
  CurrencyNoDec roeRateNoDec = 0;
  ExchRate bsrRate1 = 0;
  CurrencyNoDec bsrRate1NoDec = 0;
  ExchRate bsrRate2 = 0;
  CurrencyNoDec bsrRate2NoDec = 0;
  DateTime effectiveDate;
  DateTime discontinueDate;

  // Good differentials
  std::vector<DifferentialData*> differentialData;
  std::map<const TravelSeg*, std::string> differentialFareBasis;

  // unique FOP warning MSG's
  std::vector<std::string> warningFopMsgs;

  // Transfer surcharge
  SurchargeInfo transferSurcharge;

  // StopOver surcharge
  SurchargeInfo stopOverSurcharge;
  SurchargeInfo stopOverSurchargeByOverride;

  std::vector<AdjustedSellingDiffInfo> adjustedSellingDiffInfo;

  bool isDiscounted = false;
  std::string discountCode;
  MoneyAmount discountPercentage = 0;
  MoneyAmount commissionAmount = 0;
  MoneyAmount commissionPercent = 0;

  VendorCode vendor;

  MoneyAmount taxAmount() const { return _fcTaxInfo.taxAmount(); }
  void setTaxAmount(const MoneyAmount& money) { _fcTaxInfo.taxAmount() = money; }
  int taxNoDec() const { return _fcTaxInfo.taxNoDec(); }
  CurrencyCode taxCurrencyCode() const { return _fcTaxInfo.taxCurrencyCode(); }
  MoneyAmount taxOverride() const { return _fcTaxInfo.taxOverride(); }
  bool dispSegmentFeeMsg() const { return _fcTaxInfo.dispSegmentFeeMsg(); }
  bool isCurrEquivEqCnvBase() const { return equivCurrencyCode == convertedBaseFareCurrencyCode; }

  const TaxResponse* taxResponse() const { return _fcTaxInfo.taxResponse(); }

  const std::vector<TaxRecord*>& taxRecords() const { return _fcTaxInfo.taxRecords(); }
  const std::vector<TaxItem*>& taxItems() const { return _fcTaxInfo.taxItems(); }
  const std::vector<TaxItem*>& changeFeeTaxItems() const { return _fcTaxInfo.changeFeeTaxItems(); }
  const std::vector<PfcItem*>& pfcItems() const { return _fcTaxInfo.pfcItems(); }

  const std::vector<std::string>& publishedZpTaxInfo() const
  {
    return _fcTaxInfo.publishedZpTaxInfo();
  }
  const std::vector<std::string>& zpTaxInfo() const { return _fcTaxInfo.zpTaxInfo(); }
  const std::vector<std::string>& xfTaxInfo() const { return _fcTaxInfo.xfTaxInfo(); }
  const std::vector<std::string>& xtTaxInfo() const { return _fcTaxInfo.xtTaxInfo(); }

  // TE TX TN Tax Exemptions
  const std::set<TaxCode>& getTaxExemptCodes() const { return _fcTaxInfo.getTaxExemptCodes(); }

  std::vector<std::string> bookingCodeRebook;

  std::vector<PaxTypeFare::BkgCodeSegStatus> bkgCodeSegStatus;

  std::vector<std::string> ticketFareInfo;
  std::vector<std::string> ticketOrigin;
  std::vector<std::string> ticketDestination;
  std::vector<std::string> ticketFareAmount;
  std::vector<std::string> ticketFareBasisCode;

  // std::vector<GlobalDirection> globalDirections;

  // Ordered travel segs
  std::map<uint16_t, TravelSeg*, std::less<uint16_t>> travelSegs;

  // Connect ordered travel segs to their parent FU
  std::map<const TravelSeg*, const FareUsage*> fareUsages;

  // Connect FU to parent PU
  std::map<const FareUsage*, const PricingUnit*> pricingUnits;

  // Restriction code and appendage code
  std::map<const PricingUnit*, Indicator> maxRestCode;
  std::map<const PricingUnit*, AppendageCode> appendageCode;

  // Mileage travel segs
  std::map<const TravelSeg*, int16_t> mileageTravelSegs;

  // Extra Mileage Allowance
  std::set<const TravelSeg*> extraMileageTravelSegs;
  std::set<const FareUsage*> extraMileageFareUsages;

  // Fare break point information
  std::map<const FareUsage*, FareBreakPointInfo> fareBreakPointInfo;

  // Surcharges
  std::map<const TravelSeg*, std::vector<SurchargeData*>> surcharges;
  std::map<const TravelSeg*, std::vector<const FareUsage::StopoverSurcharge*>> stopoverSurcharges;
  std::map<const TravelSeg*, std::vector<const FareUsage::TransferSurcharge*>> transferSurcharges;

  // NVA/NVB
  std::map<int16_t, DateTime> tvlSegNVA; // nva date for travel segments
  std::map<int16_t, DateTime> tvlSegNVB; // nva date for travel segments

  std::vector<FcMessage> fcMessage;

  // fare calc methods
  bool getDispConnectionInd(const PricingTrx& trx,
                            const TravelSeg* travelSeg,
                            Indicator connectionInd) const;

  bool isConnectionPoint(const TravelSeg* travelSeg, const FareUsage* fareUsage = nullptr) const;

  std::string getFareBasisCode(PricingTrx& trx,
                               const TravelSeg* travelSeg,
                               char tktDesLength,
                               char childInfantCode,
                               std::string::size_type maxLenAll = 15,
                               std::string::size_type maxLenFB = 10) const;

  bool isPointToPoint() const;

  // convenience methods
  void static getNetRemitFareUsage(const FarePath* origFp,
                                   const FareUsage* origFu,
                                   FareUsage*& netRemitFu1,
                                   FareUsage*& netRemitFu2);

  const FareUsage* getNetFareUsage(const FarePath* origFp, const FareUsage* origFu);

  const FareUsage* getFareUsage(const TravelSeg* travelSeg) const;
  const FareUsage* getFareUsage(const TravelSeg* travelSeg, uint16_t& tvlIndex) const;

  const FareBreakPointInfo* getFareBreakPointInfo(const FareUsage* fareUsage) const;
  FareBreakPointInfo& getFareBreakPointInfo(const FareUsage* fareUsage);
  std::string getDifferentialFbc(const TravelSeg* travelSeg) const;

  // Sets number of stopover charges and the total charge, returning true if any
  // stopovers charges are present.
  bool getStopoverSummary(uint16_t& stopoverCount,
                          MoneyAmount& stopoverCharges,
                          CurrencyCode& pubCurr) const;
  bool getTransferSummary(uint16_t& transferCount,
                          MoneyAmount& transferCharges,
                          CurrencyCode& pubCurr) const;

  MoneyAmount getTotalAmountPerPax() const;

  MoneyAmount getTotalFareAmount(const CurrencyCode& currencyOverride) const;

  const Money& getEffectivePriceDeviation() const { return effectiveDeviation; }

  struct WpaInfo
  {
    // WPA Passenger Detail Reference Number
    int psgDetailRefNo = 1;

    bool verifyBooking = false;

    // WPn Passenger Detail Response
    std::string wpnDetailResponse;

    // WPA related trailer message
    std::string trailerMsg;

    // Data for possible accompanied travel restriction
    bool reqAccTvl = false;
    bool tktGuaranteed = true;
    std::string accTvlTrailerMsg;
    std::string accTvlData; // sent back to PSS for WTFR
  } wpaInfo;

  bool roundBaseFare = true;
  MoneyAmount totalNetCharges = 0; // cat35 cat 8,9,12 total in NUC

  std::vector<MileageTypeData*>& mileageTypeData() { return _mileageTypeData; }
  const std::vector<MileageTypeData*>& mileageTypeData() const { return _mileageTypeData; }

  int getTotalMileage() const;

  uint16_t privateFareIndSeq = 0;

  const FareCalc::FcTaxInfo& getFcTaxInfo() const { return _fcTaxInfo; }

  FareCalc::FcTaxInfo& getMutableFcTaxInfo() { return _fcTaxInfo; }

private:
  std::vector<MileageTypeData*> _mileageTypeData;
  FareCalc::FcTaxInfo _fcTaxInfo;
};

} // namespace tse
