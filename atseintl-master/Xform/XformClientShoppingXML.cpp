//----------------------------------------------------------------------------
//
//      File: XformClientShoppingXML.cpp
//      Description: Implementation of class to transform client XML to/from Trx
//      Created: August 04, 2004
//      Authors: David White
//  Copyright Sabre 2004
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

#include "Xform/XformClientShoppingXML.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "Common/MetricsUtil.h"
#include "Common/RBDByCabinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/BrandingTrx.h"
#include "DataModel/Diversity.h"
#include "DataModel/ExcItin.h"
#include "DataModel/Itin.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Cabin.h"
#include "Diagnostic/Diag905Collector.h"
#include "Diagnostic/Diag982Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diagnostic.h"
#include "Diagnostic/DiagTools.h"
#include "Pricing/Shopping/Diversity/DiversityModelFactory.h"
#include "Server/TseServer.h"
#include "Xform/BrandingResponseBuilder.h"
#include "Xform/CustomXMLParser/IParser.h"
#include "Xform/CustomXMLParser/IXMLSchema.h"
#include "Xform/CustomXMLParser/IXMLUtils.h"
#include "Xform/ShoppingSchemaNames.h"
#include "Xform/TaxNewShoppingRequestHandler.h"
#include "Xform/XMLBrandingResponse.h"
#include "Xform/XMLRexShoppingResponse.h"
#include "Xform/XMLShoppingHandler.h"
#include "Xform/XMLShoppingResponse.h"
#include "Xform/XMLShoppingResponseESV.h"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>

#include <algorithm>

namespace tse
{
namespace
{
ConfigurableValue<bool>
altDateReuseOutboundProcessed("SHOPPING_OPT", "ALTDATE_REUSE_OUTBOUND_PROCESSED", false);
ConfigurableValue<bool>
altDateReuseInboundProcessed("SHOPPING_OPT", "ALTDATE_REUSE_INBOUND_PROCESSED", false);
ConfigurableValue<uint16_t>
minAccCorpIds("SHOPPING_OPT", "MIN_AC_CORPIDS_FOR_FCO_FARE_REDUCTION", 5);

bool
considerTaxRq(std::string& request, DataHandle& dataHandle, Trx*& trx)
{
  auto it = std::find(request.begin(), request.end(), '<');
  if (it == request.end())
    return false;

  auto jt = std::find(it, request.end(), ' ');
  if (jt == request.end())
    return false;

  const char TaxRq[] = "<TaxRq";
  const char TAX[] = "<TAX";

  bool isTax = (std::distance(it, jt) == sizeof(TaxRq) - 1 && std::equal(it, jt, TaxRq)) ||
               (std::distance(it, jt) == sizeof(TAX) - 1 && std::equal(it, jt, TAX));

  if (!isTax)
    return false;

  std::string cleanRequest(it, request.end());
  TaxNewShoppingRequestHandler taxNewShoppingRequestHandler(trx);
  taxNewShoppingRequestHandler.parse(dataHandle, cleanRequest);
  trx->setRawRequest(cleanRequest);
  return true;
}

} // anonymous namespace

FALLBACK_DECL(fallbackInfantAlonePriceable);

static LoadableModuleRegister<Xform, XformClientShoppingXML>
_("libXformClientShoppingXML.so");

Logger
XformClientShoppingXML::_logger("atseintl.Xform.XformClientShoppingXML");
uint16_t XformClientShoppingXML::_minAccCorpIds = 5;

XformClientShoppingXML::XformClientShoppingXML(const std::string& name, ConfigMan& config)
  : Xform(name, config)
{
  _minAccCorpIds = minAccCorpIds.getValue();
}

XformClientShoppingXML::XformClientShoppingXML(const std::string& name, TseServer& srv)
  : Xform(name, srv.config())
{
  srv.config().getValue("MIN_AC_CORPIDS_FOR_FCO_FARE_REDUCTION", _minAccCorpIds, "SHOPPING_OPT");
}

XformClientShoppingXML::~XformClientShoppingXML()
{
}

namespace
{
DiagnosticTypes
mapDiagnosticNumber(const Diagnostic& diag)
{
  switch (diag.diagnosticType())
  {
  case Diagnostic915: // pu revalidation
  case Diagnostic966: // pu revalidation FF/BFF
    return Diagnostic555;

  case Diagnostic916: // Cat-12 Surcharge Rule
    return Diagnostic312;

  case Diagnostic917: // Cat-12 Final Surcharge Collection
    return Diagnostic512;

  case Diagnostic918: // HIP Minimum Fare Check
    return Diagnostic718;

  case Diagnostic919: // CTM Minimum Fare Check
    return Diagnostic719;

  case Diagnostic920: // Cat-10 Fare Path Build
    return Diagnostic600;

  case Diagnostic921: // Cat-10 Fare selection for Pricing Unit
    return Diagnostic601;

  case Diagnostic925: // Cat-10 Pricing Unit Validation
    return Diagnostic605;

  case Diagnostic926: // Cat-10 Fare Path Validation
    return Diagnostic610;

  case Diagnostic913:
  {
    const std::map<std::string, std::string>::const_iterator itor = diag.diagParamMap().find("DN");
    if (itor == diag.diagParamMap().end())
    {
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                                   "Diagnostic 913 must have a 'DN' argument supplied");
    }

    return static_cast<DiagnosticTypes>(atoi(itor->second.c_str()));
  }

  default:
    return diag.diagnosticType();
  }
}

// custom parser
ILookupMap _elemLookupMap, _attrLookupMap;

bool
bInit(IXMLUtils::initLookupMaps(shopping::shoppingElementNames,
                                shopping::_NumberElementNames_,
                                _elemLookupMap,
                                shopping::shoppingAttributeNames,
                                shopping::_NumberAttributeNames_,
                                _attrLookupMap));
} // namespace

