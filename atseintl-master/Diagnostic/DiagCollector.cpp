//----------------------------------------------------------------------------
//  File:         DiagCollector.C
//  Description:  Diagnostic Collector base class: Defines all the virtual methods
//                derived class may orverride these methods.
//
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2003
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
#include "Diagnostic/DiagCollector.h"

#include "Common/AirlineShoppingUtils.h"
#include "Common/CabinType.h"
#include "Common/FallbackUtil.h"
#include "Common/FareTypeDesignator.h"
#include "Common/Money.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Fare.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DBAccess/CarrierMixedClass.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Pricing/FPPQItem.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FareMarketPathMatrix.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"
#include "Pricing/PUPathMatrix.h"
#include "Rules/RuleConst.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Util/BranchPrediction.h"

#include <boost/lambda/lambda.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>

namespace tse
{
// Set static variable
const uint32_t DiagCollector::DEFAULT_LINEWRAP_LENGTH;

bool
DiagCollector::enableFilter(DiagnosticTypes diagType, int dtCount, size_t currentFarePathNumber)
{
  return isActive() && (_diagnosticType == diagType);
}

void
DiagCollector::printHeader(std::string diagnosticName, char fillCharacter, const int length)
{
  const int numberOfFillCharacters = length - diagnosticName.length() - 2;
  const std::string header = std::string((numberOfFillCharacters+1)/2, fillCharacter) + " " + diagnosticName + " " + std::string(numberOfFillCharacters/2, fillCharacter);
  ((std::ostringstream&)*this) << header << "\n";
}

void
DiagCollector::printHeader()
{
  if (!_active)
    return;

  switch (_rootDiag->diagnosticType())
  {
  case Diagnostic233:
    ((std::ostringstream&)*this)
        << "********************* DIAGNOSTIC 233 OUTPUT *******************\n";
    break;
  case Diagnostic240:
    ((std::ostringstream&)*this)
        << "************** FARE FOCUS RULES DIAGNOSTIC ********************\n";
    break;
  case Diagnostic609:
    ((std::ostringstream&)*this)
        << "****************** FARE PATH - PRIORITY QUEUE *****************\n";
    break;
  case Diagnostic614:
    ((std::ostringstream&)*this)
        << "******************** END ON END LIST DISPLAY ******************\n";
    break;
  case Diagnostic631:
    ((std::ostringstream&)*this)
        << "********************** OPEN JAW ANALYSIS **********************\n"
        << "*********************** SUB CATEGORY 101 **********************\n";
    break;
  case Diagnostic632:
    ((std::ostringstream&)*this)
        << "********************* ROUND TRIP ANALYSIS *********************\n"
        << "*********************** SUB CATEGORY 102 **********************\n";
    break;
  case Diagnostic633:
    ((std::ostringstream&)*this)
        << "********************* CIRCLE TRIP ANALYSIS ********************\n"
        << "*********************** SUB CATEGORY 103 **********************\n";
    break;
  case Diagnostic634:
    ((std::ostringstream&)*this)
        << "********************* END ON END ANALYSIS *********************\n"
        << "*********************** SUB CATEGORY 104 **********************\n";
    break;
  case Diagnostic636:
    ((std::ostringstream&)*this)
        << "*************** CARRIER RESTRICTION VALIDATION ****************\n"
        << "*********************** SUB CATEGORY 106 **********************\n";
    break;
  case Diagnostic637:
    ((std::ostringstream&)*this)
        << "******************* RULE TARIFF RESTRICTION *******************\n"
        << "*********************** SUB CATEGORY 107 **********************\n";
    break;
  case Diagnostic638:
    ((std::ostringstream&)*this)
        << "****************** FARE CLASS TYPE RESTRICTION ****************\n"
        << "*********************** SUB CATEGORY 108 **********************\n";
    break;
  case Diagnostic639:
    ((std::ostringstream&)*this)
        << "******************** OPEN JAW SET ANALYSIS ********************\n"
        << "*********************** SUB CATEGORY 109 **********************\n";
    break;
  case Diagnostic640:
    ((std::ostringstream&)*this)
        << "********************** QUALIFYING RULES ***********************\n";
    break;
  case Diagnostic653:
    ((std::ostringstream&)*this)
        << "********************** SAME POINT TABLE ***********************\n"
        << "************************** TABLE 993 **************************\n";
    break;
  case Diagnostic654:
    ((std::ostringstream&)*this)
        << "********************** OVERRIDE DATE TABLE ********************\n"
        << "************************** TABLE 994 **************************\n";
    break;
  case Diagnostic661:
    ((std::ostringstream&)*this)
        << "******************** OPEN JAW DEFINITION **********************\n";
    break;
  case Diagnostic682:
    ((std::ostringstream&)*this)
        << "****************** LIMITATION CHECK - PU LEVEL ****************\n";
    break;
  case Diagnostic699:
    ((std::ostringstream&)*this)
        << "**********************   GROUP FARE PATH   ********************\n";
    break;
  case Diagnostic870:
  case Diagnostic875:
  case Diagnostic876:
  case Diagnostic877:
    ((std::ostringstream&)*this)
        << "******************* SERVICE FEE DIAGNOSTIC ********************\n";
    break;

  case Diagnostic861:
    ((std::ostringstream&)*this)
        << "********************* NVB NVA DIAGNOSTIC **********************\n";
    break;
  case Diagnostic868:
      ((std::ostringstream&)*this)
        << "************** FARE RETAILER RULE DIAGNOSTIC *******************\n";
      break;
  case Diagnostic888:
  case Diagnostic889:
  case Diagnostic890:
  case Diagnostic891:
  case Diagnostic892:
  case Diagnostic894:
  case Diagnostic896:
  case Diagnostic898:
    ((std::ostringstream&)*this)
        << "******************* BRANDED FARES DIAGNOSTIC ******************\n";
    break;

  default:
    ((std::ostringstream&)*this)
        << "*********************** DIAGNOSTIC OUTPUT *********************\n";
    break;
  }
}

void
DiagCollector::printLine()
{
  if (isActive())
    ((std::ostringstream&)*this)
        << "***************************************************************\n";
}

void
DiagCollector::printValue(const std::string& nameOfValue, const bool value)
{
  ((std::ostringstream&)*this) << nameOfValue << ": " << (value ? "Y" : "N") << std::endl;
}

void
DiagCollector::printBrand(const BrandCode& brand)
{
  if (isActive())
    ((std::ostringstream&)*this) << "BRAND CODE : " << brand << "\n";
}

void
DiagCollector::lineSkip(int num)
{
  if (isActive())
  {
    DiagCollector& dc = (DiagCollector&)*this;

    switch (num)
    {
    case 0:
      dc << "--------------------------------------------------------------\n";
      break;
    case 1:
      dc << "  \n";
      break;
    case 2:
      dc << "\n\n";
      break;
    case 3:
      dc << "\n\n\n";
      break;
    }
  }
}

std::string
DiagCollector::cnvFlags(const PaxTypeFare& ptf)
{
  std::string ret;
  if (ptf.isDiscounted())
  {
    if (ptf.isFareByRule())
      ret = "Z";
    else
      ret = "D";
    if (ptf.isNegotiated())
      ret.append("@");
  }
  else if (ptf.isFareByRule())
  {
    ret = "B";
    if (ptf.isNegotiated())
      ret.append("@");
  }
  else if (ptf.isNegotiated())
  {
    ret = "N";
  }
  else if (ptf.fare()->isIndustry())
  {
    const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(ptf.fare());
    if ((indFare != nullptr) && (indFare->isMultiLateral()))
      ret = "M";
    else
      ret = "Y";

    if ((indFare != nullptr) && (!indFare->validForPricing()))
      ret.append("-");
  }
  else if (ptf.fare()->isConstructed())
    ret = "C";
  else
    ret = "P";

  if (ptf.isWebFare())
  {
    if (ret.length() > 1)
      ret.replace(1, 1, "*");
    else
      ret.append("*");
  }

  return ret;
}

void
DiagCollector::flushMsg()
{
  if (_rootDiag && _rootDiag->isActive())
  {
    if (!_useThreadDiag)
    {
      // This is created directly by constructor, instead of
      // DCFactory::create or DCFactory::createThreadDiag
      // We need to update its _rootDiag to use Diagnostic for this thread
      pthread_t pid = pthread_self();
      DCFactory* factory = DCFactory::instance();
      if (factory)
      {
        // right now PricingTrx has member of Diagnostic, Trx does not
        PricingTrx* trx = dynamic_cast<PricingTrx*>(_trx);
        if (trx)
        {
          _rootDiag = &(factory->threadDiag(*trx, pid));
          // _rootDiag should be activated already
          _useThreadDiag = true;
        }
      }
    }
    const std::string& msg = this->str();
    if (!msg.empty())
    {
      _rootDiag->insertDiagMsg(msg);
      this->str("");
    }
  }
}

void
DiagCollector::clear()
{
  if (LIKELY(_rootDiag && _rootDiag->isActive()))
  {
    this->str("");
  }
}

DiagCollector& DiagCollector::operator<< ( DiagCollector& rhs)
{
  if (_rootDiag && _rootDiag->isActive())
  {
    {
      boost::lock_guard<boost::mutex> g(_mutex);
      ((std::ostringstream&)*this) << rhs.str();
    }
    rhs.clear();
  }
  return *this;
}

// for endl to work
DiagCollector& DiagCollector::operator<< (std::ostream& ( *pf )(std::ostream&))
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << pf;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (std::ios_base& ( *pf )(std::ios_base&))
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << pf;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (char ch)
{
  if (isActive())
  {
    if (ch == 0)
      ((std::ostringstream&)*this) << '0';
    else
      ((std::ostringstream&)*this) << ch;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (int i)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << i;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (long l)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << l;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (float f)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << f;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (double d)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << d;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (uint16_t u)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << u;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (uint32_t u)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << u;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (uint64_t u)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << u;
  }
  return *this;
}

DiagCollector& DiagCollector::operator<< (const char *msg)
{
  if (isActive())
  {
    if (msg)
    {
      ((std::ostringstream&)*this) << msg;
    }
  }
  return *this;
}

DiagCollector & DiagCollector::operator<< (const std::string &msg)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << msg;
  }
  return *this;
}

