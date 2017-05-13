//-------------------------------------------------------------------
//
//  File:        BrandingService.h
//  Created:     April 2013
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

#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

namespace tse
{
class PricingTrx;
class BrandingCriteria;

class BrandingService : boost::noncopyable
{
public:
  BrandingService(PricingTrx& trx);
  bool getBrandedFares();
  const std::string& xmlData() const { return _xmlData; }
  bool& getOnlyXmlData() { return _getOnlyXmlData; }
  const bool& getOnlyXmlData() const { return _getOnlyXmlData; }
  BrandingCriteria*& brandingCriteria() { return _brandingCriteria; }
  const BrandingCriteria* brandingCriteria() const { return _brandingCriteria; }

private:
  PricingTrx& _trx;
  std::string _xmlData;
  bool _getOnlyXmlData;
  BrandingCriteria* _brandingCriteria;
};
} // tse