bool
XformClientShoppingXML::convert(DataHandle& dataHandle,
                                std::string& request,
                                Trx*& trx,
                                bool /*throttled*/)
{
  if (request.empty())
  {
    return false;
  }

  if (considerTaxRq(request, dataHandle, trx))
  {
    return true;
  }

  bool res(false);

  IValueString attrValueArray[shopping::_NumberAttributeNames_];
  int attrRefArray[shopping::_NumberAttributeNames_];

  const bool checkWellFormedness = true;

  IXMLSchema schema(_elemLookupMap,
                    _attrLookupMap,
                    shopping::_NumberAttributeNames_,
                    attrValueArray,
                    attrRefArray,
                    checkWellFormedness);

  XMLShoppingHandler parser(dataHandle);
  IParser customParser(request, parser, schema);
  try
  {
    try
    {
      customParser.parse();
      res = true;
    }
    catch (const ErrorResponseException& e)
    {
      const static std::string errorHeader("INVALID INPUT");
      std::string error(errorHeader);
      error.append(1, ':');
      error.append(e.message());
      LOG4CXX_ERROR(_logger, error);
      throw;
    }
    catch (const std::exception& e)
    {
      const static std::string errorHeader("ERROR PARSING REQUEST");
      std::string error(errorHeader);
      error.append(1, ':');
      error.append(e.what());
      LOG4CXX_ERROR(_logger, e.what());
      throw ErrorResponseException(ErrorResponseException::INVALID_INPUT, error.c_str());
    }
    if (!parser.trxResult())
    {
      return false;
    }

    if (PricingTrx* pricingTrx = parser.trx())
    {
      validateTrx(*(parser.trx()));

      if (pricingTrx->getTrxType() == PricingTrx::IS_TRX ||
          pricingTrx->getTrxType() == PricingTrx::FF_TRX ||
          pricingTrx->getTrxType() == PricingTrx::RESHOP_TRX)
      {
        if (pricingTrx->diagnostic().diagnosticType() == 905 &&
            (pricingTrx->diagnostic().diagParamMapItem("DD") == "XML"))
        {
          std::string myRequest(request);
          // Make XML format suitable on Green Screen.
          std::replace(myRequest.begin(), myRequest.end(), '<', ':');
          std::replace(myRequest.begin(), myRequest.end(), '>', ':');
          std::replace(myRequest.begin(), myRequest.end(), '|', ':');
          std::replace(myRequest.begin(), myRequest.end(), '=', '-');
          std::replace(myRequest.begin(), myRequest.end(), '"', '$');
          pricingTrx->diagnostic().activate();
          Diag905Collector* const collector =
              dynamic_cast<Diag905Collector*>(DCFactory::instance()->create(*pricingTrx));
          TSE_ASSERT(collector != nullptr);
          collector->activate();
          Diag905Collector& stream = *collector;
          stream << myRequest;
          stream.flushMsg();
        }
      }

      if (pricingTrx->getTrxType() == PricingTrx::MIP_TRX)
      {
        if (pricingTrx->diagnostic().diagnosticType() == 982)
        {
          performDiagnostic982(*pricingTrx, parser.getItinMap());
        }

        DiagManager diag(*pricingTrx, Diagnostic984);
        if (diag.isActive())
        {
          std::string myRequest(request);

          // Make XML format suitable on Green Screen.
          std::replace(myRequest.begin(), myRequest.end(), '<', ':');
          std::replace(myRequest.begin(), myRequest.end(), '>', ':');
          std::replace(myRequest.begin(), myRequest.end(), '|', ':');
          std::replace(myRequest.begin(), myRequest.end(), '=', '-');
          std::replace(myRequest.begin(), myRequest.end(), '"', '$');

          diag << myRequest;
        }
      }

      if (pricingTrx->diagnostic().diagnosticType() == 913)
      {
        std::map<std::string, std::string>::const_iterator
        end = pricingTrx->diagnostic().diagParamMap().end(),
        begin = pricingTrx->diagnostic().diagParamMap().begin();
        while (begin != end)
        {
          if (begin->first == "DN")
          {
            begin++;
          }
          else
          {
            pricingTrx->getRequest()->diagArgType().push_back(begin->first);
            pricingTrx->getRequest()->diagArgData().push_back(begin->second);
            begin++;
          }
        }
      }
      pricingTrx->diagnostic().diagnosticType() = mapDiagnosticNumber(pricingTrx->diagnostic());
    }

    trx = parser.trxResult();
    trx->setRawRequest(request);
    return res;
  }
  catch (...)
  {
    trx = nullptr;
    throw;
  }
}

