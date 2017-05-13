// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Xform/BaggageResponseFormatter.h"

#include "Common/BaggageStringFormatter.h"
#include "Common/ErrorResponseException.h"
#include "Common/Message.h"
#include "Common/TrxUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/BaggageTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "Diagnostic/Diagnostic.h"
#include "ServiceFees/OCFees.h"
#include "Util/IteratorRange.h"
#include "Xform/DataModelMap.h"
#include "Xform/PricingResponseXMLTags.h"
#include "Xform/XformUtil.h"

#include <vector>

namespace tse
{
const std::string BaggageResponseFormatter::XML_NAMESPACE_TEXT =
    "http://www.atse.sabre.com/Baggage/Response";

std::string
BaggageResponseFormatter::formatResponse(const std::string& errorString,
                                         BaggageTrx& bgTrx,
                                         ErrorResponseException::ErrorResponseCode errorCode)
{
  XMLConstruct construct;
  construct.openElement("BaggageResponse");
  construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);

  buildErrorAndDiagnosticElements(errorString, bgTrx, errorCode, construct);

  if (!errorCode && ((bgTrx.diagnostic().diagnosticType() == DiagnosticNone) ||
                     (bgTrx.diagnostic().diagnosticType() == Diagnostic854)))
  {
    std::map<const PaxType*, Itin*>& paxToOriginalItinMap =
        (static_cast<AncRequest*>(bgTrx.getRequest()))->paxToOriginalItinMap();

    for (const FarePath* farePath : bgTrx.itin().front()->farePath())
      buildITN(
          construct, paxToOriginalItinMap[farePath->paxType()]->getItinOrderNum(), farePath, bgTrx);
  }
  construct.closeElement();

  return construct.getXMLData();
}

void
BaggageResponseFormatter::formatResponse(const ErrorResponseException& ere, std::string& response)
{
  XMLConstruct construct;
  construct.openElement("BaggageResponse");
  construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);

  prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(ere.code()), ere.message());

  construct.closeElement();

  response = construct.getXMLData();
}

void
BaggageResponseFormatter::buildErrorAndDiagnosticElements(
    const std::string& errorString,
    BaggageTrx& bgTrx,
    ErrorResponseException::ErrorResponseCode errorCode,
    XMLConstruct& construct)
{
  Diagnostic& diag = bgTrx.diagnostic();

  if (diag.diagnosticType() == Diagnostic854)
  {
    prepareHostPortInfo(bgTrx, construct);
  }
  if (errorCode > 0)
  {
    if (diag.diagnosticType() != DiagnosticNone && !diag.toString().empty())
    {
      prepareResponseText(diag.toString(), construct);
    }
    prepareMessage(construct, Message::TYPE_ERROR, Message::errCode(errorCode), errorString);
  }
  else if (diag.diagnosticType() != DiagnosticNone && diag.diagnosticType() != Diagnostic854)
  {
    std::string diagResponse = diag.toString();

    if (diagResponse.empty())
    {
      std::ostringstream diagResponseStream;
      diagResponseStream << "DIAGNOSTIC " << diag.diagnosticType() << " RETURNED NO DATA";
      diagResponse += diagResponseStream.str();
    }
    prepareResponseText(diagResponse, construct);
  }
}

void
BaggageResponseFormatter::buildITN(XMLConstruct& construct,
                                   uint16_t index,
                                   const FarePath* farePath,
                                   BaggageTrx& bgTrx) const
{
  construct.openElement(xml2::ItinInfo);
  construct.addAttributeInteger(xml2::GenId, index);

  buildPXI(construct, farePath, bgTrx);
  buildDCL(construct, farePath, bgTrx);

  for (const TravelSeg* tvlSeg : farePath->itin()->travelSeg())
  {
    if (tvlSeg->isAir())
    {
      const AirSeg* airSeg = static_cast<const AirSeg*>(tvlSeg);

      const auto bagIt = farePath->baggageAllowance().find(airSeg);
      const std::string allowance =
          (bagIt != farePath->baggageAllowance().end()) ? bagIt->second : std::string();
      buildSEG(construct, airSeg, allowance);
    }
  }
  construct.closeElement();
}

void
BaggageResponseFormatter::buildPXI(XMLConstruct& construct,
                                   const FarePath* farePath,
                                   const BaggageTrx& bgTrx) const
{
  const PaxType* paxType = farePath->paxType();

  construct.openElement(xml2::PassengerInfo);

  if (paxType->age())
  {
    std::ostringstream paxTypeWithAge;

    paxTypeWithAge << paxType->paxType()[0] << std::setfill('0') << std::setw(2) << paxType->age();
    construct.addAttribute(xml2::PassengerType, paxTypeWithAge.str());
  }
  else
    construct.addAttribute(xml2::PassengerType, paxType->paxType());

  if (TrxUtil::isBaggage302GlobalDisclosureActivated(bgTrx))
  {
    if (TrxUtil::isBaggageCTAMandateActivated(bgTrx))
    {
      BaggageTripType btt = farePath->itin()->getBaggageTripType();
      construct.addAttributeChar(xml2::UsDotItinIndicator, btt.getIndicator());
    }
    else
    {
      const bool isUsDot = farePath->itin()->getBaggageTripType().isUsDot();
      construct.addAttributeBoolean(xml2::UsDotItinIndicator, isUsDot);
    }
  }

  construct.addAttributeInteger(xml2::PassengerCount, paxType->number());

  FFInfoList ffInfoList;

  collectFFInfos<OCFees*, &BaggageTravel::_allowance>(
      BaggageProvisionType(BAGGAGE_ALLOWANCE), farePath->baggageTravels(), ffInfoList);
  collectFFInfosFromVector<BaggageCharges, &BaggageTravel::_charges>(
      BaggageProvisionType(BAGGAGE_CHARGE), farePath->baggageTravels(), ffInfoList);
  collectFFInfosFromVector<EmbargoVector, &BaggageTravel::_embargoVector>(
      BaggageProvisionType(BAGGAGE_EMBARGO), farePath->baggageTravelsPerSector(), ffInfoList);

  collectFFInfos<OCFees*, &BaggageTravel::_allowance>(
      BaggageProvisionType(CARRY_ON_ALLOWANCE), farePath->baggageTravelsPerSector(), ffInfoList);
  collectFFInfosFromVector<ChargeVector, &BaggageTravel::_chargeVector>(
      CARRY_ON_CHARGE, farePath->baggageTravelsPerSector(), ffInfoList);

  for (const FFInfo& ffInfo : ffInfoList)
  {
    buildFFY(construct, ffInfo);
  }
  construct.closeElement();
}

