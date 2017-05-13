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
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>

#include "Common/DiagnosticUtil.h"
#include "Common/OCUtil.h"
#include "Common/SafeEnumToString.h"
#include "DataModel/Common/CodeIO.h"
#include "Diagnostic/NegativeDiagnostic.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/ItinsPayments.h"
#include "ServiceInterfaces/Services.h"

namespace tse
{
class Trx;
}

namespace tax
{
namespace
{
std::string
widen(std::string input, size_t width)
{
  if (input.size() == width)
    return input;

  if (input.size() > width)
    return input.substr(0, width);

  input += std::string(width - input.size(), ' ');
  return input;
}

std::string
mergeColumns(std::string leftText, std::string rightText)
{
  typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;
  boost::char_separator<char> sep("\n");

  std::ostringstream ans;
  Tokenizer lTokens(leftText, sep);
  Tokenizer rTokens(rightText, sep);
  Tokenizer::iterator lIt = lTokens.begin(), rIt = rTokens.begin();
  while (lIt != lTokens.end() || rIt != rTokens.end())
  {
    if (lIt != lTokens.end())
    {
      if (rIt != rTokens.end())
        ans << widen(*lIt++, 32);
      else
        ans << *lIt++;
    }
    else
      ans << std::string(' ', 32);

    if (rIt != rTokens.end())
      ans << *rIt++;

    ans << '\n';
  }

  return ans.str();
}

std::string
indexLineData(const PaymentDetail& detail, type::Index id)
{
  std::ostringstream ans;
  ans << id << " - ";
  ans << (detail.isStopover(id) || detail.isUSStopover(id) ? "STOP" : "CONN") << '/';
  ans << (detail.isFareBreak(id) ? "FB" : "NFB");
  return ans.str();
}

std::string
timeStopoverInfo(TaxPointProperties const& locDetails)
{
  std::string ans;
  if (locDetails.isTimeStopover && *locDetails.isTimeStopover)
    ans += "  TIME-BASED STOP\n";
  if (locDetails.isUSTimeStopover && *locDetails.isUSTimeStopover)
    ans += "  US TIME-BASED STOP\n";
  return ans;
}

std::string
connTagInfo(TaxPointProperties const& locDetails)
{
  std::string ans;

  if (locDetails.isExtendedStopover)
  {
    typedef type::ConnectionsTag Tag;
    ans += "  CONN TAGS:";
    if (locDetails.isExtendedStopover.hasTag(Tag::TurnaroundPointForConnection))
      ans += " A";
    if (locDetails.isExtendedStopover.hasTag(Tag::TurnaroundPoint))
      ans += " B";
    if (locDetails.isExtendedStopover.hasTag(Tag::FareBreak))
      ans += " C";
    if (locDetails.isExtendedStopover.hasTag(Tag::FurthestFareBreak))
      ans += " D";
    if (locDetails.isExtendedStopover.hasTag(Tag::GroundTransport))
      ans += " E";
    if (locDetails.isExtendedStopover.hasTag(Tag::DifferentMarketingCarrier))
      ans += " F";
    if (locDetails.isExtendedStopover.hasTag(Tag::Multiairport))
      ans += " G";
    if (locDetails.isExtendedStopover.hasTag(Tag::DomesticToInternational))
      ans += " H";
    if (locDetails.isExtendedStopover.hasTag(Tag::InternationalToDomestic))
      ans += " I";
    if (locDetails.isExtendedStopover.hasTag(Tag::InternationalToInternational))
      ans += " J";
    ans += '\n';
  }

  return ans;
}

std::string
stopoverTag(type::StopoverTag tag)
{
  std::string ans = "  STOPOVER TAG: ";

  if (tag == type::StopoverTag::Connection)
    ans += "CONNECTION";
  else if (tag == type::StopoverTag::Stopover)
    ans += "STOPOVER";
  else if (tag == type::StopoverTag::FareBreak)
    ans += "FARE BREAK";
  else if (tag == type::StopoverTag::NotFareBreak)
    ans += "NO FARE BREAK";
  else
    ans += "NONE";
  ans += "\n";

  return ans;
}

std::string
stopoverTag(type::Loc2StopoverTag tag)
{
  std::string ans = "  STOPOVER TAG: ";

  if (tag == type::Loc2StopoverTag::Stopover)
    ans += "STOPOVER";
  else if (tag == type::Loc2StopoverTag::FareBreak)
    ans += "FARE BREAK";
  else if (tag == type::Loc2StopoverTag::Furthest)
    ans += "FURTHEST FARE BREAK";
  else
    ans += "NONE";
  ans += "\n";
  return ans;
}

std::string
geoInfo(const PaymentDetail& detail, bool showDetails)
{
  Geo const& board = detail.getTaxPointBegin(), off = detail.getTaxPointEnd();
  std::ostringstream left, right;
  left << "BOARD: " << board.locCode() << '\n';
  left << "BOARD INDEX: " << indexLineData(detail, board.id()) << '\n';

  right << "OFF: " << off.locCode() << '\n';
  right << "OFF INDEX: " << indexLineData(detail, off.id()) << '\n';

  if (showDetails)
  {
    TaxPointProperties const& boardDetails = detail.taxPointsProperties()[board.id()];
    left << timeStopoverInfo(boardDetails);
    left << stopoverTag(detail.loc1StopoverTag());
    left << connTagInfo(boardDetails);

    TaxPointProperties const& offDetails = detail.taxPointsProperties()[off.id()];
    right << timeStopoverInfo(offDetails);
    right << stopoverTag(detail.loc2StopoverTag());
    right << connTagInfo(offDetails);
  }

  return mergeColumns(left.str(), right.str());
}

} // anonymous namespace

const uint32_t NegativeDiagnostic::NUMBER = 833;

NegativeDiagnostic::NegativeDiagnostic(const ItinsPayments& itinsPayments,
                                       const boost::ptr_vector<Parameter>& parameters,
                                       Services& services)
  : _services(services), _itinsPayments(itinsPayments), _parameters(parameters), _filter()
{
}

NegativeDiagnostic::~NegativeDiagnostic(void)
{
}

void
NegativeDiagnostic::runAll()
{
  _helpPrinter.print(*this);
  invalidTaxesPrinter().print(*this);
}

void
NegativeDiagnostic::applyParameters()
{
  if (_parameters.empty())
  {
    invalidTaxesPrinter().enable();
  }

  for (const Parameter& parameter : _parameters)
  {
    if (parameter.name() == "HE" || parameter.name() == "HELP")
    {
      _helpPrinter.enable();
    }
    else if (parameter.name() == "NA")
    {
      invalidTaxesPrinter().enable();
      type::Nation nation(UninitializedCode);
      codeFromString(parameter.value(), nation);
      _filter.nation = nation;
    }
    else if (parameter.name() == "TC")
    {
      invalidTaxesPrinter().enable();
      type::TaxCode taxCode(UninitializedCode);
      codeFromString(parameter.value(), taxCode);
      _filter.taxCode = taxCode;
    }
    else if (parameter.name() == "TT")
    {
      invalidTaxesPrinter().enable();
      type::TaxType taxType(UninitializedCode);
      codeFromString(parameter.value(), taxType);
      _filter.taxType = taxType;
    }
    else if (parameter.name() == "GP")
    {
      invalidTaxesPrinter().enable();
      _filter.showGP = true;
    }
    else if (parameter.name() == "SQ")
    {
      invalidTaxesPrinter().enable();
      DiagnosticUtil::splitSeqValues(
          parameter.value(), _filter.seq, _filter.seqLimit, _filter.isSeqRange);
    }
    else if (parameter.name() == "SC")
    {
      invalidTaxesPrinter().enable();
      type::OcSubCode subCode(UninitializedCode);
      codeFromString(parameter.value(), subCode);
      _filter.subCode = subCode;
    }
    else if (parameter.name() == "GR")
    {
      invalidTaxesPrinter().enable();
      _filter.group = parameter.value();
    }
    else if (parameter.name() == "SG")
    {
      invalidTaxesPrinter().enable();
      _filter.subGroup = parameter.value();
    }
    else if (parameter.name() == "CR")
    {
      invalidTaxesPrinter().enable();
      type::CarrierCode carrierCode(UninitializedCode);
      codeFromString(parameter.value(), carrierCode);
      _filter.carrier = carrierCode;
    }
    else if (parameter.name() == "OT")
    {
      invalidTaxesPrinter().enable();
      if (!parameter.value().empty())
      {
        switch (parameter.value()[0])
        {
        case 'P':
          _filter.type = type::OptionalServiceTag::PrePaid;
          break;

        case 'F':
          _filter.type = type::OptionalServiceTag::FlightRelated;
          break;

        case 'T':
          _filter.type = type::OptionalServiceTag::TicketRelated;
          break;

        case 'M':
          _filter.type = type::OptionalServiceTag::Merchandise;
          break;

        case 'R':
          _filter.type = type::OptionalServiceTag::FareRelated;
          break;

        case 'C':
          _filter.type = type::OptionalServiceTag::BaggageCharge;
          break;

        default:
          _filter.type = type::OptionalServiceTag::Blank;
        }
      }
    }
    else // other param (input filters for diagnostic)
    {
      invalidTaxesPrinter().enable();
    }
  }
}

bool
NegativeDiagnostic::filter(const TaxName& taxName, type::SeqNo seqNo) const
{
  bool nationMatched = _filter.nation.empty() || (taxName.nation() == _filter.nation);
  bool taxCodeMatched = _filter.taxCode.empty() || (taxName.taxCode() == _filter.taxCode);
  bool taxTypeMatched = _filter.taxType.empty() || (taxName.taxType() == _filter.taxType);

  bool isSeqMatched = DiagnosticUtil::isSeqNoMatching(
      seqNo, _filter.seq, _filter.seqLimit, _filter.isSeqRange);

  return nationMatched && taxCodeMatched && taxTypeMatched && isSeqMatched;
}

void
NegativeDiagnostic::printInvalidTaxes()
{
  printHeaderLong("FAILED SEQUENCES");

  for (const ItinPayments& itin : _itinsPayments._itinPayments)
  {
    printHeaderShort(str(boost::format("ITIN: %|-3|") % (itin.itinId() + 1)));

    _result << "\n";
    for (const Payment& payment : itin.payments(type::ProcessingGroup::Itinerary))
    {
      const TaxName& taxName = payment.taxName();
      for (const PaymentDetail* detail : payment.paymentDetail())
      {
        printPaymentDetailFiltered(taxName, *detail);
      }
    }

    for (const Payment& payment : itin.payments(type::ProcessingGroup::OC))
    {
      const TaxName& taxName = payment.taxName();
      for (const PaymentDetail* detail : payment.paymentDetail())
      {
        printOptionalServicesFiltered(taxName, *detail);
      }
    }

    for (const Payment& payment : itin.payments(type::ProcessingGroup::Baggage))
    {
      const TaxName& taxName = payment.taxName();
      for (const PaymentDetail* detail : payment.paymentDetail())
      {
        printOptionalServicesFiltered(taxName, *detail);
      }
    }

    for (const Payment& payment : itin.payments(type::ProcessingGroup::ChangeFee))
    {
      const TaxName& taxName = payment.taxName();
      for (const PaymentDetail* detail : payment.paymentDetail())
      {
        printPaymentDetailFiltered(taxName, *detail);
      }
    }
  }
}

void
NegativeDiagnostic::printPaymentDetail(const TaxName& taxName, const PaymentDetail& detail)
{
  const bool sequenceFilter =
      _filter.seq != type::SeqNo() || _filter.seqLimit != type::SeqNo() || _filter.showGP;

  _result << "NATION: " << taxName.nation() << "   TAXCODE: " << taxName.taxCode()
          << "   TAXTYPE: " << taxName.taxType() << "   SEQNO: " << detail.seqNo() << "\n";

  _result << "TXPTAG: " << taxName.taxPointTag() << "\n";
  _result << geoInfo(detail, sequenceFilter) << '\n';

  _result << "FAILED ON: " << detail.getFailureReason(_services) << "\n";

  if (!detail.applicatorFailMessage().empty())
  {
    _result << "ERROR MESSAGE: " << detail.applicatorFailMessage() << "\n";
  }

  printLine('*');
}

void
NegativeDiagnostic::printOptionalService(const TaxName& taxName,
                                         const PaymentDetail& detail,
                                         const OptionalService& optionalService)
{
  _result << "NATION: " << taxName.nation() << "   TAXCODE: " << taxName.taxCode()
          << "   TAXTYPE: " << taxName.taxType() << "   SEQNO: " << detail.seqNo() << "\n";

  _result << "OCTYPE: " << optionalService.type()
          << " (" << OCUtil::getOCTypeString(optionalService.type()) << ")"
          << "   SERVICESUBTYPECODE: " << optionalService.subCode() << "\n";
  _result << "SVCGROUP: " << optionalService.serviceGroup()
          << "   SVCSUBGROUP: " << optionalService.serviceSubGroup()
          << "   CARRIER: " << optionalService.ownerCarrier();
  if (!optionalService.pointOfDeliveryLoc().empty())
  {
    _result << "   PODELIVERYLOC: " << optionalService.pointOfDeliveryLoc() << "\n";
  }
  _result << "\n";

  _result << "TXPTAG: " << taxName.taxPointTag() << "\n";
  _result << "BOARD: " << optionalService.getTaxPointBegin().locCode()
          << "     OFF: " << optionalService.getTaxPointEnd().locCode() << "\n";

  _result << "BOARD INDEX: " << optionalService.getTaxPointBegin().id()
          << "\tOFF INDEX: " << optionalService.getTaxPointEnd().id() << "\n\n";

  _result << "FAILED ON: " << optionalService.getFailureReason(_services) << "\n";

  printLine('*');
}

bool
NegativeDiagnostic::printPaymentDetailFiltered(const TaxName& taxName, const PaymentDetail& detail)
{
  if ( detail.isValidForItinerary() || !filter(taxName, detail.seqNo()) )
    return false;

  printPaymentDetail(taxName, detail);
  return true;
}

bool
NegativeDiagnostic::printOptionalServicesFiltered(const TaxName& taxName,
                                                  const PaymentDetail& detail)
{
  if ( !filter(taxName, detail.seqNo()) )
    return false;

  for (OptionalService const& optionalService : detail.optionalServiceItems())
  {
    bool subCodeMatched = _filter.subCode.empty() || (optionalService.subCode() == _filter.subCode);
    bool serviceGroupMatched =
        _filter.group.empty() || (optionalService.serviceGroup() == _filter.group);
    bool serviceSubGroupMatched =
        _filter.subGroup.empty() || (optionalService.serviceSubGroup() == _filter.subGroup);
    bool ownerCarrierMatched =
        _filter.carrier.empty() || (optionalService.ownerCarrier() == _filter.carrier);
    bool typeMatched =
        _filter.type == type::OptionalServiceTag::Blank || (optionalService.type() == _filter.type);

    if (optionalService.isFailed() && subCodeMatched && serviceGroupMatched &&
        serviceSubGroupMatched && ownerCarrierMatched && typeMatched)
    {
      printOptionalService(taxName, detail, optionalService);
    }
  }

  return true;
}

void
NegativeDiagnostic::printHelp()
{
  printHeaderLong("HELP");

  _result << "OUTPUT FILTERS - DISPLAY ONLY SEQUENCES MATCHING FILTERS:\n"
          << "NO PARAM - FAILED SEQUENCES\n"
          << "HELP - HELP INFO\n"
          << "NAXX - TAXES FOR NATION XX\n"
          << "TCXX - TAXES FOR TAXCODE XX\n"
          << "TTX - TAXES FOR TAXTYPE X\n"
          << "SQXXX - TAXES FOR SEQUENCE XXX\n"
          << "SQXXX- - TAXES FOR SEQUENCE XXX AND HIGHER\n"
          << "SQXXX-YYY - TAXES FOR SEQUENCE RANGE XXX-YYY\n"
          << "SCXXX - TAXES ON OC/BAGGAGE FOR SUBCODE XXX\n"
          << "GRXX - TAXES ON OC/BAGGAGE FOR GROUP XX\n"
          << "SGXX - TAXES ON OC/BAGGAGE FOR SUBGROUP XX\n"
          << "CRXX - TAXES ON OC/BAGGAGE FOR CARRIER XX\n"
          << "OTX - TAXES ON OC/BAGGAGE FOR TYPE X\n"
          << "GP - SHOW GEO PROPERTIES\n"
          << "\n";

  printInputFilterHelp();
}

void
NegativeDiagnostic::printHeader()
{
  printLine('*');
  printHeaderShort("DIAGNOSTIC " + boost::lexical_cast<std::string>(NUMBER) + " - FAILED TAX");
  printLine('*');
}
}
