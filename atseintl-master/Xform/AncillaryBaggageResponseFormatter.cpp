//----------------------------------------------------------------------------
//
//  Copyright Sabre 2012
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

#include "Xform/AncillaryBaggageResponseFormatter.h"

#include "Common/BaggageStringFormatter.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/XMLConstruct.h"
#include "DataModel/AncRequest.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/ServicesSubGroup.h"
#include "DBAccess/SubCodeInfo.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OCFeesUsage.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <boost/algorithm/string.hpp>

namespace tse
{
FIXEDFALLBACK_DECL(AB240_DecoupleServiceFeesAndFreeBag);
FALLBACK_DECL(fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852);

const char AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_KILO = 'K';
const char AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_PER_5KILOS = 'L';
const char AncillaryBaggageResponseFormatter::BAG_FEE_APPLIES_AT_EACH_LOC = 'M';
const char AncillaryBaggageResponseFormatter::SERVICE_NONREFUNDABLE = 'N';
const char AncillaryBaggageResponseFormatter::FREQ_FLYER = 'F';

class MatchOCFee : public std::unary_function<OCFees*, bool>
{
  Indicator _fltTktMerchInd;
  uint32_t _btIndex;

public:
  MatchOCFee(Indicator fltTktMerchInd, uint32_t btIndex)
    : _fltTktMerchInd(fltTktMerchInd), _btIndex(btIndex)
  {
  }

