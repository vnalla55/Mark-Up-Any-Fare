// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "TaxDisplay/ViewX1SequenceDetail.h"

#include "Common/TaxName.h"
#include "Common/MoneyUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/Code.h"
#include "DataModel/Services/RulesRecord.h"
#include "Rules/MathUtils.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/TaxRoundingInfoService.h"
#include "TaxDisplay/Common/TaxDisplayRequest.h"
#include "TaxDisplay/Response/LineParams.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

using boost::format;

namespace tax
{
namespace display
{

namespace
{

inline void addSeparated(const std::string& what,
                         std::string& where,
                         const std::string& separator = ", ")
{
  if (where.size() > 0)
    where += separator;

  where += what;
}

template <typename SafeEnum>
std::list<Line> getEnumStr(std::string&& title, SafeEnum safeEnum)
{
  std::list<Line> lines;
  lines.emplace_back(title);
  lines.emplace_back(safeEnumToString(safeEnum));
  lines.emplace_back(".");
  lines.emplace_back(".");

  return lines;
}

std::string getTaxDisplayCarrierCode(const type::CarrierCode& code)
{
  if (code == "YY")
    return "YY-ALL CXRS";

  return std::move(code.asString());
}

std::string getFormatted(const type::Date& date)
{
  std::string&& formatted = date.format("%d%b%Y");
  boost::to_upper(formatted);
  return formatted;
}

std::list<Line> getLocationShortStr(const LocService& locService,
                                    const LocZone& loc,
                                    std::string&& separator = ": ")
{
  std::string locStr;
  if (loc.type() == type::LocType::Nation)
  {
    type::Nation nationCode;
    codeFromString(loc.code().asString(), nationCode);
    locStr = locService.getNationName(nationCode);
  }
  else
  {
    locStr = loc.code().asString();
  }

  // TODO: get zone where POSTYPE = M || POSTYPE = R!

  std::list<Line> ret;
  ret.emplace_back(safeEnumToString(loc.type()) + separator + locStr);
  return ret;
}

std::list<Line> getLocationStr(const LocService& locService,
                               const LocZone& loc,
                               std::string&& title = "")
{
  std::list<Line> ret;
  if (!title.empty())
    ret.emplace_back(title);
  ret.splice(ret.end(), getLocationShortStr(locService, loc));
  ret.emplace_back(".");
  ret.emplace_back(".");

  return ret;
}

std::string getRoundingUnitStr(type::MoneyAmount /*unit*/)
{
  // TODO: FINISH HIM
  return "";
}

std::string getRoundingDirectionStr(type::TaxRoundingDir direction)
{
  switch (direction)
  {
  case type::TaxRoundingDir::Nearest:
    return "N-ROUND TO NEAREST";
  case type::TaxRoundingDir::RoundDown:
    return "D-ROUND DOWN";
  case type::TaxRoundingDir::RoundUp:
    return "U-ROUND UP";
  case type::TaxRoundingDir::NoRounding:
    return "S-NO ROUNDING APPLIES";
  default:
    return "NONE SPECIFIED";
  }
}

std::string getTaxMinMaxStr(const type::CurrencyCode& currency,
                            type::CurDecimals decimals,
                            uint32_t min,
                            uint32_t max)
{
  type::MoneyAmount moneyMin = MathUtils::adjustDecimal(min, decimals);
  type::MoneyAmount moneyMax = MathUtils::adjustDecimal(max, decimals);

  format line("%-15s %-15s %s");
  line % (min > 0 ? boost::lexical_cast<std::string>(moneyMin) + "-MIN" : "NO MIN");
  line % (max > 0 ? boost::lexical_cast<std::string>(moneyMax) + "-MAX" : "NO MAX");
  line % currency.asString();

  return line.str();
}

} // anonymous namespace

bool ViewX1SequenceDetail::header()
{
  const TaxName& taxName = _rulesRecord.taxName;
  _formatter.addLine("COUNTRY CODE TAX CODE TAX TYPE   CARRIER       SEQ NO    SOURCE")
            .addLine(format("%-12s %-8s   %-6s   %-11s %10d  %s") % taxName.nation().asString()
                                                                  % taxName.taxCode().asString()
                                                                  % taxName.taxType().asString()
                                                                  % getTaxDisplayCarrierCode(taxName.taxCarrier())
                                                                  % _rulesRecord.seqNo
                                                                  % _rulesRecord.vendor.asString());
  _formatter.addLine(".")
            .addLine(format("PERCENT/FLAT: %-7s TAX POINT: %-9s TAX REMIT: %-4s") % safeEnumToString(taxName.percentFlatTag())
                                                                                  % safeEnumToString(taxName.taxPointTag())
                                                                                  % safeEnumToString(taxName.taxRemittanceId()))
            .addLine(".")
            .addLine("TAX CATEGORIES-REFER TO TXHELP FOR SELECTIVE DISPLAY")
            .addLine("CATEGORIES WITH NO APPLICABLE DATA MAY NOT DISPLAY", LineParams::withLeftMargin(2))
            .addBlankLine()
            .addLine("1-TAX                                  2-DATE APPLICATIONS")
            .addLine("3-SALE/TICKET/DELIVERY                 4-TAX DETAILS")
            .addLine("5-TICKET VALUE                         6-TAXABLE UNIT DETAILS")
            .addLine("7-TRAVEL APPLICATION                   8-TAX POINT SPECIFICATION")
            .addLine("9-TRAVEL/CARRIER QUALIFYING TAGS       10-SERVICE BAGGAGE TAX");

  if (_request.isUserSabre())
  {
    _formatter.addLine("11-TRAVEL SECTOR DETAILS               12-CONNECTION TAGS")
              .addLine("13-PROCESSING APPLICATION DETAIL");
  }
  else
  {
    _formatter.addLine("11-TRAVEL SECTOR DETAILS");
  }
  _formatter.addLine(".");
  _formatter.addLine(".");

  return true;
}

bool ViewX1SequenceDetail::body()
{
  categoryTax();

  if (_request.x1categories[static_cast<size_t>(X1Category::DATE_APPLICATIONS)])
    categoryDateApplications();

  if (_request.x1categories[static_cast<size_t>(X1Category::SALE_TICKET_DELIVERY)])
    categorySaleTicketDelivery();

  if (_request.x1categories[static_cast<size_t>(X1Category::TAX_DETAILS)])
    categoryTaxDetails();

  if (_request.x1categories[static_cast<size_t>(X1Category::TICKET_VALUE)])
    categoryTicketValue();

  if (_request.x1categories[static_cast<size_t>(X1Category::TAXABLE_UNIT_DETAILS)])
    categoryTaxableUnitDetails();

  if (_request.x1categories[static_cast<size_t>(X1Category::TRAVEL_APPLICATION)])
    categoryTravelApplication();

  if (_request.x1categories[static_cast<size_t>(X1Category::TAX_POINT_SPECIFICATION)])
    categoryTaxPointSpecification();

  if (_request.x1categories[static_cast<size_t>(X1Category::TRAVEL_CARRIER_QUALIFYING_TAGS)])
    categoryTravelCarrierQualifyingTags();

  if (_request.x1categories[static_cast<size_t>(X1Category::SERVICE_BAGGAGE_TAX)])
    categoryServiceBaggageTax();

  if (_request.x1categories[static_cast<size_t>(X1Category::TRAVEL_SECTOR_DETAILS)])
    categoryTravelSectorDetails();

  if (_request.x1categories[static_cast<size_t>(X1Category::CONNECTION_TAGS)])
    categoryConnectionTags();

  if (_request.x1categories[static_cast<size_t>(X1Category::PROCESSING_APPLICATION_DETAIL)])
    categoryProcessingApplicationDetail();

  return true;
}

bool ViewX1SequenceDetail::footer()
{
  _formatter.addLine("END OF TAX DISPLAY");
  return true;
}

void ViewX1SequenceDetail::setTravelCarrierQualifyingTagsData(
    const std::shared_ptr<const CarrierApplication>& carrierApplication,
    const std::shared_ptr<const CarrierFlight>& carrierFlightBefore,
    const std::shared_ptr<const CarrierFlight>& carrierFlightAfter,
    const std::shared_ptr<const PassengerTypeCodeItems>& passengerTypeCodeItems)
{
  _carrierApplication = carrierApplication;
  _carrierFlightBefore = carrierFlightBefore;
  _carrierFlightAfter = carrierFlightAfter;
  _passengerTypeCodeItems = passengerTypeCodeItems;
}

void ViewX1SequenceDetail::setServiceBaggageData(
    const std::shared_ptr<const ServiceFeeSecurityItems>& serviceFeeSecurityItems,
    const std::shared_ptr<const ServiceBaggage>& serviceBaggage)
{
  _serviceFeeSecurityItems = serviceFeeSecurityItems;
  _serviceBaggage = serviceBaggage;
}

void ViewX1SequenceDetail::setTravelSectorData(
    const std::shared_ptr<const SectorDetail>& sectorDetail)
{
  _sectorDetail = sectorDetail;
}

void ViewX1SequenceDetail::categoryTax()
{
  _formatter.addLine("1-TAX");

  if (_rulesRecord.taxName.percentFlatTag() == type::PercentFlatTag::Flat)
  {
    _formatter.addLine("TAX CURRENCY: " + _rulesRecord.taxCurrency.asString())
              .addLine("TAX AMOUNT: " + std::to_string(amountToDouble(_rulesRecord.taxAmt)));
  }
  else
  {
    _formatter.addLine(format("TAX AMOUNT: %f PERCENT") % _rulesRecord.taxPercent);
  }

  if (_rulesRecord.exemptTag == type::ExemptTag::Exempt)
    _formatter.addLine("EXEMPT: YES");

  if (_rulesRecord.vatInclusiveInd != type::VatInclusiveInd::Blank)
    _formatter.addLine("VAT INCLUSIVE: " + safeEnumToString(_rulesRecord.vatInclusiveInd));

  if (!_rulesRecord.currencyOfSale.empty())
    _formatter.addLine("CURRENCY OF SALE: " + _rulesRecord.currencyOfSale.asString());

  _formatter.addLine("TAX CARRIER: " + getTaxDisplayCarrierCode(_rulesRecord.taxName.taxCarrier()))
            .addLine(".")
            .addLine(".")
            .addLine(".");
}

void ViewX1SequenceDetail::categoryDateApplications()
{
  _formatter.addLine("2-DATE APPLICATIONS")
            .addBlankLine()
            .addLine(format("SALE/EFF: %9s SALE/DISC: %9s") % getFormatted(_rulesRecord.effDate)
                                                            % getFormatted(_rulesRecord.expiredDate.date()))
            .addLine(".")
            .addLine(".");

  type::Date firstTravelDate = type::Date::make_date(_rulesRecord.firstTravelYear,
                                                     _rulesRecord.firstTravelMonth,
                                                     _rulesRecord.firstTravelDay);
  type::Date lastTravelDate = type::Date::make_date(_rulesRecord.lastTravelYear,
                                                    _rulesRecord.lastTravelMonth,
                                                    _rulesRecord.lastTravelDay);

  if (!firstTravelDate.is_invalid() || !lastTravelDate.is_invalid())
  {
    _formatter.addLine(format("FIRST TRAVEL/DDMMYY: %9s LAST TRAVEL/DDMMYY: %9s") % getFormatted(firstTravelDate)
                                                                                  % getFormatted(lastTravelDate))
              .addLine(".")
              .addLine(".");
  }

  if (_rulesRecord.travelDateTag != type::TravelDateAppTag::Blank)
  {
    _formatter.addLine("TRAVEL DATE APPLICATION TAG: " + safeEnumToString(_rulesRecord.travelDateTag))
              .addLine(".")
              .addLine(".");
  }

  // if (_rulesRecord.RATD_DATE) TODO: get RATD_DATE!!!

  bool validHistEffOrDisc = !_rulesRecord.histSaleEffDate.is_invalid() || !_rulesRecord.histSaleDiscDate.is_invalid();
  bool validHistTravel = !_rulesRecord.histTrvlEffDate.is_invalid() || !_rulesRecord.histTrvlDiscDate.is_invalid();
  if (validHistEffOrDisc || validHistTravel)
  {
    _formatter.addLine("HISTORICAL DATES:");
    if (validHistEffOrDisc)
    {
      _formatter.addLine(format("SALE/EFF: %9s SALE/DISC: %9s") % getFormatted(_rulesRecord.histSaleEffDate)
                                                                % getFormatted(_rulesRecord.histTrvlDiscDate))
                .addLine(".")
                .addLine(".");
    }

    if (validHistTravel)
    {
      _formatter.addLine(format("FIRST TRAVEL/DDMMYY: %9s LAST TRAVEL/DDMMYY: %9s") % getFormatted(_rulesRecord.histTrvlEffDate)
                                                                                    % getFormatted(_rulesRecord.histTrvlDiscDate))
                .addLine(".")
                .addLine(".");
    }
  }
}

void ViewX1SequenceDetail::categorySaleTicketDelivery()
{
  _formatter.addLine("3-SALE/TICKET/DELIVERY DETAILS:");

  _formatter.addLine("POINT OF SALE:");
  if (_rulesRecord.pointOfSale.isSet())
  {
    _formatter.addLine(getLocationStr(_locService, _rulesRecord.pointOfSale));
  }
  else
  {
    _formatter.addLine("ANYWHERE");
  }

  // if (_rulesRecord) TODO: POINT OF SALE SECURITY! why are we losing POSLOCZONE field?

  _formatter.addLine("POINT OF TICKETING:");
  if (_rulesRecord.pointOfTicketing.isSet())
  {
    _formatter.addLine(getLocationStr(_locService, _rulesRecord.pointOfTicketing));
  }
  else
  {
    _formatter.addLine("ANYWHERE");
  }

  _formatter.addLine("POINT OF DELIVERY:");
  if (_rulesRecord.pointOfDelivery.isSet())
  {
    _formatter.addLine(getLocationStr(_locService, _rulesRecord.pointOfTicketing));
  }
  else
  {
    _formatter.addLine("ANYWHERE");
  }
}

void ViewX1SequenceDetail::categoryTaxDetails()
{
  _formatter.addLine("4-TAX DETAILS:");
  if (_rulesRecord.minMaxCurrency != UninitializedCode)
  {
    _formatter.addLine("MIN/MAX TAX:");
    std::string&& minMaxStr = getTaxMinMaxStr(_rulesRecord.minMaxCurrency,
                                              _rulesRecord.minMaxDecimals,
                                              _rulesRecord.minTax,
                                              _rulesRecord.maxTax);
    _formatter.addLine(minMaxStr)
              .addLine(".")
              .addLine(".");
  }

  type::MoneyAmount unit = -1;
  type::TaxRoundingDir direction(type::TaxRoundingDir::Blank);
  _roundingService.getNationRoundingInfo(_rulesRecord.taxName.nation(), unit, direction);

  _formatter.addLine("TAX ROUNDING UNIT: " +
                     (direction == type::TaxRoundingDir::Blank ?
                         "NONE SPECIFIED" :
                         getRoundingUnitStr(unit)));
  _formatter.addLine(".")
            .addLine(".")
            .addLine("TAX ROUNDING DIRECTION: " + getRoundingDirectionStr(direction))
            .addLine(".")
            .addLine(".")
            .addLine(".");
}

void ViewX1SequenceDetail::categoryTicketValue()
{
  std::list<Line> lines;

  if (_rulesRecord.tktValCurrency != UninitializedCode)
  {
    lines.emplace_back("MIN/MAX TAX:");
    std::string&& minMaxStr = getTaxMinMaxStr(_rulesRecord.tktValCurrency,
                                              _rulesRecord.tktValCurrDecimals,
                                              _rulesRecord.tktValMin,
                                              _rulesRecord.tktValMax);
  }

  if (_rulesRecord.tktValApplQualifier != type::TktValApplQualifier::Blank)
  {
    lines.splice(lines.end(), getEnumStr("TICKET VALUE APPLICATION:",
                                         _rulesRecord.tktValApplQualifier));
  }

  if (_request.isUserSabre() &&
      _rulesRecord.paidBy3rdPartyTag != type::PaidBy3rdPartyTag::Blank)
  {
    lines.splice(lines.end(), getEnumStr("FORM OF PAYMENT PAID BY THIRD PARTY:",
                                         _rulesRecord.paidBy3rdPartyTag));
  }

  if (!lines.empty())
  {
    _formatter.addLine("5-TICKET VALUE DETAILS:")
              .addLine(std::move(lines))
              .addLine(".");
  }
}

void ViewX1SequenceDetail::categoryTaxableUnitDetails()
{
  _formatter.addLine("6-TAXABLE UNIT DETAILS:")
            .addBlankLine();

  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::YqYr))
    _formatter.addLine("TAG1-FUEL AND INSURANCE");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::TicketingFee))
    _formatter.addLine("TAG2-TICKETING FEES");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCFlightRelated))
    _formatter.addLine("TAG3-FLIGHT RELATED");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCTicketRelated))
    _formatter.addLine("TAG4-TICKET RELATED");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCMerchandise))
    _formatter.addLine("TAG5-MERCHANDISE");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::OCFareRelated))
    _formatter.addLine("TAG6-FARE RELATED");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::BaggageCharge))
    _formatter.addLine("TAG7-BAGGAGE CHARGES");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::TaxOnTax))
    _formatter.addLine("TAG8-TAX ON TAX");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::Itinerary))
    _formatter.addLine("TAG9-ITINERARY");
  if (_rulesRecord.applicableTaxableUnits.hasTag(type::TaxableUnit::ChangeFee))
    _formatter.addLine("TAG10-CHANGE FEE");
  _formatter.addLine(".")
            .addLine(".");

  if (_rulesRecord.taxAppliesToTagInd != type::TaxAppliesToTagInd::Blank)
    _formatter.addLine(getEnumStr("TAX TAG APPLIES TO:", _rulesRecord.taxAppliesToTagInd));

  if (_rulesRecord.netRemitApplTag != type::NetRemitApplTag::Blank)
    _formatter.addLine(getEnumStr("TAX APPLICATION LIMIT:", _rulesRecord.netRemitApplTag));

