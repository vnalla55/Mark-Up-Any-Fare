//----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Common/Money.h"
#include "Common/XMLConstruct.h"
#include "DataModel/TktFeesPricingTrx.h"
#include "DataModel/TktFeesRequest.h"
#include "Xform/CustomXMLParser/IAttributes.h"
#include "Xform/PricingResponseFormatter.h"


#include <string>

static const std::string OBTicketingFeeRS = "OBTicketingFeeRS";
static const std::string _version = "1.0.0";

namespace tse
{

class STLTktFeesPricingResponseFormatter : public PricingResponseFormatter
{

public:
  static const MoneyAmount INVALID_AMT;
  virtual std::string formatResponse(
      const std::string&,
      TktFeesPricingTrx&,
      const ErrorResponseException::ErrorResponseCode errCode = ErrorResponseException::NO_ERROR);
  void formatResponse(const ErrorResponseException& ere, std::string& response);

  virtual void formatTktFeesResponse(XMLConstruct&, TktFeesPricingTrx&);
  virtual void
  prepareResponseText(const std::string& r, XMLConstruct&, const bool noSizeLImit = false) const;
  void prepareMessage(XMLConstruct&, const std::string&, const uint16_t, const std::string&) const;

  MoneyAmount& totalAmount() { return _totalAmount; }
  const MoneyAmount& totalAmount() const { return _totalAmount; }

  bool& OBFeeOptionMaxLimitFound() { return _OBFeeOptionMaxLimitFound; }
  const bool& OBFeeOptionMaxLimitFound() const { return _OBFeeOptionMaxLimitFound; }

protected:
  Itin* _itin = nullptr;
  MoneyAmount _tktFeesAmount = 0;
  MoneyAmount _totalAmount = 0;
  bool _OBFeeOptionMaxLimitFound = false;

  CurrencyCode& getCurrencyCode(PricingTrx& pricingTrx, const Itin& currentItin) const;
  void checkLimitOBFees(PricingTrx& pricingTrx);
  const Itin* currentItin() const { return _itin; }
  Itin*& currentItin() { return _itin; }

  Money convertOBFeeCurrencyByCode(PricingTrx& pricingTrx,
                                   const CurrencyCode& equivCurrencyCode,
                                   const TicketingFeesInfo* feeInfo);
  void
  convertOBFeeCurrencyByMoney(PricingTrx& pricingTrx, const Money& sourceMoney, Money& targetMoney);
  void buildTktFeesResponse(TktFeesPricingTrx&, Itin&, XMLConstruct&);

  void prepareHostPortInfo(PricingTrx&, XMLConstruct&);
  void prepareOBFee(PricingTrx&, XMLConstruct&, const TicketingFeesInfo*,
                    const Itin*, const MoneyAmount& chargeAmt = INVALID_AMT);
  void processTktFeesSolution(TktFeesPricingTrx&, const Itin*, XMLConstruct&,
                              const FarePath&, const TktFeesRequest::PaxTypePayment* ptp);
  bool checkForZeroMaximum(PricingTrx& pricingTrx, FarePath& farePath) const;
  void clearAllFees(PricingTrx& pricingTrx) const;

  void
  processTktFeesSolutionPerPaxType(TktFeesPricingTrx&, const Itin*, XMLConstruct&, const FarePath&);
  void processPassengerIdentity(TktFeesPricingTrx& pricingTrx,
                                XMLConstruct& construct,
                                SequenceNumber sn);

  void processPaxType(TktFeesPricingTrx& pricingTrx,
                      XMLConstruct& construct,
                      TktFeesRequest::PaxTypePayment* ptp,
                      TktFeesRequest::PassengerPaymentInfo* ppi);

  void calculateObFeeAmountFromAmount(PricingTrx& pricingTrx,
                                      XMLConstruct& construct,
                                      const TicketingFeesInfo* feeInfo,
                                      MoneyAmount& totalFeeFareAmount,
                                      const CurrencyCode& equivCurrencyCode,
                                      const MoneyAmount& chargeAmount = INVALID_AMT);

  void calculateObFeeAmountFromAmountMax(PricingTrx& pricingTrx,
                                         const TicketingFeesInfo* feeInfo,
                                         MoneyAmount& feeAmount,
                                         const CurrencyCode& paymentCurrency);

  void calculateObFeeAmountFromPercentage(PricingTrx& pricingTrx,
                                          XMLConstruct& construct,
                                          const TicketingFeesInfo* feeInfo,
                                          MoneyAmount& totalFeeFareAmount,
                                          const CurrencyCode& equivCurrencyCode,
                                          const MoneyAmount& chargeAmount);

  std::pair<const TicketingFeesInfo*, MoneyAmount>
  computeMaximumOBFeesPercent(PricingTrx& pricingTrx, FarePath& farePath, Itin* itin);

  void calculateObFeeAmountFromPercentageMax(PricingTrx& pricingTrx,
                                             const TicketingFeesInfo* feeInfo,
                                             MoneyAmount& totalFeeFareAmount,
                                             const CurrencyCode& paymentCurrency,
                                             const MoneyAmount& chargeAmount);

  CurrencyCode getPaymentCurrency(PricingTrx& pricingTrx, const Itin* itin);

  bool getChargeAmount(const TktFeesRequest::PaxTypePayment* ptp, MoneyAmount& chargeAmount) const;

}; // End class STLTktFeesPricingResponseFormatter
} // End namespace tse
