//----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Diagnostic/DiagnosticUtil.h"

#include "Common/FallbackUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Trx.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag870Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <string>

namespace tse
{

FALLBACK_DECL(fallbackDisplaySelectedItinNumInDiag)

bool
DiagnosticUtil::isvalidForCarrierDiagReq(PricingTrx& trx, const PaxTypeFare& ptFare)
{
  if (UNLIKELY(ptFare.fare()->_tariffCrossRefInfo->_tariffCat != RuleConst::PRIVATE_TARIFF))
    return true;

  return (isDisplayKeywordPresent(*trx.getOptions()) ||
          showFareAmount(trx, ptFare.fare()->_fareInfo->_carrier, trx.billing()->partitionID()));
}

bool
DiagnosticUtil::showFareAmount(PricingTrx& trx,
                               const CarrierCode& fareCarrier,
                               const CarrierCode& publishingCarrier)
{
  if (fareCarrier == INDUSTRY_CARRIER ||
      (isAirlineAgent(trx) && isFareOwnedByAirline(publishingCarrier, fareCarrier)))
    return true;

  return false;
}

bool
DiagnosticUtil::showFareAmount(const bool& isKeywordPresent,
                               PricingTrx& trx,
                               const CarrierCode& fareCarrier,
                               const CarrierCode& publishingCarrier)
{
  if (isKeywordPresent || showFareAmount(trx, fareCarrier, publishingCarrier) ||
      (isJointVenture(trx) && isFCRequested(trx)))
  {
    return true;
  }
  return false;
}

bool
DiagnosticUtil::isDisplayKeywordPresent(PricingOptions& options)
{
  static std::string key1 = "CRSAGT";
  static std::string key2 = "TMPCRS";

  if (UNLIKELY(options.isKeywordPresent(key1) || options.isKeywordPresent(key2)))
    return true;

  return false;
}

bool
DiagnosticUtil::isAirlineAgent(PricingTrx& trx)
{
  return (trx.getRequest()->ticketingAgent() &&
          trx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() && !isJointVenture(trx));
}

bool
DiagnosticUtil::isJointVenture(PricingTrx& trx)
{
  static std::string key1 = "PRIMEH";
  static std::string key2 = "JTVENT";

  if (trx.getOptions()->isKeywordPresent(key1) || trx.getOptions()->isKeywordPresent(key2))
    return true;

  return (trx.getRequest()->ticketingAgent() &&
          (trx.getRequest()->ticketingAgent()->cxrCode() == "1B" ||
           trx.getRequest()->ticketingAgent()->cxrCode() == "1J" ||
           trx.getRequest()->ticketingAgent()->cxrCode() == "1F"));
}

bool
DiagnosticUtil::isFCRequested(PricingTrx& trx)
{
  std::map<std::string, std::string>& diagParamMap = trx.diagnostic().diagParamMap();
  if ((diagParamMap.find(Diagnostic::FARE_CLASS_CODE) != diagParamMap.end()) ||
      (diagParamMap.find(Diagnostic::ADDON_FARE_CLASS_CODE) != diagParamMap.end()))
    return true;

  return false;
}

bool
DiagnosticUtil::isFareOwnedByAirline(const CarrierCode& publishingCarrier,
                                     const CarrierCode& fareCarrier)
{
  if ((publishingCarrier == fareCarrier) ||
      (MCPCarrierUtil::isSameGroupCarriers(publishingCarrier, fareCarrier)))
    return true;

  return false;
}

std::string
DiagnosticUtil::tcrTariffCatToString(TariffCategory tcrTariffCat)
{
  return tcrTariffCat == RuleConst::PRIVATE_TARIFF ? "PRIVATE" : "PUBLIC";
}

std::string
DiagnosticUtil::directionToString(FMDirection direction)
{
  std::ostringstream osDirection;

  switch ( direction )
  {
  case FMDirection::INBOUND:
    osDirection << "INBOUND";
    break;
  case FMDirection::OUTBOUND:
    osDirection << "OUTBOUND";
    break;
  case FMDirection::UNKNOWN:
  default:
    osDirection << "UNKNOWN";
    break;
  }

  return osDirection.str();
}

std::string
DiagnosticUtil::geoTravelTypeToString(GeoTravelType travelType)
{
  switch (travelType)
  {
  case GeoTravelType::Domestic:
    return "DOMESTIC";
  case GeoTravelType::International:
    return "INTERNATIONAL";
  case GeoTravelType::Transborder:
    return "TRANSBORDER";
  case GeoTravelType::ForeignDomestic:
    return "FOREIGN DOMESTIC";
  case GeoTravelType::UnknownGeoTravelType:
  default:
    return "UNKNOWN";
  }
}

std::string
DiagnosticUtil::geoTravelTypeTo3CharString(GeoTravelType travelType)
{
  switch (travelType)
  {
    case GeoTravelType::Domestic:
      return "DOM";
    case GeoTravelType::International:
      return "INT";
    case GeoTravelType::Transborder:
      return "TRN";
    case GeoTravelType::ForeignDomestic:
      return "FDM";
    case GeoTravelType::UnknownGeoTravelType:
    default:
      return "UNK";
  }
}

std::string
DiagnosticUtil::globalDirectionToString(GlobalDirection gd)
{
  std::ostringstream osGlobalDirection;

  switch ( gd )
  {
  case GlobalDirection::AF:
    osGlobalDirection << "AF   -VIA AFRICA";
    break;

  case GlobalDirection::AL:
    osGlobalDirection << "AL   -FBR ALL FARES INCLUDING EH/TS";
    break;

  case GlobalDirection::AP:
    osGlobalDirection << "AP   -VIA ATLANTIC AND PACIFIC";
    break;

  case GlobalDirection::AT:
    osGlobalDirection << "AT   -VIA ATLANTIC";
    break;

  case GlobalDirection::CA:
    osGlobalDirection << "CA   -CANADA";
    break;

  case GlobalDirection::CT:
    osGlobalDirection << "CT   -CIRCLE TRIP";
    break;

  case GlobalDirection::DI:
    osGlobalDirection << "DI   -SPECIAL USSR TC3 APP BRITSH AIRWAYS";
    break;

  case GlobalDirection::DO:
    osGlobalDirection << "DO   -DOMESTIC";
    break;

  case GlobalDirection::DU:
    osGlobalDirection << "DU   -SPECIAL USSR TC2 APP BRITSH AIRWAYS";
    break;

  case GlobalDirection::EH:
    osGlobalDirection << "EH   -WITHIN EASTERN HEMISPHERE";
    break;

  case GlobalDirection::EM:
    osGlobalDirection << "EM   -VIA EUROPE MIDDLE EAST";
    break;

  case GlobalDirection::EU:
    osGlobalDirection << "EU   -VIA EUROPE";
    break;

  case GlobalDirection::FE:
    osGlobalDirection << "FE   -FAR EAST";
    break;

  case GlobalDirection::IN:
    osGlobalDirection << "IN   -FBR FOR INTL INCLDNG AT/PA/WH/CT/PV";
    break;

  case GlobalDirection::ME:
    osGlobalDirection << "ME   -VIA MIDDLE EAST OTHER THAN ADEN";
    break;

  case GlobalDirection::NA:
    osGlobalDirection << "NA   -FBR FOR N AMERICA INCL US/CA/TB/PV";
    break;

  case GlobalDirection::NP:
    osGlobalDirection << "NP   -VIA NORTH OR CENTRAL PACIFIC";
    break;

  case GlobalDirection::PA:
    osGlobalDirection << "PA   -VIA SOUTH/CENTRAL OR NORTH PACIFIC";
    break;

  case GlobalDirection::PE:
    osGlobalDirection << "PE   -TC1 CENTRAL/SOUTHERN AFRICA VIA TC3";
    break;

  case GlobalDirection::PN:
    osGlobalDirection << "PN   -BTWN TC1/TC3 VIA PCIFIC/VIA N AMRCA";
    break;

  case GlobalDirection::PO:
    osGlobalDirection << "PO   -VIA POLAR ROUTE";
    break;

  case GlobalDirection::PV:
    osGlobalDirection << "PV   -PR/VI TO US/CA";
    break;

  case GlobalDirection::RU:
    osGlobalDirection << "RU   -RUSSIA TO AREA 3";
    break;

  case GlobalDirection::RW:
    osGlobalDirection << "RW   -ROUND THE WORLD";
    break;

  case GlobalDirection::SA:
    osGlobalDirection << "SA   -SOUTH ATLANTIC ONLY";
    break;

  case GlobalDirection::SN:
    osGlobalDirection << "SN   -VIA S ATLC RTG 1 DIR VIA N/MID ATLC";
    break;

  case GlobalDirection::SP:
    osGlobalDirection << "SP   -VIA SOUTH POLAR";
    break;

  case GlobalDirection::TB:
    osGlobalDirection << "TB   -TRANSBORDER";
    break;

  case GlobalDirection::TS:
    osGlobalDirection << "TS   -VIA SIBERIA";
    break;

  case GlobalDirection::TT:
    osGlobalDirection << "TT   -AREA2";
    break;

  case GlobalDirection::US:
    osGlobalDirection << "US   -INTRA US";
    break;

  case GlobalDirection::WH:
    osGlobalDirection << "WH   -WITHIN WESTERN HEMISPHERE";
    break;

  case GlobalDirection::XX:
    osGlobalDirection << "XX   -UNIVERSAL";
    break;

  case GlobalDirection::ZZ:
    osGlobalDirection << "ZZ   -ANY GLOBAL";
    break;

  default:
    osGlobalDirection << "UNKNOWN";
    break;
  }

  return osGlobalDirection.str();
}

bool
DiagnosticUtil::filterByFareClass(const PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  std::string fareClass;
  std::map<std::string, std::string>::const_iterator itEnd = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator it =
      trx.diagnostic().diagParamMap().find("FC");

  if (it != itEnd)
    fareClass = it->second;

  if (fareClass.empty())
  {
    return false;
  }

  if (paxTypeFare.fareClass() == fareClass || paxTypeFare.createFareBasis(0) == fareClass)
  {
    // FareClass found in this PricingUnit
    return false;
  }

  return true;
}

bool
DiagnosticUtil::filterByFareClass(const PricingTrx& trx, const PricingUnit& pu)
{
  std::string fareClass;
  std::map<std::string, std::string>::const_iterator itEnd = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator it =
      trx.diagnostic().diagParamMap().find("FC");

  if (it != itEnd)
    fareClass = it->second;

  if (fareClass.empty())
  {
    return false;
  }

  std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuItEnd = pu.fareUsage().end();

  for (; fuIt != fuItEnd; ++fuIt)
  {
    if ((*fuIt)->paxTypeFare()->fareClass() == fareClass ||
        (*fuIt)->paxTypeFare()->createFareBasis(0) == fareClass)
    {
      // FareClass found in this PricingUnit
      return false;
    }
  }

  return true;
}

bool
DiagnosticUtil::filterByFareClass(const PricingTrx& trx, const FarePath& farePath)
{
  std::string fareClass;
  std::map<std::string, std::string>::const_iterator itEnd = trx.diagnostic().diagParamMap().end();
  std::map<std::string, std::string>::const_iterator it =
      trx.diagnostic().diagParamMap().find("FC");

  if (it != itEnd)
    fareClass = it->second;

  if (fareClass.empty())
  {
    return false;
  }

  std::vector<PricingUnit*>::const_iterator puIter = farePath.pricingUnit().begin();
  std::vector<PricingUnit*>::const_iterator puIterEnd = farePath.pricingUnit().end();

  for (; puIter != puIterEnd; ++puIter)
  {
    std::vector<FareUsage*>::const_iterator fuIt = (*puIter)->fareUsage().begin();
    std::vector<FareUsage*>::const_iterator fuItEnd = (*puIter)->fareUsage().end();

    for (; fuIt != fuItEnd; ++fuIt)
    {
      if ((*fuIt)->paxTypeFare()->fareClass() == fareClass ||
          (*fuIt)->paxTypeFare()->createFareBasis(0) == fareClass)
      {
        // FareClass found in this FarePath
        return false;
      }
    }
  }

  return true;
}

bool
DiagnosticUtil::filter(const PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  return filterByFareClass(trx, paxTypeFare);
}

bool
DiagnosticUtil::filter(const PricingTrx& trx, const PricingUnit& pu)
{
  return filterByFareClass(trx, pu);
}

bool
DiagnosticUtil::filter(const PricingTrx& trx, const FarePath& farePath)
{
  return filterByFareClass(trx, farePath);
}

void
DiagnosticUtil::displayCabinCombination(const PricingTrx& trx,
                                        const FarePath& farePath,
                                        std::ostringstream& out)
{
  for (const PricingUnit* pu : farePath.pricingUnit())
    displayCabinCombination(trx, *pu, out);
}

void
DiagnosticUtil::displayCabinCombination(const PricingTrx& trx,
                                        const PricingUnit& pricingUnit,
                                        std::ostringstream& out)
{
  for (const FareUsage* fareUsage : pricingUnit.fareUsage())
  {
    const PaxTypeFare* fare = fareUsage->paxTypeFare();

    if (fare)
    {
      if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
        out << std::setw(2) << fare->cabin().getClassAlphaNumAnswer();
      else
        out << std::setw(2) << fare->cabin().getClassAlphaNum();
    }
  }
}

void
DiagnosticUtil::displayObFeesNotRequested(PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  Diag870Collector* diagPtr = dynamic_cast<Diag870Collector*>(factory->create(trx));

  diagPtr->enable(Diagnostic870);
  diagPtr->activate();
  diagPtr->printHeader();
  diagPtr->printOBFeesNotRequested();

  diagPtr->flushMsg();
}

bool
DiagnosticUtil::shouldDisplayFlexFaresGroupInfo(const Diagnostic& diagnostic,
                                                const flexFares::GroupId& groupId)
{
  if (!diagnostic.diagParamMapItemPresent(flexFares::DIAG_GROUP_ID))
    return true;

  return boost::lexical_cast<uint16_t>(diagnostic.diagParamMapItem(flexFares::DIAG_GROUP_ID)) ==
         groupId;
}

std::string
DiagnosticUtil::printPaxTypeFare(const PaxTypeFare& ptf)
{
  return ptf.createFareBasis(0) + " [" + Vendor::displayChar(ptf.vendor())
           + ' ' + ptf.carrier() + ' ' + std::to_string(ptf.tcrRuleTariff())
           + ' ' + ptf.ruleNumber() + "]";
}

const char*
DiagnosticUtil::pricingUnitTypeToString(const PricingUnit::Type puType)
{
  switch (puType)
  {
  case PricingUnit::Type::OPENJAW:
    return "OPENJAW";
  case PricingUnit::Type::ROUNDTRIP:
    return "ROUNDTRIP";
  case PricingUnit::Type::CIRCLETRIP:
    return "CIRCLETRIP";
  case PricingUnit::Type::ONEWAY:
    return "ONEWAY";
  default:
    return "UNKNOWN";
  }
}

const char*
DiagnosticUtil::pricingUnitTypeToShortString(const PricingUnit::Type puType)
{
  switch (puType)
  {
  case PricingUnit::Type::OPENJAW:
    return "OJ";
  case PricingUnit::Type::ROUNDTRIP:
    return "RT";
  case PricingUnit::Type::CIRCLETRIP:
    return "CT";
  case PricingUnit::Type::ONEWAY:
    return "OW";
  default:
    return " ";
  }
}

std::string
DiagnosticUtil::getFareBasis(const PaxTypeFare& paxTypeFare)
{
  std::string fareBasis = paxTypeFare.createFareBasis(nullptr);
  if (fareBasis.size() > 13)
  {
    fareBasis.resize(13);
    fareBasis[12] = '*';
  }
  return fareBasis;
}

char
DiagnosticUtil::getOwrtChar(const PaxTypeFare& paxTypeFare)
{
  switch (paxTypeFare.owrt())
  {
  case ONE_WAY_MAY_BE_DOUBLED:
    return 'X';
  case ROUND_TRIP_MAYNOT_BE_HALVED:
    return 'R';
  case ONE_WAY_MAYNOT_BE_DOUBLED:
    return 'O';
  default:
    return ' ';
  }
}

bool
DiagnosticUtil::showItinInMipDiag(const PricingTrx& trx, const IntIndex& itinNum)
{
  if (!fallback::fallbackDisplaySelectedItinNumInDiag(&trx))
  {
    if (trx.getTrxType() == PricingTrx::MIP_TRX &&
        !trx.diagnostic().diagParamIsSet("ITIN_NUM", "") &&
        !trx.diagnostic().diagParamIsSet("ITIN_NUM", std::to_string(itinNum)))
    {
      return false;
    }
  }
  return true;
}

namespace tools
{

void printCat10Info(DiagCollector& dc, const CombinabilityRuleInfo* pCat10)
{
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "CXR-" << std::setw(4) << pCat10->carrierCode() << "VND-" << std::setw(5)
     << pCat10->vendorCode() << "RULE-" << std::setw(6) << pCat10->ruleNumber() << "TARIFF-"
     << std::setw(5) << pCat10->tariffNumber() << "R2 FARE CLASS-" << std::setw(8)
     << pCat10->fareClass() << "\n\n"
     << "SEQ      S  N  **************** ********O  R  FT  JNT     C GL\n"
     << "NBR      T  A  T LOC1   T LOC2  FRE S D R  I  NT  CXR     S DI\n" << std::setw(9)
     << pCat10->sequenceNumber() << "      ";

  if (pCat10->loc1().locType() != 0)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << pCat10->loc1().locType() << std::setw(7) << pCat10->loc1().loc();
  }
  else
    dc << "         ";

