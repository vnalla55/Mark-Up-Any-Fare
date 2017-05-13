//-------------------------------------------------------------------
//
//  File:        IndustryFareController.h
//  Created:     June 29, 2004
//  Authors:     Mark Kasprowicz
//
//  Description: Industry fare factory
//
//  Updates:
//
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
//-------------------------------------------------------------------

#pragma once

#include "DBAccess/IndustryFareAppl.h"
#include "Fares/FareController.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DiagCollector;
class Fare;
class FareMarket;
class IndustryFare;
class IndustryFareBasisMod;
class Itin;
class PaxTypeFare;
class PricingTrx;

class IndustryFareController : public FareController
{
  friend class IndustryFareControllerTest;

public:
  IndustryFareController(PricingTrx& trx,
                         Itin& itin,
                         FareMarket& fareMarket,
                         bool processMultilateral = true,
                         bool processPureYY = true);

  virtual ~IndustryFareController();

  virtual bool process(const std::vector<Fare*>* addOnFares = nullptr);

  IndustryFare* matchFare(const PaxTypeFare& paxTypeFare,
                          const std::vector<const IndustryFareAppl*>& fareAppls,
                          const std::vector<PaxTypeFare*>* specifiedFares = nullptr);

protected:
  enum MatchResult
  {
    MATCH_CREATE_FARE = 1,
    MATCH_DONT_CREATE_FARE,
    NO_MATCH
  };

  IndustryFareController(const IndustryFareController& rhs);
  IndustryFareController& operator=(const IndustryFareController& rhs);
  bool matchFareTariff(const TariffNumber& appl, const TariffNumber& fare);
  bool matchVendor(const VendorCode& appl,
                   const Indicator userApplType,
                   const std::string& userAppl,
                   const VendorCode& fare);
  bool matchDir(const Indicator& applDir,
                const LocKey& applLoc1,
                const LocKey& applLoc2,
                const LocCode& fareOrigin,
                const LocCode& fareDest);
  bool matchRule(const RuleNumber& appl, const RuleNumber& fare);
  bool matchGlobal(const GlobalDirection& appl, const GlobalDirection& fare);
  bool matchCur(const CurrencyCode& appl, const CurrencyCode& fare);
  bool matchFareType(const FareType& appl, const FareType& fare);
  bool matchFareClass(const FareClassCode& appl, const FareClassCode& fare);
  bool matchFootnote(const Footnote& appl, const Footnote& fare1, const Footnote& fare2);
  bool matchRouting(const RoutingNumber& appl, const RoutingNumber& fare);
  bool matchOWRT(const Indicator& appl, const Indicator& fare);
  bool match(const Fare& fare, const PaxTypeFare& yyFare) const;
  static bool initialMatchOWRT(const Indicator& owrt1, const Indicator& owrt2);

  bool isIndustryFareDiagnostic(const DiagnosticTypes& diagType);

  bool
  matchIndAppl(const PaxTypeFare& paxTypeFare, const IndustryFareAppl& appl, std::string& reason);
  bool matchIndExceptAppl(const PaxTypeFare& paxTypeFare,
                          const IndustryFareAppl::ExceptAppl& except,
                          std::string& reason);

  bool matchFares(const std::vector<Fare*>& fares,
                  const std::vector<const IndustryFareAppl*>& multiFareAppls,
                  const std::vector<const IndustryFareAppl*>& indFareAppls,
                  std::vector<PaxTypeFare*>& matchedFares,
                  const std::vector<PaxTypeFare*>* specifiedFares = nullptr);

  MatchResult matchFare(const PaxTypeFare& paxTypeFare,
                        const IndustryFareAppl& appl,
                        IndustryFareAppl::ExceptAppl*& except);

  void diagHeader(void);
  void diagFare(const PaxTypeFare& paxTypeFare, const IndustryFareAppl* appl = nullptr);
  void diagFareAppl(const IndustryFareAppl& appl,
                    const IndustryFareAppl::ExceptAppl* except,
                    const IndustryFareBasisMod* basisMod);
  bool govCarrierHasFares(const PaxTypeFare& paxTypeFare, const Indicator& yyFareAppl);

  void diag273(const FareMarket&);
  void diag273(const PaxTypeFare&, const bool resolveOk = true);
  void
  diag273(const Fare&, const FareClassAppInfo*, const FareClassAppSegInfo*, const bool, const bool);
  void diag273(const IndustryFareAppl&);
  void diag273(const IndustryFareAppl::ExceptAppl&);
  void diag273(const char*);

  static char directionalityChar(const Indicator&);
  static char owrtChar(const Indicator&);
  static std::string globalDirectionString(const GlobalDirection&);
  static const char* yyFareApplString(const Indicator&);

  DiagCollector* _diag;
  bool _matchFareTypeOfTag2CxrFare = false;
  bool _show273 = false;
  bool _processMultilateral;
  bool _processPureYY;

  const std::vector<Fare*>* _addOnFares = nullptr;

private:
  Indicator _userApplType;
  std::string _userAppl;

  const static Indicator DO_NOT_CHANGE_FARE_BASIS;

  const static Indicator EXCLUDE_ALL_PRICING_FARE_QUOTE;
  const static Indicator EXCLUDE_ALL_PRICING;
  const static Indicator ALLOW_WHEN_NO_CARRIER;
  const static Indicator ALLOW_WHEN_NO_CARRIER_OF_TYPE;
  const static Indicator ALLOW_ALL;
};

} // namespace tse

