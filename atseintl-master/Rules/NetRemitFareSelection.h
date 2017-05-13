//----------------------------------------------------------------------------
//  Copyright Sabre 2006
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

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/Fare.h"
#include "Rules/RuleConst.h"

namespace tse
{
class PricingTrx;
class RepricingTrx;
class PaxTypeFare;
class PaxType;
class Loc;
class FarePath;
class PricingUnit;
class FareUsage;
class TravelSeg;
class FareMarket;
class NegFareRest;
class DiagCollector;
class Fare;

// @class NetRemitFareSelection
//
// This class is used to select a Net Remit Published Fare.

class NetRemitFareSelection
{
public:
  static bool processNetRemitFareSelection(PricingTrx& trx,
                                           const FarePath& farePath,
                                           PricingUnit& pricingUnit,
                                           FareUsage& fareUsage,
                                           const NegFareRest& negFareRest);

  virtual ~NetRemitFareSelection() = default;

protected:
  NetRemitFareSelection(PricingTrx& trx,
                        const FarePath& fPath,
                        PricingUnit& pu,
                        FareUsage& fu,
                        const NegFareRest& negFareRest);

  void process();

  void printHeader();

  void buildPblFareMarkets();

  const FareMarket* getPblFareMarket(std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr);
  const FareMarket* getPblFareMarket(const Loc* orig,
                                     const Loc* dest,
                                     const CarrierCode& cxr,
                                     const DateTime& deptDT,
                                     const DateTime& arrDT);
  virtual const Loc* getOrig(const Loc* origLoc, const Loc* destLoc) const;
  virtual const Loc* getDest(const Loc* origLoc, const Loc* destLoc) const;
  virtual RepricingTrx* runRepriceTrx(std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr);
  virtual const FareMarket*
  getFareMarket(RepricingTrx* rpTrx, std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr);

  void displayFareMarket(const FareMarket& fareMarket);
  void displayFareHeader();
  void displayFare(const PaxTypeFare& paxFare) const;
  virtual void displayTicketedFareData();

  const PaxTypeFare* selectTktPblFare(const FareMarket& fm,
                                      const FareMarket* fm2 = nullptr,
                                      const FareMarket* fm3 = nullptr,
                                      const FareMarket* fm4 = nullptr);
  // const PaxTypeFare* selectTktPblFare(const FareMarket& fm);
  const PaxTypeFare* selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares, Fare::FareState fs);
  const PaxTypeFare* selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares);
  const PaxTypeFare* selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares,
                                      Fare::FareState fs,
                                      bool isSameCurrency);
  const PaxTypeFare*
  selectTktPblFare(const std::vector<PaxTypeFare*>& ptFares, bool isSameCurrency);

  const PaxTypeFare* selectValidTktPblFare(std::vector<PaxTypeFare*>& selFares);

  const PaxTypeFare* selectValidTktPblFareForAxess(const std::vector<PaxTypeFare*>& ptFares,
                                                   std::vector<PaxTypeFare*>& selFares);

  void getTktPblFares(const std::vector<PaxTypeFare*>& ptFares,
                      std::vector<PaxTypeFare*>& selFares,
                      Fare::FareState fs,
                      bool sameCurrency);
  virtual bool isMatchedFare(const PaxTypeFare& paxTypeFare) const;

  virtual bool isFareFamilySelection();
  bool isFareFamilySelection(const FareClassCode& fareClass);

  void checkSeasons(std::vector<PaxTypeFare*>& selFares);
  void checkSeasonsBlackouts(std::vector<PaxTypeFare*>& selFares);
  void checkRoutingNo(std::vector<PaxTypeFare*>& selFares);
  void checkFootnotesC14C15(std::vector<PaxTypeFare*>& selFares);
  virtual bool checkAndBetw(std::vector<PaxTypeFare*>& selFares, bool isAbacusEnabled);
  void checkAndBetw(std::vector<PaxTypeFare*>& selFares);

  void checkRuleStatusForJalAxess(const std::vector<PaxTypeFare*>& ptFares,
                                  std::vector<PaxTypeFare*>& selFares);

  const PaxTypeFare*
  selectPublicAgainstPrivateFare(const std::vector<PaxTypeFare*>& selFares) const;

  void processRouting(const PaxTypeFare* paxTypeFare);

  void checkMileageRouting();

  void adjustFare(const FareMarket& repricedFareMarket, const PaxTypeFare& paxTypeFare);

  void adjustNucFareAmount(const PaxTypeFare& paxTypeFare, MoneyAmount fareAmount);

  bool cat15Valid(const PaxTypeFare& paxTypeFare) const;

  std::vector<PaxTypeFare*> collectFaresFromFareMarkets(const FareMarket* fm,
                                                        const FareMarket* fm2,
                                                        const FareMarket* fm3,
                                                        const FareMarket* fm4);
  const std::vector<PaxTypeFare*>& collectFaresFromFareMarket(const FareMarket* fm) const;
  void findFaresForPaxType(std::vector<PaxTypeFare*>& ptFares,
                           const PaxTypeCode& pCode,
                           const FareMarket* fm) const;

  void createFareMarkets(const Loc* originLoc,
                         const Loc* destLoc,
                         const DateTime& arrivalDT,
                         const DateTime& departureDT,
                         const Loc* betwLoc1);
  void displayPotentialFares(const FareMarket* fm,
                             const FareMarket* fm2,
                             const FareMarket* fm3,
                             const FareMarket* fm4);
  void displayFares(const FareMarket* fm) const;
  void displayFares(const FareMarket* fm, Fare::FareState fs) const;
  bool filterFare(PaxTypeFare& paxFare, Fare* fare, Fare::FareState fs) const;

  PricingTrx& _trx;
  const FarePath& _fp;
  PricingUnit& _pu;
  FareUsage& _fu;
  const NegFareRest& _negFareRest;
  const FareMarket& _fm;

  const FareMarket* _alterCxrFm;
  const FareMarket* _seg1OrigBetw;
  const FareMarket* _seg1BetwAnd;
  const FareMarket* _seg1BetwDest;
  const FareMarket* _seg1BetwOrig;
  const FareMarket* _seg1DestBetw;

  const FareMarket* _seg2BetwAnd;

  DiagCollector* _diag;
  bool _dispAllFares;
  bool _dispMatchFare;

  bool _selSeg1;
  bool _axessUser;
  bool _isCmdPricing;
  bool _isDirFlippedInNewFmForJCB;
  CurrencyCode _rpCalculationCurrency;

private:
  void buildFareMarketsTask(const Loc* loc1,
                            const Loc* loc2,
                            const DateTime& arrivalDT,
                            const DateTime& departureDT,
                            const FareMarket*& fm);
  static Logger _logger;
  friend class NetRemitFareSelectionTest;
  friend class NetFareMarketThreadTask;
};
}