// TODO: alternateRuleRefTag uint8_t ? WTF?
//  if (_rulesRecord.alternateRuleRefTag != type::AlaskaZone)
//    _formatter.addLine(getEnumStr("ALTERNATE RULE REFERENCE TAG:", _rulesRecord.alternateRuleRefTag));

  _formatter.addLine(getEnumStr("TICKETED POINT TAG:", _rulesRecord.ticketedPointTag));

  _formatter.addLine(".");
}

void ViewX1SequenceDetail::categoryTravelApplication()
{
  std::list<Line> lines;

  if (_rulesRecord.rtnToOrig != type::RtnToOrig::Blank)
    lines.splice(lines.end(), getEnumStr("RETURN TO ORIGIN:", _rulesRecord.rtnToOrig));

  if (_rulesRecord.jrnyLocZone1.isSet())
    lines.splice(lines.end(), getLocationStr(_locService,
                                             _rulesRecord.jrnyLocZone1,
                                             "JOURNEY ORIGIN LOCATION 1:"));

  if (_rulesRecord.jrnyLocZone2.isSet())
    lines.splice(lines.end(),getLocationStr(_locService,
                                            _rulesRecord.jrnyLocZone2,
                                            "JOURNEY DESTINATION/TURNAROUND LOCATION 2:"));

  if (_rulesRecord.jrnyIncludes.isSet())
    lines.splice(lines.end(), getLocationStr(_locService,
                                             _rulesRecord.jrnyIncludes,
                                             "JOURNEY LOCATIONS INCLUDE:"));

  if (_rulesRecord.trvlWhollyWithin.isSet())
    lines.splice(lines.end(), getLocationStr(_locService,
                                             _rulesRecord.trvlWhollyWithin,
                                             "TRAVEL WHOLLY WITHIN LOCATIONS:"));

  if (_rulesRecord.jrnyInd != type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ)
    lines.splice(lines.end(), getEnumStr("JOURNEY INDICATOR:", _rulesRecord.jrnyInd));

  if (!lines.empty())
  {
    _formatter.addLine("7-TRAVEL APPLICATION DETAILS:");
    _formatter.addLine("JOURNEY DEFINITIONS:", LineParams::withLeftMargin(4));
    _formatter.addLine("JOURNEY-ENTIRE SELECTED ITINERARY", LineParams::withLeftMargin(6));
    _formatter.addLine("JOURNEY ORIGIN-FIRST TICKETED POINT ON ITINEARY", LineParams::withLeftMargin(6));
    _formatter.addLine("JOURNEY DESTINATION-LAST TIKCETED POINT ON ITINERARY", LineParams::withLeftMargin(6));
    _formatter.addLine("JOURNEY TURNAROUND- (NOT APPLICABLE TO ONE-WAY JOURNEY)", LineParams::withLeftMargin(6));
    _formatter.addLine("FURTHEST STOP OR FARE BREAK FROM JOURNEY ORIGIN", LineParams::withLeftMargin(6));

    _formatter.addLine(std::move(lines));
    _formatter.addLine(".");
  }
}