  bool operator()(const OCFees* ocFees) const
  {
    return ocFees->baggageTravelIndex() == _btIndex &&
           ocFees->subCodeInfo()->fltTktMerchInd() == _fltTktMerchInd;
  }
};

AncillaryBaggageResponseFormatter::AncillaryBaggageResponseFormatter(AncillaryPricingTrx& ancTrx)
  : AncillaryPricingResponseFormatter(), _trx(ancTrx)
{
  _request = static_cast<AncRequest*>(_trx.getRequest());
}

std::string
AncillaryBaggageResponseFormatter::formatResponse(
    const std::string& errorString, ErrorResponseException::ErrorResponseCode errorCode)
{
  XMLConstruct construct;
  construct.openElement("AncillaryPricingResponse");
  construct.addAttribute("xmlns", XML_NAMESPACE_TEXT);

  buildErrorAndDiagnosticElements(_trx, construct, errorString, errorCode);

  if (!errorCode && ((_trx.diagnostic().diagnosticType() == DiagnosticNone) ||
                     (_trx.diagnostic().diagnosticType() == Diagnostic854)))
  {
    if (_request->wpbgDisplayItinAnalysis())
      prepareResponseText(_trx.response().str(), construct);
    else
    {
      std::ostringstream output;

      buildResponse(output);
      constructXMLResponse(construct, output.str());
    }
  }
  construct.closeElement();

  return construct.getXMLData();
}

void
AncillaryBaggageResponseFormatter::constructXMLResponse(XMLConstruct& construct,
                                                        const std::string& response) const
{
  uint32_t lineIndex = 1;
  boost::char_separator<char> separator("\n", "", boost::keep_empty_tokens);
  boost::tokenizer<boost::char_separator<char> > lines(response, separator);

  for (const std::string& line : lines)
  {
    if (line.size() > 64)
      LOG4CXX_WARN(_logger, "Line: [" << line << "] too long!");

    if (construct.getXMLData().size() > static_cast<size_t>(_maxTotalBuffSizeForWPDispAE))
    {
      prepareMessage(construct,
                     Message::TYPE_GENERAL,
                     lineIndex,
                     "MAX BAG DATA EXCEEDED/USE WP*BG WITH QUALIFIERS");
      break;
    }
    prepareMessage(construct, Message::TYPE_GENERAL, lineIndex++, line);
  }
}

void
AncillaryBaggageResponseFormatter::buildResponse(std::ostringstream& output)
{
  const std::set<uint32_t>& requestedBTIndices = _request->displayBaggageTravelIndices();

  if (!validateRequestedBTIndices(requestedBTIndices))
  {
    output << "INCORRECT BAGGAGE TRAVEL NO\n";
    return;
  }

  for (const Itin* itin : _trx.itin())
  {
    uint32_t btIndex = 0;
    const std::vector<OCFees*> ocFees = getFees(itin);

    for (const BaggageTravel* baggageTravel : _trx.baggageTravels()[itin])
    {
      ++btIndex;

      if (!requestedBTIndices.empty() &&
          requestedBTIndices.find(btIndex) == requestedBTIndices.end())
      {
        continue;
      }
      std::vector<OCFees*>::const_iterator allowanceOCFeesIter =
          std::find_if(ocFees.begin(), ocFees.end(), MatchOCFee('A', btIndex));

      std::vector<OCFees*> chargesOCFees;
      std::remove_copy_if(ocFees.begin(),
                          ocFees.end(),
                          std::back_inserter(chargesOCFees),
                          std::not1(MatchOCFee('C', btIndex)));

      if (_request->wpbgDisplayAllowance())
      {
        buildAllowance(output,
                       baggageTravel,
                       allowanceOCFeesIter == ocFees.end() ? (const OCFees*)nullptr
                                                           : (*allowanceOCFeesIter));
      }

      if (_request->wpbgDisplayCharges())
        buildCharges(output, baggageTravel, chargesOCFees);
    }
  }
  if (_request->wpbgDisplayCharges())
    buildFootnotesLegend(output);
}

void
AncillaryBaggageResponseFormatter::buildFootnotesLegend(std::ostringstream& output)
{
  if (_showFootnoteLegend[BAG_FEE_APPLIES_PER_KILO])
    output << BAG_FEE_APPLIES_PER_KILO << "   BAG FEE APPLIES PER EACH KILO\n";

  if (_showFootnoteLegend[BAG_FEE_APPLIES_PER_5KILOS])
    output << BAG_FEE_APPLIES_PER_5KILOS << "   BAG FEE APPLIES PER EACH 5 KILO\n";

  if (_showFootnoteLegend[BAG_FEE_APPLIES_AT_EACH_LOC])
    output << BAG_FEE_APPLIES_AT_EACH_LOC << "   BAG FEES APPLY AT EACH CHECK IN LOCATION\n";

  if (_showFootnoteLegend[SERVICE_NONREFUNDABLE])
    output << SERVICE_NONREFUNDABLE << "   SERVICE IS NONREFUNDABLE\n";

  if (_showFootnoteLegend[FREQ_FLYER])
    output << "    FREQUENT FLYER\n";

  if (!_showFootnoteLegend.empty())
    output << " \n";
}

void
AncillaryBaggageResponseFormatter::buildAllowanceHeader(std::ostringstream& output,
                                                        const OptionalServicesInfo* s7,
                                                        const SubCodeInfo* s5) const
{
  output << "    CXR ALLOWANCE\n";

  if (s5)
  {
    output << "    " << std::setw(3) << s5->carrier() << " ";

    if (s7)
    {
      std::ostringstream allowance;

      allowance << std::setfill('0') << std::setw(2);
      allowance << std::right;

      const bool displayPieces = s7->freeBaggagePcs() > 0;
      const bool displayWeight = s7->baggageWeight() > 0;

      if (!displayPieces && !displayWeight)
        allowance << "NIL";
      else
      {
        if (displayPieces)
          allowance << s7->freeBaggagePcs() << "P";

        if (displayWeight)
        {
          if (displayPieces)
            allowance << "/";

          allowance << formatWeight(*s7);
        }
      }

      output << std::setw(9) << std::left << allowance.str();

      const uint16_t ffStatus = s7->frequentFlyerStatus();

      if (ffStatus >= 1 && ffStatus <= 9)
        output << ffStatus << " FREQUENT FLYER";
    }
    else
      output << "DATA UNKNOWN";

    output << std::left << std::setfill(' ') << "\n";
  }
}

void
AncillaryBaggageResponseFormatter::buildCharges(std::ostringstream& output,
                                                const BaggageTravel* baggageTravel,
                                                const std::vector<OCFees*>& chargesOCFees)
{
  uint32_t index = 1;

  output << "BAG CHARGES - " << formatPaxType(baggageTravel->paxType(),
                                              _request->wpbgPostTicket()) << " - ";

  if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(&_trx))
    BaggageStringFormatter::old_printBaggageTravelSegments(*baggageTravel, output);
  else
    output << BaggageStringFormatter::printBaggageTravelSegmentsWithoutNumbering(*baggageTravel);

  output << std::left << "\n";

