//-------------------------------------------------------------------
//
//  File:        S8BrandedFaresSelector.h
//  Created:     2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include "BrandedFares/BrandedFareDiagnostics.h"
#include "BrandedFares/BrandedFaresUtil.h"
#include "BrandedFares/BrandedFaresValidator.h"
#include "BrandedFares/CbasBrandedFaresSelector.h"
#include "BrandedFares/S8BrandedFaresSelector.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{

class PricingTrx;
class PaxTypeFare;
class BrandProgram;
class BrandInfo;
class Logger;
class Diag889Collector;
class Diag894Collector;
class FareMarket;

class BrandedFareValidatorFactory
{
public:
  virtual const BrandedFaresValidator& getValidator(BrandSource status) const;
  BrandedFareValidatorFactory(PricingTrx& trx)
  : _s8BrandedFareValidator(trx), _cbasBrandedFareValidator(trx)
  {
  }
  virtual ~BrandedFareValidatorFactory() {}
private:
  const S8BrandedFaresSelector _s8BrandedFareValidator;
  const CbasBrandedFareValidator _cbasBrandedFareValidator;
};

class BrandedFaresSelector : boost::noncopyable
{
  friend class BrandedFaresSelectorTest;
  friend class BrandedFaresSelectorDirectionalityTest;

public:
  BrandedFaresSelector(PricingTrx& trx, const BrandedFareValidatorFactory& brandSelectorFactory);
  virtual ~BrandedFaresSelector(){}
  virtual void validate(FareMarket& fareMarket);
  void validate(std::vector<PaxTypeFare*>& fares);
  void clear(){}

  void brandFares(std::set<PaxTypeFare*>& faresToBrand);

protected:
  bool matchFareMarketForDiagParam(const FareMarket& fareMarket);
  bool processPaxTypeFare(PaxTypeFare* paxTypeFare, BrandedFareDiagnostics& diagnostics,
    Diag894Collector* diag894 = nullptr);


  bool isT189exist(int marketID);
  bool validatePaxTypeFare(PaxTypeFare* paxTypeFare, int marketID, BrandedFareDiagnostics& diagnostics);
  bool getBrandData(PaxTypeFare* paxTypeFare, const std::vector<BrandProgram*>& brandPrograms, BrandedFareDiagnostics& diagnostics);
  void createBrandProgramMap(const FareMarket& fareMarket, std::map<BrandProgram*, std::vector<BrandInfo*>,
                             BrandProgramComparator>& brandProgramMap) const;
  bool matchGlobalDirection(const BrandProgram& brandPr, const PaxTypeFare& paxTypeFare);
  bool matchDirectionality(const BrandProgram& brandPr, const PaxTypeFare& paxTypeFare,
    Direction programDirection);
  bool matchDirectionalityShopping(const LocCode& originLoc, const PaxTypeFare& paxTF);
  PaxTypeFare::BrandStatus validateFare(PaxTypeFare* paxTypeFare,
                           BrandProgram* brandPr,
                           BrandInfo* brand,
                           bool& needBrandSeparator,
                           BrandedFareDiagnostics& diagnostics,
                           int brandProgramIndex = -1,
                           bool skipHardPassValidation = false);
  void displayBrandProgramMapSize(const PaxTypeFare* paxTypeFare, const FareMarket& fareMarket,
                                  BrandedFareDiagnostics& diagnostics) const;
  bool isFMforExchange(const FareMarket& fareMarket) const;
  BrandCode getBrandFromExcFM(const PaxTypeFare& ptf) const;

  PricingTrx& _trx;
  static Logger _logger;
  Diag889Collector* _diag889;
  bool _ignoreYyFares;
  const BrandedFareValidatorFactory& _brandSelectorFactory;
};
} // tse