void ViewX1SequenceDetail::categoryTaxPointSpecification()
{
  std::list<Line> lines;

  if (_rulesRecord.taxPointLoc1IntlDomInd != type::AdjacentIntlDomInd::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 1 ADJACENT INTERNATIONAL/DOMESTIC INDICATOR:",
                                         _rulesRecord.taxPointLoc1IntlDomInd));

  if (_rulesRecord.taxPointLoc1TransferType != type::TransferTypeTag::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 1 TRANSFER TYPE:",
                                         _rulesRecord.taxPointLoc1TransferType));

  if (_rulesRecord.taxPointLoc1StopoverTag != type::StopoverTag::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 1 STOPOVER TAG:",
                                         _rulesRecord.taxPointLoc1StopoverTag));

  if (_rulesRecord.taxPointLocZone1.isSet())
    lines.splice(lines.end(), getLocationStr(_locService,
                                             _rulesRecord.taxPointLocZone1,
                                             "TAX POINT LOCATION 1:"));

  if (_rulesRecord.taxPointLoc2IntlDomInd != type::IntlDomInd::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 2 INTERNATIONAL/DOMESTIC INDICATOR:",
                                         _rulesRecord.taxPointLoc2IntlDomInd));

  if (_rulesRecord.taxPointLoc2Compare != type::TaxPointLoc2Compare::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 2 COMPARISON:",
                                         _rulesRecord.taxPointLoc2Compare));

  if (_rulesRecord.taxPointLoc2StopoverTag != type::Loc2StopoverTag::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 2 STOPOVER TAG:",
                                         _rulesRecord.taxPointLoc2StopoverTag));

  if (_rulesRecord.taxPointLocZone2.isSet())
    lines.splice(lines.end(), getLocationStr(_locService,
                                             _rulesRecord.taxPointLocZone2,
                                             "TAX POINT LOCATION 2:"));

  if (_rulesRecord.taxPointLoc3GeoType != type::TaxPointLoc3GeoType::Blank)
    lines.splice(lines.end(), getEnumStr("TAX POINT LOCATION 3 APPLICATION TAG:",
                                         _rulesRecord.taxPointLoc3GeoType));

  if (_rulesRecord.taxPointLocZone3.isSet())
    lines.splice(lines.end(), getLocationStr(_locService,
                                             _rulesRecord.taxPointLocZone3,
                                             "TAX POINT LOCATION 3:"));

  if (!lines.empty())
  {
    _formatter.addLine("8-TAX POINT SPECIFICATION DETAILS:");
    _formatter.addLine(std::move(lines));
    _formatter.addLine(".");
  }
}

