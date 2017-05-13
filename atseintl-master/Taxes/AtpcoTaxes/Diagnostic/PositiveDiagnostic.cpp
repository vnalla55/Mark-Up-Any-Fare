// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include <iomanip>
#include <sstream>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "Common/CalcDetails.h"
#include "Common/MoneyUtil.h"
#include "Common/OCUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/Types.h"
#include "Diagnostic/PositiveDiagnostic.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/ItinsPayments.h"
#include "ServiceInterfaces/FallbackService.h"
#include "ServiceInterfaces/Services.h"

namespace tse
{
class Trx;
}

namespace tax
{
const std::string PositiveDiagnostic::TAX_INCLUDED = "* - TAX INCLUDED\n\n";
namespace
{
std::string
stopoverAndFareBreakHead(const PaymentDetail& detail, type::Index loc)
{
  boost::format fmt("%1% / %2%");
  fmt % (detail.isStopover(loc) || detail.isUSStopover(loc) ? "STOPOVER  " : "CONNECTION");
  fmt % (detail.isFareBreak(loc) ? "FARE BREAK" : "NO FARE BREAK");
  return boost::str(fmt);
}

std::string
timeStopoverInfo(TaxPointProperties const& locDetails)
{
  std::ostringstream ans;

  if (locDetails.isTimeStopover && *locDetails.isTimeStopover)
    ans << "  TIME-BASED STOPOVER\n";

  if (locDetails.isUSTimeStopover && *locDetails.isUSTimeStopover)
    ans << "  US TIME-BASED STOPOVER\n";

  return ans.str();
}

std::string
connTagInfo(TaxPointProperties const& locDetails)
{
  std::ostringstream ans;

  if (locDetails.isExtendedStopover)
  {
    typedef type::ConnectionsTag Tag;
    if (locDetails.isExtendedStopover.hasTag(Tag::TurnaroundPointForConnection))
      ans << "  A CONN TAG: TURNAROUND POINT FOR CONNECTION\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::TurnaroundPoint))
      ans << "  B CONN TAG: TURNAROUND POINT\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::FareBreak))
      ans << "  C CONN TAG: FARE BREAK\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::FurthestFareBreak))
      ans << "  D CONN TAG: FURTHEST FARE BREAK\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::GroundTransport))
      ans << "  E CONN TAG: GROUND TRANSPORT\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::DifferentMarketingCarrier))
      ans << "  F CONN TAG: MARKETING CARRIER CHANGE\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::Multiairport))
      ans << "  G CONN TAG: MULTI-AIRPORT\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::DomesticToInternational))
      ans << "  H CONN TAG: DOMESTIC TO INTERNATIONAL\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::InternationalToDomestic))
      ans << "  I CONN TAG: INTERNATIONAL TO DOMESTIC\n";
    if (locDetails.isExtendedStopover.hasTag(Tag::InternationalToInternational))
      ans << "  J CONN TAG: INTERNATIONAL TO INTERNATIONAL\n";
  }

  return ans.str();
}

std::string
gpLoc1Details(const PaymentDetail& detail)
{
  type::Index loc1 = detail.getTaxPointLoc1().id();
  std::ostringstream ans;

  ans << boost::format("TXPT LOC1: %1% - %2%\n") % detail.getTaxPointLoc1().locCode() %
             stopoverAndFareBreakHead(detail, loc1);

  TaxPointProperties const& locDetails = detail.taxPointsProperties()[loc1];
  ans << timeStopoverInfo(locDetails);

  ans << "  STOPOVER TAG: ";
  if (detail.loc1StopoverTag() == type::StopoverTag::Connection)
    ans << "CONNECTION";
  else if (detail.loc1StopoverTag() == type::StopoverTag::Stopover)
    ans << "STOPOVER";
  else if (detail.loc1StopoverTag() == type::StopoverTag::FareBreak)
    ans << "FARE BREAK";
  else if (detail.loc1StopoverTag() == type::StopoverTag::NotFareBreak)
    ans << "NO FARE BREAK";
  else
    ans << "NONE";
  ans << "\n";

  ans << connTagInfo(locDetails);
  return ans.str();
}

std::string
gpLoc2Details(const PaymentDetail& detail)
{
  type::Index loc2 = detail.getTaxPointLoc2().id();
  std::ostringstream ans;

  ans << boost::format("TXPT LOC2: %1% - %2%\n") % detail.getTaxPointLoc2().locCode() %
             stopoverAndFareBreakHead(detail, loc2);

  TaxPointProperties const& locDetails = detail.taxPointsProperties()[loc2];
  ans << timeStopoverInfo(locDetails);

  ans << "  STOPOVER TAG: ";
  if (detail.loc2StopoverTag() == type::Loc2StopoverTag::Stopover)
    ans << "STOPOVER";
  else if (detail.loc2StopoverTag() == type::Loc2StopoverTag::FareBreak)
    ans << "FARE BREAK";
  else if (detail.loc2StopoverTag() == type::Loc2StopoverTag::Furthest)
    ans << "FURTHEST FARE BREAK";
  else
    ans << "NONE";
  ans << "\n";

  ans << connTagInfo(locDetails);
  return ans.str();
}

std::string
gpLoc3Details(const PaymentDetail& detail)
{
  if (!detail.hasLoc3())
    return "TXPT LOC3: \n";

  type::Index loc3 = detail.getLoc3().id();
  std::ostringstream ans;

  ans << boost::format("TXPT LOC3: %1% - %2%\n") % detail.getLoc3().locCode() %
             stopoverAndFareBreakHead(detail, loc3);

  TaxPointProperties const& locDetails = detail.taxPointsProperties()[loc3];
  ans << timeStopoverInfo(locDetails);
  ans << connTagInfo(locDetails);
  return ans.str();
}

std::string
gpInfo(const PaymentDetail& detail)
{
  std::ostringstream ans;

  ans << "JRNY LOC1: " << (detail.hasJourneyLoc1() ? detail.getJourneyLoc1().locCode().asString() : "")
      << "\n";
  ans << "JRNY LOC2: " << (detail.hasJourneyLoc2() ? detail.getJourneyLoc2().locCode().asString() : "")
      << "\n";
  ans << gpLoc1Details(detail);
  ans << gpLoc2Details(detail);
  ans << gpLoc3Details(detail);
  ;
  ans << "RT OR OJ: " << detail.roundTripOrOpenJaw()
      << (detail.specUS_RTOJLogic() ? " APP TAG 01 - SPEC PROC" : "") << "\n";
  ans << "HIDDEN: " << detail.unticketedTransfer() << "\n";

  return ans.str();
}

std::string
roundingInfo(const PaymentDetail& detail)
{
  std::ostringstream ans;

  if (detail.calcDetails().taxBeforeRounding > 0)
  {
    ans << "UNROUNDED TAX: " << amountToDouble(detail.calcDetails().taxBeforeRounding) << "\n";
  }
  if (detail.calcDetails().roundingDir != type::TaxRoundingDir::Blank &&
      detail.calcDetails().roundingUnit != 0)
  {
    ans << "ROUNDING: ";
    switch (detail.calcDetails().roundingDir)
    {
    case type::TaxRoundingDir::RoundDown:
      ans << "DOWN TO " << amountToDouble(detail.calcDetails().roundingUnit) << "\n";
      break;
    case type::TaxRoundingDir::RoundUp:
      ans << "UP TO " << amountToDouble(detail.calcDetails().roundingUnit) << "\n";
      break;
    case type::TaxRoundingDir::Nearest:
      ans << "NEAREST " << amountToDouble(detail.calcDetails().roundingUnit) << "\n";
      break;
    case type::TaxRoundingDir::NoRounding:
      ans << "NO ROUNDING\n";
      break;
    default:
      ans << "EMPTY\n";
    }
  }

  return ans.str();
}

std::string
exchangeInfo(const PaymentDetail& detail)
{
  std::ostringstream ans;

  if (detail.taxName().percentFlatTag() == type::PercentFlatTag::Flat &&
      detail.taxCurrency() != detail.taxEquivalentCurrency())
  {
    ans << "EXCHANGERATE: " << amountToDouble(detail.calcDetails().exchangeRate1) << "\n";
    if (!detail.calcDetails().intermediateCurrency.empty())
    {
      ans << "INTERMEDIATE: " << detail.calcDetails().intermediateAmount << detail.calcDetails().intermediateCurrency.asString() << "\n";
      ans << "EXCHANGERATE: " << amountToDouble(detail.calcDetails().exchangeRate2) << "\n";
    }
  }

  return ans.str();
}

std::string
dcInfo(const TaxName& taxName,
       const PaymentDetail& detail)
{
  std::ostringstream ans;

  if (taxName.percentFlatTag() == type::PercentFlatTag::Flat)
  {
    ans << "FIXED TAX: " << amountToDouble(detail.taxAmt()) << " " << detail.taxCurrency() << "\n";
  }
  else
  {
    ans << "PERCENT TAX: " << 100 * amountToDouble(detail.taxAmt()) << "\n";
    ans << "FARE AMOUNT: "
        << amountToDouble(detail.getTotalFareAmount() ? *detail.getTotalFareAmount() : 0) << "\n";
  }

  if (!detail.taxOnTaxItems().empty())
  {
    ans << "TOTAL TAXABLE TAXES AMOUNT: " << amountToDouble(detail.totalTaxOnTaxAmount()) << "\n";
    ans << "TAXONTAX ITEMS: " << detail.taxOnTaxItems().size() << "\n";
    ans << "TAXONTAX INFO DETAILS: \n";
    type::Index index = 1;
    for (const PaymentDetail* _paymentDetail : detail.taxOnTaxItems())
    {
      ans << " " << index << ". CODE = " << _paymentDetail->taxName().taxCode()
          << ", TYPE = " << _paymentDetail->taxName().taxType()
          << ", AMT = " << (_paymentDetail->isCommandExempt()
                                ? 0.0
                                : amountToDouble(_paymentDetail->taxEquivalentAmount()))
          << ", BEGIN = " << _paymentDetail->getTaxPointBegin().id()
          << ", END = " << _paymentDetail->getTaxPointEnd().id() << "\n";
      index++;
    }
  }

  type::MoneyAmount taxableYqYrsAmount;
  const TaxableYqYrs& taxableYqYrs = detail.getYqYrDetails();
  const std::vector<TaxableYqYr>& yqYrItems = taxableYqYrs._subject;
  if (!taxableYqYrs.areAllFailed())
  {
    ans << "TOTAL TAXABLE YQYR AMOUNT: " << amountToDouble(taxableYqYrs._taxableAmount) << "\n";
    ans << "TAXONYQYR ITEMS: " << taxableYqYrs.getValidCount() << "\n";
    ans << "TAXONYQYR INFO DETAILS: \n";
    for (type::Index index = 1, i = 0; i < yqYrItems.size(); ++i)
    {
      if (!taxableYqYrs.isFailedRule(i))
      {
        type::Index begin = std::min(taxableYqYrs._ranges[i].first, taxableYqYrs._ranges[i].second);
        type::Index end = std::max(taxableYqYrs._ranges[i].first, taxableYqYrs._ranges[i].second);
        const TaxableYqYr& yqYr = yqYrItems[i];
        ans << " " << index << ". CODE = " << yqYr._code.asString() << ", TYPE = " << yqYr._type
            << ", AMT = " << amountToDouble(yqYr._amount)
            << ", TAXINCLUDED = " << yqYr._taxIncluded
            << ", BEGIN = " << begin
            << ", END = " << end << "\n";
        ++index;
      }
    }
    taxableYqYrsAmount = taxableYqYrs._taxableAmount;
  }

  type::MoneyAmount totalTaxableAmount =
      (detail.getTotalFareAmount() ? *detail.getTotalFareAmount() : 0) +
      detail.totalTaxOnTaxAmount() + taxableYqYrsAmount;
  ans << "TOTAL TAXABLE AMOUNT: " << amountToDouble(totalTaxableAmount) << "\n";

  ans << exchangeInfo(detail);
  ans << roundingInfo(detail);
  ans << "FINAL TAXAMOUNT: " << amountToDouble(detail.taxEquivalentAmount()) << " "
      << detail.taxEquivalentCurrency() << "\n";

  return ans.str();
}

std::string
taxId(const TaxName& taxName, const PaymentDetail& detail, std::string prefix = "")
{
  std::ostringstream ans;
  ans << prefix << "NATION: " << taxName.nation() << "\n";
  ans << prefix << "TXCODE: " << taxName.taxCode() << "\n";
  ans << prefix << "TXTYPE: " << taxName.taxType() << "\n";
  ans << prefix << "SEQNO: " << detail.seqNo() << "\n";
  ans << prefix << "TXPTAG: " << taxName.taxPointTag() << "\n";

  return ans.str();
}

std::string
txGeneralInfo(const TaxName& taxName, const PaymentDetail& detail)
{
  std::ostringstream ans;
  ans << taxId(taxName, detail);
  ans << "BOARD: " << detail.getTaxPointBegin().locCode() << "\n";
  ans << "OFF: " << detail.getTaxPointEnd().locCode() << "\n";

  return ans.str();
}

std::string
taxAmount(const TaxName& taxName,
          const PaymentDetail& detail,
          const OptionalService& oc,
          std::string prefix = "")
{
  std::ostringstream ans;

  if (taxName.percentFlatTag() == type::PercentFlatTag::Flat)
  {
    ans << prefix << "FIXED TAX: " << amountToDouble(oc.taxAmount()) << " " << detail.taxCurrency()
        << "\n";
  }
  else
  {
    ans << prefix << "PERCENT TAX: " << 100 * amountToDouble(detail.taxAmt()) << "\n";
    ans << prefix << "BASE AMOUNT: " << amountToDouble(oc.amount()) << "\n";
  }
  return ans.str();
}

boost::format&
formatter()
{
  static boost::format format(
      "%|2|%|=6| %|=4| %|=4| %|=6| %|=3|%|7.2f|%|9.2f|%|1|%|=5| %|=3|%|7|\n");
  return format;
}

} // anonymous namespace

const uint32_t PositiveDiagnostic::NUMBER = 832;

PositiveDiagnostic::PositiveDiagnostic(const ItinsPayments& itinsPayments,
                                       const boost::ptr_vector<Parameter>& parameters)
  : _itinsPayments(itinsPayments),
    _parameters(parameters),
    _filter(),
    _columnNames()
{
  _columnNames = str(formatter() % "" % "NATION" % "CODE" % "TYPE" % "TXPTAG" % "P/F" % "TXAMT" %
                     "TOTAL" % " " % "BOARD" % "OFF" % "SEQNO");
}

PositiveDiagnostic::~PositiveDiagnostic(void)
{
}

void
PositiveDiagnostic::runAll()
{
  _helpPrinter.print(*this);
  _validTaxesPrinter.print(*this);
  _taxPrinter.print(*this);
  _geoPropertiesPrinter.print(*this);
  _filterPrinter.print(*this);
}

void
PositiveDiagnostic::applyParameters()
{
  bool diagEnabled = false;
  bool isTxEnabled = false;

  for (const Parameter& parameter : _parameters)
  {
    if (parameter.name() == "TX")
    {
      isTxEnabled = true;
      break;
    }
  }

  for (const Parameter& parameter : _parameters)
  {
    if (parameter.name() == "HE" || parameter.name() == "HELP")
    {
      _helpPrinter.enable();
      diagEnabled = true;
    }
    else if (parameter.name() == "DC")
    {
      if (!isTxEnabled)
      {
        _taxPrinter.enable();
        diagEnabled = true;
      }
      _filter.isDCPrinter = true;
    }
    else if (parameter.name() == "GP")
    {
      if (!isTxEnabled)
      {
        _geoPropertiesPrinter.enable();
        diagEnabled = true;
      }
      _filter.isGPPrinter = true;
    }
    else if (parameter.name() == "TX")
    {
      _filterPrinter.enable();
      _filter.itemNumber = type::Index();
      try
      {
        _filter.itemNumber = boost::lexical_cast<type::Index>(parameter.value());
      }
      catch (boost::bad_lexical_cast&)
      {
      }
    }
    else if (parameter.name() == "PA")
    {
      type::PassengerCode paxCode (UninitializedCode);
      codeFromString(parameter.value(), paxCode);
      _filter.passengerCode = paxCode;
    }
  }

  if (!diagEnabled)
  {
    _validTaxesPrinter.enable();
  }
}

void
PositiveDiagnostic::printTaxes()
{
  printHeaderLong("SEQS DETAILED ANALYSIS");
  _result << "\n";

  for (const ItinPayments& itinPayments: _itinsPayments._itinPayments)
  {
    if (!_filter.isValidPassenger(itinPayments.requestedPassengerCode()))
    {
      continue;
    }

    printHeaderLong("ITINERARY " + std::to_string(itinPayments.itinId() + 1));

    printTaxesGroup(itinPayments, type::ProcessingGroup::Itinerary);
    printTaxesGroupOCType(itinPayments, type::ProcessingGroup::OC);
    printTaxesGroupOCType(itinPayments, type::ProcessingGroup::Baggage);
    printTaxesGroup(itinPayments, type::ProcessingGroup::OB);
    printTaxesGroup(itinPayments, type::ProcessingGroup::ChangeFee);
  }
}

void
PositiveDiagnostic::printTaxesGroup(const ItinPayments& itinPayments,
                                    const type::ProcessingGroup& processingGroup)
{
  for (const Payment& payment : itinPayments.payments(processingGroup))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      if (detail->isValidForItinerary() || !detail->getYqYrDetails().areAllFailed())
      {
        printTaxHeader(taxName, detail->seqNo(), itinPayments.requestedPassengerCode());

        _result << dcInfo(taxName, *detail);
        _result << "\n";
      }

      if (detail->taxOnChangeFeeAmount())
      {
        printTaxHeader(taxName, detail->seqNo(), itinPayments.requestedPassengerCode());

        _result << "CHANGEFEE: " << amountToDouble(detail->changeFeeAmount()) << "\n"
                << "TAXONCHANGEFEE: " << amountToDouble(detail->taxOnChangeFeeAmount()) << "\n";
      }
    }
  }
}

