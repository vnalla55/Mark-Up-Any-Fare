#pragma once

#include "Common/TseEnums.h"
#include "FareCalc/FareAmountAdjustment.h"
#include "FareCalc/FcDispItem.h"
#include "FareCalc/FcStream.h"

#include <vector>

namespace tse
{

class FareCalcCollector;
class FarePath;
class FareUsage;
class CalcTotals;
class PricingUnit;
class MinFarePlusUpItem;
class DifferentialOverride;

class FcDispFarePath : public FcDispItem
{
  friend class FcDispFarePathTest;

public:
  FcDispFarePath(PricingTrx& trx,
                 const FarePath& fp,
                 const FareCalcConfig& fcConfig,
                 FareCalcCollector& fcCollector,
                 const CalcTotals* calcTotals,
                 bool useNetPubFbc = false);

  virtual ~FcDispFarePath();

  virtual std::string toString() const override;

  const std::string& fareCalculationLine() const { return _fareCalculationLine; }

protected:
  void displaySurcharges(FareCalc::FcStream& os) const;
  void displayMinFarePlusUp(FareCalc::FcStream& os) const;
  void displayDifferential(FareCalc::FcStream& os, const std::vector<FareUsage*>& fuv) const;
  void displayTotalTransferSurcharge(FareCalc::FcStream& os) const;
  void displayTotalStopOverSurcharge(FareCalc::FcStream& os) const;
  void displayLocalNucCurrency(const FareCalc::FareAmountAdjustment& fareAmountAdjustment,
                               FareCalc::FcStream& os) const;
  void displayConsolidatorPlusUp(FareCalc::FcStream& os, bool firstPosition) const;
  void displayROE(FareCalc::FcStream& os) const;
  void displayIsiCode(FareCalc::FcStream& os) const;
  void displayTaxInfo(FareCalc::FcStream& os) const;
  void displayBSR(FareCalc::FcStream& os) const;
  void differentialForExchange(FareCalc::FcStream& os) const;
  void displayDifferentialForExchange(FareCalc::FcStream& os, DifferentialOverride& diffOver) const;

private: // Methods
  // Calculate how many outbound fare component need the odd-cent adjustment
  bool applicableTaxesMoreThanTaxBox() const;

private: // Data Members
  const FarePath& _farePath;
  const CalcTotals* _calcTotals;

  bool _useNetPubFbc;
  int _noDec;

  mutable std::string _fareCalculationLine;
};

class collect_minfare_plusup
{
public:
  collect_minfare_plusup(std::map<MinimumFareModule, std::string>& messages,
                         const FarePath& farePath,
                         PricingTrx& trx)
    : _messages(messages), _farePath(farePath), _trx(trx)
  {
  }

  void operator()();
  void operator()(PricingUnit* pricingUnit);
  void operator()(const FareUsage* fareUsage);

protected:
  void insertPlusUpMessage(MinimumFareModule module,
                           const MinFarePlusUpItem& plusUp,
                           const MinFarePlusUpItem* hipPlusUp = nullptr);
  std::string getPlusUpStr(MinimumFareModule module,
                           const MinFarePlusUpItem& plusUp,
                           const MinFarePlusUpItem* hipPlusUp = nullptr);

  std::map<MinimumFareModule, std::string>& _messages;
  const FarePath& _farePath;
  PricingTrx& _trx;
};

} // namespace tse

