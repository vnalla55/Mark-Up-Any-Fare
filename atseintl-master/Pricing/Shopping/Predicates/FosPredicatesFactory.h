
//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/FiltersAndPipes/NamedPredicateWrapper.h"
#include "Pricing/Shopping/Predicates/CabinClassValidity.h"
#include "Pricing/Shopping/Predicates/IFosPredicatesFactory.h"
#include "Pricing/Shopping/Predicates/InterlineTicketingAgreement.h"
#include "Pricing/Shopping/Predicates/MinimumConnectTime.h"
#include "Pricing/Shopping/Predicates/PositiveConnectTime.h"

#include <boost/utility.hpp>

namespace tse
{

namespace utils
{

class FosPredicatesFactory : public IFosPredicatesFactory, boost::noncopyable
{
public:
  FosPredicatesFactory(ShoppingTrx& trx) : _trx(trx) {}

  INamedPredicate<SopCombination>*
  createFosPredicate(FOS_PREDICATE_TYPE type, const std::string& name = "") override
  {
    IPredicate<SopCombination>* predicate = nullptr;
    std::string nameToGive;

    switch (type)
    {
    case CABIN_CLASS_VALIDITY:
      predicate = &_trx.dataHandle().safe_create<CabinClassValidity>(_trx);
      nameToGive = "Cabin Class Validity";
      break;
    case MINIMUM_CONNECT_TIME:
      predicate = &_trx.dataHandle().safe_create<MinimumConnectTime>(_trx);
      nameToGive = "Minimum Connect Time";
      break;
    case POSITIVE_CONNECT_TIME:
      predicate = &_trx.dataHandle().safe_create<PositiveConnectTime>(_trx);
      nameToGive = "Positive Connect Time";
      break;
    case INTERLINE_TICKETING_AGREEMENT:
      predicate = &_trx.dataHandle().safe_create<InterlineTicketingAgreement>(_trx);
      nameToGive = "Interline Ticketing Agreement";
      break;
    default:
      TSE_ASSERT(!"Unknown FOS_PREDICATE_TYPE");
    }

    if (!name.empty())
    {
      nameToGive = name;
    }

    return wrapPredicateWithName(predicate, nameToGive, _trx);
  }

private:
  ShoppingTrx& _trx;
};

} // namespace utils

} // namespace tse

