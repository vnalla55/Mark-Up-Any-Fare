//----------------------------------------------------------------------------
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
#pragma once

#include "Common/TseConsts.h"
#include "DataModel/AltPricingTrx.h"
#include "DataModel/AltPricingTrxData.h"
#include "DataModel/PaxDetail.h"
#include "DataModel/PricingOptions.h"
#include "Diagnostic/Diagnostic.h"
#include "Xform/PricingResponseXMLTags.h"

#ifdef TRACKING
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/TracksPrinter.h"
#endif

#include <algorithm>
#include <experimental/string_view>
#include <string>
#include <vector>

namespace tse
{
class AltPricingDetailObFeesTrx;
class AltPricingTrx;
class PricingDetailTrx;
class NoPNROptions;
class RexBaseTrx;

namespace predicate
{
class SurfacePredicate
    : public std::unary_function<const AltPricingTrx::AccompRestrictionInfo&, bool>
{
public:
  bool operator()(const AltPricingTrx::AccompRestrictionInfo& ari)
  {
    return ari.surfaceRestricted();
  }
};
}

class XMLConvertUtils
{
public:
  static void initialize(const uint32_t maxTotalBuffSize) { _maxTotalBuffSize = maxTotalBuffSize; }
  static inline void tracking(PricingTrx& trx)
  {
#ifdef TRACKING
    if (trx.diagnostic().diagnosticType() == DiagnosticNone)
      return;
    DiagManager dm(trx, trx.diagnostic().diagnosticType());
    TracksPrinter printer(trx, dm.collector());
    printer.process();
#endif
  }
  static std::string formatWtfrResponse(AltPricingTrx* altTrx);
  static std::string formatWpaWtfrResponse(AltPricingTrx* altTrx);
  static std::string formatWpanWtfrErrorResponse();
  static int formatResponse(std::string& tmpResponse,
                            std::string& xmlResponse,
                            const std::string& msgType = "X");
  static int formatResponse(std::string& tmpResponse,
                            std::string& xmlResponse,
                            int recNum,
                            const std::string& msgType = "X",
                            PricingDetailTrx* trx = nullptr);
  static bool skipServiceFeeTemplate(PricingDetailTrx* trx, const char* msg);
  static std::string
  prepareResponseText(const std::string& responseString, bool noSizeLimit = false);
  static std::string printDiagReturnedNoData(const DiagnosticTypes number);
  static void wrapWpnRespWitnMainTag(std::string& response);
  template <typename T>
  static std::string formatWpanDetailResponse(T* altTrx);
  static std::string formatWpanDetailResponse(AltPricingDetailObFeesTrx* altTrx);
  template <typename T>
  static std::string getWpanDetailPsgSum(T* altTrx);
  static std::string formatBaggageResponse(std::string baggageResponse, int& recNum);
  static std::string rexPricingTrxResponse(RexBaseTrx& trx);
  static std::string formatWpnResponse(PricingDetailTrx& pricingDetailTrx);
  static std::string formatWpnDetailsResponse(std::string wpnDetails,
                                              bool isDetailedWQ,
                                              bool isFirst,
                                              std::string& wqApplicableBookingCodesLine,
                                              NoPNROptions* noPnrOptions,
                                              int& recNum,
                                              PricingDetailTrx& trx);

private:
  static uint32_t _maxTotalBuffSize;
};

template <typename T>
std::string
XMLConvertUtils::formatWpanDetailResponse(T* altTrx)
{
  std::ostringstream stream;
  std::string xmlResponse;
  int recNumber = 2;

  if (std::any_of(altTrx->accompRestrictionVec().begin(),
                  altTrx->accompRestrictionVec().end(),
                  predicate::SurfacePredicate()))
    return formatWpanWtfrErrorResponse();

  std::string response("<PricingResponse>");
  std::string psgSum = getWpanDetailPsgSum(altTrx);

  if (psgSum.find("S66=\"") != std::string::npos)
  {
    response += altTrx->agentXml();
    response += altTrx->billingXml();
    response += psgSum;

    if (altTrx->getOptions()->isRecordQuote())
    {
      std::string tmp = "PRICE QUOTE RECORD RETAINED\n \n";
      formatResponse(tmp, xmlResponse, recNumber);
    }
  }

  for (const auto paxDetail : altTrx->paxDetails())
  {
    std::string singleResponse;

    const char* const searchBookCodes = "APPLICABLE BOOKING CLASS - ";
    std::string wpnDetails(paxDetail->wpnDetails());
    size_t loc = 0;
    while ((loc = wpnDetails.find("\\n", loc)) != std::string::npos)
    {
      size_t next = wpnDetails.find(searchBookCodes);
      if (altTrx->rebook() && (next != std::string::npos))
      {
        size_t diff = (next - loc);
        if ((diff == 2) || ((diff == 7) && (altTrx->vendorCrsCode() == ABACUS_MULTIHOST_ID)))
        {
          wpnDetails.replace(loc, 2, "\n");
          size_t size = wpnDetails.size();
          next = wpnDetails.find_last_of("\\n", size);

          wpnDetails = wpnDetails.erase(loc + 1, (next + 3 - loc + 1));
          continue;
        }
      }
      wpnDetails.replace(loc, 2, "\n");
    }

    singleResponse += wpnDetails + "\n";
    if (!altTrx->rebook() || (paxDetail != altTrx->paxDetails().back()))
      singleResponse += " \n";

    recNumber = formatResponse(singleResponse, xmlResponse, recNumber);
    xmlResponse += formatBaggageResponse(paxDetail->baggageResponse(), recNumber);
  }

  response += xmlResponse;
  response += "</PricingResponse>";
  return response;
}

template <typename T>
std::string
XMLConvertUtils::getWpanDetailPsgSum(T* altTrx)
{
  bool ticketGuaranteed = !std::any_of(altTrx->accompRestrictionVec().begin(),
                                       altTrx->accompRestrictionVec().end(),
                                       [](AltPricingTrx::AccompRestrictionInfo restrictionInfo)
                                       { return !restrictionInfo.guaranteed(); });

  const char* const searchTag = "<SUM ";
  const size_t searchTagLen = strlen(searchTag);
  std::string attrStr(xml2::TicketGuaranteed);
  attrStr += "=\"";
  attrStr += (ticketGuaranteed ? "T" : "F");
  attrStr += "\" ";

  std::string response;

  for (auto& restrictionInfo : altTrx->accompRestrictionVec())
  {
    std::string& selXml = restrictionInfo.selectionXml();
    size_t idx = selXml.find(searchTag);
    if (idx != std::string::npos)
    {
      selXml.insert(idx + searchTagLen, attrStr);
    }
    response += selXml;
  }

  return response;
}

class XMLSingleLevelFinder
{
public:
  using ElementPair = std::pair<std::size_t, std::size_t>;

  XMLSingleLevelFinder(const char* content, std::size_t length) : _str{content, length} {}
  ElementPair singleTagLookup(const char* pattern) const;
  std::vector<ElementPair> multipleTagLookup(const char* pattern) const;

  static void swapStringsByPosition(std::string& output, ElementPair from, ElementPair to);

private:
  ElementPair singleTagLookupImpl(const char* pattern, std::size_t from = 0) const;
  std::experimental::string_view _str;
};
}