void ViewX1SequenceDetail::categoryTravelCarrierQualifyingTags()
{
  _formatter.addLine("9-TRAVEL/CARRIER QUALIFYING TAG DETAILS:")
            .addBlankLine()
            .addLine("STOPOVER TIME/UNIT:");

  if (!_rulesRecord.stopoverTimeTag.empty())
  {
    _formatter.addLine(format("%03s %s (%c)") % _rulesRecord.stopoverTimeTag.asString()
                                              % safeEnumToString(_rulesRecord.stopoverTimeUnit)
                                              % static_cast<char>(_rulesRecord.stopoverTimeUnit));
  }
  else
  {
    _formatter.addLine("NO RESTRICTION");
  }
  _formatter.addLine(".")
            .addLine(".");

  if (_carrierApplication)
  {
    std::list<Line> display = getCarrierApplicationDisplay(*_carrierApplication);
    if (!display.empty())
    {
      if (_request.isUserSabre())
        _formatter.addLine("VALIDATING CARRIER APPLICATION (TABLE 190):");
      else
        _formatter.addLine("VALIDATING CARRIER APPLICATION:");

      _formatter.addLine(std::move(display));
    }
  }

  if (_carrierFlightBefore)
  {
    std::list<Line> table = getCarrierFlightDisplay(*_carrierFlightBefore);
    if (!table.empty())
    {
      if (_request.isUserSabre())
        _formatter.addLine("CARRIER FLIGHT APPLICATION 1 (TABLE 186):");
      else
        _formatter.addLine("CARRIER FLIGHT APPLICATION 1:");

      _formatter.addLine(std::move(table));
    }
  }

  if (_carrierFlightAfter)
  {
    std::list<Line> table = getCarrierFlightDisplay(*_carrierFlightAfter);
    if (!table.empty())
    {
      if (_request.isUserSabre())
        _formatter.addLine("CARRIER FLIGHT APPLICATION 2 (TABLE 186):");
      else
        _formatter.addLine("CARRIER FLIGHT APPLICATION 2:");

      _formatter.addLine(std::move(table));
    }
  }

  if (_passengerTypeCodeItems)
  {
    std::list<Line> table = getPassengerApplicationDisplay(*_passengerTypeCodeItems);
    if (!table.empty())
    {
      if (_request.isUserSabre())
        _formatter.addLine("PASSENGER TYPE APPLICATION (TABLE 169):");
      else
        _formatter.addLine("PASSENGER TYPE APPLICATION:");

      _formatter.addLine(std::move(table));
    }
  }

  _formatter.addLine(".");
}