bool
XformClientShoppingXML::convert(Trx& trx, std::string& response)
{
  if (PricingTrx* const pricingTrx = dynamic_cast<PricingTrx*>(&trx))
  {
    if (TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx))
    {
      response = taxTrx->response().str();
      return true;
    }

    if (!pricingTrx->taxRequestToBeReturnedAsResponse().empty())
    {
      response = pricingTrx->taxRequestToBeReturnedAsResponse();
      return true;
    }

    BrandingTrx* brandingTrx = dynamic_cast<BrandingTrx*>(&trx);
    if (brandingTrx)
    {
      BrandingResponseType resMap = BrandingRequestUtils::trxToResponse(*brandingTrx);
      XMLBrandingResponse xmlBrandingResponse(*brandingTrx);
      xmlBrandingResponse.setToken(brandingTrx->token());

      if (utils::shouldDiagBePrintedForTrx(*brandingTrx))
      {
        const int diagCode = static_cast<int>(brandingTrx->diagnostic().diagnosticType());
        // truncate text if needed
        const std::string diagText = utils::truncateDiagIfNeeded(
            utils::getDiagnosticPrintout(*brandingTrx, BUILD_LABEL_STRING), *brandingTrx);
        xmlBrandingResponse.setDiagOutput(diagCode, diagText);
      }
      xmlBrandingResponse.buildXmlBrandingResponse(response, resMap);
      return true;
    }

    if (PricingTrx::ESV_TRX == pricingTrx->getTrxType())
    {
      const bool res = XMLShoppingResponseESV::generateXML(*pricingTrx, response);

      return res;
    }
    else if (PricingTrx::RESHOP_TRX == pricingTrx->getTrxType()) // tytan
    {
      const bool res = XMLRexShoppingResponse::generateXML(*pricingTrx, response);

      return res;
    }
    else
    {
      const bool res = XMLShoppingResponse::generateXML(*pricingTrx, response);

      DiagManager diag(*pricingTrx, Diagnostic985);
      if (diag.isActive())
      {
        std::string responseCopy(response);

        // Make XML format suitable on Green Screen.
        std::replace(responseCopy.begin(), responseCopy.end(), '<', ':');
        std::replace(responseCopy.begin(), responseCopy.end(), '>', ':');
        std::replace(responseCopy.begin(), responseCopy.end(), '=', '-');
        std::replace(responseCopy.begin(), responseCopy.end(), '"', '$');

        diag << responseCopy;
      }

      return res;
    }
  }
  else
  {
    LOG4CXX_ERROR(_logger, "Unexpected transaction type in convert");
    return false;
  }
}

