// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/YQYR/V2ISYQYRCalculator.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
class FareMarket;
class FarePath;
class PaxTypeBucket;
class PaxTypeFare;
struct PrecalculatedTaxesAmount;

struct ShoppingSurchargesCortegeContext
{
  ShoppingSurchargesCortegeContext(FareMarket& fareMarket,
                                   PaxTypeBucket& cortege,
                                   ShoppingTrx::Leg& leg,
                                   uint32_t legIndex);

  FareMarket& fareMarket;
  PaxTypeBucket& cortege;
  ItinIndex& index;
  const bool aso;
  const uint32_t legIndex;
  const uint32_t* const legsBegin;
  const uint32_t* const legsEnd;

  ItinIndex::Key key;
};

class ShoppingSurcharges
{
public:
  ShoppingSurcharges(ShoppingTrx& trx);
  ~ShoppingSurcharges() { flushTaxDiagnosticsMsg(); }

  void process();

private:
  ShoppingTrx& _trx;
  YQYR::V2ISYQYRCalculator _yqyrCalculator;
  DiagCollector* _dc = nullptr;

  uint32_t _cat12Fares;
  uint32_t _yqyrFares;
  bool _enabled = false;
  bool _cat12QuickGet = false;

  void initTaxDiagnostics();
  void flushTaxDiagnosticsMsg();
  void collectTaxDiagnostics(TaxResponse& taxResponse);

  std::vector<CarrierCode> getCarrierList();

  void precalculateTaxesOnCarrier(CarrierCode carrier);
  bool precalculateTaxesOnCortege(ShoppingSurchargesCortegeContext& ctx);
  std::vector<PaxTypeFare*> getApplicableFares(ShoppingSurchargesCortegeContext& ctx);
  void precalculateTaxes(ShoppingSurchargesCortegeContext& ctx,
                         YQYR::V2ISCortegeCalculator* calculator,
                         const std::vector<PaxTypeFare*>& applicableFares,
                         PaxTypeFare& fare,
                         PrecalculatedTaxesAmount* amounts);

  void sortFares(FareMarket& fareMarket);

  bool calculateCAT12(FarePath& farePath);
  MoneyAmount getAmountOfCAT12(FarePath& farePath);

  TaxResponse* buildTaxResponse(FarePath& farePath);

  TaxResponse* calculateYQYR(YQYR::V2ISCortegeCalculator& calculator,
                             const ItinIndex::ItinIndexIterator& cellIterator,
                             const StdVectorFlatSet<CarrierCode>& carriersToProcess,
                             FarePath& farePath,
                             const PaxTypeFare& paxTypeFare);
  MoneyAmount getYQYR(FarePath& farePath, PaxTypeFare& paxTypeFare, TaxResponse& taxResponse);
};
}
