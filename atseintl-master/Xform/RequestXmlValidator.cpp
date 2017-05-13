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
#include "Xform/RequestXmlValidator.h"

#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/NoPNRPricingTrx.h"
#include "DataModel/PaxType.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Rules/RuleConst.h"

namespace tse
{
FALLBACK_DECL(fallbackPriceByCabinActivation);

const std::string RequestXmlValidator::RESIDENCY = "R";
const std::string RequestXmlValidator::EMPLOYEE = "E";
const std::string RequestXmlValidator::NATIONALITY = "N";

RequestXmlValidator::RequestXmlValidator() {}

RequestXmlValidator::~RequestXmlValidator() {}

void
RequestXmlValidator::getAgentLocationAndCurrency(PricingTrx* trx)
{
  if (entryFromSubscriber(trx))
  {
    getSubscriberLocation(trx);
    getSubscriberCurrency(trx);
    getSubscriberCrsCode(trx);
    getIata(trx);
    return;
  }

  // get airline partition AAA nation currency
  getAirlinePartitionCurrency(trx);
  getAirlineCrsCode(trx);
}

void
RequestXmlValidator::validateCurrencyCode(PricingTrx* trx)
{
  if (trx->getOptions()->currencyOverride().empty())
    trx->getOptions()->currencyOverride() =
        trx->getRequest()->ticketingAgent()->currencyCodeAgent();
  else
    validateCurrency(trx, trx->getOptions()->currencyOverride());

  if (!trx->getOptions()->alternateCurrency().empty())
    validateCurrency(trx, trx->getOptions()->alternateCurrency());
}

void
RequestXmlValidator::validateCurrency(PricingTrx* trx, CurrencyCode& currencyCode)
{
  if ( trx->dataHandle().getCurrency( currencyCode ) )
    return;

  throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
      "UNABLE TO IDENTIFY CURRENCY CODE");
}
void
RequestXmlValidator::getOrigDestLoc(PricingTrx* trx, TravelSeg* travelSeg)
{
  validateSameOrigDest(trx, travelSeg);
  const Loc* originLoc = getLocation(trx, travelSeg->origAirport());
  if (!originLoc && !travelSeg->origAirport().empty())
    validateLoc(trx, travelSeg->pnrSegment());
  travelSeg->origin() = originLoc;

  const Loc* destinationLoc = getLocation(trx, travelSeg->destAirport());
  if (!destinationLoc && !travelSeg->destAirport().empty())
    validateLoc(trx, travelSeg->pnrSegment());
  travelSeg->destination() = destinationLoc;
}

void
RequestXmlValidator::validateSaleTicketOverride(PricingTrx* trx)
{
  if ((!trx->getRequest()->salePointOverride().empty() &&
       getLocation(trx, trx->getRequest()->salePointOverride()) == nullptr) ||
      (!trx->getRequest()->ticketPointOverride().empty() &&
       getLocation(trx, trx->getRequest()->ticketPointOverride()) == nullptr))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "UNABLE TO IDENTIFY CITY CODE");
}

void
RequestXmlValidator::validateSameOrigDest(PricingTrx* trx, TravelSeg* travelSeg)
{
  if (travelSeg->origAirport() == travelSeg->destAirport())
  {
    std::string errorMessage = "ORIGIN/DESTINATION CITIES ARE THE SAME  - SEGMENT " +
                               convertNumToString(travelSeg->pnrSegment());
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         errorMessage.c_str());
  }
}

void
RequestXmlValidator::validateLoc(PricingTrx* trx, int16_t pnrSeg)
{
  if (dynamic_cast<NoPNRPricingTrx*>(trx) != nullptr)
  {
    std::string errorMessage =
        "UNABLE TO IDENTIFY CITY CODE  - SEGMENT " + convertNumToString(pnrSeg);
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         errorMessage.c_str());
  }
  throw ErrorResponseException(ErrorResponseException::AIRPORT_CODE_NOT_IN_SYS);
}

std::string
RequestXmlValidator::convertNumToString(int16_t pnrSeg)
{
  char tmpBuf[10];
  sprintf(tmpBuf, "%2d", pnrSeg);
  std::string pnrSegmentNumber(tmpBuf);
  return pnrSegmentNumber;
}

const Loc*
RequestXmlValidator::getLocation(PricingTrx* trx, const LocCode& locCode)
{
  return trx->dataHandle().getLoc(locCode, trx->ticketingDate());
}

bool
RequestXmlValidator::entryFromSubscriber(PricingTrx* trx)
{
  if (!trx->getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
      trx->getRequest()->ticketingAgent()->agentTJR() != nullptr)
    return true;
  return false;
}

void
RequestXmlValidator::getSubscriberLocation(PricingTrx* trx)
{
  Agent& agent = *(trx->getRequest()->ticketingAgent());
  Customer* customer = agent.agentTJR();
  if (agent.agentCity().empty())
  {
    agent.agentCity() = customer->aaCity();
    agent.agentLocation() = getLocation(trx, customer->aaCity());
  }
}

void
RequestXmlValidator::getSubscriberCurrency(PricingTrx* trx)
{
  // We may have issue in SWS path since we are setting ticketingAgent->currencyCodeAgent
  // using CUSTOMER table. To follow PSS,it should be set using NATION table prime currency.
  if (trx->getRequest()->ticketingAgent()->currencyCodeAgent().empty())
    trx->getRequest()->ticketingAgent()->currencyCodeAgent() =
        trx->getRequest()->ticketingAgent()->agentTJR()->defaultCur();
}