void
PositiveDiagnostic::printTaxesGroupOCType(const ItinPayments& itinPayments,
                                          const type::ProcessingGroup& processingGroup)
{
  for (const Payment& payment : itinPayments.payments(processingGroup))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      for (OptionalService const& oc : detail->optionalServiceItems())
      {
        if (oc.isFailed())
          continue;

        printTaxHeader(taxName, detail->seqNo(), itinPayments.requestedPassengerCode());
        printOCHeader(oc);

        _result << taxAmount(taxName, *detail, oc);

        _result << exchangeInfo(*detail);
        _result << roundingInfo(*detail);
        _result << "FINAL TAXAMOUNT: " << amountToDouble(oc.getTaxEquivalentAmount()) << " "
                << detail->taxEquivalentCurrency() << "\n";

        _result << "\n";
      }
    }
  }
}

void
PositiveDiagnostic::printValidTaxes()
{
  printHeaderLong("SEQS ANALYSIS");
  _result << "\n";

  for (const ItinPayments& itinPayments : _itinsPayments._itinPayments)
  {
    if (!_filter.isValidPassenger(itinPayments.requestedPassengerCode()))
    {
      continue;
    }

    uint32_t number = 0;

    printHeaderLong("ITINERARY " + std::to_string(itinPayments.itinId() + 1));

    printValidTaxesGroup(itinPayments, type::ProcessingGroup::Itinerary, "TAXES ON ITIN", number);
    printValidTaxesGroup(
        itinPayments, type::ProcessingGroup::ChangeFee, "TAXES ON CHANGEFEE - ITIN", number);

    printValidTaxesGroupOC(itinPayments, "TAXES ON OC - ITIN", type::ProcessingGroup::OC, number);
    printValidTaxesGroupOC(
        itinPayments, "TAXES ON BAGGAGE - ITIN", type::ProcessingGroup::Baggage, number);

    _result << "\n";
  }
}