void ViewX1SequenceDetail::categoryServiceBaggageTax()
{
  std::list<Line> lines;
  if (_request.isUserSabre() && _serviceFeeSecurityItems)
  {
    std::list<Line> table = getServiceFeeSecurityItemsDisplay(*_serviceFeeSecurityItems);
    if (!table.empty())
    {
      lines.emplace_back("SERVICE FEE SECURITY APPLICATION (TABLE 183):");
      lines.splice(lines.end(), std::move(table));
    }
  }

  if (_rulesRecord.serviceBaggageApplTag != type::ServiceBaggageApplTag::Blank)
  {
    lines.splice(lines.end(), getEnumStr("SERVICE/BAGGAGE APPLICATION TAG:",
                                         _rulesRecord.serviceBaggageApplTag));
  }

  if (_serviceBaggage)
  {
    std::list<Line> table = getServiceBaggageDisplay(*_serviceBaggage);
    if (!table.empty())
    {
      if (_request.isUserSabre())
        lines.emplace_back("SERVICE/BAGGAGE APPLICATION (TABLE 168):");
      else
        lines.emplace_back("SERVICE/BAGGAGE APPLICATION:");

      lines.splice(lines.end(), std::move(table));
    }
  }

  if (!lines.empty())
  {
    _formatter.addLine("10-SERVICE/BAGGAGE TAX DETAILS:");
    _formatter.addLine(std::move(lines));
    _formatter.addLine(".");
  }
}