void
RequestXmlValidator::getSubscriberCrsCode(PricingTrx* trx)
{
  if (trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() == ABACUS_MULTIHOST_ID)
  {
    trx->getRequest()->ticketingAgent()->cxrCode() = ABACUS_MULTIHOST_ID;
    trx->getRequest()->ticketingAgent()->vendorCrsCode() = ABACUS_MULTIHOST_ID;
    return;
  }

  if (trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() == AXESS_MULTIHOST_ID)
  {
    trx->getRequest()->ticketingAgent()->cxrCode() = SABRE_MULTIHOST_ID;
    trx->getRequest()->ticketingAgent()->vendorCrsCode() = AXESS_MULTIHOST_ID;
    return;
  }

  if (trx->getRequest()->ticketingAgent()->agentTJR()->crsCarrier() == SABRE_MULTIHOST_ID)
    trx->getRequest()->ticketingAgent()->cxrCode() = SABRE_MULTIHOST_ID;
}

void
RequestXmlValidator::getIata(PricingTrx* trx)
{
  Agent& agent = *(trx->getRequest()->ticketingAgent());
  const Customer& customer = *(agent.agentTJR());
  if (agent.tvlAgencyIATA().empty())
    agent.tvlAgencyIATA() = customer.arcNo();
  if (agent.homeAgencyIATA().empty())
    agent.homeAgencyIATA() = customer.homeArcNo();
}

void
RequestXmlValidator::getAirlinePartitionCurrency(PricingTrx* trx)
{
  if (trx->getRequest()->ticketingAgent()->currencyCodeAgent().empty() &&
      trx->getRequest()->ticketingAgent()->agentLocation() != nullptr)
    getLocationCurrency(trx);
}

void
RequestXmlValidator::getAirlineCrsCode(PricingTrx* trx)
{
  if (trx->getRequest()->ticketingAgent()->cxrCode().empty())
    trx->getRequest()->ticketingAgent()->cxrCode() = SABRE_MULTIHOST_ID;
}

bool
RequestXmlValidator::requestFromPo(PricingTrx* trx)
{
  if (trx->billing() != nullptr && (trx->billing()->requestPath() == PSS_PO_ATSE_PATH ||
                              trx->billing()->requestPath() == SWS_PO_ATSE_PATH ||
                              trx->billing()->requestPath() == LIBERTY_PO_ATSE_PATH))
    return true;
  return false;
}

void
RequestXmlValidator::getLocationCurrency(PricingTrx* trx)
{
  NationCode nation = trx->getRequest()->ticketingAgent()->agentLocation()->nation();
  CurrencyCode aaaCurrency;
  if (CurrencyUtil::getNationalCurrency(nation, aaaCurrency, trx->ticketingDate()))
    trx->getRequest()->ticketingAgent()->currencyCodeAgent() = aaaCurrency;
}

void
RequestXmlValidator::setTicketingDate(PricingTrx* trx, bool ticketDatePresent)
{
  if (ticketDatePresent)
  {
    trx->getOptions()->isTicketingDateOverrideEntry() = true;
    return;
  }
  short utcOffset = getTimeDiff(trx, trx->getRequest()->ticketingDT());
  if (utcOffset)
  {
    trx->getRequest()->ticketingDT() = trx->getRequest()->ticketingDT().addSeconds(utcOffset * 60);
    trx->dataHandle().setTicketDate(trx->getRequest()->ticketingDT());
  }
}

short
RequestXmlValidator::getTimeDiff(PricingTrx* trx, const DateTime& time)
{
  short utcOffset = 0;
  if (trx->getRequest()->ticketingAgent())
  {
    const Loc* hdqLoc = getLocation(trx, RuleConst::HDQ_CITY);
    const Loc* salesLoc = trx->getRequest()->salePointOverride().empty()
                              ? trx->getRequest()->ticketingAgent()->agentLocation()
                              : getLocation(trx, trx->getRequest()->salePointOverride());
    if (hdqLoc && salesLoc)
    {
      if (LocUtil::getUtcOffsetDifference(
              *salesLoc, *hdqLoc, utcOffset, trx->dataHandle(), time, time))
        return utcOffset;
    }
  }
  return 0;
}

void
RequestXmlValidator::setBookingDate(PricingTrx* trx, TravelSeg* travelSeg)
{
  travelSeg->bookingDT() = trx->getRequest()->ticketingDT();
}

void
RequestXmlValidator::setDepartureDate(PricingTrx* trx, TravelSeg* travelSeg)
{
  DateTime todayDate = DateTime::localTime();
  short utcOffset = getTimeDiff(trx, todayDate);
  if (utcOffset)
  {
    todayDate = todayDate.addSeconds(utcOffset * 60);
  }
  travelSeg->departureDT() = todayDate;
}

void
RequestXmlValidator::validateShipRegistry(PricingTrx* trx)
{
  if (trx->getOptions()->fareByRuleShipRegistry().empty())
    return;

  validateShipRegistryCountry(trx);
  validateShipRegistryPaxType(trx);
}

void
RequestXmlValidator::validateShipRegistryCountry(PricingTrx* trx)
{
  const Nation* nation = getNation(trx, trx->getOptions()->fareByRuleShipRegistry());
  if (!nation)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INVALID COUNTRY CODE");
}

const Nation*
RequestXmlValidator::getNation(PricingTrx* trx, const NationCode& nationCode)
{
  return trx->dataHandle().getNation(nationCode, trx->ticketingDate());
}

void
RequestXmlValidator::validateShipRegistryPaxType(PricingTrx* trx)
{
  std::vector<PaxTypeCode> paxTypeCodes;
  paxTypeCodes.push_back(SEA);

  if (PaxTypeUtil::isPaxInTrx(*trx, paxTypeCodes))
    return;

  throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                       "SHIP REGISTRY REQUIRES SEA PASSENGER TYPE");
}