void
PositiveDiagnostic::printValidTaxesGroup(const ItinPayments& itinPayments,
                                         const type::ProcessingGroup& processingGroup,
                                         const std::string& header,
                                         uint32_t& number)
{
  bool headerPrinted = false;

  for (const Payment& payment : itinPayments.payments(processingGroup))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      if ((processingGroup == type::ProcessingGroup::Itinerary) &&
          !detail->isValidForGroup(type::ProcessingGroup::Itinerary))
      {
        continue;
      }

      if ((processingGroup == type::ProcessingGroup::ChangeFee)
          && !detail->isCalculated())
      {
        continue;
      }

      number++;

      if (!headerPrinted)
      {
        headerPrinted = true;
        printValidTaxHeader(itinPayments, header);

        _result << _columnNames << "\n";
      }

      type::MoneyAmount total;
      if (processingGroup == type::ProcessingGroup::Itinerary)
      {
        total = detail->taxEquivalentAmount();
      }
      else if (processingGroup == type::ProcessingGroup::ChangeFee)
      {
        total = detail->taxOnChangeFeeAmount();
      }

      printTaxLine(number,
                   taxName,
                   *detail,
                   detail->getTaxPointBegin().locCode(),
                   detail->getTaxPointEnd().locCode(),
                   total,
                   ' ');
    }
  }
}