DiagCollector & DiagCollector::operator<< (const boost::container::string &msg)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << msg;
  }
  return *this;
}

/**
   This one supports the format of the Diagnostics 608
   You need to override it if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  std::vector<FarePath*>& fpVect)
{
  if (!_active)
  {
    return *this;
  }

  if (fpVect.empty())
    return *this;

  std::vector<FarePath*>::const_iterator fpathIter = fpVect.begin();
  Itin* itin = (*fpathIter)->itin();
  std::vector<TravelSeg*>::iterator start = itin->travelSeg().begin();
  std::vector<TravelSeg*>::reverse_iterator end = itin->travelSeg().rbegin();
  *this << (*start)->origin()->loc() << "-- " << (*end)->destination()->loc() << std::endl;

  for (; fpathIter != fpVect.end(); ++fpathIter)
  {
    *this << *(*fpathIter);
    *this << std::endl;
  }
  return *this;
}

void
DiagCollector::printFareUsagesInfo(const PricingUnit& pu)
{
  const bool displayDetail =
          _trx->diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLFARES";

  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  for (; fuIter != pu.fareUsage().end(); ++fuIter)
  {
    *this << *(*fuIter);
    *this << " " << DiagnosticUtil::pricingUnitTypeToShortString(pu.puType()) << "\n";

    if(displayDetail && pu.isFareChangedCat10overrideCat25((*fuIter)->paxTypeFare()))
    {
      *this << " CAT25 OVERRIDE FARE MAY BE USED -\n";
      *this << *(pu.getOldFareCat10overrideCat25((*fuIter)->paxTypeFare()));
      *this << "\n";
    }

    RexBaseTrx* rexTrx = dynamic_cast<RexBaseTrx*>(_trx);
    if ((rexTrx != nullptr) && (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE))
    {
      displayRetrievalDate(*(*fuIter)->paxTypeFare());
      *this << "\n";
    }
  }
}

/**
   This one supports the format of the Diagnostics 608
   You need to override it if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  FarePath& fPath)
{
  if (!_active)
  {
    return *this;
  }

  (*this).setf(std::ios::right, std::ios::adjustfield);
  (*this).setf(std::ios::fixed, std::ios::floatfield);
  (*this).precision(2);
  (*this) << std::setw(8) << " AMOUNT: " << fPath.getTotalNUCAmount()
          << " REQUESTED PAXTYPE: " << fPath.paxType()->paxType();

  if (fPath.isExchange())
    (*this) << "    EXCHANGE";
  else if (fPath.isReissue())
    (*this) << "    REISSUE";

  if (!fPath.getBrandCode().empty())
  {
    (*this) << "    BRAND: " << fPath.getBrandCode();
  }

  const PricingTrx* trx = dynamic_cast<const PricingTrx*>(_trx);

  if ((trx != nullptr) && (trx->isAltDates()))
  {
    const std::pair<DateTime, DateTime>* itinDatePair = fPath.itin()->datePair();
    if (itinDatePair != nullptr)
    {
      (*this) << " DT-PAIR: " << itinDatePair->first.dateToString(DDMMM, "") << "-"
              << itinDatePair->second.dateToString(DDMMM, "");
    }
  }
  (*this) << std::endl;

  if (trx && trx->getTrxType() == PricingTrx::MIP_TRX && trx->isFlexFare())
    (*this) << " FLEX FARES GROUP ID: " << fPath.getFlexFaresGroupId() << std::endl;

  if(! fPath.validatingCarriers().empty())
  {
    (*this) << " VALIDATING CXR: ";
    for (CarrierCode valCxr : fPath.validatingCarriers())
      (*this) << valCxr << " ";
    (*this) << std::endl;
  }

  std::vector<PricingUnit*>::const_iterator puIter = fPath.pricingUnit().begin();
  for (; puIter != fPath.pricingUnit().end(); ++puIter)
  {
    if (!(*puIter)->isSideTripPU())
      *this << *(*puIter);
  }

  return *this;
}

/**
   This one supports the format of the Diagnostics 608
   You need to override it if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  PricingUnit& pu)
{
  if (LIKELY(!_active))
    return *this;

  uint16_t i = 0;
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  for (; fuIter != pu.fareUsage().end(); ++fuIter)
  {
    ++i;
    *this << " " << std::setw(3)
          << (std::string)(*fuIter)->paxTypeFare()->fareMarket()->boardMultiCity() << "-"
          << std::setw(2) << (*fuIter)->paxTypeFare()->fareMarket()->governingCarrier() << "-"
          << std::setw(3) << (std::string)(*fuIter)->paxTypeFare()->fareMarket()->offMultiCity()
          << "  ";

    if (i % 4 == 0)
      *this << "\n";
  }
  const PricingTrx* trx = dynamic_cast<const PricingTrx*>(_trx);
  if (trx && trx->getRequest()->getBrandedFareSize() > 1)
  {
    const PaxTypeFare* ptf = pu.fareUsage()[0]->paxTypeFare();
    (*this) << " BRAND IDS: " << AirlineShoppingUtils::getBrandIndexes(*trx, *ptf);
  }

  if(! pu.validatingCarriers().empty())
  {
    (*this) << "    VAL-CXR: ";
    for (CarrierCode valCxr : pu.validatingCarriers())
      (*this) << valCxr << " ";

    (*this) << std::endl;
  }

  if (i % 4 != 0)
    *this << "\n";

  printFareUsagesInfo(pu);

  fuIter = pu.fareUsage().begin();
  for (; fuIter != pu.fareUsage().end(); ++fuIter)
  {

    FareUsage& farU = *(*fuIter);
    if (!farU.sideTripPUs().empty())
    {
      *this << " SIDE TRIP FOR FAREMARKET: " << std::setw(3)
            << (std::string)farU.paxTypeFare()->fareMarket()->boardMultiCity() << "-"
            << std::setw(2) << farU.paxTypeFare()->fareMarket()->governingCarrier() << "-"
            << std::setw(3) << (std::string)farU.paxTypeFare()->fareMarket()->offMultiCity()
            << "\n";
      *this << "----------------" << std::endl;

      std::vector<PricingUnit*>::iterator it = farU.sideTripPUs().begin();
      std::vector<PricingUnit*>::iterator itEnd = farU.sideTripPUs().end();
      for (; it != itEnd; ++it)
        *this << *(*it);

      *this << "----------------" << std::endl;
    }
  }
  return *this;
}

void
DiagCollector::displayRetrievalDate(const PaxTypeFare& fare)
{
  switch (static_cast<unsigned int>(fare.retrievalFlag()))
  {
  case FareMarket::RetrievHistorical:
    *this << " HISTORICAL FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievKeep:
  case FareMarket::RetrievExpndKeep:
  case FareMarket::RetrievExpndKeep | FareMarket::RetrievKeep:
    *this << " KEEP FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievTvlCommence:
    *this << " TRAVEL COMMENCEMENT FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievCurrent:
    *this << " CURRENT FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievLastReissue:
    *this << " LAST REISSUED FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievCurrent | FareMarket::RetrievTvlCommence:
    *this << " CURRENT/TRAVEL COMMENCEMENT FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievHistorical | FareMarket::RetrievKeep:
  case FareMarket::RetrievHistorical | FareMarket::RetrievExpndKeep:
  case FareMarket::RetrievHistorical | FareMarket::RetrievExpndKeep | FareMarket::RetrievKeep:
    *this << " HISTORICAL/KEEP FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievTvlCommence | FareMarket::RetrievKeep:
  case FareMarket::RetrievTvlCommence | FareMarket::RetrievExpndKeep:
  case FareMarket::RetrievTvlCommence | FareMarket::RetrievExpndKeep | FareMarket::RetrievKeep:
    *this << " TRAVEL COMMENCEMENT/KEEP FARE RETRIEVAL DATE ";
    break;
  case FareMarket::RetrievCurrent | FareMarket::RetrievTvlCommence | FareMarket::RetrievKeep:
  case FareMarket::RetrievCurrent | FareMarket::RetrievTvlCommence | FareMarket::RetrievExpndKeep:
  case FareMarket::RetrievCurrent | FareMarket::RetrievTvlCommence | FareMarket::RetrievExpndKeep |
      FareMarket::RetrievKeep:
    *this << " CURRENT/TRAVEL COMMENCEMENT/KEEP FARE RETRIEVAL DATE ";
    break;
  default:
    return;
  }

  *this << fare.retrievalDate().dateToString(YYYYMMDD, "-");
}

void
DiagCollector::displayFareDates(const PaxTypeFare& paxFare)
{
  *this << "     CREATE DATE: " << paxFare.fare()->createDate().dateToString(YYYYMMDD, "-")
        << "      EXPIRE DATE: " << paxFare.fare()->expirationDate().dateToString(YYYYMMDD, "-")
        << "\n"
        << "  EFFECTIVE DATE: " << paxFare.fare()->effectiveDate().dateToString(YYYYMMDD, "-")
        << " DISCONTINUE DATE: " << paxFare.fare()->discDate().dateToString(YYYYMMDD, "-") << "\n";
}

DiagCollector&
DiagCollector::operator<<(PricingUnit::Type puType)
{
  if (!_active)
    return *this;

  DiagCollector& dc = *this;
  switch (puType)
  {
  case PricingUnit::Type::ROUNDTRIP:
    dc << " RT";
    break;
  case PricingUnit::Type::CIRCLETRIP:
    dc << " CT";
    break;
  case PricingUnit::Type::ONEWAY:
    dc << " OW";
    break;
  case PricingUnit::Type::OPENJAW:
    dc << " OJ";
    break;
  default:
    dc << "UNK";
  }
  return *this;
}

DiagCollector & DiagCollector::operator<< (const  FareUsage& fareU)
{
  if (!_active)
    return *this;

  const PaxTypeFare* fare = fareU.paxTypeFare();

  if (fare)
  {
    *this << *fare;
    *this << std::setw(3) << fareU.paxTypeFare()->fcaFareType();

    (*this).setf(std::ios::right, std::ios::adjustfield);
    if (fareU.rec2Cat10())
      *this << std::setw(7) << fareU.rec2Cat10()->sequenceNumber();
    else
      *this << "       ";
  }
  else
    *this << "******** NO FARE TO DISPLAY ********\n";

  return *this;
}

/**
   This one supports the format of the Diagnostics 608
   You need to override it if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  Fare& fare)
{
  if (!_active)
    return *this;

  DiagCollector& dc = *this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(9) << fare.fareClass() << std::setw(4) << fare.vendor() << std::setw(5)
     << fare.tcrRuleTariff() << std::setw(3) << fare.carrier() << std::setw(5) << fare.ruleNumber();

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc << std::setw(8) << Money(fare.fareAmount(), fare.currency());

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(2) << fare.owrt() << std::setw(2)
     << (fare.directionality() == FROM ? "O" : (fare.directionality() == TO ? "I" : " "));

  std::string gd;
  globalDirectionToStr(gd, fare.globalDirection());

  dc << std::setw(3) << gd;
  return *this;
}

DiagCollector & DiagCollector::operator<< (const  std::vector<PricingUnit*>& puVect)
{
  if (!_active)
    return *this;

  return *this;
}

/**
   This one supports the format of the Diagnostics 600
   You need to override if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  FareMarket& fm)
{
  if (isActive())
  {
    *this << fm.boardMultiCity();
    *this << " ";
    *this << fm.offMultiCity();
  }
  return *this;
}

/**
   This one supports the format of the Diagnostics 600
   You need to override if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  MergedFareMarket& mfm)
{
  if (isActive())
  {
    *this << mfm.boardMultiCity();
    *this << " ";
    *this << mfm.offMultiCity();
  }
  return *this;
}

void
DiagCollector::printPuType(PricingUnit::Type puType)
{
  *this << "PU TYPE: ";

  switch (puType)
  {
  case PricingUnit::Type::OPENJAW:
    *this << "101/OJ";
    break;
  case PricingUnit::Type::ROUNDTRIP:
    *this << "102/RT";
    break;
  case PricingUnit::Type::CIRCLETRIP:
    *this << "103/CT";
    break;
  case PricingUnit::Type::ONEWAY:
    *this << "104/OW";
    break;
  case PricingUnit::Type::ROUNDTHEWORLD_SFC:
    *this << "RW SFC";
    break;
  case PricingUnit::Type::CIRCLETRIP_SFC:
    *this << "CT SFC";
    break;
  default:
    *this << "UNKNOWN";
  }

  *this << std::endl;
}

/**
   This one supports the format of the Diagnostics 600
   You need to override if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  ItinPUPath& itinPUPath)
{
  if (!_active)
  {
    return *this;
  }

  PUPath& puPath = *itinPUPath.puPath;
  const Itin& itin = *itinPUPath.itin;

  std::vector<PU*>::const_iterator pathIter = puPath.puPath().begin();
  for (; pathIter != puPath.puPath().end(); ++pathIter)
  {
    PU* pu = (*pathIter);
    printPuType(pu->puType());

    std::vector<MergedFareMarket*>::const_iterator fmIter = pu->fareMarket().begin();
    std::vector<Directionality>::const_iterator dirIt = pu->fareDirectionality().begin();

    for (; fmIter != pu->fareMarket().end() && dirIt != pu->fareDirectionality().end();
         ++fmIter, ++dirIt)
    {
      MergedFareMarket* fm = *fmIter;

      *this << fm->boardMultiCity();
      *this << " ";
      *this << fm->offMultiCity();
      *this << " : ";

      std::vector<TravelSeg*>::const_iterator tvIt = fm->travelSeg().begin();
      for (; tvIt != fm->travelSeg().end(); ++tvIt)
        *this << itin.segmentOrder(*tvIt) << " ";

      if (*dirIt == FROM)
        *this << " DIR: " << 'O';
      else if (*dirIt == TO)
        *this << " DIR: " << 'I';
      else
        *this << " DIR: " << ' ';

      if (!puPath.sideTripPUPath().empty())
      {
        std::map<MergedFareMarket*, std::vector<PUPath*> >::const_iterator it =
            puPath.sideTripPUPath().find(fm);

        // if found ST for this mkt
        if (it != puPath.sideTripPUPath().end())
        {
          *this << "  ";
          std::vector<PUPath*>::const_iterator pathIt = it->second.begin();
          std::vector<PUPath*>::const_iterator pathItEnd = it->second.end();
          for (; pathIt != pathItEnd; ++pathIt)
          {
            *this << " ST:";
            std::vector<PU*>::const_iterator puIt = (*pathIt)->puPath().begin();
            for (; puIt != (*pathIt)->puPath().end(); ++puIt)
            {
              *this << " ";
              std::vector<MergedFareMarket*>::const_iterator fmIt = (*puIt)->fareMarket().begin();
              LocCode dest;
              for (; fmIt != (*puIt)->fareMarket().end(); ++fmIt)
              {
                MergedFareMarket& fm = *(*fmIt);
                *this << fm.boardMultiCity() << "-";
                dest = fm.offMultiCity();
              }
              *this << dest << " ";
            }
            if (pathIt != pathItEnd - 1)
            {
              *this << "\n                       ";
            }
          }
        }
      } // if side trip

      *this << std::endl;
    }
  }

  if (!puPath.sideTripPUPath().empty())
  {
    std::map<MergedFareMarket*, std::vector<PUPath*> >::const_iterator iter =
        puPath.sideTripPUPath().begin();
    for (; iter != puPath.sideTripPUPath().end(); ++iter)
    {
      std::vector<PUPath*>::const_iterator pathIt = iter->second.begin();
      for (; pathIt != iter->second.end(); ++pathIt)
      {
        *this << iter->first->boardMultiCity();
        *this << "-";
        *this << iter->first->offMultiCity();
        *this << " SIDE TRIPS:\n";
        ItinPUPath tmp;
        tmp.itin = itinPUPath.itin;
        tmp.puPath = *pathIt;
        *this << tmp;
      }
    }
  }

  return *this;
}

/**
   This one supports the format of the Diagnostics 600
   You have to override this if you need different format.
*/
DiagCollector & DiagCollector::operator<< (const  Itin& itin)
{
  if (!_active)
  {
    return *this;
  }

  LocCode prevDestAirport;
  std::vector<TravelSeg*>::const_iterator tvlIter = itin.travelSeg().begin();
  for (; tvlIter != itin.travelSeg().end(); ++tvlIter)
  {
    TravelSeg* tvlS = *tvlIter;

    if (prevDestAirport != tvlS->origAirport())
    { //"X AA 1234 20JUN S80 "
      *this << "                      " << tvlS->origAirport() << " " << tvlS->boardMultiCity()
            << std::endl;
    }

    ArunkSeg* arunkS = dynamic_cast<ArunkSeg*>(tvlS);
    if (!arunkS)
    {
      if (tvlS->stopOver() || tvlS->isForcedStopOver())
        *this << "O ";
      else
        *this << "X ";
    }
    else
    {
      *this << "  ";
    }

    AirSeg* airS = dynamic_cast<AirSeg*>(tvlS);
    if (airS)
    {
      *this << std::setw(4) << airS->carrier() << " " << std::setw(4) << airS->flightNumber()
            << " ";
    }
    else
    {
      *this << "          ";
    }

    if (arunkS)
      *this << "      ";
    else
      *this << tvlS->departureDT().dateToString(DDMMM, "") << " ";

    *this << std::setw(3) << tvlS->equipmentType() << " ";

    *this << tvlS->destAirport() << " " << tvlS->offMultiCity() << " ";

    *this << tvlS->getBookingCode() << std::endl;

    prevDestAirport = tvlS->destAirport();
  }
  return *this;
}