void
RequestXmlValidator::validateDepartureDate(PricingTrx* trx, Itin* itin, TravelSeg* travelSeg, int i)
{
  if (dynamic_cast<NoPNRPricingTrx*>(trx) == nullptr)
    return;
  if (travelSeg->pssDepartureDate().empty())
    return;
  validateTicketingDate(trx, travelSeg->departureDT(), i + 1, false);
  if (i <= 0)
    return;
  while (i >= 1)
  {
    TravelSeg* previousTvlSeg = itin->travelSeg()[i - 1];
    if (!previousTvlSeg->pssDepartureDate().empty())
    {
      if (travelSeg->departureDT() < previousTvlSeg->departureDT())
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "TRAVEL DATES OUT OF SEQUENCE");
      break;
    }
    else
      i--;
  }
}

void
RequestXmlValidator::validateTicketingDate(PricingTrx* trx, Itin* itin)
{

  if (itin == nullptr || itin->travelSeg().empty())
    return;

  validateTicketingDate(trx, trx->ticketingDate(), itin->travelSeg().size(), true);
  const DateTime& tktDt = trx->ticketingDate();
  const DateTime& tvlDt = itin->travelSeg()[0]->departureDT();
  DateTime ticketingDate(tktDt.year(), tktDt.month(), tktDt.day());
  DateTime firstTvlDate(tvlDt.year(), tvlDt.month(), tvlDt.day());

  if (ticketingDate <= firstTvlDate)
  {
    return;
  }
  else
  {
    if (itin->travelSeg()[0]->segmentType() == Open && !firstTvlDate.isEmptyDate())
      return;
    short utcOffset = getTimeDiff(trx, itin->travelSeg()[0]->origAirport());
    DateTime refOrigDT = itin->travelSeg()[0]->departureDT().subtractSeconds(utcOffset * 60);

    if (ticketingDate <= refOrigDT)
      return;

    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "TKT DATE MUST BE EQUAL OR EARLIER THAN TVL DATE");
  }
}

void
RequestXmlValidator::validateTicketingDate(PricingTrx* trx,
                                           DateTime& dateTime,
                                           int16_t i,
                                           bool ticketingDate)
{
  DateTime todayDate = DateTime::localTime();
  short utcOffset = getTimeDiff(trx, todayDate);
  if (utcOffset)
  {
    todayDate = todayDate.addSeconds(utcOffset * 60);
  }
  DateTime currentDate(todayDate.year(), todayDate.month(), todayDate.day());
  DateTime date(dateTime.year(), dateTime.month(), dateTime.day());

  DateTime futureDate;
  if (TrxUtil::isProject331PlusDaysEnabled(*trx))
    futureDate = currentDate.addDays(NO_PNR_FUTURE_DAY_362);
  else
    futureDate = currentDate.addDays(NO_PNR_FUTURE_DAY);

  if (date > futureDate)
    handleDateErrorMessage(i, ticketingDate);

  DateTime beoungDate = currentDate.subtractDays(BEYOND_MAXIMUM_HISTORICAL_DATE);
  if (date < beoungDate)
  {
    std::string errorMsg = "BEYOND MAXIMUM HISTORICAL DATE " + beoungDate.dateToSqlString();
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, errorMsg.c_str());
  }
}

void
RequestXmlValidator::handleDateErrorMessage(int16_t i, bool ticketingDate)
{
  if (ticketingDate)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "TICKET DATE NOT VALID");

  std::string errorMessage = "TRAVEL DATE NOT VALID SEG " + convertNumToString((int16_t)i);
  throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, errorMessage.c_str());
}

PaxType*
RequestXmlValidator::getDefaultPaxType(PricingTrx* trx)
{
  PaxType* paxType = nullptr;
  trx->dataHandle().get(paxType);
  paxType->paxType() = ADULT;

  if (trx->getRequest()->ticketingAgent()->agentTJR() != nullptr &&
      !trx->getRequest()->ticketingAgent()->agentTJR()->defaultPassengerType().empty())
  {
    const PaxTypeCode& ptc =
        trx->getRequest()->ticketingAgent()->agentTJR()->defaultPassengerType();
    if (getPaxType(trx, ptc))
      paxType->paxType() = ptc;
  }
  paxType->number() = 1;
  return paxType;
}

void
RequestXmlValidator::validatePassengerType(PricingTrx* trx, PaxType& paxType)
{
  if (paxType.number() == 0)
    paxType.number() = 1;
  // inhibit ENF/HNN/KNN/XNN  for V2.
  if (paxType.paxType() == "ENF" || paxType.paxType() == "HNN" || paxType.paxType() == "KNN" ||
      paxType.paxType() == "XNN" || !getPaxType(trx, paxType.paxType()))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "UNABLE TO IDENTIFY PASSENGER TYPE - MODIFY AND REENTER.");

  if (const MaxPenaltyInfo* maxPenaltyInfo = paxType.maxPenaltyInfo())
  {
    if (maxPenaltyInfo->_mode == smp::INFO &&
        (maxPenaltyInfo->_changeFilter._departure != smp::BOTH ||
         maxPenaltyInfo->_refundFilter._departure != smp::BOTH ||
         maxPenaltyInfo->_changeFilter._query || maxPenaltyInfo->_changeFilter._maxFee ||
         maxPenaltyInfo->_refundFilter._query || maxPenaltyInfo->_refundFilter._maxFee))
    {
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "NO ADDITIONAL ATTRIBUTES SHOULD BE SPECIFIED IN MAXIMUM PENALTY INFO MODE");
    }
    else if (maxPenaltyInfo->_mode != smp::INFO &&
             !maxPenaltyInfo->_changeFilter._query && !maxPenaltyInfo->_changeFilter._maxFee &&
             !maxPenaltyInfo->_refundFilter._query && !maxPenaltyInfo->_refundFilter._maxFee)
    {
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "ADDITIONAL ATTRIBUTES SHOULD BE SPECIFIED FOR MAXIMUM PENALTY");
    }
    else if (maxPenaltyInfo->_mode == smp::OR &&
             (maxPenaltyInfo->_changeFilter._query != maxPenaltyInfo->_refundFilter._query ||
              maxPenaltyInfo->_changeFilter._maxFee != maxPenaltyInfo->_refundFilter._maxFee))
    {
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "MAX CHANGE AND MAX REFUND ATTRIBUTES SHOULD BE IDENTICAL IN MAXIMUM PENALTY OR MODE");
    }
    else if (maxPenaltyInfo->_changeFilter._query && maxPenaltyInfo->_changeFilter._maxFee)
    {
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "MAX CHANGE AMOUNT SHOULD NOT BE SPECIFIED IN ANY/NONCHANGEABLE MODE");
    }
    else if (maxPenaltyInfo->_refundFilter._query && maxPenaltyInfo->_refundFilter._maxFee)
    {
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "MAX REFUND AMOUNT SHOULD NOT BE SPECIFIED IN ANY/NONCHANGEABLE MODE");
    }
  }
}

