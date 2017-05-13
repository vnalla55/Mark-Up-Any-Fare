//---------------------------------------------------------------------------
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
//----------------------------------------------------------------------------
#ifndef TAX_MAP_H
#define TAX_MAP_H

#include <boost/type_traits.hpp>
#include <boost/unordered_map.hpp>
#include <string>
#include <typeinfo>

#include "DBAccess/DataHandle.h"
#include "Taxes/LegacyTaxes/Tax.h"
#include "Taxes/LegacyTaxes/TaxFactory.h"
#include "Taxes/LegacyTaxes/TaxOnOC.h"
#include "Taxes/LegacyTaxes/TaxOnChangeFee.h"

namespace tse
{

class Logger;
class Itin;

class TaxMap
{
public:
  typedef std::pair<TaxFactoryBase*, Tax*> TaxFactoryPair;
  typedef boost::unordered_map<uint16_t, TaxFactoryPair> TaxFactoryMap;

  TaxMap(DataHandle& dataHandle)
    : _dataHandle(dataHandle),
      _taxOnOC(nullptr),
      _taxOnChangeFee(nullptr),
      _routeToOldUS2Initialized(false),
      _routeToOldUS2(false)
  {
  }

  void initialize() { buildTaxFactoryMap(_dataHandle, _taxFactoryMap); }

  void initialize(TaxFactoryMap& taxFactoryMap) { _taxFactoryMap = taxFactoryMap; }

  virtual ~TaxMap() {}

  static void buildTaxFactoryMap(DataHandle& dataHandle, TaxFactoryMap& taxFactoryMap);

  Tax* getSpecialTax(uint16_t specialTaxNumber)
  {
    TaxFactoryMap::iterator taxIter = _taxFactoryMap.find(specialTaxNumber);

    if (taxIter != _taxFactoryMap.end())
    {
      TaxFactoryPair& taxFactoryPair = taxIter->second;

      if (!taxFactoryPair.second)
      {
        taxFactoryPair.second = taxFactoryPair.first->getTax(_dataHandle);
      }

      return taxFactoryPair.second;
    }

    return nullptr;
  }

  Tax* getTaxOnOC()
  {
    if (!_taxOnOC)
    {
      _dataHandle.get(_taxOnOC);
    }

    return _taxOnOC;
  }

  Tax* getTaxOnChangeFee()
  {
    if (!_taxOnChangeFee)
    {
      _dataHandle.get(_taxOnChangeFee);
    }

    return _taxOnChangeFee;
  }

  bool routeToOldUS2(Itin& itin, PricingTrx& trx);
  static void setupNewUS2Itins();

protected:
  template <typename TTax>
  static void addTaxFactoryToMap(DataHandle& dataHandle,
                                 TaxFactoryMap& taxFactoryMap,
                                 uint16_t specialTaxNumber)
  {
    TaxFactory<TTax>* taxFactory = nullptr;
    dataHandle.get(taxFactory);
    TTax* tax = nullptr;
    TaxFactoryPair taxFactoryPair = std::make_pair(taxFactory, tax);

    taxFactoryMap.insert(TaxFactoryMap::value_type(specialTaxNumber, taxFactoryPair));
  }

  DataHandle& _dataHandle;
  TaxFactoryMap _taxFactoryMap;
  TaxOnOC* _taxOnOC;
  TaxOnChangeFee* _taxOnChangeFee;
  bool _routeToOldUS2Initialized;
  bool _routeToOldUS2;

  static std::set<std::string> _newUS2Itins;

  TaxMap(const TaxMap& map);
  TaxMap& operator=(const TaxMap& map);

private:
  static Logger _logger;
};

} // end namespace tse

#endif // TAX_MAP_H