void ViewX1SequenceDetail::categoryTravelSectorDetails()
{
  std::list<Line> lines;

  if (_rulesRecord.sectorDetailApplTag != type::SectorDetailApplTag::Blank)
    lines.splice(lines.end(), getEnumStr("SECTOR DETAIL APPLICATION TAG:",
                                         _rulesRecord.sectorDetailApplTag));

  if (_sectorDetail)
  {
    std::list<Line> table = getSectorDetailDisplay(*_sectorDetail);
    if (!table.empty())
    {
      if (_request.isUserSabre())
        lines.emplace_back("SECTOR DETAIL APPLICATION (TABLE 167):");
      else
        lines.emplace_back("SECTOR DETAIL APPLICATION:");

      lines.splice(lines.end(), std::move(table));
    }
  }

  if (!lines.empty())
  {
    _formatter.addLine("11-SECTOR DETAILS:");
    _formatter.addLine(std::move(lines));
    _formatter.addLine(".");
  }
}

void ViewX1SequenceDetail::categoryConnectionTags()
{
  if (!_rulesRecord.connectionsTags.empty())
  {
    _formatter.addLine("12-CONNECTION TAG:");
    _formatter.addLine("CONNECTIONS TAGS:");
    unsigned tagNo = 1;
    for (const type::ConnectionsTag& tag : _rulesRecord.connectionsTags)
    {
      _formatter.addLine("TAG" + std::to_string(tagNo++) + "-" + safeEnumToString(tag));
    }

    _formatter.addLine(".");
    _formatter.addLine(".");
    _formatter.addLine(".");
  }
}

void ViewX1SequenceDetail::categoryProcessingApplicationDetail()
{

}