void
PositiveDiagnostic::printValidTaxesGroupOC(const ItinPayments& itinPayments,
                                           const std::string& header,
                                           const type::ProcessingGroup& processingGroup,
                                           uint32_t& number)
{
  bool headerPrinted = false;
  bool printFooter = false;

  for (const Payment& payment : itinPayments.payments(processingGroup))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      for (const OptionalService& oc : detail->optionalServiceItems())
      {
        if (oc.isFailed())
          continue;

        number++;

        if (!headerPrinted)
        {
          headerPrinted = true;
          printValidTaxHeader(itinPayments, header);

          _result << _columnNames << "\n";
        }

        if (oc.taxInclInd())
          printFooter = true;

        char isTaxInclInd = oc.taxInclInd() ? '*' : ' ';

        printTaxLine(number,
                     taxName,
                     *detail,
                     oc.getTaxPointBegin().locCode(),
                     oc.getTaxPointEnd().locCode(),
                     oc.getTaxEquivalentAmount(),
                     isTaxInclInd);
      }
    }
  }

  _result << (printFooter ? TAX_INCLUDED : "\n");
}

void
PositiveDiagnostic::printFilterTax()
{
  printHeaderLong("TX ANALYSIS");
  _result << "\n";

  for (const ItinPayments& itinPayments : _itinsPayments._itinPayments)
  {
    if (!_filter.isValidPassenger(itinPayments.requestedPassengerCode()))
    {
      continue;
    }

    uint32_t number = 0;

    printHeaderLong("ITINERARY " + std::to_string(itinPayments.itinId() + 1));

    printFilterTaxGroup(itinPayments, type::ProcessingGroup::Itinerary, "TAXES ON ITIN", number);
    printFilterTaxGroup(
        itinPayments, type::ProcessingGroup::ChangeFee, "TAXES ON CHANGEFEE - ITIN", number);

    if (_filter.itemNumber > number)
    {
      printFilterOS(itinPayments, "TAXES ON OC - ITIN", type::ProcessingGroup::OC, number);
      printFilterOS(
          itinPayments, "TAXES ON BAGGAGE - ITIN", type::ProcessingGroup::Baggage, number);
    }

    _result << "\n";
  }
}