void
RequestXmlValidator::validateDuplicatePassengerType(PricingTrx* trx, const PaxType& paxType)
{
  for(const PaxType* prevPaxType : trx->paxType())
  {
    bool sameBirthDate = false;
    if (TrxUtil::isTaxExemptionForChildActive(*trx))
    {
      sameBirthDate = (prevPaxType->birthDate() == paxType.birthDate());
    }

    if (prevPaxType->paxType() == paxType.paxType() &&
        prevPaxType->age() == paxType.age() &&
        sameBirthDate)
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "DUPLICATE PAX TYPES NOT ALLOWED");
  }
}

const PaxTypeInfo*
RequestXmlValidator::getPaxType(PricingTrx* trx, const PaxTypeCode& paxType)
{
  return trx->dataHandle().getPaxType(paxType, Vendor::EMPTY);
}

void
RequestXmlValidator::validatePassengerStatus(PricingTrx* trx,
                                             const NationCode& countryCode,
                                             const NationCode& stateRegionCode,
                                             const std::string& currentCRC,
                                             const NationCode& residency)
{
  if (!validateGenParmForPassengerStatus(countryCode, stateRegionCode, currentCRC, residency) ||
      (currentCRC == EMPLOYEE && !validatePassengerStatusEmployee(trx, countryCode, residency)) ||
      (currentCRC == NATIONALITY &&
       !validatePassengerStatusNationality(trx, countryCode, stateRegionCode, residency)) ||
      (currentCRC == RESIDENCY && !validatePassengerStatusResidency(trx, countryCode, residency)))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INVALID COUNTRY CODE");
}

bool
RequestXmlValidator::isValidCountryCode(PricingTrx* trx, const NationCode& countryCode)
{
  return (getNation(trx, countryCode) != nullptr);
}

bool
RequestXmlValidator::validatePassengerStatusResidency(PricingTrx* trx,
                                                      const NationCode& countryCode,
                                                      const NationCode& residency)
{
  if (countryCode.empty() && residency.empty())
    return false;
  if (!countryCode.empty())
    return (isValidCountryCode(trx, countryCode));
  if (!residency.empty())
    return (getLocation(trx, residency) != nullptr);

  return true;
}

bool
RequestXmlValidator::validatePassengerStatusNationality(PricingTrx* trx,
                                                        const NationCode& countryCode,
                                                        const NationCode& stateRegionCode,
                                                        const NationCode& residency)
{
  if (countryCode.empty() || !stateRegionCode.empty() || !residency.empty() ||
      !isValidCountryCode(trx, countryCode))
    return false;

  return true;
}

bool
RequestXmlValidator::validatePassengerStatusEmployee(PricingTrx* trx,
                                                     const NationCode& countryCode,
                                                     const NationCode& residency)
{
  if (countryCode.empty() || !residency.empty() || !isValidCountryCode(trx, countryCode))
    return false;
  return true;
}

bool
RequestXmlValidator::validateGenParmForPassengerStatus(const NationCode& countryCode,
                                                       const NationCode& stateRegionCode,
                                                       const std::string& currentCRC,
                                                       const NationCode& residency)
{
  // the following checks should be done in PO;
  //  xml A40(country)         countryCode.size() = 2     (char alpha)
  //  xml AH0(state)           stateRegionCode.size() = 2 (char alpha)
  //  xml A50(city)            residency.size() =3        (char alpha)
  //  xml N0M(psg_status_type) currentCRC.size() =1       (char alpha = R/E/N)
  if (currentCRC.empty() && countryCode.empty() && stateRegionCode.empty() && residency.empty())
    return true; // most likely
  if ((currentCRC.empty() &&
       (!countryCode.empty() || !stateRegionCode.empty() || !residency.empty())) ||
      (!currentCRC.empty() && countryCode.empty() && stateRegionCode.empty() &&
       residency.empty()) ||
      (!residency.empty() && (!countryCode.empty() || !stateRegionCode.empty())) ||
      (countryCode.empty() && !stateRegionCode.empty()))
    return false;
  return true;
}

void
RequestXmlValidator::processMissingArunkSegForPO(PricingTrx* trx,
                                                 Itin* itin,
                                                 const TravelSeg* travelSeg,
                                                 bool& isMissingArunkForPO)
{
  if (needToAddArunkSegment(trx, *itin, *travelSeg))
    addArunkSegmentForWQ(trx, *itin, *travelSeg, isMissingArunkForPO);
}