  if (pCat10->loc2().locType() != 0)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(2) << pCat10->loc2().locType() << std::setw(6) << pCat10->loc2().loc();
  }
  else
    dc << "         ";

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(4) << pCat10->fareType() << std::setw(2) << pCat10->seasonType() << std::setw(2)
     << pCat10->dowType() << std::setw(3) << pCat10->owrt() << std::setw(3) << pCat10->routingAppl()
     << std::setw(4) << pCat10->footNote1() << std::setw(7) << pCat10->jointCarrierTblItemNo()
     << "\n"
     << "SAME PTS - " << std::setw(2) << pCat10->samepointstblItemNo() << "\n\n"
     << "            P   S   O   D   C    T    F                  A\n"
     << "            E   O   /   O   X    /    /    SRC           P\n"
     << "            R   J   D   J   R    R    T    TRF    RULE   L\n"
     << "  101-OJ        " << std::setw(4) << pCat10->sojInd() << std::setw(4)
     << pCat10->sojorigIndestInd() << std::setw(4) << pCat10->dojInd() << std::setw(2)
     << pCat10->dojCarrierRestInd() << std::setw(3) << pCat10->dojSameCarrierInd() << std::setw(2)
     << pCat10->dojTariffRuleRestInd() << std::setw(3) << pCat10->dojSameRuleTariffInd()
     << std::setw(2) << pCat10->dojFareClassTypeRestInd() << std::setw(3)
     << pCat10->dojSameFareInd() << "\n"
     << "  102-CT2   " << std::setw(16) << pCat10->ct2Ind() << std::setw(2)
     << pCat10->ct2CarrierRestInd() << std::setw(3) << pCat10->ct2SameCarrierInd() << std::setw(2)
     << pCat10->ct2TariffRuleRestInd() << std::setw(3) << pCat10->ct2SameRuleTariffInd()
     << std::setw(2) << pCat10->ct2FareClassTypeRestInd() << std::setw(3)
     << pCat10->ct2SameFareInd() << "\n"
     << "  103-CT2P  " << std::setw(16) << pCat10->ct2plusInd() << std::setw(2)
     << pCat10->ct2plusCarrierRestInd() << std::setw(3) << pCat10->ct2plusSameCarrierInd()
     << std::setw(2) << pCat10->ct2plusTariffRuleRestInd() << std::setw(3)
     << pCat10->ct2plusSameRuleTariffInd() << std::setw(2) << pCat10->ct2plusFareClassTypeRestInd()
     << std::setw(3) << pCat10->ct2plusSameFareInd() << "\n"
     << "  104-END   " << std::setw(16) << pCat10->eoeInd() << std::setw(2)
     << pCat10->eoeCarrierRestInd() << std::setw(3) << pCat10->eoeSameCarrierInd() << std::setw(2)
     << pCat10->eoeTariffRuleRestInd() << std::setw(3) << pCat10->eoeSameRuleTariffInd()
     << std::setw(2) << pCat10->eoeFareClassTypeRestInd() << std::setw(3)
     << pCat10->eoeSameFareInd() << "\n\n"
     << "          V         S S S  O T  N SD 3  E E  P  TO  I O  D \n"
     << "      CAT N ITEM    C R F  D X  E OO F  O E  R  RT  N T  I \n"
     << "TABLE LST D NBR     R T T  T T  G JJ B  E I  D  PH  B B  R \n";

  for (const auto& setPtr: pCat10->categoryRuleItemInfoSet())
  {
    dc << "*\n";
    for (const auto& item: *setPtr)
    {
      dc.setf(std::ios::left, std::ios::adjustfield);

      {
        // backward compatibility
        boost::io::ios_all_saver ias(dc);
        if (item.relationalInd() != CategoryRuleItemInfo::IF)
        {
          dc << std::right;
        }
        dc << std::setw(4) << tostring(item.relationalInd());
      }

      dc << "  " << std::setw(3)
         << item.itemcat() << "-0-" << std::setw(8) << item.itemNo() << std::setw(2)
         << item.sameCarrierInd() << std::setw(2) << item.sameRuleTariffInd()
         << std::setw(5) << item.sameFareInd() << std::setw(11) << item.textonlyInd()
         << std::setw(2) << item.eoervalueInd() << std::setw(11) << item.eoeallsegInd()
         << std::setw(4) << item.inOutInd() << std::setw(2) << item.directionality()
         << "\n";
    }
  }
}

} // namespace tools

} // end namespace tse
