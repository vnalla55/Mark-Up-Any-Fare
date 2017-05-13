// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#include "Taxes/LegacyFacades/AtpcoTaxDisplayDriver.h"

#include "DataModel/Billing.h"
#include "DataModel/TaxRequest.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Code.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/DefaultServices.h"
#include "Taxes/AtpcoTaxes/TaxDisplay/Common/TaxDisplayRequest.h"
#include "Taxes/AtpcoTaxes/TaxDisplay/TaxDisplay.h"
#include "Taxes/LegacyFacades/CarrierApplicationServiceV2.h"
#include "Taxes/LegacyFacades/CarrierFlightServiceV2.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/LocServiceV2.h"
#include "Taxes/LegacyFacades/NationServiceV2.h"
#include "Taxes/LegacyFacades/PassengerTypesServiceV2.h"
#include "Taxes/LegacyFacades/RulesRecordsServiceV2.h"
#include "Taxes/LegacyFacades/ReportingRecordServiceV2.h"
#include "Taxes/LegacyFacades/ServiceBaggageServiceV2.h"
#include "Taxes/LegacyFacades/ServiceFeeSecurityServiceV2.h"
#include "Taxes/LegacyFacades/SectorDetailServiceV2.h"
#include "Taxes/LegacyFacades/TaxCodeTextServiceV2.h"
#include "Taxes/LegacyFacades/TaxReissueServiceV2.h"
#include "Taxes/LegacyFacades/TaxRoundingInfoServiceV2.h"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>

#include <stdexcept>

namespace
{

tax::display::TaxDisplayRequest::EntryType
validateEntryCmdByTax(const std::string& entryCmd)
{
  if (entryCmd == "TX1")
    // TX1**[TAX CODE]
    return tax::display::TaxDisplayRequest::EntryType::RULESRECORD_ENTRY_BY_TAX;

  // TX**[TAX CODE]
  return tax::display::TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_TAX;
}

tax::display::TaxDisplayRequest::EntryType
validateEntryCmdByNation(const std::string& entryCmd)
{
  if (entryCmd == "TX1")
    // TX1*[NATION CODE]
    return tax::display::TaxDisplayRequest::EntryType::RULESRECORD_ENTRY_BY_NATION;

  // TX*[NATION CODE]
  return tax::display::TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_NATION;
}

tax::display::TaxDisplayRequest::EntryType
getEntryType(const std::string& entryCmd, const std::string& entryType)
{
  if (entryType == "*")
    return validateEntryCmdByNation(entryCmd);

  if (entryType == "**")
    return validateEntryCmdByTax(entryCmd);

  if (entryType == "/HELP")
    return tax::display::TaxDisplayRequest::EntryType::ENTRY_HELP_CALCULATION_BREAKDOWN;

  return tax::display::TaxDisplayRequest::EntryType::ENTRY_HELP;
}

tax::display::TaxDisplayRequest::UserType
getUserType(const std::string& pseudoCityCode)
{

  if (pseudoCityCode == "HDQ")
    return tax::display::TaxDisplayRequest::UserType::SABRE;

  if (pseudoCityCode.length() == 4)
    return tax::display::TaxDisplayRequest::UserType::TN;

  return tax::display::TaxDisplayRequest::UserType::AS;
}

std::vector<tax::type::CarrierCode>
getCarrierCodes(const std::string& carrierCodesStr)
{
  std::vector<tax::type::CarrierCode> ret;

  const boost::char_separator<char> carriersSeparator("|");
  boost::tokenizer<boost::char_separator<char>> tokens(carrierCodesStr, carriersSeparator);
  for (const std::string& carrierStr : tokens)
  {
    tax::type::CarrierCode carrierCode;
    carrierCode.fromString(carrierStr.c_str(), carrierStr.size());
    ret.push_back(std::move(carrierCode));
  }

  return ret;
}

void setCategories(const std::string& categoriesStr,
                   std::vector<bool>& categoriesVec,
                   tax::display::CategoryNo min,
                   tax::display::CategoryNo max)
{
  if (categoriesStr.empty())
  {
    std::fill(categoriesVec.begin(), categoriesVec.end(), true);
    return;
  }

  assert(max <= categoriesVec.size());

  const boost::char_separator<char> categoriesSeparator("|");
  boost::tokenizer<boost::char_separator<char>> tokens(categoriesStr, categoriesSeparator);
  for (const std::string& categoryStr : tokens)
  {
    tax::display::CategoryNo category =
        boost::lexical_cast<tax::display::CategoryNo>(categoryStr);
    if (category >= min && category < max)
    {
      categoriesVec[category] = true;
    }
  }
}

} // anonymous namespace

