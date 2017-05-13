#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "FareCalc/FcMessage.h"

#include <algorithm>

namespace tse
{
class PricingTrx;
class FareUsage;
class FareCalcCollector;
class FareCalcConfig;
class CalcTotals;
class PaxType;
class FareBreakPointInfo;
class FarePath;
class PricingUnit;
class TaxResponse;
class Itin;
class SurchargeData;
namespace FareCalc
{

class FcCollector
{
  friend class FcCollectorTest;

public:
  FcCollector(PricingTrx* trx,
              const FarePath* farePath,
              const FareCalcConfig* fcConfig,
              FareCalcCollector* fcCollector,
              CalcTotals* calcTotals);

  virtual ~FcCollector() = default;

  virtual void collect();

  virtual void operator()(const PricingUnit* pu);

  virtual void operator()(const FareUsage* fu);

  void collectFareTotals(const FareUsage* fu);
  void collectBookingCode(const FareUsage* fu);
  void collectDifferentials(const FareUsage* fu);
  void collectSurcharge(const FareUsage* fu);
  void collectStopOverSurcharge(const FareUsage* fu);
  void collectTransferSurcharge(const FareUsage* fu);
  void collectMileageSurcharge(const FareUsage* fu);
  void collectPvtFareIndicator(const FareUsage* fu);

  bool& needProcessPvtIndFromTJR() { return _needProcessPvtIndFromTJR; }
  const bool& needProcessPvtIndFromTJR() const { return _needProcessPvtIndFromTJR; }

  bool needProcessPvtIndicator();
  void collectPvtIndicatorMessage();

  void collectTaxTotals();

  void collectAccTvlInfo();

  void set(const PricingUnit* pu) { _pricingUnit = pu; }
  void set(const FareUsage* fu) { _prevFareUsage = fu; }

protected:
  bool convertAmounts();
  void convertNetChargeAmounts();
  void collectZpTaxInfo(const TaxResponse* taxResponse);
  bool collectTicketingTaxInfo(const TaxResponse* taxResponse);
  void collectBSR();

  virtual void collectMessage();
  void collectFareTypeMessage();
  void collectPtcMessage();
  void collectValidatingCarrierMessage();
  void collectCat15HasTextTable();
  void collectRetailerRuleMessage();

  std::string createDefaultTrailerMsg(
      const FarePath& fp,
      const SettlementPlanType& sp,
      const CarrierCode& defValCxr) const;

  std::string createAltTrailerMsg(
      const FarePath& fp,
      const SettlementPlanType& sp,
      const std::vector<CarrierCode>& mktCxrs,
      std::vector<CarrierCode>& cxrs) const;

  std::string createTrailerMsg(
      const SettlementPlanType& sp,
      std::vector<CarrierCode>& cxrs) const;

  bool isAcxrsAreNSPcxrs(const std::vector<CarrierCode>& v1, const std::vector<CarrierCode>& v2) const;
  std::string buildValidatingCarrierMessage(FcMessage::MessageType& msgType) const;
  void buildValidatingCarrierMessage(const FarePath& fp,
                                     const SettlementPlanType& sp,
                                     const CarrierCode& defaultVcxr,
                                     const std::vector<CarrierCode>& mktCxrs,
                                     const std::vector<CarrierCode>& valCxrs,
                                     std::vector<std::string>& def,
                                     std::vector<std::string>& alt,
                                     std::vector<std::string>& opt) const;
  void buildNSPValidatingCarrierMessage(const FarePath& ,
                                        const CarrierCode& defValCxr,
                                        const std::vector<CarrierCode>& valCxrs,
                                        std::string& nspHeader,
                                        std::vector<std::string>&) const;
  void prepareValCxrMsgForMultiSp() const;
  void collectServiceFeesTemplate();
  void collectMatchedAccCodeTrailerMessage();
  void correctSTOSurOverrideCnt();
  void collectBaggage();
  void collectMaxPenaltyMessage();

  void
  getNetPubFareAmountAndFbc(FareBreakPointInfo& fbpInfo, const FareUsage* fu, bool getFbc) const;
  void checkPerDirSurchargeConsecutive() const;
  bool determineCat15HasTextTable(const Itin& itin) const;
  void mergePerDirSurchargeConsecutive(const Itin* itin,
                                       std::vector<FareUsage*> fus,
                                       SurchargeData* sd) const;

  void processNSPVcxrMsg() const;
  void collectValidatingCarrierMessageForGSA() const;
  void collectValidatingCarrierMessageForNonGSA() const;
  void collectCommissionTrailerMessages() const;
  void constructAgencyTrailerMsg(
      const std::string& cxrStr,
      const std::string& text,
      MoneyAmount commAmt) const;
  std::string createAgencyCommTrailerMsg(
      const std::string& valCxr,
      const std::string& text,
      MoneyAmount commAmt) const;
  void setValCxrTrailerMessages(
      const std::string& header,
      const std::vector<std::string>& trailerMsgCol) const;
  bool isDefaultVcxrFromPreferred(const CarrierCode& defValCxr) const;

protected:
  static const unsigned int WINDOW_WIDTH = 63;

  PricingTrx* _trx = nullptr;
  const FarePath* _farePath = nullptr;
  const PricingUnit* _pricingUnit = nullptr;
  const FareUsage* _prevFareUsage = nullptr;

  const FareCalcConfig* _fcConfig = nullptr;
  FareCalcCollector* _fcCollector = nullptr;
  CalcTotals* _calcTotals = nullptr;

  CurrencyCode _lastBaseFareCurrencyCode;
  CurrencyNoDec _lastConvertedBaseFareNoDec = 0;

  ExchRate _firstROE = 0;
  CurrencyNoDec _firstROENoDec = 0;
  bool _needProcessPvtIndFromTJR = false;

private:
  bool requestAndTruePaxTypeSame();
  void constructCommTrailerMsgForDefaultValCxr(
      const FarePath& fp,
      const CarrierCode& defValCxr,
      const std::string& text) const;
  void constructCommTrailerMsgForAlternateValCxr(
      const FarePath& fp,
      const std::string& text) const;
  void createCommNotFoundTrailerMsg(const CarrierCode& cxr,
      const std::string& text,
      std::string& msg) const
  {
    if (msg.empty() && !cxr.empty())
    {
      msg = "AGENCY COMMISSION DATA NOT FOUND FOR";
      if (!text.empty())
      {
        msg += " ";
        msg += text;
      }
      msg += " VAL CXR ";
      msg += cxr;
    }
    else if (!cxr.empty())
    {
      msg += "/";
      msg += cxr;
    }
  }
};

} // FareCalc
} // tse
