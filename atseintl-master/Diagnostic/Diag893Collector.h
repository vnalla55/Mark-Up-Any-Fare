//----------------------------------------------------------------------------
//  File:        Diag893Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 893 Branded Fares - display reviewed and parsed data from Branded service
//  Updates:
//
//  Copyright Sabre 2014
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

#include "Diagnostic/DiagCollector.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptor/transformed.hpp>

namespace tse
{
class MarketResponse;
class BrandProgram;
class BrandInfo;
class BrandFeatureItem;

class Diag893Collector : public DiagCollector
{
  friend class Diag893CollectorTest;

public:
  explicit Diag893Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag893Collector() {}

  Diag893Collector& operator << ( const PricingTrx::BrandedMarketMap& brandedMarketMap );
  Diag893Collector& operator << ( const MarketResponse* marketResponse );
  Diag893Collector& operator << ( const BrandProgram* brandProgram );
  Diag893Collector& operator << ( const BrandInfo* brandInfo );
  Diag893Collector& operator << ( const SvcFeesFareIdInfo* svcFeesFareIdInfo );
  Diag893Collector& operator << ( const BrandFeatureItem* brandedFeatureItem);

  bool isDDINFO() const
  {
    return rootDiag()->diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "INFO");
  }

private:
  template <typename Collection>
  std::string joinElementsInCollection(const Collection& collection, const std::string& separator = std::string(",")) const
  {
    typedef typename Collection::value_type ElementType;
    return  boost::algorithm::join(collection
      | boost::adaptors::transformed(
          boost::bind(&ElementType::operator std::string, boost::lambda::_1)), separator);
  }
};

} // namespace tse