bool
RequestXmlValidator::needToAddArunkSegment(PricingTrx* trx, Itin& itin, const TravelSeg& travelSeg)
{
  std::vector<TravelSeg*>& ts = itin.travelSeg();
  if (ts.size() == 0)
    return false;
  if (travelSeg.segmentType() == Arunk)
    return false;
  TravelSeg* prevTvlSeg = ts.back();
  if (prevTvlSeg->segmentType() == Arunk)
    return false;

  if (prevTvlSeg->offMultiCity() != travelSeg.boardMultiCity() &&
      prevTvlSeg->destAirport() != travelSeg.origAirport())
    return true;
  return false;
}

void
RequestXmlValidator::addArunkSegmentForWQ(PricingTrx* trx,
                                          Itin& itin,
                                          const TravelSeg& travelSeg,
                                          bool& isMissingArunkForPO)
{
  std::vector<TravelSeg*>& ts = itin.travelSeg();
  TravelSeg* prevTvlSeg = ts.back();

  ArunkSeg* arunkSeg = nullptr;
  trx->dataHandle().get(arunkSeg);
  if (arunkSeg == nullptr)
    return;

  arunkSeg->segmentType() = Arunk;
  arunkSeg->forcedStopOver() = 'T';

  arunkSeg->origAirport() = prevTvlSeg->destAirport();
  arunkSeg->origin() = prevTvlSeg->destination();
  arunkSeg->boardMultiCity() = prevTvlSeg->offMultiCity();

  arunkSeg->destAirport() = travelSeg.origAirport();
  arunkSeg->destination() = travelSeg.origin();
  arunkSeg->offMultiCity() = travelSeg.boardMultiCity();
  arunkSeg->segmentOrder() = prevTvlSeg->segmentOrder() + 1;

  ts.push_back(arunkSeg);
  isMissingArunkForPO = true;
  trx->travelSeg().push_back(arunkSeg);
}

void
RequestXmlValidator::setForcedStopoverForNoPnrPricing(Itin* itin)
{
  if (itin->travelSeg().empty())
    return;

  std::vector<TravelSeg*>& ts = itin->travelSeg();
  int tvlSegSize = ts.size();

  for (int i = 0; i < tvlSegSize; i++)
  {
    TravelSeg& tvlSeg = *(ts[i]);
    if (!tvlSeg.isForcedConx() && i < tvlSegSize - 1)
      tvlSeg.forcedStopOver() = 'T';
  }
}

void
RequestXmlValidator::validateETicketOptionStatus(PricingTrx* trx)
{
  if (trx->getRequest() && trx->getRequest()->ticketingAgent())
  {
    Agent& agent = *(trx->getRequest()->ticketingAgent());
    if (agent.agentTJR() != nullptr) // subscriber
    {
      if (agent.agentTJR()->eTicketCapable() == 'Y')
        trx->getRequest()->electronicTicket() =
            trx->getRequest()->ticketingAgent()->agentTJR()->eTicketCapable();
    }
    else
    { // carrier partition
      trx->getRequest()->electronicTicket() = 'Y';
    }
  }
}

void
RequestXmlValidator::validateFareQuoteCurrencyIndicator(FareDisplayTrx* fqTrx)
{
  if (fqTrx->getRequest() && fqTrx->getRequest()->ticketingAgent() &&
      fqTrx->getRequest()->ticketingAgent()->agentTJR() != nullptr &&
      fqTrx->getRequest()->ticketingAgent()->agentTJR()->fareQuoteCur() == 'Y')
    fqTrx->getOptions()->sellingCurrency() = 'T';
}

void
RequestXmlValidator::checkItinBookingCodes(NoPNRPricingTrx& noPNRTrx)
{
  AirSeg* airSeg = dynamic_cast<AirSeg*>(noPNRTrx.travelSeg().front());
  if (airSeg == nullptr)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT SURFACE BREAK DESIGNATION");
  CarrierCode carrier = airSeg->marketingCarrierCode();
  if (carrier.empty())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "CARRIER OPTION REQUIRED - SEGMENT 1");
  BookingCode firstRbd = airSeg->getBookingCode();

  std::vector<TravelSeg*>::iterator tvlI = noPNRTrx.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlE = noPNRTrx.travelSeg().end();
  for (; tvlI != tvlE; tvlI++)
  {
    airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      continue;
    if (firstRbd.empty())
    {
      if (!airSeg->getBookingCode().empty())
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "BOOKING CODE OPTION REQUIRED - SEGMENT 1");
    }
    else
    {
      if (airSeg->getBookingCode().empty())
        airSeg->setBookingCode(firstRbd);
      else
        firstRbd = airSeg->getBookingCode();
    }

    if (airSeg->marketingCarrierCode().empty())
      airSeg->setMarketingCarrierCode(carrier);
    else
      carrier = airSeg->marketingCarrierCode();
  }
}

void
RequestXmlValidator::validateItin(PricingTrx* trx, const Itin& itin)
{
  if (itin.travelSeg().empty())
    return;

  if (itin.travelSeg().front()->segmentType() == Arunk ||
      itin.travelSeg().back()->segmentType() == Arunk)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INCORRECT SURFACE BREAK DESIGNATION");

  if (itin.travelSeg().back()->isForcedConx() && trx->billing() != nullptr &&
      trx->billing()->requestPath() == SWS_PO_ATSE_PATH)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "LAST SEGMENT CAN NOT BE A FORCED CONNECTION");

  if (itin.travelSeg().back()->isForcedFareBrk())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "BREAK FARE NOT ALLOWED ON LAST SEGMENT");

  if (itin.travelSeg().back()->isForcedNoFareBrk())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "NO BREAK FARE NOT ALLOWED ON LAST SEGMENT");

  validateSideTrips(trx, itin);
}

void
RequestXmlValidator::setMOverride(PricingTrx* trx)
{
  if (!trx->getOptions()->currencyOverride().empty())
    trx->getOptions()->mOverride() = 'T';
}