namespace tse
{

bool AtpcoTaxDisplayDriver::buildResponse()
{
  tax::DefaultServices services;
  services.setRulesRecordsService(&RulesRecordsServiceV2::instance());
  services.setReportingRecordService(
      new ReportingRecordServiceV2(_trx.getRequest()->requestDate()));
  services.setTaxRoundingInfoService(new TaxRoundingInfoServiceV2(_trx));
  services.setLocService(new LocServiceV2(_trx.getRequest()->requestDate()));
  services.setNationService(new NationServiceV2());
  services.setTaxCodeTextService(new TaxCodeTextServiceV2(_trx.dataHandle()));
  services.setCarrierApplicationService(new CarrierApplicationServiceV2(_trx.getRequest()->requestDate()));
  services.setCarrierFlightService(new CarrierFlightServiceV2(_trx.getRequest()->requestDate()));
  services.setPassengerTypesService(new PassengerTypesServiceV2(_trx.getRequest()->requestDate()));
  services.setServiceFeeSecurityService(new ServiceFeeSecurityServiceV2(_trx.getRequest()->requestDate()));
  services.setServiceBaggageService(new ServiceBaggageServiceV2(_trx.getRequest()->requestDate()));
  services.setSectorDetailService(new SectorDetailServiceV2(_trx.getRequest()->requestDate()));
  services.setTaxReissueService(new TaxReissueServiceV2());

  tax::display::TaxDisplayRequest request;
  prepareRequest(request);
  request.userType = getUserType(_trx.billing()->userPseudoCityCode());

  tax::display::TaxDisplay taxDisplay(request, services);
  return taxDisplay.buildResponse(_trx.response());
}

void AtpcoTaxDisplayDriver::prepareRequest(tax::display::TaxDisplayRequest& request)
{
  const TaxRequest& tseRequest = *_trx.getRequest();
  request.airportCode = toTaxAirportCode(tseRequest.airportCode());

  std::vector<tax::type::CarrierCode> carrierCodes = getCarrierCodes(tseRequest.carrierCodes());
  try
  {
    request.carrierCode1 = carrierCodes.at(0);
    request.carrierCode2 = carrierCodes.at(1);
    request.carrierCode3 = carrierCodes.at(2); // up to 3 carriers, ignore the rest
  }
  catch (const std::out_of_range&) {} // do nothing, it only means there wasn't that much carriers given in the request


  request.detailLevels = tseRequest.txEntryDetailLevels();
  request.nationCode = toTaxNationCode(tseRequest.nation());
  request.nationName = tseRequest.nationName();
  request.isReissued = tseRequest.getReissue();

  if (tseRequest.sequenceNumber() != TaxRequest::EMPTY_SEQUENCE)
    request.seqNo = tseRequest.sequenceNumber();

  request.taxCode = toTaxCode(tseRequest.taxCode());
  request.taxType = toTaxType(tseRequest.taxType());
  request.entryType = getEntryType(_trx.getRequest()->txEntryCmd(),
                                   _trx.getRequest()->txEntryType());

  request.x1categories.resize(static_cast<size_t>(tax::display::X1Category::ENUM_CATEGORY_SIZE), false);
  if (request.entryType == tax::display::TaxDisplayRequest::EntryType::RULESRECORD_ENTRY_BY_NATION ||
      request.entryType == tax::display::TaxDisplayRequest::EntryType::RULESRECORD_ENTRY_BY_TAX)
  {
    setCategories(tseRequest.categories(),
                  request.x1categories,
                  static_cast<tax::display::CategoryNo>(tax::display::X1Category::TAX),
                  static_cast<tax::display::CategoryNo>(tax::display::X1Category::ENUM_CATEGORY_SIZE));
  }

  request.x2categories.resize(static_cast<size_t>(tax::display::X2Category::ENUM_CATEGORY_SIZE), false);
  if (request.entryType == tax::display::TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_NATION ||
      request.entryType == tax::display::TaxDisplayRequest::EntryType::REPORTINGRECORD_ENTRY_BY_TAX)
  {
    setCategories(tseRequest.categories(),
                  request.x2categories,
                  static_cast<tax::display::CategoryNo>(tax::display::X2Category::DEFINITION),
                  static_cast<tax::display::CategoryNo>(tax::display::X2Category::ENUM_CATEGORY_SIZE));
  }
  request.showCategoryForEachTax = !tseRequest.categories().empty();

  request.requestDate =
      DaoDataFormatConverter::fromTimestamp(tseRequest.requestDate());
  request.travelDate =
      DaoDataFormatConverter::fromTimestamp(tseRequest.travelDate());
}

} /* namespace tse */