std::string
DiagCollector::printItinMIP(const Itin& itin)
{
  std::ostringstream ss;
  ss.setf(ios_base::right, ios_base::adjustfield);

  std::vector<TravelSeg*>::const_iterator tSegIt = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tSegEndIt = itin.travelSeg().end();

  for (int fltIndex = 1; tSegIt != tSegEndIt; ++tSegIt, ++fltIndex)
  {
    if (!*tSegIt)
      continue;

    std::string leg(" -");
    std::ostringstream os;

    // Prepare leg index. -1 == no leg information.
    if (0 <= (*tSegIt)->legId())
    {
      os << ((*tSegIt)->legId() + 1);
      leg = os.str();
      os.str("");
    }

    const AirSeg* aSegPtr = dynamic_cast<const AirSeg*>(*tSegIt);

    if (!aSegPtr)
    {
      const TravelSeg& tSeg = **tSegIt;

      // Example: " 3 1 LAX SJC - ARUNK" or " 3 - LAX SJC - ARUNK"
      ss << std::setw(2) << fltIndex // int,     usually 1 char
         << std::setw(2) << leg // int16_t, usually 1 char
         << " " << std::setw(3) << tSeg.origin()->loc() // Code<8>, always? 3 chars
         << " " << std::setw(3) << tSeg.destination()->loc() // Code<8>, always? 3 chars
         << " - ARUNK";
    }
    else
    {
      const AirSeg& aSeg = *aSegPtr;

      const uint32_t timeLen = 5;
      const DateTime& depDT = aSeg.departureDT();
      const DateTime& arrDT = aSeg.arrivalDT();
      std::string depTime = depDT.timeToString(HHMM_AMPM, "");
      std::string arrTime = arrDT.timeToString(HHMM_AMPM, "");
      std::string dow; // day of week

      // Prepare departure and arrival times. Remove trailing 'M'.
      depTime = depTime.substr(0, depTime.length() - 1);
      arrTime = arrTime.substr(0, arrTime.length() - 1);

      if (depTime.length() < timeLen)
        depTime = '0' + depTime;
      if (arrTime.length() < timeLen)
        arrTime = "0" + arrTime;

      // Prepare day of week. Substitute codes for Thursday and Saturday.
      os << depDT.dayOfWeek();
      dow = os.str();
      os.str("");

      switch (toupper(dow.at(0)))
      {
      case 'T':
        if (toupper(dow.at(1)) == 'H')
          dow = 'Q'; // Q for Thursday
        else
          dow = 'T'; // T for Tuesday
        break;

      case 'S':
        if (toupper(dow.at(1)) == 'A')
          dow = 'J'; // J for Saturday
        else
          dow = 'S'; // S for Sunday
        break;

      default:
        dow = toupper(dow.at(0));
        break;
      }

      // Example: " 4 1 UA 1234 0 8 29SEP W GDL TIJ 0950P 1050P 320 0 /E"
      ss << std::setw(2) << fltIndex // int,       usually 1 char
         << std::setw(2) << leg // int16_t,   usually 1 char
         << std::setw(3) << aSeg.carrier() // Code<3>,   usually 2 chars
         << " " << std::setw(4) << aSeg.flightNumber() // int32_t,   up to 4 chars
         << std::setw(2) << aSeg.getBookingCode() // Code<2>,   usually 1 char
         << std::setw(2) << aSeg.bookedCabin() // CabinType, usually?/always? 1 char
         << " " << depDT.dateToString(DDMMM, "") // string,    5 chars
         << " " << dow // string,    1 char
         << " " << std::setw(3) << aSeg.origin()->loc() // Code<8>,   always? 3 chars
         << " " << std::setw(3) << aSeg.destination()->loc() // Code<8>,   always? 3 chars
         << " " << depTime // string,    5 chars
         << " " << arrTime // string,    5 chars
         << " " << std::setw(3) << aSeg.equipmentType() // Code<3>,   usually?/always? 3 chars
         << " " << aSeg.hiddenStops().size(); // uint,      usually 1 char

      if (aSeg.eticket())
        ss << " /E"; // string,    0 or 2 chars
    }

    ss << "\n";
  } // for fltIndex
  return ss.str();
}