void
RequestXmlValidator::validateSideTrips(const PricingTrx* trx, const Itin& itin)
{
  if (itin.travelSeg().front()->isForcedSideTrip() || itin.travelSeg().back()->isForcedSideTrip())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "SIDE TRIP NOT ALLOWED ON FIRST/LAST SEGMENT");
  std::vector<const TravelSeg*> sideTrip;
  std::vector<TravelSeg*>::const_iterator tvlI = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlE = itin.travelSeg().end();
  bool withInSideTrip = false;
  for (; tvlI != tvlE; tvlI++)
  {
    const TravelSeg& tvlSeg = **tvlI;
    if (!withInSideTrip)
    {
      if (tvlSeg.isForcedSideTrip())
      {
        sideTrip.push_back(&tvlSeg);
        withInSideTrip = true;
      }
      else if (tvlSeg.segmentType() == Arunk)
      {
        sideTrip.clear();
        sideTrip.push_back(
            &tvlSeg); // Only keep previous arunk before first forced side trip segment
      }
    }
    else
    {
      if (tvlSeg.isForcedSideTrip())
      {
        sideTrip.push_back(&tvlSeg);
      }
      else if (tvlSeg.segmentType() == Arunk)
      {
        // Check if there is complete side trip before it.
        bool finishedSideTrip = sideTrip.front()->origAirport() == sideTrip.back()->destAirport();
        if (!finishedSideTrip)
        {
          uint16_t size = sideTrip.size();
          if (size >= 3 && sideTrip.front()->segmentType() == Arunk &&
              sideTrip[1]->origAirport() == sideTrip.back()->destAirport())
            finishedSideTrip = true;
        }

        if (finishedSideTrip)
        {
          sideTrip.clear();
          withInSideTrip = false;
        }

        sideTrip.push_back(&tvlSeg);
      }
      else
      {
        bool validSideTrip = sideTrip.front()->origAirport() == sideTrip.back()->destAirport();
        if (!validSideTrip)
        {
          uint16_t size = sideTrip.size();
          if (size >= 3)
          {
            if ((sideTrip.front()->segmentType() == Arunk &&
                 sideTrip[1]->origAirport() == sideTrip.back()->destAirport()) ||
                (sideTrip.back()->segmentType() == Arunk &&
                 sideTrip[0]->origAirport() == sideTrip[size - 2]->destAirport()) ||
                (size >= 4 && sideTrip.front()->segmentType() == Arunk &&
                 sideTrip.back()->segmentType() == Arunk &&
                 sideTrip[1]->origAirport() == sideTrip[size - 2]->destAirport()))
              validSideTrip = true;
          }
        }

        if (!validSideTrip)
        {
          throw NonFatalErrorResponseException(
              ErrorResponseException::INVALID_INPUT,
              "SAME CITY MUST BE USED FOR BEGINNING/END OF SIDE TRIP");
        }
        withInSideTrip = false;
        sideTrip.clear();
      }
    }
  }
}

void
RequestXmlValidator::validateItinForFlownSegments(const Itin& itin)
{
  const std::vector<TravelSeg*>& ts = itin.travelSeg();
  int tvlSegSize = ts.size();
  if (ts.size() < 2)
    return;

  for (int i = 0; i < tvlSegSize - 1; i++)
  {
    TravelSeg& tSeg = *(ts[i]);
    TravelSeg& tSegNext = *(ts[i + 1]);
    //  check the sequence for flown/unflown segnents
    if (tSeg.unflown() && !tSegNext.unflown())
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "WRONG SEQUENCE FOR FLOWN/UNFLOWN SEGMENTS");
    //    if (tSeg.segmentType() == Arunk ||
    //        tSeg.segmentType() == Open  ||
    //        !tSeg.arrivalDT().isValid() ||
    //        !tSegNext.departureDT().isValid())
    //      continue;
    //    if (tSeg.arrivalDT() > tSegNext.departureDT())
    //    {
    //      std::string dest = tSeg.destAirport();
    //      std::string errorMessage =
    //      "DEPARTURE/ARRIVAL DATES OUT OF SEQUENCE IN " + dest;
    //      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
    // errorMessage.c_str());
    //    }
  }
}

void
RequestXmlValidator::setOperatingCarrierCode(TravelSeg& travelSeg)
{
  if (travelSeg.segmentType() == Air)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(&travelSeg);
    if (airSeg != nullptr && airSeg->operatingCarrierCode().empty() && airSeg->flightNumber() != 0)
    {
      airSeg->setOperatingCarrierCode(airSeg->marketingCarrierCode());
    }
  }
}

