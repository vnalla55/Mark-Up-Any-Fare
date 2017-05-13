//-------------------------------------------------------------------
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class PricingTrx;
class FareDisplayTrx;
class NoPNRPricingTrx;
class TravelSeg;
class Loc;
class Nation;
class Itin;
class PaxType;
class PaxTypeInfo;
class AirSeg;

class RequestXmlValidator
{
  friend class RequestXmlValidatorTest;

public:
  static constexpr int NO_PNR_FUTURE_DAY = 331;
  static constexpr int NO_PNR_FUTURE_DAY_362 = 362;
  static constexpr int BEYOND_MAXIMUM_HISTORICAL_DATE = 732;
  static const std::string RESIDENCY;
  static const std::string EMPLOYEE;
  static const std::string NATIONALITY;
  RequestXmlValidator();
  virtual ~RequestXmlValidator();
  void getAgentLocationAndCurrency(PricingTrx* trx);
  void validateCurrencyCode(PricingTrx* trx);
  void getOrigDestLoc(PricingTrx* trx, TravelSeg* travelSeg);
  void validateSaleTicketOverride(PricingTrx* trx);
  bool requestFromPo(PricingTrx* trx);
  void setTicketingDate(PricingTrx* trx, bool ticketDatePresent);
  void setBookingDate(PricingTrx* trx, TravelSeg* travelSeg);
  void setDepartureDate(PricingTrx* trx, TravelSeg* travelSeg);
  void validateShipRegistry(PricingTrx* trx);
  void validateDepartureDate(PricingTrx* trx, Itin* itin, TravelSeg* travelSeg, int i);
  void validateTicketingDate(PricingTrx* trx, Itin* itin);
  PaxType* getDefaultPaxType(PricingTrx* trx);
  void validatePassengerType(PricingTrx* trx, PaxType& paxType);
  void validateDuplicatePassengerType(PricingTrx* trx, const PaxType& paxType);
  void validatePassengerStatus(PricingTrx* trx,
                               const NationCode& countryCode,
                               const NationCode& stateRegionCode,
                               const std::string& currentCRC,
                               const NationCode& residency);

  void setMOverride(PricingTrx* trx);
  void validateETicketOptionStatus(PricingTrx* trx);
  void processMissingArunkSegForPO(PricingTrx* trx,
                                   Itin* itin,
                                   const TravelSeg* travelSeg,
                                   bool& isMissingArunkForPO);
  void setForcedStopoverForNoPnrPricing(Itin* itin); 
  void checkCominationPDOorPDRorXRSOptionalParameters(PricingTrx* trx);
  void validateFareQuoteCurrencyIndicator(FareDisplayTrx* trx);
  void checkItinBookingCodes(NoPNRPricingTrx& noPNRTrx);
  void validateItin(PricingTrx* trx, const Itin& itin);
  void validateItinForFlownSegments(const Itin& itin);
  void validatePassengerTypeWithAges(PaxType& paxType);
  void setOperatingCarrierCode(TravelSeg& travelSeg);
  void checkRequestCrossDependencies(PricingTrx* trx);
  void validateCxrOverride(PricingTrx* trx, Itin* itin);
  void validateFQDateRange(FareDisplayTrx* trx);
  void validateFQReturnDate(FareDisplayTrx* trx, AirSeg* airSeg);
  void validateMultiTicketRequest(PricingTrx* trx);
  void setUpCorrectCurrencyConversionRules(PricingTrx& trx);

private:
  void getSubscriberLocation(PricingTrx* trx);
  void getSubscriberCurrency(PricingTrx* trx);
  void getSubscriberCrsCode(PricingTrx* trx);
  void getIata(PricingTrx* trx);
  void getAirlinePartitionCurrency(PricingTrx* trx);
  void getAirlineCrsCode(PricingTrx* trx);
  bool entryFromSubscriber(PricingTrx* trx);
  virtual const Loc* getLocation(PricingTrx* trx, const LocCode& locCode);
  virtual void getLocationCurrency(PricingTrx* trx);
  virtual short getTimeDiff(PricingTrx* trx, const DateTime& time);
  virtual short getTimeDiff(const PricingTrx* trx, const LocCode& tvlLocT) const;
  void validateLoc(PricingTrx* trx, int16_t pnrSeg);
  std::string convertNumToString(int16_t pnrSeg);
  void validateCurrency(PricingTrx* trx, CurrencyCode& currencyCode);
  void validateSameOrigDest(PricingTrx* trx, TravelSeg* travelSeg);
  void validateShipRegistryCountry(PricingTrx* trx);
  void validateShipRegistryPaxType(PricingTrx* trx);
  virtual const Nation* getNation(PricingTrx* trx, const NationCode& nationCode);
  void validateTicketingDate(PricingTrx* trx, DateTime& tvlDate, int16_t i, bool ticketingDate);
  void handleDateErrorMessage(int16_t i, bool ticketingDate);
  virtual const PaxTypeInfo* getPaxType(PricingTrx* trx, const PaxTypeCode& paxType);
  bool isValidCountryCode(PricingTrx* trx, const NationCode& countryCode);
  bool validatePassengerStatusResidency(PricingTrx* trx,
                                        const NationCode& countryCode,
                                        const NationCode& residency);
  bool validatePassengerStatusNationality(PricingTrx* trx,
                                          const NationCode& countryCode,
                                          const NationCode& stateRegionCode,
                                          const NationCode& residency);
  bool validatePassengerStatusEmployee(PricingTrx* trx,
                                       const NationCode& countryCode,
                                       const NationCode& residency);
  bool validateGenParmForPassengerStatus(const NationCode& countryCode,
                                         const NationCode& stateRegionCode,
                                         const std::string& currentCRC,
                                         const NationCode& residency);
  bool needToAddArunkSegment(PricingTrx* trx, Itin& itin, const TravelSeg& travelSeg);
  void addArunkSegmentForWQ(PricingTrx* trx,
                            Itin& itin,
                            const TravelSeg& travelSeg,
                            bool& isMissingArunkForPO);
  void validateSideTrips(const PricingTrx* trx, const Itin& itin);
};
} // tse namespace