DiagCollector & DiagCollector::operator<< (const  PaxTypeFare& fare)
{
  if (!_active)
  {
    return *this;
  }

  DiagCollector& dc = *this;

  std::string fareBasis = fare.createFareBasis(nullptr);
  if (fareBasis.size() > getFareBasicWide())
  {
    fareBasis = fareBasis.substr(0, (getFareBasicWide() - 1)) + "*";
  }

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(13) << fareBasis << " " << std::setw(4) << fare.vendor() << std::setw(5)
     << fare.tcrRuleTariff() << std::setw(3) << fare.carrier() << std::setw(5) << fare.ruleNumber();

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc << std::setw(8) << Money(fare.fareAmount(), fare.currency());

  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
  if (pricingTrx)
    if (pricingTrx->awardRequest())
    {
      dc.setf(std::ios::right);
      dc << std::setw(8) << " " << fare.mileage() << " MIL";
    }

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " " << std::setw(1) << fare.owrt() << std::setw(2)
     << (fare.directionality() == FROM ? "O" : (fare.directionality() == TO ? "I" : " "));

  std::string gd;
  globalDirectionToStr(gd, fare.globalDirection());

  dc << std::setw(3) << gd;

  return *this;
}

DiagCollector & DiagCollector::operator<< (const Money& x)
{
  if (isActive())
  {
    ((std::ostringstream&)*this) << x;
  }
  return *this;
}