void
RequestXmlValidator::validatePassengerTypeWithAges(PaxType& paxType)
{
  if (paxType.paxType().length() != 3)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "UNABLE TO IDENTIFY PASSENGER TYPE - MODIFY AND REENTER.");
  else if (isalpha(paxType.paxType()[0]) && isdigit(paxType.paxType()[1]) &&
           isdigit(paxType.paxType()[2]))
  {
    // Sabre does not support HNN,KNN,XNN paxtypes (but they are filed in the "PSGTYPE" table)
    if (paxType.paxType()[0] == 'H' || paxType.paxType()[0] == 'K' || paxType.paxType()[0] == 'X')
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "UNABLE TO IDENTIFY PASSENGER TYPE - MODIFY AND REENTER.");
    paxType.age() = std::atoi(paxType.paxType().substr(1, 2).c_str());
    if (paxType.age() <= 1)
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_FORMAT,
          "INVALID PASSENGER TYPE FOR INFANT - MODIFY AND REENTER.");
    paxType.paxType() = std::string(paxType.paxType(), 0, 1) + "NN";
  }
}
void
RequestXmlValidator::checkRequestCrossDependencies(PricingTrx* trx)
{
  if (trx == nullptr)
    return;

  if (!trx->getOptions())
    return;

  if (!trx->getRequest())
    return;

  if (trx->getOptions()->publishedFares() && trx->getOptions()->privateFares())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "SPECIFY PUBLIC OR PRIVATE");

  if (trx->getOptions()->publishedFares() && trx->getOptions()->cat35Net())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "PUBLIC FARE NOT VALID WITH NET FARE OPTION");

  if (trx->getOptions()->forceCorpFares())
  {
    if (trx->getRequest()->corpIdVec().empty() && trx->getRequest()->accCodeVec().empty() &&
        trx->getRequest()->corporateID().empty() && trx->getRequest()->accountCode().empty())
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "FORCE CORPORATE REQUIRES ACCOUNT CODE OR CORP ID");
  }

  if (trx->getRequest()->exemptAllTaxes() && trx->getRequest()->exemptSpecificTaxes())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "ONLY ONE TAX DEFINED /TE OR /TN");

  std::vector<PaxType*>::const_iterator paxI = trx->paxType().begin();
  std::vector<PaxType*>::const_iterator paxE = trx->paxType().end();
  for (; paxI != paxE; paxI++)
  {
    const PaxType& paxType = **paxI;
    if (paxType.paxTypeInfo().isChild() || paxType.paxTypeInfo().isInfant())
    {
      if (paxType.age() == 1)
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                             "CHECK FARE QUOTE/RULES FOR NEW INFANT PSGR TYPES");
    }
  }

  bool isDAEntry;
  bool isDPEntry;
  if (TrxUtil::newDiscountLogic(*trx))
  {
    isDAEntry = trx->getRequest()->isDAEntryNew();
    isDPEntry = trx->getRequest()->isDPEntryNew();
  }
  else
  {
    isDAEntry = trx->getRequest()->isDAEntry();
    isDPEntry = trx->getRequest()->isDPEntry();
  }

  if (TrxUtil::newDiscountLogic(*trx))
  {
    if (isDPEntry && isDAEntry)
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "DISCOUNT AMOUNT AND DISCOUNT PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST");

    if (isDAEntry && trx->getRequest()->isPAEntry())
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "DISCOUNT AMOUNT AND MARK UP AMOUNT NOT ALLOWED WITHIN SAME REQUEST");

    if (isDAEntry && trx->getRequest()->isPPEntry())
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "DISCOUNT AMOUNT AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST");

    if (isDPEntry && trx->getRequest()->isPAEntry())
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "DISCOUNT PERCENTAGE AND MARK UP AMOUNT NOT ALLOWED WITHIN SAME REQUEST");

    if (isDPEntry && trx->getRequest()->isPPEntry())
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "DISCOUNT PERCENTAGE AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST");

    if (trx->getRequest()->isPAEntry() && trx->getRequest()->isPPEntry())
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "MARK UP AMOUNT AND MARK UP PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST");
  }
  else
  {
    if (isDPEntry && isDAEntry)
      throw NonFatalErrorResponseException(
          ErrorResponseException::INVALID_INPUT,
          "DISCOUNT AMOUNT AND PERCENTAGE NOT ALLOWED WITHIN SAME REQUEST");
  }

  if (trx->getOptions()->cat35Net() &&
      (isDPEntry || isDAEntry))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "MANUAL DISCOUNT DOES NOT APPLY TO NEGOTIATED FARES");

  if (trx->getOptions()->cat35Net() &&
      (trx->getRequest()->isCollectOCFees() && trx->getOptions()->isProcessAllGroups()))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "SECONDARY ACTION CODE NOT VALID WITH WPNET ENTRY");

  if (!fallback::fallbackPriceByCabinActivation(trx))
  {
    if (!trx->getOptions()->cabin().isUndefinedClass() &&                    // PriceByCabin requested
        !trx->getRequest()->isLowFareNoAvailability()  &&                    // WPNCS
        !trx->getRequest()->isLowFareRequested() &&                          // WPNC/WPNCB
        !(trx->getRequest()->isWpas() && trx->getRequest()->wpaXm()))        // WPA(s)'XM
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                          "SECONDARY ACTION CODE TC- NOT VALID WITH REQUEST ENTERED");
   }
}

void
RequestXmlValidator::validateCxrOverride(PricingTrx* trx, Itin* itin)
{

  if (itin == nullptr || itin->travelSeg().empty())
    return;

  if (!trx->getRequest())
    return;

  bool carrierMatch = false;
  CarrierCode cxrOverride;
  CarrierCode preCxrOverride;

  if (trx->getRequest()->governingCarrierOverrides().size())
  {
    for (std::map<int16_t, CarrierCode>::const_iterator
             it = trx->getRequest()->governingCarrierOverrides().begin(),
             itend = trx->getRequest()->governingCarrierOverrides().end();
         it != itend;
         ++it)
    {
      cxrOverride = (*it).second;
      if (preCxrOverride.empty())
        preCxrOverride = cxrOverride;
      else if (cxrOverride != preCxrOverride)
      {
        if (!carrierMatch) // still no segment match the carrier before changing governing carrier,
                           // it should fail now.
          break;
        else
          carrierMatch = false; // start new match.
      }

      if (cxrOverride == INDUSTRY_CARRIER)
      {
        carrierMatch = true;
      }
      else
      {
        int16_t segmentOrder = (*it).first;
        std::vector<TravelSeg*>& ts = itin->travelSeg();
        size_t tvlSegSize = ts.size();
        for (size_t i = 0; i < tvlSegSize; ++i)
        {
          TravelSeg& tvlSeg = *(ts[i]);
          if (tvlSeg.isAir() && segmentOrder == tvlSeg.segmentOrder())
          {
            AirSeg* airSeg = static_cast<AirSeg*>(&tvlSeg);
            if (cxrOverride == airSeg->carrier())
            {
              carrierMatch = true;
              break;
            }
          }
        }
      }
    }
    if (!carrierMatch)
    {
      std::string errMsg =
          std::string("TRAVEL NOT VIA ") + cxrOverride + std::string(" ON SELECTED SEGMENT");
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, errMsg.c_str());
    }
  }
  return;
}