void
PositiveDiagnostic::printFilterTaxGroup(const ItinPayments& itinPayments,
                                        const type::ProcessingGroup& processingGroup,
                                        const std::string& header,
                                        uint32_t& number)
{
  bool headerPrinted = false;

  for (const Payment& payment : itinPayments.payments(processingGroup))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      if ((processingGroup == type::ProcessingGroup::Itinerary) && !detail->isValidForItinerary())
        continue;

      number++;
      if (number != _filter.itemNumber)
      {
        continue;
      }

      if (!headerPrinted)
      {
        headerPrinted = true;
        printValidTaxHeader(itinPayments, header);

        _result << "\n";
      }

      _result << txGeneralInfo(taxName, *detail);

      if (_filter.isDCPrinter || _filter.noPrinterEnabled())
        _result << "\n" << dcInfo(taxName, *detail);

      if (_filter.isGPPrinter || _filter.noPrinterEnabled())
        _result << "\n" << gpInfo(*detail);

      return;
    }
  }
}

void
PositiveDiagnostic::printFilterOS(const ItinPayments& itinPayments,
                                  const std::string& header,
                                  const type::ProcessingGroup& processingGroup,
                                  uint32_t& number)
{
  bool headerPrinted = false;

  for (const Payment& payment : itinPayments.payments(processingGroup))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      for (const OptionalService& oc : detail->optionalServiceItems())
      {
        if (oc.isFailed())
          continue;

        number++;
        if (number != _filter.itemNumber)
        {
          continue;
        }

        if (!headerPrinted)
        {
          headerPrinted = true;
          printValidTaxHeader(itinPayments, header);

          _result << "\n";
        }

        _result << taxId(taxName, *detail, " ");
        _result << taxAmount(taxName, *detail, oc, " ");
        _result << " BOARD: " << oc.getTaxPointBegin().locCode() << "\n";
        _result << " OFF: " << oc.getTaxPointEnd().locCode() << "\n";

        _result << " SERVICESUBTYPECODE: " << oc.subCode() << "\n";
        _result << " SVCGROUP: " << oc.serviceGroup() << "\n";
        _result << " SVCSUBGROUP: " << oc.serviceSubGroup() << "\n";
        _result << " CARRIER: " << oc.ownerCarrier() << "\n";
        _result << " OCTYPE: " << OCUtil::getOCTypeString(oc.type()) << "\n";
        return;
      }
    }
  }

  _result << "\n";
}