bool
XformClientShoppingXML::convert(ErrorResponseException& ere, Trx& trx, std::string& response)
{
  if (TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx))
  {
    response = taxTrx->response().str();
    return true;
  }
  else if (PricingTrx* const pricingTrx = dynamic_cast<PricingTrx*>(&trx))
  {
    if (!pricingTrx->taxRequestToBeReturnedAsResponse().empty())
    {
      response = pricingTrx->taxRequestToBeReturnedAsResponse();
      return true;
    }
    return XMLShoppingResponse::generateXML(*pricingTrx, ere, response);
  }
  else
  {
    return false;
  }
}

bool
XformClientShoppingXML::convert(ErrorResponseException& ere, std::string& response)
{
  return XMLShoppingResponse::generateXML(ere, response);
}

bool
XformClientShoppingXML::initialize(int argc, char** argv)
{
  try
  {
    xercesc::XMLPlatformUtils::Initialize();
  }
  catch (const xercesc::XMLException& e)
  {
    LOG4CXX_ERROR(_logger, "Could not initialize XML parser");
    return false;
  }

  return true;
}

namespace
{
bool
posPaxTypeEqual(const PosPaxType* p1, const PosPaxType* p2)
{
  return (p1->paxType() == p2->paxType() && p1->pcc() == p2->pcc() &&
          p1->corpID() == p2->corpID() && p1->positive() == p2->positive());
}

//----------------------------------------------------------------------------
// Binary function for determining the ordering between two Itin objects
// based on travelseg size. The bigger size will be on the top.
//----------------------------------------------------------------------------
class SortByTravelSegSize : public std::binary_function<Itin*, Itin*, bool>
{
public:
  bool operator()(const Itin* x, const Itin* y)
  {
    return (x->travelSeg().size() > y->travelSeg().size());
  }
};

inline void
errorInRequest(const std::string& msg)
{
  throw ErrorResponseException(ErrorResponseException::INVALID_INPUT,
                               std::string("Error in Request: " + msg).c_str());
}

} // namespace