DiagCollector & DiagCollector::operator<< (const  TaxCodeReg& taxCodeReg)
{
  if (!_active)
  {
    return *this;
  }

  *this << taxCodeReg.taxCode() << "  SEQUENCE NUMBER: " << taxCodeReg.seqNo() << "\n"
        << taxCodeReg.taxCode() << "  SPECIAL TAX NUMBER: " << taxCodeReg.specialProcessNo() << "\n"
        << taxCodeReg.taxCode() << "  DECIMAL PLACE: " << taxCodeReg.taxNodec() << "\n"
        << taxCodeReg.taxCode() << "  ROUND DECIMAL PLACE: " << taxCodeReg.taxcdRoundUnitNodec()
        << "\n" << taxCodeReg.taxCode() << "  DISCOUNT PERCENTAGE: " << taxCodeReg.discPercent()
        << "\n" << taxCodeReg.taxCode()
        << "  DISCOUNT PERCENTAGE DECIMAL PLACE: " << taxCodeReg.discPercentNodec() << "\n"
        << taxCodeReg.taxCode() << "  TAX AMOUNT: " << taxCodeReg.taxAmt() << "\n"
        << taxCodeReg.taxCode() << "  MINIMUM TAX: " << taxCodeReg.minTax() << "\n"
        << taxCodeReg.taxCode() << "  MAXIMUM TAX: " << taxCodeReg.maxTax() << "\n"
        << taxCodeReg.taxCode() << "  PLUS UP AMOUNT: " << taxCodeReg.plusupAmt() << "\n"
        << taxCodeReg.taxCode() << "  LOW RANGE: " << taxCodeReg.lowRange() << "\n"
        << taxCodeReg.taxCode() << "  HIGH RANGE: " << taxCodeReg.highRange() << "\n"
        << taxCodeReg.taxCode() << "  RANGE INCREMENT: " << taxCodeReg.rangeincrement() << "\n"
        << taxCodeReg.taxCode() << "  FARE RANGE DECIMAL PLACE: " << taxCodeReg.fareRangeNodec()
        << "\n" << taxCodeReg.taxCode() << "  LOCATION 1 EXCLUDED: " << taxCodeReg.loc1ExclInd()
        << "\n" << taxCodeReg.taxCode() << "  LOCATION 1: " << taxCodeReg.loc1() << "\n"
        << taxCodeReg.taxCode() << "  LOCATION 1 TYPE: " << taxCodeReg.loc1Type() << "\n"
        << taxCodeReg.taxCode() << "  LOCATION 2 EXCLUDED: " << taxCodeReg.loc2ExclInd() << "\n"
        << taxCodeReg.taxCode() << "  LOCATION 2: " << taxCodeReg.loc2() << "\n"
        << taxCodeReg.taxCode() << "  LOCATION 2 TYPE: " << taxCodeReg.loc2Type() << "\n"
        << taxCodeReg.taxCode() << "  TYPE: " << taxCodeReg.taxType() << "\n"
        << taxCodeReg.taxCode() << "  TAX NATION: " << taxCodeReg.nation() << "\n"
        << taxCodeReg.taxCode() << "  ROUND RULE: " << taxCodeReg.taxcdRoundRule() << "\n"
        << taxCodeReg.taxCode() << "  FULL FARE: " << taxCodeReg.taxfullFareInd() << "\n"
        << taxCodeReg.taxCode() << "  EQUIVALENT AMOUNT: " << taxCodeReg.taxequivAmtInd() << "\n"
        << taxCodeReg.taxCode() << "  EXCESS BAGGAGE: " << taxCodeReg.taxexcessbagInd() << "\n"
        << taxCodeReg.taxCode() << "  TRAVEL DATE ORIGIN: " << taxCodeReg.tvlDateasoriginInd()
        << "\n" << taxCodeReg.taxCode() << "  DISPLAY ONLY: " << taxCodeReg.displayonlyInd() << "\n"
        << taxCodeReg.taxCode() << "  FEE: " << taxCodeReg.feeInd() << "\n" << taxCodeReg.taxCode()
        << "  INTERLINABLE: " << taxCodeReg.interlinableTaxInd() << "\n" << taxCodeReg.taxCode()
        << "  SEPARATE: " << taxCodeReg.showseparateInd() << "\n" << taxCodeReg.taxCode()
        << "  POINT OF SALE EXCLUDED: " << taxCodeReg.posExclInd() << "\n" << taxCodeReg.taxCode()
        << "  POINT OF SALE LOCATION TYPE: " << taxCodeReg.posLocType() << "\n"
        << taxCodeReg.taxCode() << "  POINT OF SALE LOCATION: " << taxCodeReg.posLoc() << "\n"
        << taxCodeReg.taxCode() << "  POINT OF ISSUANCE EXCLUDE: " << taxCodeReg.poiExclInd()
        << "\n" << taxCodeReg.taxCode()
        << "  POINT OF ISSUANCE LOCATION TYPE: " << taxCodeReg.poiLocType() << "\n"
        << taxCodeReg.taxCode() << "  POINT OF ISSUANCE LOCATION: " << taxCodeReg.poiLoc() << "\n"
        << taxCodeReg.taxCode() << "  SELL CURRENCY EXCLUDE: " << taxCodeReg.sellCurExclInd()
        << "\n" << taxCodeReg.taxCode() << "  SELL CURRENCY: " << taxCodeReg.sellCur() << "\n"
        << taxCodeReg.taxCode() << "  OCCURENCE: " << taxCodeReg.occurrence() << "\n"
        << taxCodeReg.taxCode() << "  FREE TICKET EXEMPT: " << taxCodeReg.freeTktexempt() << "\n"
        << taxCodeReg.taxCode() << "  ID TRAVEL EXEMPT: " << taxCodeReg.idTvlexempt() << "\n"
        << taxCodeReg.taxCode() << "  RANGE TYPE: " << taxCodeReg.rangeType() << "\n"
        << taxCodeReg.taxCode() << "  RANGE INDICATOR: " << taxCodeReg.rangeInd() << "\n"
        << taxCodeReg.taxCode()
        << "  NEXT STOP OVER RESTRICTION: " << taxCodeReg.nextstopoverrestr() << "\n"
        << taxCodeReg.taxCode() << "  SPECIAL ROUNDING: " << taxCodeReg.spclTaxRounding() << "\n"
        << taxCodeReg.taxCode() << "  CURRENCY: " << taxCodeReg.taxCur() << "\n"
        << taxCodeReg.taxCode() << "  CURRENCY DECIMAL PLACES: " << taxCodeReg.taxCurNodec() << "\n"
        << taxCodeReg.taxCode() << "  FARE CLASS EXCLUDE: " << taxCodeReg.fareclassExclInd() << "\n"
        << taxCodeReg.taxCode() << "  TICKET DESIGNATOR EXCLUDE: " << taxCodeReg.tktdsgExclInd()
        << "\n" << taxCodeReg.taxCode() << "  CARRIER EXCLUDE: " << taxCodeReg.valcxrExclInd()
        << "\n" << taxCodeReg.taxCode()
        << "  EXEMPT EQUIPMENT EXCLUDE: " << taxCodeReg.exempequipExclInd() << "\n"
        << taxCodeReg.taxCode() << "  PASSENGER EXCLUDE: " << taxCodeReg.psgrExclInd() << "\n"
        << taxCodeReg.taxCode() << "  EXEMPT CXR EXCLUDE: " << taxCodeReg.exempcxrExclInd() << "\n"
        << taxCodeReg.taxCode() << "  FARE TYPE EXCLUDE: " << taxCodeReg.fareTypeExclInd() << "\n"
        << taxCodeReg.taxCode() << " MULTI OCCURRENCE: " << taxCodeReg.multioccconvrndInd() << "\n"
        << taxCodeReg.taxCode() << "  ORIGIN LOCATION TYPE: " << taxCodeReg.originLocType() << "\n"
        << taxCodeReg.taxCode() << "  ORIGIN LOCATION: " << taxCodeReg.originLoc() << "\n"
        << taxCodeReg.taxCode() << "  ORIGIN LOCATION EXCLUDE: " << taxCodeReg.originLocExclInd()
        << "\n" << taxCodeReg.taxCode() << "  LOCATION 1 APPLY: " << taxCodeReg.loc1Appl() << "\n"
        << taxCodeReg.taxCode() << "  LOCATION 2 APPLY: " << taxCodeReg.loc2Appl() << "\n"
        << taxCodeReg.taxCode() << "  TRIP TYPE: " << taxCodeReg.tripType() << "\n"
        << taxCodeReg.taxCode() << "  TRAVEL TYPE: " << taxCodeReg.travelType() << "\n"
        << taxCodeReg.taxCode() << "  ITINERARY TYPE: " << taxCodeReg.itineraryType() << "\n"
        << taxCodeReg.taxCode() << "  TAX ON TAX EXCLUDE: " << taxCodeReg.taxOnTaxExcl() << "\n"
        << taxCodeReg.taxCode() << " SPECIAL CONFIGURATION: " << taxCodeReg.specConfigName() << "\n"
        << "\n";

  return *this;
}