void
PositiveDiagnostic::printGeoProperties()
{
  printHeaderLong("GEO PROPERTIES");
  _result << "\n";

  for (const ItinPayments& itinPayments : _itinsPayments._itinPayments)
  {
    if (!_filter.isValidPassenger(itinPayments.requestedPassengerCode()))
    {
      continue;
    }

    printGeoPropertiesGroup(itinPayments, type::ProcessingGroup::Itinerary);
    printGeoPropertiesGroupOCType(itinPayments, type::ProcessingGroup::OC);
    printGeoPropertiesGroupOCType(itinPayments, type::ProcessingGroup::Baggage);
    printGeoPropertiesGroup(itinPayments, type::ProcessingGroup::OB);
    printGeoPropertiesGroup(itinPayments, type::ProcessingGroup::ChangeFee);
  }
}

void
PositiveDiagnostic::printGeoPropertiesGroup(const ItinPayments& itinPayments,
                                            const type::ProcessingGroup& group)
{
  for (const Payment& payment : itinPayments.payments(group))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      if (detail->isValidForItinerary())
      {
        printTaxHeader(taxName, detail->seqNo(), itinPayments.requestedPassengerCode());

        _result << gpInfo(*detail);
        _result << "\n";
      }
    }
  }
}

void
PositiveDiagnostic::printGeoPropertiesGroupOCType(const ItinPayments& itinPayments,
                                                  const type::ProcessingGroup& group)
{
  for (const Payment& payment : itinPayments.payments(group))
  {
    const TaxName& taxName = payment.taxName();
    for (const PaymentDetail* detail : payment.paymentDetail())
    {
      for (const OptionalService& oc : detail->optionalServiceItems())
      {
        if (oc.isFailed())
          continue;

        printTaxHeader(taxName, detail->seqNo(), itinPayments.requestedPassengerCode());
        printOCHeader(oc);

        _result << "SERVICESUBTYPECODE: " << oc.subCode() << "\n";
        _result << "SVCGROUP: " << oc.serviceGroup() << "\n";
        _result << "SVCSUBGROUP: " << oc.serviceSubGroup() << "\n";
        _result << "CARRIER: " << oc.ownerCarrier() << "\n";
        _result << "OC BEGIN: " << oc.getTaxPointBegin().locCode() << "\n";
        _result << "OC END: " << oc.getTaxPointEnd().locCode() << "\n";
        _result << "\n";
      }
    }
  }
}