void
XformClientShoppingXML::validateTrx(PricingTrx& trx)
{
  if (trx.getRequest() == nullptr)
    errorInRequest("'Request' not initialized");

  if (trx.getOptions() == nullptr)
    errorInRequest("'Options' not initialized");

  if (trx.billing() == nullptr)
    errorInRequest("'Billing' not initialized");

  if (trx.getRequest()->ticketingAgent() == nullptr)
    errorInRequest("'Ticketing Agent' not initialized");

  if (trx.getRequest()->ticketingAgent()->agentDuty().empty() &&
      trx.billing()->partitionID().empty() == false)
    errorInRequest("'Ticketing Agent' duty code (AGI/N0G) is required.");

  if (trx.getTrxType() != PricingTrx::IS_TRX && trx.getTrxType() != PricingTrx::ESV_TRX &&
      trx.getTrxType() != PricingTrx::FF_TRX && trx.getTrxType() != PricingTrx::RESHOP_TRX &&
      trx.diagnostic().diagnosticType() >= 900 && trx.diagnostic().diagnosticType() <= 969)
  {
    if (!trx.diagnostic().processAllServices())
      errorInRequest("Itinerary Selector diagnostic specified for non-IS and non-ESV request");
  }

  // display short version of metric if diagnostic is not entered
  if (DiagnosticNone == trx.diagnostic().diagnosticType())
  {
    trx.recordTopLevelMetricsOnly() = true;
  }

  for (std::vector<TravelSeg*>::const_iterator i = trx.travelSeg().begin();
       i != trx.travelSeg().end();
       ++i)
  {
    if (UNLIKELY((*i)->origin() == nullptr))
      errorInRequest("Invalid origin in travel segment");

    if (UNLIKELY((*i)->destination() == nullptr))
      errorInRequest("Invalid destination in travel segment");
  }

  // For settlement types we don't need to check itin/pricing/etc
  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.getRequest()->isSettlementTypesRequest())
    return;

  if (trx.getTrxType() == PricingTrx::MIP_TRX && trx.itin().empty())
    errorInRequest("Request has no itineraries");

  for (std::vector<Itin*>::const_iterator i = trx.itin().begin(); i != trx.itin().end(); ++i)
  {
    if (UNLIKELY((*i)->travelSeg().empty()))
      errorInRequest("Itinerary has no travel segments");
  }

  if (trx.getRequest()->originBasedRTPricing() && trx.itin().front()->legID().size() != 1)
    errorInRequest("originBasedRTPricing qualifier used, mip request has to be OW");

  if (trx.paxType().empty())
    errorInRequest("No Pax types given");

  std::vector<PaxType*>::const_iterator paxTypeItem = trx.paxType().begin();
  std::vector<PaxType*>::const_iterator paxTypeEnd = trx.paxType().end();
  std::set<PaxTypeCode> paxTypeCodeSet;
  if (paxTypeCodeSet.size() > MAX_PAX_COUNT)
    errorInRequest("Format - Too many psgr types -0084");

  if (trx.getOptions()->isCarnivalSumOfLocal() && (trx.paxType().size() != 1 || trx.isAltDates()))
    errorInRequest("Sum of local processing supports only single "
                   "date and single passenger type requests");

  // move the right booking cabin for WPNI.C
  if (trx.billing()->actionCode().find("WPNI") == 0)
  {
    populateBookCabinForWPNIC(trx.itin(), trx);
  }

  // validate posPaxType ( fare group ) for shopping transactions
  RexShoppingTrx* const rexShoppingTrxPtr = dynamic_cast<RexShoppingTrx*>(&trx);
  ShoppingTrx* const shoppingTrxPtr = dynamic_cast<ShoppingTrx*>(&trx);
  FlightFinderTrx* const ffinderTrx = dynamic_cast<FlightFinderTrx*>(&trx);

  if (ffinderTrx != nullptr)
  {
    if (ffinderTrx->isBffReq())
    {
      // BFF validation
      if (ffinderTrx->bffStep() == FlightFinderTrx::STEP_1)
      {
        if (ffinderTrx->departureDT().isEmptyDate() || ffinderTrx->numDaysFwd() == 0)
          errorInRequest("BFF/STEP_1 Invalid input");

        if (!ffinderTrx->avlInS1S3Request())
        {
          if (trx.diagnostic().diagnosticType() == Diagnostic902 ||
              trx.diagnostic().diagnosticType() == Diagnostic911)
            errorInRequest("Invalid Bff diagnostic specified");
        }
      }
      else if (ffinderTrx->bffStep() == FlightFinderTrx::STEP_2)
      {
        if (!ffinderTrx->departureDT().isEmptyDate() || ffinderTrx->numDaysFwd() != 0)
          errorInRequest("BFF/STEP_2 Invalid input");
      }
      else if (ffinderTrx->bffStep() == FlightFinderTrx::STEP_3)
      {
        if (ffinderTrx->numDaysFwd() == 0 || ffinderTrx->isOW() ||
            ffinderTrx->altDatePairs().empty())
          errorInRequest("BFF/STEP_3 Invalid input");

        if (!ffinderTrx->avlInS1S3Request())
        {
          if (trx.diagnostic().diagnosticType() == Diagnostic902 ||
              trx.diagnostic().diagnosticType() == Diagnostic911)
            errorInRequest("Invalid Bff diagnostic specified");
        }
      }
      else if (ffinderTrx->bffStep() == FlightFinderTrx::STEP_4)
      {
        if (!ffinderTrx->departureDT().isEmptyDate() || ffinderTrx->numDaysFwd() != 0 ||
            ffinderTrx->isOW() || ffinderTrx->altDatePairs().empty())
          errorInRequest("BFF/STEP_4 Invalid input");
      }
      else if (ffinderTrx->bffStep() == FlightFinderTrx::STEP_5)
      {
        if (!ffinderTrx->departureDT().isEmptyDate() || ffinderTrx->numDaysFwd() != 0 ||
            !ffinderTrx->altDatePairs().empty())
          errorInRequest("BFF/STEP_5 Invalid input");
      }
      else if (ffinderTrx->bffStep() == FlightFinderTrx::STEP_6)
      {
        if (!ffinderTrx->departureDT().isEmptyDate() || ffinderTrx->numDaysFwd() != 0 ||
            ffinderTrx->isOW() || !ffinderTrx->altDatePairs().empty())
          errorInRequest("BFF/STEP_6 Invalid input");
      }
    }
    else // FF request
    {
      bool isRoundTrip = ffinderTrx->legs().size() == 2;
      PricingTrx::AltDatePairs& datePairs = ffinderTrx->altDatePairs();

      for (PricingTrx::AltDatePairs::iterator iter = datePairs.begin(); iter != datePairs.end();)
      {
        const DatePair& datePair = iter->first;
        if (isRoundTrip && (datePair.first.isEmptyDate() || datePair.second.isEmptyDate() ||
                            !datePair.first.isValid() || !datePair.second.isValid()))
        {
          datePairs.erase(iter++);
        }
        else
        {
          ++iter;
        }
      }
    }
  }
  else if (rexShoppingTrxPtr != nullptr)
  {
    RexShoppingTrx& rexTrx = *rexShoppingTrxPtr;

    for (std::vector<TravelSeg*>::const_iterator i = rexTrx.travelSeg().begin();
         i != rexTrx.travelSeg().end();
         ++i)
    {
      if ((*i)->origin() == nullptr)
        errorInRequest("Invalid origin in exchange travel segment");

      if ((*i)->destination() == nullptr)
        errorInRequest("Invalid destination in exchange travel segment");
    }

    if (rexTrx.exchangeItin().empty())
      errorInRequest("No exchange itin");

    if (rexTrx.newItin().empty())
      errorInRequest("No new itin");

    for (std::vector<ExcItin*>::const_iterator i = rexTrx.exchangeItin().begin();
         i != rexTrx.exchangeItin().end();
         ++i)
    {
      if ((*i)->travelSeg().empty())
        errorInRequest("Exchange itinerary has no travel segments");
    }

    if (rexTrx.excLegs().empty())
      errorInRequest("No legs found in RESHOP request");
  }
  else if (shoppingTrxPtr != nullptr)
  {
    ShoppingTrx& shoppingTrx = *shoppingTrxPtr;

    if (shoppingTrx.legs().empty())
      errorInRequest("No legs found in IS request");

    if (shoppingTrx.isSimpleTrip() && ShoppingUtil::isBrazilDomestic(shoppingTrx))
    {
      shoppingTrx.diversity().setModel(DiversityModelType::V2);
    }

    if (shoppingTrx.isSumOfLocalsProcessingEnabled() &&
        !DiversityModelFactory::checkModelType(shoppingTrx, shoppingTrx.diversity().getModel()))
    {
      if (shoppingTrx.isAltDates())
        errorInRequest("INCORRECT DIVERSITY MODEL - "
                       "ONLY PRICE IS CURRENTLY SUPPORTED FOR ALTDATES");
      else
        errorInRequest("INCORRECT DIVERSITY MODEL");
    }

    if (shoppingTrx.isAltDates())
    {
      if (shoppingTrx.legs().size() > 2)
        errorInRequest("MORE THAN 2 LEGS FOR ALTERNATE DATE REQUEST");

      if (shoppingTrx.legs().size() == 2)
      {
        if (false == shoppingTrx.isSimpleTrip())
          errorInRequest("ALT DATE VALID ONLY FOR ONE WAY OR SIMPLE ROUND TRIP RQST");
      }

      if (shoppingTrx.getRequestedNumOfEstimatedSolutions() > 0)
      {
        trx.getOptions()->setRequestedNumberOfSolutions(9);
        shoppingTrx.setRequestedNumOfEstimatedSolutions(-1);
      }
      shoppingTrx.altDateReuseOutBoundProcessed() = altDateReuseOutboundProcessed.getValue();
      shoppingTrx.altDateReuseInBoundProcessed() = altDateReuseInboundProcessed.getValue();
    }
    for (std::vector<ShoppingTrx::Leg>::const_iterator i = shoppingTrx.legs().begin();
         i != shoppingTrx.legs().end();
         ++i)
    {
      if (i->sop().empty())
        errorInRequest("Leg has no scheduling options");
    }

    if (!shoppingTrx.isSumOfLocalsProcessingEnabled())
    {
      shoppingTrx.startShortCutPricingItin(0);
    }
  }

  paxTypeItem = trx.paxType().begin();
  paxTypeEnd = trx.paxType().end();

  bool posPaxTypePresent = false;
  uint16_t paxIndex = 0;
  std::string corpID;
  for (; paxTypeItem != paxTypeEnd; ++paxTypeItem, ++paxIndex)
  {
    // check to whether all pax types have fare group input (posPaxType).
    if (!(trx.posPaxType()[paxIndex].empty()))
    {
      posPaxTypePresent = true;
      if (trx.getOptions()->xoFares())
        errorInRequest("Not allow Fare group and XO in the same request");

      std::vector<PosPaxType*>& posPaxTypeVec = trx.posPaxType()[paxIndex];
      posPaxTypeVec.erase(std::unique(posPaxTypeVec.begin(), posPaxTypeVec.end(), posPaxTypeEqual),
                          posPaxTypeVec.end());

      std::vector<PosPaxType*>::iterator posPaxType = posPaxTypeVec.begin();

      for (; posPaxType != posPaxTypeVec.end(); ++posPaxType)
      {
        // if corID is present, all posPaxTypes must have the same corpID.
        if (!((*posPaxType)->corpID().empty()))
        {
          if (corpID.empty())
          {
            corpID = (*posPaxType)->corpID();
          }
          else if (corpID != (*posPaxType)->corpID())
            errorInRequest("Fare Groups have different corpID");
        }
        // skip check for the 1st item, since there is nothing to compare.
        // if pcc is present, for the same pax type must have pcc.
        // The empty pcc must be the last item for the same pax type after sorted.
        if (posPaxType + 1 != posPaxTypeVec.end())
        {
          if (((*posPaxType)->pcc().empty()) != ((*(posPaxType + 1))->pcc().empty()))
          {
            if ((*posPaxType)->paxType() == (*(posPaxType + 1))->paxType())
              errorInRequest("Fare Group has with and without pcc for the same pax type");
          }
        }
      } // for posPaxType
    }
  } // for paxType
  // set up the request corp id
  if (posPaxTypePresent)
  {
    // trx.getRequest()->fareGroupRequested() = true;
    if ((*trx.paxType().begin())->paxType() == "JCB")
    {
      trx.getOptions()->web() = false; // turn off web request
    }
    if (!corpID.empty())
    {
      trx.getRequest()->corporateID() = corpID;
    }
  }

  if ((trx.getRequest()->corporateID().empty() == false) &&
      (!trx.dataHandle().corpIdExists(trx.getRequest()->corporateID(), DateTime::localTime())))
  {
    errorInRequest("INVALID CORPORATE ID " + trx.getRequest()->corporateID());
  }

  if (((trx.getRequest()->exemptSpecificTaxes()) || (trx.getRequest()->exemptAllTaxes())) &&
      (trx.getRequest()->taxOverride().empty() == false))
    errorInRequest("FORMAT - CANNOT USE TX WITH TE");

  // Mark multi account code / corporate ID scenario
  if (!trx.getRequest()->accCodeVec().empty() || !trx.getRequest()->corpIdVec().empty() ||
      !trx.getRequest()->incorrectCorpIdVec().empty())
  {
    trx.getRequest()->isMultiAccCorpId() = true;

    if ((trx.getRequest()->corpIdVec().size() + trx.getRequest()->accCodeVec().size()) >=
        _minAccCorpIds)
    {
      trx.getRequest()->isExpAccCorpId() = true;
    }
  }

  const std::vector<std::string>& incorrectCorpIdVec = trx.getRequest()->incorrectCorpIdVec();

  if (!incorrectCorpIdVec.empty() && trx.getRequest()->corpIdVec().empty() &&
      trx.getRequest()->accCodeVec().empty())
  {
    std::string errMsg = "INVALID CORPORATE ID";
    std::vector<std::string>::const_iterator corpIdIter = incorrectCorpIdVec.begin();
    std::vector<std::string>::const_iterator corpIdEnd = incorrectCorpIdVec.end();

    while (corpIdIter != corpIdEnd)
    {
      errMsg = errMsg + " " + (*corpIdIter);
      ++corpIdIter;
    }

    errorInRequest(errMsg);
  }

  if (trx.getRequest()->brandedFareEntry())
  {
    if (trx.getRequest()->getBrandedFareSize() < 1)
      errorInRequest("Could not process branded fare entry without brands");
  }
  else if (!trx.getRequest()->isBrandedFaresRequest())
  {
    if (trx.getRequest()->getBrandedFareSize() > 0)
      errorInRequest("Brands could not be processed for non branded fare entry");
  }

  if (trx.getRequest()->isAdvancedBrandProcessing())
  {
    if (trx.getRequest()->getBrandedFareSize() > 1)
      errorInRequest("Advanced brands could not process more than one brand");

    if (!trx.getRequest()->hasConsistenBrandedBookingCodes())
      errorInRequest("Advanced brands could not process overlapped booking codes");
  }

  if (trx.getTrxType() == PricingTrx::MIP_TRX)
  {
    std::sort(trx.itin().begin(), trx.itin().end(), SortByTravelSegSize());
  }
}