void
BaggageResponseFormatter::buildFFY(XMLConstruct& construct, const FFInfo& ffInfo) const
{
  construct.openElement("FFY");
  construct.addAttribute(xml2::BaggageProvision, ffInfo._baggageProvision);
  construct.addAttributeShort(xml2::FQTVCxrFiledTierLvl, ffInfo._tierLevel);
  construct.addAttribute(xml2::CxrCode, ffInfo._carrier);

  for (uint32_t segmentNumber : ffInfo._segmentNumbers)
    buildQ00(construct, segmentNumber);

  construct.closeElement();
}

void
BaggageResponseFormatter::buildQ00(XMLConstruct& construct, int id) const
{
  construct.openElement(xml2::TravelSegId);
  const std::string s = boost::lexical_cast<std::string>(id);
  construct.addElementData(s.c_str());
  construct.closeElement();
}

void
BaggageResponseFormatter::buildDCL(XMLConstruct& construct,
                                   const FarePath* farePath,
                                   BaggageTrx& trx) const
{
  construct.openElement(xml2::BaggageDisclosure);
  std::vector<std::string> lines;

  BaggageResponseBuilder builder(trx, *farePath);
  builder.getPqResponse(lines, &trx);

  for (const std::string& line : lines)
    prepareMessage(construct, Message::TYPE_GENERAL, 0, line);

  construct.closeElement();
}

void
BaggageResponseFormatter::buildSEG(XMLConstruct& construct,
                                   const AirSeg* airSeg,
                                   const std::string& allowance) const
{
  construct.openElement(xml2::SegmentInformation);
  construct.addAttribute(xml2::SegmentDepartureCity, airSeg->boardMultiCity());
  construct.addAttribute(xml2::SegmentDepartureAirport, airSeg->origAirport());
  construct.addAttribute(xml2::SegmentArrivalCity, airSeg->offMultiCity());
  construct.addAttribute(xml2::SegmentArrivalAirport, airSeg->destAirport());
  construct.addAttributeInteger(xml2::ItinSegmentNumber, airSeg->pnrSegment());

  const uint8_t baggageStringLength = allowance.length();

  if (baggageStringLength > 0)
  {
    if (allowance.substr(0, baggageStringLength - 1) == "NI")
    {
      construct.addAttributeChar(xml2::BaggageIndicator, ' ');
    }
    else
    {
      construct.addAttributeChar(xml2::BaggageIndicator, allowance[baggageStringLength - 1]);
    }
    construct.addAttribute(xml2::BaggageValue, allowance.substr(0, baggageStringLength - 1));
  }

  construct.addAttribute(xml2::MarketingCarrier, airSeg->marketingCarrierCode());
  construct.addAttribute(xml2::OperatingCarrier, airSeg->operatingCarrierCode());
  construct.closeElement();
}

void
BaggageResponseFormatter::getSegmentNumbers(const BaggageTravel* baggageTravel,
                                            std::set<int>& segmentNumbers) const
{
  for (const TravelSeg* travelSeg :
       makeIteratorRange(baggageTravel->getTravelSegBegin(), baggageTravel->getTravelSegEnd()))
  {
    if (travelSeg->pnrSegment() && travelSeg->pnrSegment() != ARUNK_PNR_SEGMENT_ORDER)
      segmentNumbers.insert(travelSeg->pnrSegment());
  }
}

void
BaggageResponseFormatter::collectFFInfo(const OCFees* ocFees,
                                        const BaggageProvisionType& baggageProvision,
                                        const BaggageTravel* baggageTravel,
                                        BaggageResponseFormatter::FFInfoList& ffInfoList) const
{
  if (ocFees && ocFees->optFee() &&
      (ocFees->softMatchS7Status().isSet(OCFees::S7_FREQFLYER_SOFT) || !ocFees->isDisplayOnly()) &&
      ocFees->optFee()->frequentFlyerStatus() != 0)
  {
    std::set<int> segmentNumbers;
    getSegmentNumbers(baggageTravel, segmentNumbers);

    const BaggageResponseFormatter::FFInfo ffInfo(baggageProvision,
                                                  ocFees->optFee()->frequentFlyerStatus(),
                                                  ocFees->optFee()->carrier(),
                                                  segmentNumbers);

    BaggageResponseFormatter::FFInfoList::iterator it =
        std::find_if(ffInfoList.begin(), ffInfoList.end(), FFInfoComparator(ffInfo));

    if (it == ffInfoList.end())
      ffInfoList.push_back(ffInfo);
    else
      (*it)._segmentNumbers.insert(segmentNumbers.begin(), segmentNumbers.end());
  }
}

} // namespace tse