DiagCollector&
DiagCollector::operator<< (const TaxResponse& taxResponse)
{
  if (!_active)
  {
    return (*this);
  }

  (*this) << std::endl << "TAX RESPONSE:" << std::endl
          << "  PAX TYPE: " << taxResponse.paxTypeCode() << std::endl;

  if (!(taxResponse.pfcItemVector().empty()))
  {
    (*this) << std::endl << "PFC ITEMS (" << taxResponse.pfcItemVector().size()
            << "): " << std::endl;

    std::vector<PfcItem*>::const_iterator pfcItemIter = taxResponse.pfcItemVector().begin();
    std::vector<PfcItem*>::const_iterator pfcItemIterEnd = taxResponse.pfcItemVector().end();

    (*this) << "CXR LEG PFC AIRPORT PFC      PFC CURRENCY\n"
            << "    ID  CODE        AMOUNT   CODE        \n"
            << "--- --- ----------- -------- ------------\n";

    for (; pfcItemIter != pfcItemIterEnd; ++pfcItemIter)
    {
      const PfcItem* pfcItem = (*pfcItemIter);

      (*this) << (*pfcItem);
    }
  }

  if (!(taxResponse.taxRecordVector().empty()))
  {
    (*this) << std::endl << "TAX RECORDS (" << taxResponse.taxRecordVector().size()
            << "): " << std::endl;

    std::vector<TaxRecord*>::const_iterator taxRecordIter = taxResponse.taxRecordVector().begin();
    std::vector<TaxRecord*>::const_iterator taxRecordIterEnd = taxResponse.taxRecordVector().end();

    (*this) << "CXR LEG LOCAL PUBLISHED PUBLISHED     TAX      TAX  TAX CURRENCY TAX    TAX \n"
            << "    ID  BOARD AMOUNT    CURRENCY CODE AMOUNT   CODE CODE         NATION TYPE\n"
            << "--- --- ----- --------- ------------- -------- ---- ------------ ------ ----\n";

    for (; taxRecordIter != taxRecordIterEnd; ++taxRecordIter)
    {
      const TaxRecord* taxRecord = (*taxRecordIter);

      (*this) << (*taxRecord);
    }
  }

  if (!(taxResponse.taxItemVector().empty()))
  {
    (*this) << std::endl << "TAX ITEMS (" << taxResponse.taxItemVector().size()
            << "): " << std::endl;

    TaxResponse::TaxItemVector::const_iterator taxItemIter = taxResponse.taxItemVector().begin();
    TaxResponse::TaxItemVector::const_iterator taxItemIterEnd = taxResponse.taxItemVector().end();

    (*this) << "CXR INTERLINE LEG PAYMENT  TAX      TRAVEL SEG TRAVEL SEG\n"
            << "    FLAG      ID  CURRENCY AMOUNT   START      END       \n"
            << "--- --------- --- -------- -------- ---------- ----------\n";

    for (; taxItemIter != taxItemIterEnd; ++taxItemIter)
    {
      const TaxItem* taxItem = (*taxItemIter);

      (*this) << (*taxItem);
    }
  }

  return (*this);
}

DiagCollector&
DiagCollector::operator<< (const PfcItem& pfcItem)
{
  if (!_active)
  {
    return (*this);
  }
  DiagCollector& dc(*this);

  dc << std::setw(3) << pfcItem.carrierCode() << std::setw(1) << " ";
  dc << std::setw(3) << pfcItem.legId() << std::setw(1) << " ";
  dc << std::setw(11) << pfcItem.pfcAirportCode() << std::setw(1) << " ";
  dc << std::setw(8) << pfcItem.pfcAmount() << std::setw(1) << " ";
  dc << std::setw(12) << pfcItem.pfcCurrencyCode() << std::setw(1) << " ";
  dc << std::endl;

  return (*this);
}

DiagCollector&
DiagCollector::operator<< (const TaxRecord& taxRecord)
{
  if (!_active)
  {
    return (*this);
  }
  DiagCollector& dc(*this);

  dc << std::setw(3) << taxRecord.carrierCode() << std::setw(1) << " ";
  dc << std::setw(3) << taxRecord.legId() << std::setw(1) << " ";
  dc << std::setw(5) << taxRecord.localBoard() << std::setw(1) << " ";
  dc << std::setw(9) << taxRecord.publishedAmount() << std::setw(1) << " ";
  dc << std::setw(13) << taxRecord.publishedCurrencyCode() << std::setw(1) << " ";
  dc << std::setw(8) << taxRecord.getTaxAmount() << std::setw(1) << " ";
  dc << std::setw(4) << taxRecord.taxCode() << std::setw(1) << " ";
  dc << std::setw(12) << taxRecord.taxCurrencyCode() << std::setw(1) << " ";
  dc << std::setw(6) << taxRecord.taxNation() << std::setw(1) << " ";
  dc << std::setw(4) << taxRecord.taxType() << std::setw(1) << " ";
  dc << std::endl;

  return (*this);
}