void
RequestXmlValidator::validateMultiTicketRequest(PricingTrx* trx)
{
  if (!trx->getRequest() || !trx->getOptions())
    return;

  PricingRequest* request = trx->getRequest();
  PricingOptions* options = trx->getOptions();

  if ((request->ticketingAgent()->agentTJR() == nullptr &&
       trx->billing() &&
       !trx->billing()->partitionID().empty() && trx->billing()->aaaCity().size() < 4) // Hosted carriers
      || request->ticketingAgent()->axessUser()                                        // AXESS
      || request->isTicketEntry()                                                      // Ticketing entry
      || trx->getTrxType() != PricingTrx::PRICING_TRX                                  // non-core Pricing entry
      || trx->excTrxType() != PricingTrx::NOT_EXC_TRX)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "FORMAT-CHECK ENTRY COMMENCING WITH $MT");
  bool isDAorPAentry = false;
  bool isDPorPPentry = false;
  if (!TrxUtil::newDiscountLogic(*trx))
  {
    isDAorPAentry = request->isDAorPAentry();
    isDPorPPentry = request->isDPorPPentry();
  }
  else
  {
    isDAorPAentry = request->isDAEntry();
    isDPorPPentry = request->isDPEntry();
  }

  if (options->fbcSelected()                      // WPQ
      || request->isSpecifiedTktDesignatorEntry() // WPQ/
      || isDAorPAentry                            // WPQ/TD/DAXX or WPQ/TD/PAXX
      || isDPorPPentry)                           // WPQ/TD/DPXX or WPQ/TD/PPXX
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WPQ ENTRY");
  else if (trx->noPNRPricing())                   // WQ
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WQ ENTRY");
  else if (trx->altTrxType() != PricingTrx::WP)   // WPA
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WPA ENTRY");
  else if (options->isRtw())                      // WPRW
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WPRW ENTRY");
  else if (options->isCat35Net())                 // WPNET
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WPNET ENTRY");
  else if (options->isFareFamilyType())           // WPT/
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WPT/ ENTRY");
  else if (TrxUtil::determineIfNotCurrentRequest(*trx))
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, "SECONDARY ACTION CODE MT NOT VALID WITH WPB ENTRY");

  if (request->ticketingAgent()->agentTJR() != nullptr &&
      request->ticketingAgent()->agentTJR()->pricingApplTag4() == 'Y')
    request->multiTicketActive() = true;
  else if (request->diagnosticNumber() != Diagnostic198)
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "FORMAT - MULTIPLE TICKET PRICING NOT ACTIVE");
}

void
RequestXmlValidator::setUpCorrectCurrencyConversionRules(PricingTrx& trx)
{
  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX && trx.getRequest()->ticketingAgent() &&
      !trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
      trx.getRequest()->ticketingAgent()->agentLocation() &&
      trx.getRequest()->ticketingAgent()->agentLocation()->nation() != "US" &&
      trx.getRequest()->ticketingAgent()->isArcUser() &&
      trx.getRequest()->ticketingAgent()->agentTJR()->defaultCur() ==
          trx.getOptions()->currencyOverride())
  {
    trx.getOptions()->mOverride() = 0;
  }
}

void
RequestXmlValidator::checkCominationPDOorPDRorXRSOptionalParameters(PricingTrx* trx)
{
  if (trx == nullptr || !trx->getOptions())
    return;

  PricingOptions* options = trx->getOptions();

  if (options->isPDOForFRRule() && options->isXRSForFRRule())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE ORG AND XRS");

  else if (options->isPDRForFRRule() && options->isXRSForFRRule())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE RET AND XRS");

  else if (options->isPDRForFRRule() && options->isPDOForFRRule())
    throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                           "UNABLE TO COMBINE RET AND ORG");
}
void
RequestXmlValidator::validateFQDateRange(FareDisplayTrx* fqTrx)
{
  if (fqTrx->getRequest() &&
      fqTrx->getRequest()->dateRangeLower() > fqTrx->getRequest()->dateRangeUpper())
    throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                         "INVALID TRAVEL DATE RANGE");
}
void
RequestXmlValidator::validateFQReturnDate(FareDisplayTrx* fqTrx, AirSeg* airSeg)
{
  if (fqTrx->getRequest() && airSeg && fqTrx->getRequest()->returnDate().isValid() &&
      (fqTrx->getRequest()->returnDate() < airSeg->departureDT()))
    throw NonFatalErrorResponseException(ErrorResponseException::RETURN_DATE_CONTINUITY);
}

short
RequestXmlValidator::getTimeDiff(const PricingTrx* trx, const LocCode& tvlLocT) const
{
  DateTime localTime = DateTime::localTime();
  short utcOffset = 0;
  const Loc* saleLoc;

  if (trx->getRequest()->PricingRequest::salePointOverride().size())
    saleLoc =
        trx->dataHandle().getLoc(trx->getRequest()->PricingRequest::salePointOverride(), localTime);
  else
    saleLoc = trx->getRequest()->ticketingAgent()->agentLocation();

  const Loc* tvlLoc = trx->dataHandle().getLoc(tvlLocT, localTime);

  if (saleLoc && tvlLoc)
  {
    if (!LocUtil::getUtcOffsetDifference(
            *tvlLoc, *saleLoc, utcOffset, trx->dataHandle(), localTime, localTime))
      utcOffset = 0;
  }
  return utcOffset;
}
}