std::list<Line> ViewX1SequenceDetail::getCarrierApplicationDisplay(
    const CarrierApplication& carrierApplication)
{
  bool permittedAllCarriers = false, exceptionOnAllCarriers = false;
  std::string permitted, exceptions;
  for (const CarrierApplicationEntry& entry : carrierApplication.entries)
  {
    if (entry.applind == type::CarrierApplicationIndicator::Positive
        && !permittedAllCarriers)
    {
      if (entry.carrier == "$$")
      {
        permittedAllCarriers = true;
        continue;
      }

      addSeparated(entry.carrier.asString(), permitted);
    }
    else if (entry.applind == type::CarrierApplicationIndicator::Negative
             && !exceptionOnAllCarriers)
    {
      if (entry.carrier == "$$")
      {
        exceptionOnAllCarriers = true;
        continue;
      }

      addSeparated(entry.carrier.asString(), exceptions);
    }
  }

  std::list<Line> ret;
  if (permittedAllCarriers)
    ret.emplace_back("PERMITTED ON: ALL CARRIERS");
  else if (permitted.size() > 0)
    ret.emplace_back("PERMITTED ON: " + permitted);

  if (exceptionOnAllCarriers)
    ret.emplace_back("EXCEPT CARRIERS: ALL CARRIERS");
  else if (exceptions.size() > 0)
    ret.emplace_back("EXCEPT CARRIERS: " + exceptions);

  if (ret.size() > 0)
  {
    ret.emplace_back(".");
    ret.emplace_back(".");
  }

  return ret;
}

std::list<Line> ViewX1SequenceDetail::getCarrierFlightDisplay(
    const CarrierFlight& carrierFlight)
{
  std::list<Line> rows;

  //          MARKETING/OPERATING FLIGHT 1-THROUGH–FLIGHT 2
  format row("%2s       /%2s        %4s             %4s");
  format number("%04d");
  for (const CarrierFlightSegment& segment : carrierFlight.segments)
  {
    row % segment.marketingCarrier.asString()
        % segment.operatingCarrier.asString();

    if (segment.flightFrom > 0)
    {
      row % (number % segment.flightFrom).str();
    }
    else
    {
      row % "";
    }

    if (segment.flightTo > 0 &&
        segment.flightTo != tax::MAX_FLIGHT_NUMBER &&
        segment.flightTo != segment.flightFrom)
    {
      row % (number % segment.flightTo).str();
    }
    else
    {
      row % "";
    }

    rows.emplace_back(row.str());
  }

  std::list<Line> ret;
  if (!rows.empty())
  {
    ret.emplace_back("FLIGHT INTO LOC1 FOR DEPARTURE/FLIGHT OUT OF LOC1 FOR ARRIVAL");
    ret.emplace_back("MARKETING/OPERATING FLIGHT 1-THROUGH-FLIGHT 2");
    ret.emplace_back("CARRIER  /CARRIER");
    ret.splice(ret.end(), std::move(rows));
    ret.emplace_back(".");
    ret.emplace_back(".");
  }

  return ret;
}

std::list<Line> ViewX1SequenceDetail::getPassengerApplicationDisplay(
    const PassengerTypeCodeItems& items)
{
  static const int8_t locationStmtMargin = 3;
  static const int8_t locationMargin = 13;

  std::list<Line> rows;

  //          PERMIT - TYPE - MIN - MAX - PTC MATCH   -    PASSENGER
  //          XXX    - XXX  - XX  - XX  - XXXXXXXXXXX -    XXXXXXXXX
  format row("%-3s    - %-3s  - %2s  - %2s  - %-11s -    %s");
  for (const PassengerTypeCodeItem& item : items)
  {
    row % (item.applTag == type::PtcApplTag::Permitted ? "YES" : "NO")
        % item.passengerType.asString()
        % (item.minimumAge > 0 ? std::to_string(item.minimumAge) : "")
        % (item.maximumAge > 0 ? std::to_string(item.maximumAge) : "")
        % safeEnumToString(item.matchIndicator)
        % (item.statusTag != type::PassengerStatusTag::Blank ?
             safeEnumToString(item.statusTag) :
             "");

    rows.emplace_back(row.str());

    if (item.location.isSet())
    {
      std::list<Line> location = getLocationShortStr(_locService, item.location);
      location.front().str().insert(0, "LOCATION: ");
      location.front().params().setLeftMargin(locationStmtMargin);
      std::for_each(location.begin(), location.end(),
                    [](Line& line)
                    { line.params().setLeftMargin(locationMargin); });

      rows.splice(rows.end(), std::move(location));
    }
  }

  std::list<Line> ret;
  if (!rows.empty())
  {
    ret.emplace_back("PERMIT - TYPE - MIN - MAX - PTC MATCH   -    PASSENGER");
    ret.emplace_back("Y/N      CODE   AGE   AGE   TYPE             STATUS");
    ret.splice(ret.end(), std::move(rows));
    ret.emplace_back(".");
    ret.emplace_back(".");
  }

  return ret;
}

