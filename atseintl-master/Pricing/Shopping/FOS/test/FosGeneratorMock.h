// -------------------------------------------------------------------
//
//
//  Copyright (C) Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------
#ifndef FOSGENERATORMOCK_H_
#define FOSGENERATORMOCK_H_

#include "Pricing/Shopping/FOS/SolFosGenerator.h"
#include "Pricing/Shopping/FOS/DetailedSop.h"

namespace tse
{
namespace fos
{

class FosGeneratorMock : public SolFosGenerator
{
  friend class FosGeneratorTest;

public:
  FosGeneratorMock(ShoppingTrx& trx, FosFilterComposite& fosFilterComposite)
    : SolFosGenerator(trx, fosFilterComposite)
  {
    _fosGenerators.push_back(createFosGenerator());
  }

  void addDetailedSop(uint32_t legId, uint32_t sopId, const SopDetailsPtrVec& sopDetails)
  {
    DetailedSop sop(legId, sopId);
    sop._sopDetailsVec = sopDetails;
    _sopsWithDetailsSet.insert(sop);
  }

  void collectSops(std::vector<std::vector<int> > sopsByLeg)
  {
    _fosGenerators[0]->setNumberOfLegs(sopsByLeg.size());
    for (uint32_t legId = 0; legId < sopsByLeg.size(); ++legId)
      for (uint32_t sopId = 0; sopId < sopsByLeg[legId].size(); ++sopId)
        _fosGenerators[0]->addSop(legId, static_cast<uint32_t>(sopsByLeg[legId][sopId]));
  }
};

} // fos
} // tse

#endif