  if (chargesOCFees.empty() && !baggageTravel->_chargeS5Available)
    output << "-------------------NO DATA FOUND-----------------------\n";
  else
  {
    output << "\n    CXR DESCRIPTION            1ST LAST WEIGHT CUR  FEE\n";

    for (const OCFees* ocFees : chargesOCFees)
    {
      if (ocFees->ocfeeUsage().empty())
        output << "   " << std::setw(5) << ocFees->subCodeInfo()->carrier() << "  DATA UNKNOWN\n";
      else
      {
        std::vector<std::string> descriptionLines;
        formatSubCodeDescription(ocFees->subCodeInfo(), 21, descriptionLines);

        for (const OCFeesUsage* usage : ocFees->ocfeeUsage())
        {
          buildChargeLine(output,
                          index++,
                          usage,
                          ocFees->subCodeInfo()->carrier(),
                          descriptionLines,
                          baggageTravel->itin()->getBaggageTripType().isUsDot());
        }
        output << " \n";
      }
    }
  }
}

void
AncillaryBaggageResponseFormatter::buildAllowance(std::ostringstream& output,
                                                  const BaggageTravel* baggageTravel,
                                                  const OCFees* ocFees) const
{
  output << "BAG ALLOWANCE - " << formatPaxType(baggageTravel->paxType(),
                                                _request->wpbgPostTicket()) << " - ";

  if (fallback::fallbackAddSegmentsNumbersToCheckedPortionSectionInDiagnostic852(&_trx))
    BaggageStringFormatter::old_printBaggageTravelSegments(*baggageTravel, output);
  else
    output << BaggageStringFormatter::printBaggageTravelSegmentsWithoutNumbering(*baggageTravel);

  output << std::left << "\n";

  if (!ocFees)
    output << "-------------------NO DATA FOUND-----------------------\n";
  else
  {
    buildAllowanceHeader(output, ocFees->optFee(), ocFees->subCodeInfo());

    if (!ocFees->ocfeeUsage().empty())
    {
      output << "            AMT  DESCRIPTION\n";

      for (const OCFeesUsage* usage : ocFees->ocfeeUsage())
        buildAllowance(output, usage->getBaggageProperty());
    }
  }
  output << " \n";
}

void
AncillaryBaggageResponseFormatter::buildAllowance(
    std::ostringstream& output, const std::vector<OCFees::BaggageItemProperty>& itemProperties)
    const
{
  std::ostringstream freeTextTable;
  uint32_t index = 1;

  output << std::left;

  for (const OCFees::BaggageItemProperty& itemProperty : itemProperties)
  {
    std::vector<std::string> descriptionLines;

    if (itemProperty.isFreeText())
      freeTextTable << "        *" << itemProperty.getFreeText() << "\n";
    else
    {
      if (!formatSubCodeDescription(itemProperty.getSubCode(), 44, descriptionLines))
        continue;

      output << "        " << std::setw(4) << index++;
      output << std::right << std::setfill('0') << std::setw(2) << itemProperty.getNumber() << " ";
      output << std::left << std::setfill(' ');

      if (!descriptionLines.empty())
      {
        output << "  " << descriptionLines[0] << "\n";

        if (descriptionLines.size() > 1)
        {
          for (uint32_t i = 1; i < descriptionLines.size(); ++i)
            output << "                 " << descriptionLines[i] << "\n";
        }
      }
      else
        output << "\n";
    }
  }
  if (!freeTextTable.str().empty())
    output << " \n" << freeTextTable.str();
}