DiagCollector&
DiagCollector::operator<< (const TaxItem& taxItem)
{
  if (!_active)
  {
    return (*this);
  }
  DiagCollector& dc(*this);

  dc << std::setw(3) << taxItem.carrierCode() << std::setw(1) << " ";
  dc << std::setw(9) << taxItem.interline() << std::setw(1) << " ";
  dc << std::setw(3) << taxItem.legId() << std::setw(1) << " ";
  dc << std::setw(8) << taxItem.paymentCurrency() << std::setw(1) << " ";
  dc << std::setw(8) << taxItem.taxAmount() << std::setw(1) << " ";
  dc << std::setw(10) << taxItem.travelSegStartIndex() << std::setw(1) << " ";
  dc << std::setw(10) << taxItem.travelSegEndIndex() << std::setw(1) << " ";
  dc << std::endl;

  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const std::vector<TravelSeg*>& x)
{
  if (!_active)
    return *this;

  std::vector<TravelSeg*>::const_iterator tvlSegI = x.begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = x.end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    const TravelSeg* tvlSeg = *tvlSegI;

    *this << tvlSeg->origAirport() << tvlSeg->destAirport() << " ";
  }
  return *this;
}

DiagCollector&
DiagCollector::operator<<(const GroupFarePath& x)
{
  return *this << x.groupFPPQItem();
}

DiagCollector&
DiagCollector::operator<<(const std::vector<FPPQItem*>& fppqItems)
{
  if (isActive())
  {
    for (const FPPQItem* item : fppqItems)
      *this << *item->farePath();
  }

  return *this;
}

DiagCollector&
DiagCollector::operator<<(const Record3ReturnTypes& x)
{
  switch (x)
  {
  case PASS:
    (*this) << "PASS";
    break;
  case SOFTPASS:
    (*this) << "SOFTPASS";
    break;
  case FAIL:
    (*this) << "FAIL";
    break;
  case SKIP:
    (*this) << "SKIP";
    break;
  case STOP:
    (*this) << "STOP";
    break;
  case NOTPROCESSED:
    (*this) << "NOTPROCESSED";
    break;
  default:
    (*this) << static_cast<uint16_t>(x);
    break;
  }
  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const CabinType& x)
{
  (*this) << x.getCabinIndicator();
  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const FareTypeDesignator &x)
{
  (*this) << x.fareTypeDesig();
  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const PenaltyInfo& x)
{
  (*this) << "      NOREFUNDIND       : " << x.noRefundInd()
          << "\n      PENALTYAPPL       : " << x.penaltyAppl()
          << "\n      ISNONREF"
          << "\n       CANCELREFUNDAPPL : " << x.cancelRefundAppl()
          << "\n       PENALTYREFUND    : " << x.penaltyRefund()
          << "\n       PENALTYCANCEL    : " << x.penaltyCancel()
          << "\n      ISNONCHG"
          << "\n       VOLAPPL          : " << x.volAppl()
          << "\n       PENALTYREISSUE   : " << x.penaltyReissue()
          << "\n       PENALTYNOREISSUE : " << x.penaltyNoReissue()
          << "\n      PENALTY"
          << "\n       PERCENT          : " << x.penaltyPercent()
          << "\n       AMT 1,2          : " << x.penaltyAmt1() << "," << x.penaltyAmt2()
          << "\n       CUR 1,2          : " << x.penaltyCur1() << "," << x.penaltyCur2()
          << "\n      UNAVAILTAG        : " << x.unavailTag() << "\n";

  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const VoluntaryChangesInfoW& x)
{
  (*this) << "     ORIGINAL RECORD : \n";
  (*this) << *x.orig();

  if (x.overriding())
  {
    (*this) << " OVERRIDED BY RECORD : \n";
    (*this) << *x.overriding();
  }

  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const VoluntaryChangesInfo& x)
{
  (*this) << "     ITEMNO : " << x.itemNo()
          << "\n     DEPARTUREIND : " << x.departureInd()
          << "\n     PUIND : "      << x.priceableUnitInd()
          << "\n     FCIND : "      << x.fareComponentInd()
          << "\n     CHANGEIND : "  << x.changeInd()
          << "\n     DOMINTLIND : " << x.domesticIntlComb()
          << "\n      PERCENT     : " << x.percent()
          << "\n      AMT 1,2     : " << x.penaltyAmt1() << "," << x.penaltyAmt2()
          << "\n      CUR 1,2     : " << x.cur1() << "," << x.cur2()
          << "\n";

  return (*this);
}

DiagCollector&
DiagCollector::operator<<(const VoluntaryRefundsInfo& x)
{
  (*this) << "     DEPARTUREIND : " << x.depOfJourneyInd()
          << "\n     PUIND : "      << x.puInd()
          << "\n     FCIND : "      << x.fareComponentInd()
          << "\n      PERCENT     : " << x.penaltyPercent()
          << "\n      AMT 1,2     : " << x.penalty1Amt() << "," << x.penalty2Amt()
          << "\n      CUR 1,2     : " << x.penalty1Cur() << "," << x.penalty2Cur() << "\n";
  return (*this);
}

uint32_t
DiagCollector::getCurrentMessageLength(std::string& outMsg)
{
  if (!_active)
  {
    return (0);
  }

  outMsg = this->str();
  if (outMsg.empty())
  {
    return (0);
  }
  uint32_t sz = outMsg.length();

  return (sz);
}

void
DiagCollector::setDiagWrapMode(DiagCollector::DiagWrapMode wrapMode)
{
  _wrapMode = wrapMode;
  _alignmentMark = 0;
  _lineWrapLength = 0;
  _lineWrapAnchor = 0;
}

void
DiagCollector::setAlignmentMark()
{
  std::string msg;
  uint32_t alignStart = getCurrentMessageLength(msg);
  std::string searchString("\n");
  std::string::size_type srchIndex = msg.find_last_of(searchString);
  if (srchIndex == std::string::npos)
  {
    _alignmentMark = 0;
    return;
  }

  uint32_t previousEndLine = static_cast<uint32_t>(srchIndex);
  _alignmentMark = alignStart - previousEndLine;
}

void
DiagCollector::setLineWrapAnchor()
{
  std::string msg = this->str();
  std::string searchString("\n");
  std::string::size_type srchIdx = msg.find_last_of(searchString);
  if (srchIdx == std::string::npos)
  {
    _lineWrapAnchor = 0;
    return;
  }
  uint32_t previousEndLine = static_cast<uint32_t>(srchIdx);
  _lineWrapAnchor = previousEndLine;
}

void
DiagCollector::setLineWrapLength(uint32_t lineWrapLength)
{
  _lineWrapLength = lineWrapLength;
}

bool
DiagCollector::wrapLine()
{
  if (!_active)
  {
    return (false);
  }

  if (_lineWrapLength == 0)
  {
    return (false);
  }

  switch (_wrapMode)
  {
  case DiagWrapNone:
    // Nothing to be done here
    break;
  case DiagWrapSimple:
    // Simple wrapping method
    performSimpleLineWrap();
    return (_wrappedLine);
  case DiagWrapAligned:
    performAlignedLineWrap();
    return (_wrappedLine);
  }

  return (false);
}

void
DiagCollector::performSimpleLineWrap()
{
  _wrappedLine = false;
  std::string msg;
  uint32_t curMsgLen = getCurrentMessageLength(msg);
  uint32_t anchorDiff = curMsgLen - _lineWrapAnchor;
  if (anchorDiff >= _lineWrapLength)
  {
    ((std::ostringstream&)*this) << "\n";
    _wrappedLine = true;
  }
}

void
DiagCollector::performAlignedLineWrap()
{
  // Wrap the line if needed
  performSimpleLineWrap();
  if (!_wrappedLine || _alignmentMark == 0)
  {
    return;
  }

  // Align the output
  std::ostringstream& strRef = *this;
  for (uint32_t j = 0; j < _alignmentMark + 1; j++)
  {
    strRef << " ";
  }
}

