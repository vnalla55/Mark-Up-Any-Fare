/*
 * AltDatesTaxes.h
 *
 *  Created on: May 3, 2012
 *      Author: Masud Khan
 *  Changed on: Oct 4, 2012
 *        - by: Oleksiy Shchukin
 *
 * The copyright to the computer program(s) herein
 * is the property of Sabre.
 * The program(s) may be used and/or copied only with
 * the written permission of Sabre or in accordance
 * with the terms and conditions stipulated in the
 * agreement/contract under which the program(s)
 * have been supplied.
 *
 */

#pragma once

#include "DataModel/ShoppingTrx.h"
#include "Pricing/GroupFarePath.h"
#include "Pricing/Shopping/PQ/SopIdxVec.h"

#include <map>
#include <set>

namespace tse
{
class Diag941Collector;
class DiagCollector;
class Logger;
class SoloSurcharges;
class TaxItem;
class TaxResponse;

namespace shpq
{
class SoloGroupFarePath;
}

class AltDatesTaxes
{
public:
  typedef std::pair<shpq::SopIdxVec, GroupFarePath*> Solution;
  typedef std::pair<TaxCode, const TaxItem*> TaxComponent;
  typedef std::vector<TaxComponent> TaxComponentVec;
  typedef std::pair<Solution, bool> SolutionKeepItem;
  typedef std::vector<SolutionKeepItem> SolutionKeepVec;

  AltDatesTaxes(ShoppingTrx& trx, SoloSurcharges& soloSurcharges);
  ~AltDatesTaxes() { flushTaxDiagnostics(); }

  void removeUnwantedSolutions(ShoppingTrx::FlightMatrix& flightMatrix);

private:
  struct SortByTotalNUCAmount
  {
    bool operator()(const Solution& lhs, const Solution& rhs)
    {
      GroupFarePath* gfp1 = lhs.second, *gfp2 = rhs.second;
      return gfp1->getTotalNUCAmount() < gfp2->getTotalNUCAmount();
    }
  };
  typedef std::multiset<Solution, SortByTotalNUCAmount> SolutionsInDatePair;
  typedef std::map<DatePair, SolutionsInDatePair> SortedFlightMatrix;

  /**
   * @param sortedFlightMatrix to collect solutions sorted by totalNUCAmount after tax into
   */
  void applyTaxes(ShoppingTrx::FlightMatrix& flightMatrix, SortedFlightMatrix& sortedFlightMatrix);
  void removeUnwantedSolutionsForDatePair(ShoppingTrx::FlightMatrix& flightMatrix,
                                          const std::pair<DatePair, SolutionsInDatePair>& scope,
                                          Diag941Collector* dc);

  void getTaxedPrice(shpq::SoloGroupFarePath* gfp, Solution solution);
  MoneyAmount getTax(FarePath* farePath, Solution solution);
  std::string getCarrierCode(shpq::SopIdxVec sopVec);
  bool checkAndAddCarrierCode(std::string cxrCode);

  void initTaxDiagnostics();
  void flushTaxDiagnostics();
  void printTaxDiagnostics(const TaxResponse& taxResponse);

  size_t _cSolutionPerDatePair; // Max number of solutions to return per datepair. Configurable
  size_t _cSolutionFirstCxr;
  ShoppingTrx& _trx;
  SoloSurcharges& _soloSurcharges;

  std::vector<std::string> _cxrCodeVec;

  // for diag 941
  const bool _isDiag941;
  std::map<shpq::SopIdxVec, TaxComponentVec> _optTaxData;
  DiagCollector* _taxDiag;

  static Logger _logger;
};
}
