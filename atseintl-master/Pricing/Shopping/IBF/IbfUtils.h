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

#include "Pricing/Shopping/FiltersAndPipes/IFilter.h"
#include "Pricing/Shopping/IBF/IbfData.h"
#include "Pricing/Shopping/Utils/FosPipelineGenerator.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <iterator>
#include <vector>

namespace tse
{

class ShoppingTrx;

class IbfUtils
{
public:
  typedef std::back_insert_iterator<std::vector<utils::SopCandidate> > SopCandidateOutputIterator;

  // Loads all SOPs in trx to a given collector
  static void
  loadSopsFromTrx(utils::ICollector<utils::SopCandidate>& collector, const ShoppingTrx& trx);

  // Collects sops from trx which have valid cabin class
  // The sops are placed in given collector
  // An observer can be supplied to track discarded SOPs
  static void collectCabinClassValidSops(ShoppingTrx& trx,
                                         utils::ICollector<utils::SopCandidate>& collector,
                                         utils::IFilterObserver<utils::SopCandidate>* observer = nullptr);

  // Loads generator with sops from given collection
  static void loadGeneratorWithSops(utils::FosPipelineGenerator& generator,
                                    const std::vector<utils::SopCandidate>& validSops,
                                    utils::ICollector<utils::SopCandidate>* passedSopsObserver = nullptr);
};

} // namespace tse

