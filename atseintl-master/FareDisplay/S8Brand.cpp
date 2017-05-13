//-------------------------------------------------------------------
//
//  File:        S8Brand.cpp
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

#include "FareDisplay/S8Brand.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "BrandedFares/MarketResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "FareDisplay/Comparator.h"
#include "FareDisplay/ComparatorFactory.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupHeader.h"

namespace tse
{
S8Brand::S8Brand() {}
//------------------------------------------------------
// S8Brand::initializeS8BrandGroup()
//------------------------------------------------------
void
S8Brand::initializeS8BrandGroup(FareDisplayTrx& trx, std::vector<Group*>& groups)
{
  // Create Brand Group
  Group* grp = nullptr;
  trx.dataHandle().get(grp);

  grp->groupType() = Group::GROUP_BY_S8BRAND;
  grp->sortType() = Group::ASCENDING;

  ComparatorFactory rVFactory(trx);

  grp->comparator() = rVFactory.getComparator(grp->groupType());

  buildProgramBrandMap(trx, *grp);

  if (grp->comparator() != nullptr)
  {
    grp->comparator()->group() = grp;
    grp->comparator()->prepare(trx);
  }

  groups.push_back(grp);

  // Initialize Brand header
  GroupHeader header(trx);
  header.setS8BrandHeader();
}

void
S8Brand::buildProgramBrandMap(FareDisplayTrx& trx, Group& grp)
{
  std::map<int, std::vector<MarketResponse*> >::iterator bM;
  for (bM = trx.brandedMarketMap().begin(); bM != trx.brandedMarketMap().end(); ++bM)
  {
    std::vector<MarketResponse*>& marketResp = bM->second;
    for (const auto mR : marketResp)
    {
      for (std::vector<ProgramID>::iterator pI = mR->programIDList().begin();
           pI != mR->programIDList().end();
           ++pI)
      {
        std::vector<OneProgramOneBrand*> spbVec;
        buildOneProgramOneBrand(trx, *mR, spbVec);
        grp.programBrandMap().insert(std::make_pair(mR->carrier(), spbVec));
      }
    }
  }
}

void
S8Brand::buildOneProgramOneBrand(PricingTrx& trx,
                                 MarketResponse& mR,
                                 std::vector<OneProgramOneBrand*>& spbVec)
{
  std::vector<BrandProgram*>::const_iterator bPr = mR.brandPrograms().begin(); // S8 vector
  for (; bPr != mR.brandPrograms().end(); ++bPr)
  {
    BrandProgram& brandPr = **bPr;
    BrandProgram::BrandsData::const_iterator brI = brandPr.brandsData().begin();
    for (; brI != brandPr.brandsData().end(); ++brI)
    {
      const BrandInfo* brand = *brI;
      OneProgramOneBrand* sPb = nullptr;
      trx.dataHandle().get(sPb);
      sPb->carrier() = mR.carrier();
      sPb->programCode() = (*bPr)->programCode();
      sPb->programName() = (*bPr)->programName();
      sPb->brandCode() = brand->brandCode();
      sPb->brandName() = brand->brandName();
      sPb->tier() = brand->tier();
      sPb->passengerType() = (*bPr)->passengerType();
      sPb->systemCode() = (*bPr)->systemCode();
      spbVec.push_back(sPb);
    }
  }
}
}
