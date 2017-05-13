#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DataHandle.h"
#include "Rules/CategoryGeoCommon.h"
#include "Rules/RuleApplicationBase.h"
#include "Rules/RuleUtil.h"

#include <iostream>

namespace tse
{

class CarrierFlightSeg;
class DCFactory;
class DiagCollector;
class DiagManager;
class FareMarket;
class FarePath;
class FareUsage;
class FlightAppRule;
class Or;
class Predicate;
class PricingTrx;
class PricingUnit;
class RuleItemInfo;

class PredicateObserver
{
public:
  virtual ~PredicateObserver() {}

  virtual void getNotified() = 0;
};

class FlightApplication : public PredicateObserver, CategoryGeoCommon, public RuleApplicationBase
{
  friend class FlightApplicationTest;
public:
  FlightApplication();

  void initialize(const FlightAppRule* fap,
                  bool isQualifiedCategory,
                  const VendorCode& vc,
                  const PricingTrx* trx,
                  const DateTime& ticketDate = DateTime::localTime());

  virtual Record3ReturnTypes process(PaxTypeFare& paxTypeFare, PricingTrx& trx);

  Record3ReturnTypes process(PricingTrx& trx, const PricingUnit&, const FareUsage&);

  void printTree() const;
  DataHandle& getDH() { return _dataHandle; }
  bool& isForCppUnitTest() { return _isForCppUnitTest; }

  void displayRule(const FlightAppRule& fa, PricingTrx& trx, DiagCollector& diag);

  void getNotified() override;

  Record3ReturnTypes validate(PricingTrx& trx,
                              Itin& itin,
                              const PaxTypeFare& fare,
                              const RuleItemInfo* rule,
                              const FareMarket& fareMarket) override
  {
    return FAIL;
  }

  Record3ReturnTypes validate(PricingTrx& trx,
                              const RuleItemInfo* rule,
                              const FarePath& farePath,
                              const PricingUnit& pricingUnit,
                              const FareUsage& fareUsage) override
  {
    return FAIL;
  }

private:
  void buildPredicateTree();
  void addRelation(const Indicator rel, Predicate* rhs, bool notIndicator);
  bool allFlights(const FlightAppRule* r3) const;
  bool negate(const FlightAppRule* r3) const;
  Predicate* equipment(const FlightAppRule* r3);
  Predicate* dayOfWeek(const FlightAppRule* r3);
  Predicate* travelIndicator(const FlightAppRule* r3) const;
  Predicate* geoTables(const FlightAppRule* r3) const;
  Predicate* typeOfFlight(const FlightAppRule* r3);

  Predicate* flightOrTable(const FlightNumber flightNo, const CarrierCode& carrier, int fltTable);

  Predicate* addTable986(int table);

  template <typename T>
  void addTableElementPredicate(Or* predicate, CarrierFlightSeg* item);

  bool unknownDirectionaltyFMValidationFailed(Indicator inOutInd, const FareUsage& fareUsage) const;

  static Record3ReturnTypes negateReturnValue(Record3ReturnTypes value);

  // fields
  Predicate* _root;
  DataHandle _dataHandle; // No need to use Trx's dataHandle, because Trx and
  // FlightApplication can possibly have diferent
  // lifetimes.
  bool _isForCppUnitTest;
  bool _isQualifiedCategory; // different return for Open Segments
  // qualifier: SKIP, otherwise: PASS

  const FlightAppRule* _ruleItemInfo;

  VendorCode _rec2Vendor;

  // vars for WQ
  bool _WQautopass;
  bool _isViaTSI;

  const PricingTrx* _pricingTrx;

public:
  // consts
  // Generic Indicator
  static constexpr Indicator BLANK = ' ';
  static constexpr Indicator INVALID = '0';
  static constexpr Indicator VALID = '1';

  // Flight application
  static constexpr Indicator MUST_NOT = '0';
  static constexpr Indicator MUST = '1';
  static constexpr Indicator MUST_NOT_RANGE = '2';
  static constexpr Indicator MUST_RANGE = '3';

  // Flight Relational
  static constexpr Indicator AND_OR = '0';
  static constexpr Indicator CONNECTING = '1';
  static constexpr Indicator AND = '2';

  // Geo/Via application indicator, besides MUST_NOT and MUST
  static constexpr Indicator CONDITIONAL = '2';
  static constexpr Indicator NOT_APPLY = ' ';

  // locAppl
  static constexpr Indicator BETWEEN_AND = 'B';
  static constexpr Indicator VIA = 'V';
  static constexpr Indicator FROM_TO_VIA = 'X';

  static const CarrierCode ANY_CARRIER;

protected:
  bool initDiag(DCFactory*& factory,
                DiagCollector*& diagPtr,
                Diagnostic& trxDiag,
                const PaxTypeFare& paxTypeFare) const;

  Record3ReturnTypes
  process(const PaxTypeFare& paxTypeFare, const FareMarket& fareMarket, PricingTrx& trx);

  Record3ReturnTypes
  validate(const std::vector<FareUsage*>& fareUsages, PricingTrx& trx, const Indicator inOutInd);

  Record3ReturnTypes
  validateGeoVia(PricingTrx& trx, const FareMarket& fareMarket, DiagCollector* diag);

  Record3ReturnTypes matchResultAndAppl(PricingTrx& trx,
                                        const FlightAppRule& fa,
                                        const std::vector<TravelSeg*>& filteredBtwAndTvlSegs,
                                        const std::vector<TravelSeg*>& fliteredViaTvlSegs,
                                        DiagCollector* diag);

  Record3ReturnTypes testFlightAppl(const std::vector<TravelSeg*>& tvlSegs,
                                    PricingTrx& trx,
                                    const Indicator& geoAppl,
                                    const bool isPositiveVia,
                                    const bool isVia);

  Record3ReturnTypes passAnySeg(const std::vector<TravelSeg*>& tvlSegs,
                                PricingTrx& trx,
                                const Indicator geoAppl = MUST);

  bool isRltOnTwoFlights(const FlightAppRule& flightAppInfo);

  void applyHidden(GeoBools& geoBools);

  bool findMatchedTvlSegs(std::vector<TravelSeg*>& tvlSegsRtn,
                          const std::vector<TravelSeg*>& totalTvlSegs,
                          const std::vector<TravelSeg*>& keyTvlSegs);

  void diagTvlSegs(const std::vector<TravelSeg*>& tvlSegs, DiagCollector& diag);

  void diagSvcRestr(Indicator& serviceRestr, const Indicator& restrInd, DiagCollector& diag);

  void diagFltRelat(const Indicator& fltRelat, const Indicator& fltAppl, DiagCollector& diag);

  void diagStatus(const Record3ReturnTypes result, DiagCollector& diag) const;

  Record3ReturnTypes diagAndRtn(const Record3ReturnTypes result, DiagCollector* diag) const;

  Record3ReturnTypes
  diagAndRtn(const Record3ReturnTypes result, DiagManager& diag, bool diagEnabled) const;

  bool isRemoveArnk(const FlightAppRule* flightAppInfo) const;

  enum GeoRelation
  {
    AND_RELATION,
    OR_RELATION
  };

  static constexpr Indicator OUTBOUND = '1';
  static constexpr Indicator INBOUND = '2';
  static constexpr Indicator IN_XOR_OUTBOUND = '3';
  static constexpr Indicator IN_OR_OUTBOUND = '4';
  static constexpr Indicator IN_AND_OUTBOUND = '5';
};

} // namespace tse