void
AncillaryBaggageResponseFormatter::buildChargeLine(std::ostringstream& output,
                                                   uint32_t index,
                                                   const OCFeesUsage* usage,
                                                   const CarrierCode& carrier,
                                                   const std::vector<std::string>& descriptionLines,
                                                   bool usDot)
{
  output << std::left << std::setw(4) << index;
  output << std::setw(4) << carrier;
  output << std::setw(23) << (!descriptionLines.empty() ? descriptionLines[0] : " ");

  const OptionalServicesInfo* s7 = usage->optFee();

  if (s7->baggageOccurrenceFirstPc() >= 0)
  {
    output << std::right << std::setw(2) << std::setfill('0') << s7->baggageOccurrenceFirstPc()
           << "  ";
  }
  else
    output << std::setw(4) << "N/A";

  output << std::left << std::setfill(' ');

  if (s7->baggageOccurrenceLastPc() >= 0)
  {
    output << std::right << std::setw(2) << std::setfill('0') << s7->baggageOccurrenceLastPc()
           << "   ";
  }
  else
    output << std::setw(5) << "N/A";

  output << std::left << std::setfill(' ');

  if (s7->baggageWeight() >= 0)
    output << std::setw(7) << formatWeight(*s7);
  else
    output << std::setw(7) << "N/A";

  ServiceFeeUtil util(_trx);
  const Money targetMoney = util.convertOCFeeCurrency(*usage);

  output << std::setw(5) << targetMoney.code();
  output << std::left << std::fixed;

  if (s7->notAvailNoChargeInd() != 'X')
  {
    output << std::setprecision(targetMoney.noDec(_trx.ticketingDate()));
    output << std::setw(7);
    output << getMoneyAmount(usage);
  }
  else
    output << "NOTAVAIL";

  const std::string footnotes = formatFootnotes(*s7, usDot);

  if (!footnotes.empty())
    output << std::right << std::setw(4) << footnotes;

  output << std::left << "\n";

  if (descriptionLines.size() > 1)
  {
    for (uint32_t i = 1; i < descriptionLines.size(); ++i)
      output << "        " << std::setw(23) << descriptionLines[i] << "\n";
  }
}

MoneyAmount
AncillaryBaggageResponseFormatter::getMoneyAmount(const OCFeesUsage* usage) const
{
  ServiceFeeUtil util(_trx);
  MoneyAmount moneyAmount = usage->feeAmount();
  Money money("");

  bool const baggageRequest =
      (static_cast<const AncRequest*>(_trx.getRequest())->majorSchemaVersion() >= 2) &&
      ServiceFeeUtil::checkServiceGroupForAcs(usage->subCodeInfo()->serviceGroup()) &&
      ServiceFeeUtil::isFeeFarePercentage(*usage->optFee());

  if (baggageRequest)
  {
    money = util.convertBaggageFeeCurrency(*usage);
    CurrencyRoundingUtil roundingUtil;
    roundingUtil.round(moneyAmount, usage->feeCurrency(), _trx);
  }
  else
  {
    money = util.convertOCFeeCurrency(*usage);
  }

  return ((usage->feeCurrency().empty() || usage->feeCurrency() == money.code()) && baggageRequest)
             ? moneyAmount
             : money.value();
}

bool
AncillaryBaggageResponseFormatter::formatSubCodeDescription(const SubCodeInfo* subCode,
                                                            uint32_t maxLineLen,
                                                            std::vector<std::string>& output) const
{
  if (!subCode)
    return false;

  std::string description;
  const ServicesDescription* svcDesc1 =
      _trx.dataHandle().getServicesDescription(subCode->description1());

  if (svcDesc1)
  {
    if (subCode->serviceSubTypeCode() != "0DG" && !subCode->serviceSubGroup().empty())
    {
      const ServicesSubGroup* servicesSubGroup = _trx.dataHandle().getServicesSubGroup(
          subCode->serviceGroup(), subCode->serviceSubGroup());

      if (servicesSubGroup && !servicesSubGroup->definition().empty())
      {
        if (subCode->serviceGroup().equalToConst("PT"))
          description += "PET ";

        description += servicesSubGroup->definition() + "/";
      }
    }

    description += svcDesc1->description();

    if (!subCode->description2().empty())
    {
      const ServicesDescription* svcDesc2 =
          _trx.dataHandle().getServicesDescription(subCode->description2());

      if (svcDesc2)
      {
        description += ((FreeBaggageUtil::isAlpha(svcDesc1->description()) ||
                         FreeBaggageUtil::isAlpha(svcDesc2->description()))
                            ? " "
                            : " AND ");
        description += svcDesc2->description();
      }
    }
  }
  else
    description += subCode->commercialName();

  split(description, maxLineLen, output);

  return true;
}

std::string
AncillaryBaggageResponseFormatter::formatPaxType(const PaxType* paxType, bool wpbgPostTicket) const
{
  std::string result;

  if (wpbgPostTicket && !paxType->psgTktInfo().empty())
    result = "TR" + paxType->psgTktInfo().front()->tktRefNumber();
  else
    result = paxType->paxType();

  return result;
}

