//----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/TktFeesRequest.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcCollector.h"

#include <vector>

namespace tse
{
FALLBACK_DECL(obFeesWPAforAbacus);
FALLBACK_DECL(obFeesWPAforInfini);

class AltPricingDetailObFeesTrx;
class PricingDetailTrx;

namespace OBFeesUtils
{

enum OBFeesFailStatus : char
{
  OB_FEES_NOT_REQUESTED = 'N',
  ALL_SEGMENTS_OPEN = 'O',
  NOT_ALL_SEGS_CONFIRM = 'U'
};

const std::string ObfSectionStart = "START OF OBF SECTION\n";
const std::string ObfSectionEnd = "END OF OBF SECTION\n";

inline bool
fallbackObFeesWPA(const PricingTrx* trx)
{
  return (fallback::obFeesWPAforAbacus(trx) && trx->getRequest()->ticketingAgent() != nullptr &&
          trx->getRequest()->ticketingAgent()->abacusUser()) ||
         (fallback::obFeesWPAforInfini(trx) && trx->getRequest()->ticketingAgent() != nullptr &&
          trx->getRequest()->ticketingAgent()->infiniUser());
}

int
getLastIdx(std::string response);

std::string
convertIdxToString(const int idx, const int width = 6, const char fill = '0');

std::string
formatBinNumber(const FopBinNumber& fopBinNumber);

std::string
prepareHeaderObFeeMsg();

std::string
prepareColumnsNamesObFeeMsg(const PaxTypeCode& paxTypeCode);

std::string
wrapObFeeMsgLine(const std::string& line, int msgIdx);

std::string
prepareTotalFeeAmout(const MoneyAmount& totalFeeAmount, const CurrencyNoDec& noDec);

std::string
prepareObFeeMsg2CC(TicketingFeesInfo* feeInfo,
                   const MoneyAmount& feeAmount,
                   const CurrencyNoDec& noDec);

std::string
prepareObFeeMsg(PricingTrx& pricingTrx,
                const CalcTotals& calcTotals,
                TicketingFeesInfo* feeInfo,
                bool limitMaxOBFees,
                MoneyAmount feeAmt = 0.0,
                MoneyAmount fareAmtWith2CCFee = 0.0);

void
calculateObFeeAmount(PricingTrx& pricingTrx,
                     const CalcTotals& calcTotals,
                     const TicketingFeesInfo* feeInfo,
                     MoneyAmount& totalFeeFareAmount,
                     CurrencyNoDec& noDec,
                     bool limitMaxOBFees,
                     const MoneyAmount& chargeAmount = 0.0);

void
calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                               const CalcTotals& calcTotals,
                               const TicketingFeesInfo* feeInfo,
                               MoneyAmount& totalFeeFareAmount,
                               CurrencyNoDec& noDec);

void
calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                   const CalcTotals& calcTotals,
                                   const TicketingFeesInfo* feeInfo,
                                   MoneyAmount& totalFeeFareAmount,
                                   CurrencyNoDec& noDec,
                                   bool limitMaxOBFees,
                                   const MoneyAmount& chargeAmount);

void
calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                   const CalcTotals& calcTotals,
                                   const TicketingFeesInfo* feeInfo,
                                   Money& targetMoney,
                                   MoneyAmount& totalFeeFareAmount,
                                   const MoneyAmount& chargeAmount);

MoneyAmount
calculateResidualObFeeAmount(PricingTrx& pricingTrx,
                             const MoneyAmount& totalPaxAmount,
                             const TicketingFeesInfo* feeInfo);

MoneyAmount
getLowestObFeeAmount(const CurrencyCode& maxFeeCur,
                     const MoneyAmount& calcAmount,
                     const MoneyAmount& maxAmount);

Money
convertOBFeeCurrency(PricingTrx& pricingTrx,
                     const CalcTotals& calcTotals,
                     const TicketingFeesInfo* feeInfo);

void
convertOBFeeCurrency(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney);

void
roundOBFeeCurrency(PricingTrx& pricingTrx, Money& targetMoney);

bool
getFeeRounding(PricingTrx& pricingTrx,
               const CurrencyCode& currencyCode,
               RoundingFactor& roundingFactor,
               CurrencyNoDec& roundingNoDec,
               RoundingRule& roundingRule);

MoneyAmount
calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                  const CalcTotals& calcTotals,
                                  const TicketingFeesInfo* feeInfo);

MoneyAmount
calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                      const CalcTotals& calcTotals,
                                      const TicketingFeesInfo* feeInfo);

std::pair<const TicketingFeesInfo*, MoneyAmount>
computeMaximumOBFeesPercent(PricingTrx& pricingTrx, const CalcTotals& calcTotals);

void
prepareMessage(XMLConstruct& construct,
               const char msgType,
               const uint16_t msgCode,
               const std::string& msgText);

bool
hasMaxOBFeesOptionsVec(PricingTrx& pricingTrx);

void
clearAllFeesAndSetMaximum(const Itin& itin, const TicketingFeesInfo* maxFee);

bool
checkLimitOBFees(TktFeesPricingTrx& tktFeesPricingTrx, const Itin& itin);