void
PositiveDiagnostic::printTaxLine(uint32_t number,
                                 const TaxName& taxName,
                                 const PaymentDetail& detail,
                                 const type::AirportCode& taxPointBegin,
                                 const type::AirportCode& taxPointEnd,
                                 const type::MoneyAmount& total,
                                 char isTaxInclInd)
{
  const double taxAmt = (taxName.percentFlatTag() == type::PercentFlatTag::Flat
                             ? amountToDouble(detail.taxAmt())
                             : 100 * amountToDouble(detail.taxAmt()));

  _result << formatter() % number % taxName.nation() % taxName.taxCode() % taxName.taxType() %
                 taxName.taxPointTag() % taxName.percentFlatTag() % taxAmt %
                 amountToDouble(total) % isTaxInclInd % taxPointBegin % taxPointEnd %
                 detail.seqNo();
}

void
PositiveDiagnostic::printValidTaxHeader(const ItinPayments& itinPayments,
                                        const std::string& header)
{
  std::ostringstream headerAndPsgr;
  headerAndPsgr << header << " "
                << (itinPayments.itinId() + 1)
                << " PSGR "
                << itinPayments.requestedPassengerCode();

  printHeaderShort(headerAndPsgr.str());
  printHeaderShort("VALIDATING CARRIER: " + itinPayments.validatingCarrier().asString());
}