const Cabin*
XformClientShoppingXML::getCabin(PricingTrx& trx, const AirSeg& airSeg)
{
  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(trx))
  {
    RBDByCabinUtil rbdCabin(trx, SHP_WPNI);
    return rbdCabin.getCabinForAirseg(airSeg);
  }
  else
  {
    return trx.dataHandle().getCabin(
        airSeg.carrier(), airSeg.getBookingCode(), airSeg.departureDT());
  }
  return nullptr;
}

void
XformClientShoppingXML::populateBookCabinForWPNIC(std::vector<Itin*>& itins, PricingTrx& trx)
{
  std::vector<Itin*>::iterator itinIter = itins.begin();
  std::vector<Itin*>::iterator itinEnd = itins.end();
  for (; itinIter != itinEnd; ++itinIter)
  {
    std::vector<TravelSeg*>::iterator tvlIter = (*itinIter)->travelSeg().begin();
    std::vector<TravelSeg*>::iterator tvlEnd = (*itinIter)->travelSeg().end();
    for (; tvlIter != tvlEnd; ++tvlIter)
    {
      const AirSeg* airSeg = (dynamic_cast<const AirSeg*>(*tvlIter));
      if (airSeg == nullptr)
      {
        continue;
      }
      const Cabin* cabin = getCabin(trx, *airSeg);

      if (cabin != nullptr)
      {
        CabinType cabinType = cabin->cabin();

        (*tvlIter)->bookedCabin() = cabinType;

        if (trx.legPreferredCabinClass()[airSeg->legId()] > cabinType)
        {
          trx.legPreferredCabinClass()[airSeg->legId()] = cabinType;
        }
      }
    }
  }
}

void
XformClientShoppingXML::performDiagnostic982(PricingTrx& trx, const std::map<int, Itin*>& itinMap)
    const
{
  if (trx.diagnostic().diagnosticType() == Diagnostic982 &&
      trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ITINMIP")
  {
    DCFactory* factory = DCFactory::instance();
    Diag982Collector* diag982 = dynamic_cast<Diag982Collector*>(factory->create(trx));
    diag982->enable(Diagnostic982);

    diag982->showMIPItin(itinMap);

    diag982->flushMsg();
  }
}

} // tse