std::list<Line> ViewX1SequenceDetail::getSectorDetailDisplay(
    const SectorDetail& sectorDetail)
{
  std::list<Line> rows;

  const LineParams withMargin12 = LineParams::withLeftMargin(12);
  format row("%03u  %-4s   %s");
  for (const SectorDetailEntry& entry : sectorDetail.entries)
  {
    std::vector<std::string> entriesStr;

    if (!entry.equipmentCode.empty())
      entriesStr.push_back("EQUIPMENT CODE: " + entry.equipmentCode);

    if (!entry.fareOwnerCarrier.empty())
      entriesStr.push_back("FARE OWNING CARRIER: " + entry.fareOwnerCarrier.asString());

    if (entry.cabinCode != type::CabinCode::Blank)
       entriesStr.push_back("CABIN CODE: " + safeEnumToString(entry.cabinCode));

    if (!entry.rule.empty())
       entriesStr.push_back("RULE: " + entry.rule);

    for (const type::ReservationDesignatorCode& rbd : entry.reservationCodes)
    {
      if (!rbd.empty())
        entriesStr.push_back("RBD: " + rbd);
    }

    if (!entry.tariff.asCode().empty())
      entriesStr.push_back("TARIFF TYPE:" + entry.tariff.asCode().asString());

    if (entry.tariff.asNumber() > -1)
      entriesStr.push_back((format("RULE TARIFF: %03d") % entry.tariff.asNumber()).str());

    if (!entry.fareBasisTktDesignator.empty())
       entriesStr.push_back("FARE BASIS/TICKET DESIGNATOR: " + entry.fareBasisTktDesignator);

    if (!entry.ticketCode.empty())
       entriesStr.push_back("TICKET CODE: " + entry.ticketCode);

    bool isThisFirstLine = true;
    for (std::string& entryStr : entriesStr)
    {
      if (isThisFirstLine)
      {
        row % entry.seqNo
            % safeEnumToString(entry.applTag)
            % std::move(entryStr);

        rows.emplace_back(row.str());
      }
      else
      {
        rows.emplace_back(Line(std::move(entryStr), withMargin12));
      }
    }
  }

  std::list<Line> ret;
  if (!rows.empty())
  {
    ret.emplace_back("SEQ  APPLY  FIELD: VALUE");
    ret.splice(ret.end(), std::move(rows));
    ret.emplace_back(".");
    ret.emplace_back(".");
  }

  return ret;
}

std::list<Line> ViewX1SequenceDetail::getServiceFeeSecurityItemsDisplay(
    const ServiceFeeSecurityItems& items)
{
  std::list<Line> rows;

  //          SEQ TVL CXR DUTY CODE                  CODE     VIEW/BOOK/TKT
  //          XXX  X  XX   XX  X-XXXXXXXXXXXXXXXXXXX XXXXXXXX X-XXXXXXXXXXXXX
  format row("%03u  %c  %2s   %2s  %-21s %-8s %s");
  unsigned seq = 0;
  for (const ServiceFeeSecurityItem& item : items)
  {
    row % ++seq
        % (item.travelAgencyIndicator == type::TravelAgencyIndicator::Agency ? 'X' : ' ')
        % item.carrierGdsCode
        % item.dutyFunctionCode
        % safeEnumToString(item.codeType)
        % item.code
        % safeEnumToString(item.viewBookTktInd);

    rows.emplace_back(row.str());

    if (item.location.isSet())
    {
      std::list<Line> location = getLocationShortStr(_locService, item.location, "-");
      location.emplace_front("LOCATION");
      rows.splice(rows.end(), std::move(location));
    }
  }

  std::list<Line> ret;
  if (!rows.empty())
  {
    ret.emplace_back("SEQ TVL CXR DUTY CODE                  CODE     VIEW/BOOK/TKT");
    ret.emplace_back("NO. AGT GDS FUNC TYPE");
    ret.splice(ret.end(), std::move(rows));
    ret.emplace_back(".");
    ret.emplace_back(".");
  }

  return ret;
}

std::list<Line> ViewX1SequenceDetail::getServiceBaggageDisplay(
    const ServiceBaggage& serviceBaggage)
{
  std::list<Line> rows;

  //      SEQ APPLY  FEE TAX  TAX/TYPE SERVICE TYPE        ATTRIBUTES
  //      XXX X-XXX  XX  XX   XXX      X-XXXXXXXXXXXXXXXXX XXX   XXX
  format row("%03u %-5s  %2s  %2s   %3s      %-19s %-3s   %s");
  unsigned seq = 0;
  for (const ServiceBaggageEntry& entry : serviceBaggage.entries)
  {
    row % ++seq
        % safeEnumToString(entry.applTag)
        % entry.feeOwnerCarrier.asString()
        % entry.taxCode.asString()
        % entry.taxTypeSubcode
        % (entry.optionalServiceTag != type::OptionalServiceTag::Blank ?
             safeEnumToString(entry.optionalServiceTag) :
             "")
        % entry.group
        % entry.subGroup;

    rows.emplace_back(row.str());
  }

  std::list<Line> ret;
  if (!rows.empty())
  {
    ret.emplace_back("SEQ APPLY  FEE TAX  TAX/TYPE SERVICE TYPE        ATTRIBUTES");
    ret.emplace_back("NBR Y/N    CXR CODE SUB/CODE                     GRP - SUB");
    ret.splice(ret.end(), std::move(rows));
    ret.emplace_back(".");
    ret.emplace_back(".");
  }

  return ret;
}
} // namespace display
} // namespace tax
