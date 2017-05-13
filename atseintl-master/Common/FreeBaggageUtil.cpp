// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Common/FreeBaggageUtil.h"

#include "Common/FallbackUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/ServicesDescription.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"

#include <boost/foreach.hpp>
#include <boost/regex.hpp>

namespace tse
{
const std::string FreeBaggageUtil::BaggageTagHead = "BAGGAGETEXT";
namespace
{
struct CheckSubCodeCharges : std::unary_function<const SubCodeInfo*, bool>
{
  bool operator()(const SubCodeInfo* subCodeInfo) const
  {
    return subCodeInfo->fltTktMerchInd() == BAGGAGE_CHARGE;
  }
};

struct CheckSubCodeInfo : std::unary_function<const SubCodeInfo*, bool>
{
  ServiceSubTypeCode _serviceSubTypeCode;

  CheckSubCodeInfo(const ServiceSubTypeCode& serviceSubTypeCode)
    : _serviceSubTypeCode(serviceSubTypeCode)
  {
  }

  bool operator()(const SubCodeInfo* subCodeInfo) const
  {
    return subCodeInfo->fltTktMerchInd() == BAGGAGE_CHARGE &&
         subCodeInfo->serviceSubTypeCode() == _serviceSubTypeCode;
  }
};
}

FreeBaggageUtil::S5MatchLogic::S5MatchLogic(const std::vector<ServiceSubTypeCode>& subCodes)
  : _subCodes(subCodes)
{
}

bool
FreeBaggageUtil::S5MatchLogic::isFirstConditionOk(const SubCodeInfo* s5) const
{
  return (std::find(_subCodes.begin(), _subCodes.end(), s5->serviceSubTypeCode()) !=
          _subCodes.end() &&
          s5->fltTktMerchInd() == BAGGAGE_CHARGE);
}

bool
FreeBaggageUtil::S5MatchLogic::isSecondConditionOk(const SubCodeInfo* s5)
{
  try { return (boost::lexical_cast<short>(s5->description1()) && s5->serviceSubGroup().empty()); }
  catch (boost::bad_lexical_cast&) { return false; }
}

bool
FreeBaggageUtil::S5MatchLogic::isMatched(const SubCodeInfo* s5) const
{
  return isFirstConditionOk(s5) && isSecondConditionOk(s5);
}

bool
FreeBaggageUtil::S5RecordsRetriever::S5Comparator::
operator()(const SubCodeInfo* s5first, const SubCodeInfo* s5second) const
{
  if (!s5first || !s5second)
    return false;

  return (s5first->serviceSubTypeCode() < s5second->serviceSubTypeCode());
}

FreeBaggageUtil::S5RecordsRetriever::S5RecordsRetriever(
    const boost::function<bool(const SubCodeInfo* const)>& acceptS5Condition,
    const CarrierCode& carrier,
    const PricingTrx& trx)
{
  std::vector<SubCodeInfo*> atpSubCodes;
  getS5Records(ATPCO_VENDOR_CODE, carrier, atpSubCodes, trx);

  remove_copy_if(atpSubCodes.begin(),
                 atpSubCodes.end(),
                 inserter(_filteredSubCodes, _filteredSubCodes.end()),
                 std::not1(acceptS5Condition));

  std::vector<SubCodeInfo*> mmgrSubCodes;
  getS5Records(MERCH_MANAGER_VENDOR_CODE, carrier, mmgrSubCodes, trx);

  remove_copy_if(mmgrSubCodes.begin(),
                 mmgrSubCodes.end(),
                 inserter(_filteredSubCodes, _filteredSubCodes.end()),
                 std::not1(acceptS5Condition));
}

void
FreeBaggageUtil::S5RecordsRetriever::get(std::vector<const SubCodeInfo*>& containerForFilteredS5s)
    const
{
  std::copy(
      _filteredSubCodes.begin(), _filteredSubCodes.end(), back_inserter(containerForFilteredS5s));
}

FreeBaggageUtil::CarryOnAllowanceS5RecordsForTable196Strategy::
    CarryOnAllowanceS5RecordsForTable196Strategy(const CarrierCode& carrier, const PricingTrx& trx)
  : FreeBaggageUtil::S5RecordsRetriever(CheckSubCodeCharges(), carrier, trx)
{
}

const SubCodeInfo*
FreeBaggageUtil::CarryOnAllowanceS5RecordsForTable196Strategy::
operator()(const ServiceSubTypeCode& subcodeType) const
{
  std::set<SubCodeInfo*, S5Comparator>::iterator it =
      find_if(_filteredSubCodes.begin(), _filteredSubCodes.end(), CheckSubCodeInfo(subcodeType));

  return (it != _filteredSubCodes.end() ? *it : nullptr);
}

FreeBaggageUtil::FreeBaggageUtil() {}
bool
FreeBaggageUtil::isAlpha(const std::string& str)
{
  return std::find_if(str.begin(), str.end(), std::ptr_fun((int (*)(int))std::isdigit)) ==
         str.end();
}

void
FreeBaggageUtil::getServiceSubCodes(const std::vector<std::string>& txtMsgs,
                                    std::multimap<ServiceSubTypeCode, int>& result)
{
  static const boost::regex expression("^//(\\d{2})/(\\w{3})$");

  for (std::string txtMsg : txtMsgs)
  {
    boost::cmatch what;
    if (boost::regex_match(txtMsg.c_str(), what, expression))
    {
      result.insert(std::pair<std::string, int>(what[2].first, std::atoi(what[1].first)));
    }
  }
}

void
FreeBaggageUtil::getServiceSubCodes(PricingTrx& trx,
                                    const OptionalServicesInfo* s7,
                                    std::multimap<ServiceSubTypeCode, int>& result)
{
  if (s7 && s7->freeBaggagePcs() > 0)
  {
    const TaxText* taxText = trx.dataHandle().getTaxText(s7->vendor(), s7->taxTblItemNo());

    if (taxText)
    {
      std::multimap<ServiceSubTypeCode, int> parsedMap;

      getServiceSubCodes(taxText->txtMsgs(), parsedMap);
      ServiceSubTypeCode serviceSubTypeCode;
      int pieces;

      BOOST_FOREACH(std::tie(serviceSubTypeCode, pieces), parsedMap)
      {
        if (pieces > 0)
          result.insert(std::make_pair(serviceSubTypeCode, pieces));
      }
    }
  }
}

void
FreeBaggageUtil::getServiceSubCodes(PricingTrx& trx,
                                    const OptionalServicesInfo* s7,
                                    std::vector<ServiceSubTypeCode>& result)
{
  if (s7 && s7->taxTblItemNo() && s7->freeBaggagePcs())
  {
    const TaxText* taxText = trx.dataHandle().getTaxText(s7->vendor(), s7->taxTblItemNo());

    if (taxText)
    {
      std::multimap<ServiceSubTypeCode, int> parsedMap;
      getServiceSubCodes(taxText->txtMsgs(), parsedMap);
      ServiceSubTypeCode serviceSubTypeCode;
      int pieces;

      BOOST_FOREACH(std::tie(serviceSubTypeCode, pieces), parsedMap)
      {
        if (pieces == s7->freeBaggagePcs())
          result.push_back(serviceSubTypeCode);
      }
    }
  }
}

void
FreeBaggageUtil::getS5Records(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              std::vector<SubCodeInfo*>& s5vector,
                              const PricingTrx& trx)
{
  const std::vector<SubCodeInfo*>& bgSubCodes =
      trx.dataHandle().getSubCode(vendor, carrier, "OC", "BG", trx.ticketingDate());
  const std::vector<SubCodeInfo*>& ptSubCodes =
      trx.dataHandle().getSubCode(vendor, carrier, "OC", "PT", trx.ticketingDate());
  s5vector.reserve(bgSubCodes.size() + ptSubCodes.size());
  copy(bgSubCodes.begin(), bgSubCodes.end(), back_inserter(s5vector));
  copy(ptSubCodes.begin(), ptSubCodes.end(), back_inserter(s5vector));
}

const SubCodeInfo*
FreeBaggageUtil::getS5Record(const CarrierCode& carrier,
                             const std::vector<ServiceSubTypeCode>& subCodes,
                             const PricingTrx& trx)
{
  const SubCodeInfo* s5 = getS5Record(ATPCO_VENDOR_CODE, carrier, subCodes, trx);

  if (!s5)
    s5 = getS5Record(MERCH_MANAGER_VENDOR_CODE, carrier, subCodes, trx);

  return s5;
}

const SubCodeInfo*
FreeBaggageUtil::getS5Record(const VendorCode& vendor,
                             const CarrierCode& carrier,
                             const std::vector<ServiceSubTypeCode>& subCodes,
                             const PricingTrx& trx)
{
  const std::vector<SubCodeInfo*>& s5records = retrieveS5Records(vendor, carrier, trx);

  S5MatchLogic matchLogic(subCodes);
  for (const SubCodeInfo* subCodeInfo : s5records)
  {
    if (matchLogic.isMatched(subCodeInfo))
      return subCodeInfo;
  }

  return nullptr;
}

const std::vector<SubCodeInfo*>&
FreeBaggageUtil::retrieveS5Records(const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const PricingTrx& trx)
{
  return trx.dataHandle().getSubCode(vendor, carrier, "OC", "BG", trx.ticketingDate());
}

const SubCodeInfo*
FreeBaggageUtil::getS5RecordCarryOn(const CarrierCode& carrier,
                                    ServiceSubTypeCode subCodeType,
                                    const PricingTrx& trx)
{
  return CarryOnAllowanceS5RecordsForTable196Strategy(carrier, trx)(subCodeType);
}

bool
FreeBaggageUtil::isSortNeeded(const AncillaryPricingTrx* trx)
{
  return isItBaggageDataTransaction(trx);
}

bool
FreeBaggageUtil::isItBaggageDataTransaction(const AncillaryPricingTrx* trx)
{
  return trx->billing()->requestPath() == ACS_PO_ATSE_PATH ||
         trx->billing()->actionCode().substr(0, 5) == "MISC6" ||
         (static_cast<const AncRequest*>(trx->getRequest())->isWPBGRequest());
}

uint32_t
FreeBaggageUtil::calcFirstChargedPiece(const OCFees* allowance)
{
  if (!allowance || !allowance->optFee())
    return 0;

  const OptionalServicesInfo& s7 = *allowance->optFee();

  // If allowance record has only weight coded don't process charges at all (per baggage FRD).
  if (s7.freeBaggagePcs() < 0)
    return MAX_BAG_PIECES;

  return s7.freeBaggagePcs();
}

bool
FreeBaggageUtil::matchOccurrence(const OptionalServicesInfo& s7, int32_t bagNo)
{
  const bool isFirstBlank = s7.baggageOccurrenceFirstPc() <= 0;
  const bool isLastBlank = s7.baggageOccurrenceLastPc() <= 0;

  if (isFirstBlank && isLastBlank)
    return true;

  if (bagNo < s7.baggageOccurrenceFirstPc())
    return false;

  return isLastBlank || bagNo <= s7.baggageOccurrenceLastPc();
}

bool
FreeBaggageUtil::isItinHasEnoughKnownCharges(const PricingTrx& trx, Itin& itin)
{
  for (const FarePath* farePath : itin.farePath())
  {
    for (const BaggageTravel* bt : farePath->baggageTravels())
    {
      uint32_t freePieces = FreeBaggageUtil::calcFirstChargedPiece(bt->_allowance);

      for (auto i = freePieces; i < trx.getBaggagePolicy().getRequestedBagPieces(); ++i)
      {
        if (!bt->_charges[i])
        {
          itin.errResponseCode() = ErrorResponseException::UNKNOWN_BAGGAGE_CHARGES;
          return false;
        }
      }
    }
  }

  return true;
}

} /* namespace tse */
