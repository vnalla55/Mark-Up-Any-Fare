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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStlTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/PricingUnit.h"

#include <string>

namespace tse
{
class Diagnostic;
class FareMarket;
class FarePath;
class PaxTypeFare;
class PricingTrx;
class Trx;

class DiagnosticUtil
{
public:
  DiagnosticUtil() = delete;
  virtual ~DiagnosticUtil() = delete;
  friend class DiagnosticUtilTest;

  static const size_t MAX_WIDTH = 64;

private:
  static bool showFareAmount(PricingTrx& trx,
                             const CarrierCode& fareCarrier,
                             const CarrierCode& publishingCarrier);

public:
  static bool isvalidForCarrierDiagReq(PricingTrx& trx, const PaxTypeFare& ptFare);
  static bool isDisplayKeywordPresent(PricingOptions& options);
  static bool showFareAmount(const bool& isKeywordPresent,
                             PricingTrx& trx,
                             const CarrierCode& fareCarrier,
                             const CarrierCode& publishingCarrier);

  static bool isAirlineAgent(PricingTrx& trx);
  static bool isJointVenture(PricingTrx& trx);
  static bool isFCRequested(PricingTrx& trx);
  static bool
  isFareOwnedByAirline(const CarrierCode& publishingCarrier, const CarrierCode& fareCarrier);

  template <typename T>
  static std::string containerToString(const T& s, bool isPipeSeperated = false)
  {
    if (LIKELY(s.empty()))
      return "";

    std::stringstream out;
    typename T::const_iterator i = s.begin();
    out << *i++;

    for (; i != s.end(); i++)
      out << (isPipeSeperated ? "|" : ",") << *i;

    return out.str();
  }

  //Iterates over given container and adds its elements to an output string,
  //with separator (before or after the element, based on separatorAtTheEnd)
  //and verifies if string is no longer than maxLen characters per line (if
  //zero than line length is ignored). If so it breaks the line and adds
  //initialLen of space characters at the beginning of the new line (to aligns
  //lines). markEmpty indicates if '?' symbol should be printed if empty element
  //found in container
  template <typename T>
  static std::string containerToString(
      const T& containter, std::string separator, size_t maxLen = 0,
      size_t initialLen = 0, bool separatorAtTheEnd = true, bool markEmpty = false);

  static std::string tcrTariffCatToString(TariffCategory tcrTariffCat);
  static std::string directionToString(FMDirection direction);
  static std::string geoTravelTypeToString(GeoTravelType travelType);
  static std::string geoTravelTypeTo3CharString(GeoTravelType travelType);
  static std::string globalDirectionToString(GlobalDirection gd);

  // To Filter 555 and 601-610 diag msg based on FareClass
  static bool filterByFareClass(const PricingTrx& trx, const PaxTypeFare& paxTypeFare);
  static bool filterByFareClass(const PricingTrx& trx, const PricingUnit& pricingUnit);
  static bool filterByFareClass(const PricingTrx& trx, const FarePath& farePath);

  // To Filter 555 and 601-610 diag msg based on FareClass and ItinIndex
  static bool filter(const PricingTrx& trx, const PaxTypeFare& paxTypeFare);
  static bool filter(const PricingTrx& trx, const PricingUnit& pu);
  static bool filter(const PricingTrx& trx, const FarePath& farePath);

  static void displayCabinCombination(const PricingTrx& trx,
                                      const FarePath& farePath,
                                      std::ostringstream& out);
  static void displayCabinCombination(const PricingTrx& trx,
                                      const PricingUnit& pricingUnit,
                                      std::ostringstream& out);
  static void displayObFeesNotRequested(PricingTrx& trx);

  static bool shouldDisplayFlexFaresGroupInfo(const Diagnostic&, const flexFares::GroupId&);

  static std::string printPaxTypeFare(const PaxTypeFare& ptf);

  static const char* pricingUnitTypeToString(const PricingUnit::Type puType);
  static const char* pricingUnitTypeToShortString(const PricingUnit::Type puType);
  static std::string getFareBasis(const PaxTypeFare& paxTypeFare);
  static char getOwrtChar(const PaxTypeFare& paxTypeFare);

  static bool showItinInMipDiag(const PricingTrx& trx, const IntIndex& itinNum);
};

template <typename T>
std::string DiagnosticUtil::containerToString(
    const T& containter, std::string separator, size_t maxLen,
    size_t initialLen, bool separatorAtTheEnd, bool markEmpty)
{
  if (containter.empty())
    return "";

  std::stringstream out;

  size_t len = initialLen;

  typename T::const_iterator iter = containter.begin();
  typename T::const_iterator iterLast = --containter.end();
  for (; iter != containter.end(); ++iter)
  {
    bool printQuestionMark = false;
    std::stringstream tmp;
    tmp << *iter;
    if (markEmpty && tmp.str().length() == 0)
    {
      printQuestionMark = true;
      tmp << "?";
    }

    bool last = (iter == iterLast);
    len += tmp.str().length() + (last && separatorAtTheEnd ? 0 : separator.length());
    if (maxLen > 0 && len > maxLen)
    {
      out << "\n";
      len = tmp.str().length() +
            (last && separatorAtTheEnd ? 0 : separator.length());
      if (initialLen > 0)
      {
        out << std::string(initialLen, ' ');
        len += initialLen;
      }
    }
    if (!separatorAtTheEnd)
    {
      out << separator;
    }
    out << *iter;
    if (printQuestionMark)
    {
      out << "?";
    }
    if (separatorAtTheEnd && !last)
    {
      out << separator;
    }
  }

  return out.str();
}

namespace tools
{

void printCat10Info(DiagCollector& dc, const CombinabilityRuleInfo* pCat10);

} // namespace tools

} // end tse namespace