void AncillaryBaggageResponseFormatter::getFees(std::vector<OCFees*>& ocFees, const FarePath* farePath,
                                                const std::vector<ServiceFeesGroup*>& serviceFeesGroupVector,
                                                const std::vector<ServiceGroup>& requestedGroupCodes) const
{
  for (const ServiceFeesGroup* sfg : serviceFeesGroupVector)
  {
    if (std::find(requestedGroupCodes.begin(), requestedGroupCodes.end(), sfg->groupCode()) !=
        requestedGroupCodes.end())
    {
      ServiceFeeUtil::createOCFeesUsages(*sfg, _trx);

      std::map<const FarePath*, std::vector<OCFees*> >::const_iterator feesForFarePathIter =
          sfg->ocFeesMap().find(farePath);

      if (feesForFarePathIter != sfg->ocFeesMap().end())
      {
        ocFees.insert(
            ocFees.end(), feesForFarePathIter->second.begin(), feesForFarePathIter->second.end());
      }
    }
  }
}

std::vector<OCFees*>
AncillaryBaggageResponseFormatter::getFees(const Itin* itin) const
{
  std::vector<OCFees*> ocFees;
  std::vector<ServiceGroup> requestedGroupCodes;

  _trx.getOptions()->getGroupCodes(_trx.getOptions()->serviceGroupsVec(), requestedGroupCodes);
  getFees(ocFees, itin->farePath().front(), itin->ocFeesGroup(), requestedGroupCodes);
  if(!fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
  {
    getFees(ocFees, itin->farePath().front(), itin->ocFeesGroupsFreeBag(), requestedGroupCodes);
  }
  return ocFees;
}

std::string
AncillaryBaggageResponseFormatter::formatFootnotes(const OptionalServicesInfo& s7, bool usDot)
{
  std::stringstream footnotes;

  switch (s7.frequentFlyerMileageAppl())
  {
  case 'C':
  case 'P':
  case 'H':
  case 'K':
  {
    footnotes << BAG_FEE_APPLIES_PER_KILO;
    _showFootnoteLegend[BAG_FEE_APPLIES_PER_KILO] = true;
    break;
  }
  case 'F':
  {
    footnotes << BAG_FEE_APPLIES_PER_5KILOS;
    _showFootnoteLegend[BAG_FEE_APPLIES_PER_5KILOS] = true;
    break;
  }
  case '3':
  {
    if (usDot)
    {
      footnotes << BAG_FEE_APPLIES_AT_EACH_LOC;
      _showFootnoteLegend[BAG_FEE_APPLIES_AT_EACH_LOC] = true;
    }
  }
  }

  const uint16_t ffStatus = s7.frequentFlyerStatus();

  if (ffStatus >= 1 && ffStatus <= 9)
  {
    footnotes << ffStatus;
    _showFootnoteLegend[FREQ_FLYER] = true;
  }

  if (s7.refundReissueInd() == 'N' || s7.refundReissueInd() == 'R')
  {
    footnotes << SERVICE_NONREFUNDABLE;
    _showFootnoteLegend[SERVICE_NONREFUNDABLE] = true;
  }

  return footnotes.str();
}

std::string
AncillaryBaggageResponseFormatter::formatWeight(const OptionalServicesInfo& s7) const
{
  std::stringstream weight;

  weight << s7.baggageWeight() << (s7.baggageWeightUnit() == 'P' ? "LB" : "KG");

  return weight.str();
}

bool
AncillaryBaggageResponseFormatter::validateRequestedBTIndices(
    const std::set<uint32_t>& reqBTIndices) const
{
  for (const Itin* itin : _trx.itin())
  {
    for (uint32_t reqBTIndex : reqBTIndices)
    {
      if (reqBTIndex < 1 || reqBTIndex > _trx.baggageTravels()[itin].size())
        return false;
    }
  }

  return true;
}

void
AncillaryBaggageResponseFormatter::split(const std::string& str,
                                         uint32_t maxLen,
                                         std::vector<std::string>& output) const
{
  uint32_t offsets[1];

  offsets[0] = maxLen;
  boost::offset_separator lineLenSeparator(offsets, offsets + 1);
  boost::tokenizer<boost::offset_separator> lineLenTokens(str, lineLenSeparator);

  for (const std::string& lineLenToken : lineLenTokens)
    output.push_back(boost::trim_left_copy_if(lineLenToken, boost::is_any_of(" ")));
}
} // namespace
