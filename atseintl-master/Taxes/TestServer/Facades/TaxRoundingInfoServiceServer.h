// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "ServiceInterfaces/TaxRoundingInfoService.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <map>


namespace tax {

class TaxRoundingInfoServiceServer : public TaxRoundingInfoService
{
public:
  TaxRoundingInfoServiceServer();

  void
  initialize(const boost::ptr_vector<TaxRounding>& data);

  void
  getFareRoundingInfo(const tax::type::CurrencyCode& /*currency*/,
                      tax::type::MoneyAmount& /*unit*/,
                      tax::type::TaxRoundingDir& /*dir*/) const override {}

  void
  getTrxRoundingInfo(const type::Nation& nation,
                     type::MoneyAmount& unit,
                     type::TaxRoundingDir& dir) const override;

  void
  getNationRoundingInfo(const type::Nation& /*nation*/,
                        type::MoneyAmount& /*unit*/,
                        type::TaxRoundingDir& /*dir*/) const override {}

  void
  doStandardRound(type::MoneyAmount& amount,
                  type::MoneyAmount& unit,
                  type::TaxRoundingDir& dir,
                  type::MoneyAmount currencyUnit = -1,
                  bool isOcFee = false) const override;
private:
  std::map<type::Nation, TaxRounding> _dataMap;
};

}