bool
checkLimitOBFees(PricingTrx& pricingTrx, FareCalcCollector& fareCalcCollector);

bool
checkForZeroMaximum(PricingTrx& pricingTrx, FarePath& farePath);

std::pair<const TicketingFeesInfo*, MoneyAmount>
computeMaximumOBFeesPercent(PricingTrx& pricingTrx, FarePath& farePath, const Itin* itin);

void
calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                      const TicketingFeesInfo* feeInfo,
                                      MoneyAmount& totalFeeFareAmount,
                                      const CurrencyCode& paymentCurrency,
                                      const MoneyAmount& chargeAmount);

void
calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                  const TicketingFeesInfo* feeInfo,
                                  MoneyAmount& feeAmount,
                                  const CurrencyCode& paymentCurrency);

CurrencyCode
getPaymentCurrency(TktFeesPricingTrx& tktFeesPricingTrx, const Itin* itin);

Money
convertOBFeeCurrencyByCode(PricingTrx& pricingTrx,
                           const CurrencyCode& equivCurrencyCode,
                           const TicketingFeesInfo* feeInfo);

void
convertOBFeeCurrencyByMoney(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney);

bool
getChargeAmount(const TktFeesRequest::PaxTypePayment* ptp, MoneyAmount& chargeAmount);

const FarePath*
clearAllFeesAndGetLastPTC(const std::vector<CalcTotals*>& calcTotals);

bool
checkForZeroMaximum(PricingTrx& pricingTrx, const CalcTotals& calcTotals);

const TicketingFeesInfo*
mockOBFeeInPaymentCurrency(PricingTrx& pricingTrx,
                           const TicketingFeesInfo& sourceFee,
                           const MoneyAmount& feeAmount,
                           const CurrencyCode& paymentCurrency);

bool
isBinCorrect(const FopBinNumber& bin);

void
prepareOBFees(PricingTrx& pricingTrx,
              CalcTotals& calcTotals,
              XMLConstruct& construct,
              bool limitMaxOBFees);

void
prepareOBFee(PricingTrx& pricingTrx,
             CalcTotals& calcTotals,
             XMLConstruct& construct,
             const TicketingFeesInfo* feeInfo,
             bool limitMaxOBFees,
             MoneyAmount feeAmt = 0.0,
             MoneyAmount fareAmtWith2CCFee = 0.0);

void
prepareOBFee(PricingTrx& pricingTrx,
             CalcTotals& calcTotals,
             XMLConstruct& construct,
             TicketingFeesInfo* feeInfo,
             const FopBinNumber& fopBin,
             const MoneyAmount& chargeAmount,
             const CurrencyNoDec& numDec,
             const MoneyAmount& feeAmt,
             const MoneyAmount& fareAmtWith2CCFee);
void
prepareAllOBFees(PricingTrx& pricingTrx,
                 CalcTotals& calcTotals,
                 XMLConstruct& construct,
                 size_t maxOBFeesOptions,
                 bool limitMaxOBFees,
                 const std::vector<TicketingFeesInfo*>& collectedOBFees);

void
calculate2CardsOBFee(PricingTrx& pricingTrx, const CalcTotals& calcTotals,
                   const std::vector<TicketingFeesInfo*>& collectedOBFees,
                   MoneyAmount& firstCardChargeAmount, MoneyAmount& feeAmt1,
                   MoneyAmount& secondCardChargeAmount, MoneyAmount& feeAmt2,
                   MoneyAmount& totalFeeFareAmount, CurrencyNoDec& noDec);

void
prepare2CardsOBFee(PricingTrx& pricingTrx,
                   CalcTotals& calcTotals,
                   XMLConstruct& construct,
                   const std::vector<TicketingFeesInfo*>& collectedOBFees);

void
getNumberOfOBFees(PricingTrx& trx,
                  const FarePath& farePath,
                  size_t& maxNumFType,
                  size_t& maxNumTType,
                  size_t& maxNumRType);

void
prepareCalcTotalsForObFees(const PaxDetail& paxDetail,
                           const FarePath& farePath,
                           CalcTotals& calcTotals);

void
formatWpnObFeeResponseMaxOptions(const MoneyAmount& feeAmount, int& msgIdx, std::string& response);

bool
validatePaxTypeCode(PaxTypeCode farePathPaxType, PaxTypeCode paxDetailPaxType);

void
formatWpnObFeeResponse(PricingDetailTrx& pricingDetailTrx,
                       const PaxDetail& paxDetail,
                       const FarePath& farePath,
                       bool obFeesLimit,
                       int& msgIdx,
                       std::string& response);

void
addObFeesInfo(PricingDetailTrx& pricingDetailTrx,
              std::string& response,
              const PaxDetail* paxDetail,
              const Itin* itin);

void
addObFeeInfoWpan(AltPricingDetailObFeesTrx& trx);

std::string
displayGreenScreenMsg(PricingTrx& trx, const CalcTotals& calcTotals);

std::string
prepareMaxAmtMsg(const PricingTrx& trx, const TicketingFeesInfo& feeInfo);

void
setDefaultValidatingCxrForObFees(const PaxDetail& paxDetail, FarePath& farePath);
} // namespace OBFeesUtils


} // namespace tse