std::string
DiagCollector::getRelationString(uint32_t relationInd) const
{
  std::string relation;

  switch (relationInd)
  {
  case CategoryRuleItemInfo::IF:
    relation = "IF";
    break;

  case CategoryRuleItemInfo::THEN:
    relation = "THEN";
    break;

  case CategoryRuleItemInfo::OR:
    relation = "OR";
    break;

  case CategoryRuleItemInfo::AND:
    relation = "AND";
    break;

  case CategoryRuleItemInfo::ELSE:
    relation = "ELSE";
    break;

  default:
    relation = "UNKN";
    break;
  }
  return relation;
}

void
DiagCollector::displayRelation(const CategoryRuleItemInfo* rule, Record3ReturnTypes statusRule)
{
  if (isActive())
  {
    std::string relation(getRelationString(rule->relationalInd()));

    (*this).setf(std::ios::left, std::ios::adjustfield);

    *this << "     ";
    *this << std::setw(5) << rule->itemcat();
    *this << std::setw(7) << rule->itemNo();
    *this << std::setw(12) << relation;
    *this << std::setw(8) << getStatusString(statusRule);

    *this << std::endl;
  }
}

bool
DiagCollector::parseFareMarket(PricingTrx& trx, const FareMarket& fareMarket)
{
  const DiagParamMap::const_iterator e = trx.diagnostic().diagParamMap().end();

  // check to see if we care about specified faremarket

  if (trx.excTrxType() == PricingTrx::AR_EXC_TRX)
  {
    const DiagParamMap::const_iterator h =
        trx.diagnostic().diagParamMap().find(Diagnostic::FARE_ASSIGNMENT);

    if (h != e && h->second[0] != fareMarket.fareRetrievalFlagToStr(fareMarket.retrievalFlag())[0])
      return false;
  }

  const DiagParamMap::const_iterator i =
      trx.diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);

  if (i != e)
  {
    LocCode boardCity = i->second.substr(0, 3);
    LocCode offCity = i->second.substr(3, 3);

    if (((fareMarket.origin()->loc() != boardCity) && (fareMarket.boardMultiCity() != boardCity)) ||
        ((fareMarket.destination()->loc() != offCity) && (fareMarket.offMultiCity() != offCity)))
      return false;
  }
  return true;
}

void
DiagCollector::displayFltTktMerchInd(Indicator ind)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  if (ind == 'F')
    dc << "FLIGHT   ";
  else if (ind == 'T')
    dc << "TICKET   ";
  else if (ind == 'R')
    dc << "RULE     ";
  else if (ind == 'M')
    dc << "MERCHANT ";
  else if (ind == 'A')
    dc << "ALLOWANCE";
  else if (ind == 'C')
    dc << "CHARGE   ";
  else if (ind == 'P')
    dc << "PREPAID  ";
  else if (ind == 'B')
    dc << "CARRYFBA ";
  else if (ind == 'E')
    dc << "EMBARGO  ";
  else
    dc << "         ";
}

void
DiagCollector::printStopAtFirstMatchMsg() const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " *** STOP AFTER FIRST MATCH - NO GROUP CODE IN THE ENTRY ***\n";
}

void
DiagCollector::displayTFDPSC(DiagCollector& dc,
                             const std::vector<const NegFareRestExtSeq*>& negFareRestExtSeq,
                             bool displayMatchedSeqs)
{
  dc.setf(std::ios::left, std::ios::adjustfield);

  if (displayMatchedSeqs)
  {
    dc << "---------- MATCHED SEQUENCES FOR TICKETED FARE DATA ---------- \n"
       << "SEQNO  FRM TO  CXR VIA POINTS      FARE BASIS   SPNV\n";
  }
  else
  {
    dc << "  TICKETED FARE DATA PER SEGMENT/COMPONENT :\n"
          "  SEQNO  FRM TO  CXR VIA POINTS      FARE BASIS  UNFBC    SPNV\n";
  }

  for (const NegFareRestExtSeq* restExtSeq : negFareRestExtSeq)
  {
    std::string viaCity;
    viaCity += restExtSeq->viaCity1();
    viaCity += (viaCity.empty() || restExtSeq->viaCity2().empty() ? "" : "/");
    viaCity += restExtSeq->viaCity2();
    viaCity += (viaCity.empty() || restExtSeq->viaCity3().empty() ? "" : "/");
    viaCity += restExtSeq->viaCity3();
    viaCity += (viaCity.empty() || restExtSeq->viaCity4().empty() ? "" : "/");
    viaCity += restExtSeq->viaCity4();

    if (!displayMatchedSeqs)
      dc << "  ";
    dc << std::setw(7) << restExtSeq->seqNo() << std::setw(4) << restExtSeq->cityFrom()
       << std::setw(4) << restExtSeq->cityTo() << std::setw(4) << restExtSeq->carrier()
       << std::setw(16) << viaCity << std::setw(12) << restExtSeq->publishedFareBasis();
    if (!displayMatchedSeqs)
      dc << std::setw(8) << restExtSeq->uniqueFareBasis();
    dc << (restExtSeq->suppressNvbNva() != RuleConst::BLANK ? " Y\n" : " N\n");
  }
}

void
DiagCollector::displayFareMarketItinNums(const std::string& title,
                                         const PricingTrx& trx,
                                         const FareMarket& fareMarket)
{
  if (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ITNUMS")
  {
    DiagCollector& dc(*this);
    dc << title;
    for (const auto itin : trx.itin())
    {
      std::vector<FareMarket*>::const_iterator jt =
          std::find(itin->fareMarket().begin(), itin->fareMarket().end(), &fareMarket);
      if (jt != itin->fareMarket().end())
        dc << " " << itin->itinNum();
    }
    dc << "\n";
  }
}

void
DiagCollector::displayFareMarketItinNumsMultiTkt(const std::string& title,
                                                 const PricingTrx& trx,
                                                 const FareMarket& fareMarket)
{
  DiagCollector& dc(*this);
  dc << title;
  bool isMainItin = false;

  for (const auto itin : trx.itin())
  {
    std::vector<FareMarket*>::const_iterator jt =
        std::find(itin->fareMarket().begin(), itin->fareMarket().end(), &fareMarket);
    if (jt != itin->fareMarket().end())
    {
      switch (itin->getMultiTktItinOrderNum())
      {
      case 0:
        dc << " ITIN";
        isMainItin = true;
        break;
      case 1:
      case 2:
        if (isMainItin)
          dc << ", ";
        dc << "SUB-ITIN " << itin->getMultiTktItinOrderNum();
        break;
      default:
        break;
      }
    }
  }
  dc << "\n";
}

const std::string
DiagCollector::getStatusString(Record3ReturnTypes ruleStatus) const
{
  switch (ruleStatus)
  {
  case PASS:
    return "PASS";

  case SKIP:
    return "SKIP";

  case NOTPROCESSED:
    return "NYP";

  default:
    return "FAIL";
  }
}

void
DiagCollector::splitIntoLines(const std::string& text, std::vector<std::string>& lines, uint32_t lineWidth) const
{
  std::string::const_iterator currentIt = text.begin();
  while(std::distance(currentIt, text.end()) > lineWidth)
  {
    lines.push_back(std::string(currentIt, currentIt + lineWidth));
    std::advance(currentIt, lineWidth);
  }
  if(currentIt != text.end())
    lines.push_back(std::string(currentIt, text.end()));
}

void
DiagCollector::addMultilineInfo(std::ostream& stream, const std::string& info, uint32_t lineWidth) const
{
  if(info.size() > lineWidth)
  {
    std::vector<std::string> lines;
    splitIntoLines(info, lines);
    for (const std::string& line : lines)
      stream << std::setw(lineWidth) << std::left << line << "\n";
  }
  else
  {
    stream << std::setw(lineWidth) << std::left << info << "\n";
  }
}
} // namespace tse