void
PositiveDiagnostic::printTaxHeader(const TaxName& taxName,
                                   const type::SeqNo& seqNo,
                                   const type::PassengerCode& passengerCode)
{
  std::ostringstream s;
  s << " TAX " << taxName.nation() << " " << taxName.taxCode() << " " << taxName.taxType()
    << " SEQ " << seqNo << " - PSGR " << passengerCode << " ";

  printHeaderShort(s.str());
}

void
PositiveDiagnostic::printOCHeader(const OptionalService& oc)
{
  std::ostringstream s;
  s << " FOR OC --- " << oc.subCode() << " " << oc.serviceGroup() << " " << oc.serviceSubGroup()
    << " " << oc.ownerCarrier() << " (" << OCUtil::getOCTypeString(oc.type()) << ") ";

  printHeaderShort(s.str());
}

void
PositiveDiagnostic::printHelp()
{
  printHeaderLong("HELP");

  _result << "NO PARAM - SEQS ANALYSIS\n"
          << "DC - DETAILED CALCULATIONS\n"
          << "GP - GEO PROPERTIES\n"
          << "HELP - HELP INFO\n"
          << "TXAA - ALL AVAILABLE INFORMATION OF TAXITEM NUMBER AA\n"
          << "TXAA/DC - FOR DETAILED CALCULATION OF TAXITEM NUMBER AA\n"
          << "TXAA/GP - FOR GEO PROPERTIES OF TAXITEM NUMBER AA\n";

  printInputFilterHelp();
}

void
PositiveDiagnostic::printHeader()
{
  printLine('*');
  printHeaderShort("DIAGNOSTIC " + boost::lexical_cast<std::string>(NUMBER) + " - TAX APPLICATION");
  printLine('*');
}

} // namespace tax
