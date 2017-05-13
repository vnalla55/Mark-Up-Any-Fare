//-------------------------------------------------------------------
//
//  Copyright Sabre 2009
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
#include "Common/ShoppingRexUtil.h"
#include "Diagnostic/DiagCollector.h"

#include <algorithm>

namespace tse
{
void
ShoppingRexUtil::mergeFareByteCxrApplRestrictions(
                                const FareByteCxrApplVect& fareByteCxrApplVect,
                                FareByteCxrAppl& fareByteCxrAppl,
                                DiagCollector* dc)
{
  std::set<CarrierCode>& mergedRestrictedCxrAppl = fareByteCxrAppl.restCxr;
  std::set<CarrierCode>& mergedApplicableCxrAppl = fareByteCxrAppl.applCxr;

  mergeRestrictedCrxAppl(fareByteCxrApplVect, mergedRestrictedCxrAppl); // intersect
  mergeApplicableCrxAppl(fareByteCxrApplVect, mergedApplicableCxrAppl); // sum

  if (dc)
  {
    *dc << "\n\n      MERGED RESTRICTED CXR:";
    printCxr(mergedRestrictedCxrAppl, dc);
    *dc << "\n      MERGED APPLICABLE CXR:";
    printCxr(mergedApplicableCxrAppl, dc);
  }

  FareByteCxrApplVect::const_iterator vectIter = fareByteCxrApplVect.begin();
  for (; vectIter != fareByteCxrApplVect.end(); ++vectIter)
  {
    const std::set<CarrierCode>& restrictedCxrAppl = vectIter->restCxr;
    const std::set<CarrierCode>& applicableCxrAppl = vectIter->applCxr;

    if (restrictedCxrAppl.empty())
    {
      std::set<CarrierCode> mergedRestrictedCxrApplDiff;
      std::set_difference(
          mergedRestrictedCxrAppl.begin(),
          mergedRestrictedCxrAppl.end(),
          applicableCxrAppl.begin(),
          applicableCxrAppl.end(),
          std::inserter(mergedRestrictedCxrApplDiff, mergedRestrictedCxrApplDiff.begin()));

      mergedRestrictedCxrAppl.swap(mergedRestrictedCxrApplDiff);
    }
  }
  if (dc)
  {
    *dc << "\n      MERGED RESTRICTED CXR - AFTER REMOVE APPL:";
    printCxr(mergedRestrictedCxrAppl, dc);
    *dc << "\n";
  }
}

void
ShoppingRexUtil::mergeRestrictedCrxAppl(
                                const FareByteCxrApplVect& fareByteCxrApplVect,
                                std::set<CarrierCode>& mergedRestrictedCxrAppl)
{
  if (fareByteCxrApplVect.empty())
    return;

  if (fareByteCxrApplVect.size() == 1)
  {
    mergedRestrictedCxrAppl.insert(fareByteCxrApplVect.begin()->restCxr.begin(),
                                   fareByteCxrApplVect.begin()->restCxr.end());
  }
  else
  {
    FareByteCxrApplVect::const_iterator firstNotEmpty =
        findFirstNotEmptyCxr(fareByteCxrApplVect);
    if (firstNotEmpty != fareByteCxrApplVect.end())
    {
      std::set<CarrierCode>::const_iterator firstCxrIter = firstNotEmpty->restCxr.begin();
      for (; firstCxrIter != firstNotEmpty->restCxr.end(); ++firstCxrIter)
      {
        bool found = false;
        FareByteCxrApplVect::const_iterator vectIter = fareByteCxrApplVect.begin();
        for (; vectIter != fareByteCxrApplVect.end(); ++vectIter)
        {
          const std::set<CarrierCode>& restrictedCxrAppl = vectIter->restCxr;
          if (restrictedCxrAppl.empty()) // skip empty ones
            continue;
          found = false;
          std::set<CarrierCode>::const_iterator cxrIter = restrictedCxrAppl.begin();
          for (; cxrIter != restrictedCxrAppl.end(); ++cxrIter)
          {
            if (*cxrIter == *firstCxrIter)
            {
              found = true;
              break;
            }
          }
          if (!found)
            break;
        }
        if (found)
          mergedRestrictedCxrAppl.insert(*firstCxrIter);
      }
    }
  }
}

void
ShoppingRexUtil::mergeApplicableCrxAppl(
                                const FareByteCxrApplVect& fareByteCxrApplVect,
                                std::set<CarrierCode>& mergedApplicableCxrAppl)
{
  if (fareByteCxrApplVect.empty())
    return;

  FareByteCxrApplVect::const_iterator vectIter = fareByteCxrApplVect.begin();
  for (; vectIter != fareByteCxrApplVect.end(); ++vectIter)
  {
    const std::set<CarrierCode>& applicableCxrAppl = vectIter->applCxr;
    std::set<CarrierCode>::const_iterator cxrIter = applicableCxrAppl.begin();
    for (; cxrIter != applicableCxrAppl.end(); ++cxrIter)
    {
      mergedApplicableCxrAppl.insert(*cxrIter);
    }
  }
}

void
ShoppingRexUtil::printCxr(const std::set<CarrierCode>& mergedCxrAppl,
                          DiagCollector* dc)
{
  std::set<CarrierCode>::const_iterator resIter = mergedCxrAppl.begin();
  for (; resIter != mergedCxrAppl.end(); ++resIter)
  {
    *dc << " " << *resIter;
  }
}

FareByteCxrApplVect::const_iterator
ShoppingRexUtil::findFirstNotEmptyCxr(const FareByteCxrApplVect& fareByteCxrApplVect)
{
  FareByteCxrApplVect::const_iterator vectIter = fareByteCxrApplVect.begin();
  for (; vectIter != fareByteCxrApplVect.end(); ++vectIter)
  {
    if (!(vectIter->restCxr.empty()))
      return vectIter;
  }
  return vectIter;
}

void
ShoppingRexUtil::mergePortion(
                      const std::vector<PortionMergeTvlVectType>& restrictions,
                      PortionMergeTvlVectType& mergedRestrictions,
                      DiagCollector* dc)
{
  if (restrictions.empty())
    return;

  if (restrictions.size() == 1)
  {
    mergedRestrictions.insert(
        mergedRestrictions.begin(), restrictions.begin()->begin(), restrictions.begin()->end());
  }
  else
  {
    PortionMergeTvlVectType::const_iterator firstElemIter =
        restrictions.begin()->begin();
    for (; firstElemIter != restrictions.begin()->end(); ++firstElemIter)
    {
      bool found = false;
      std::vector<PortionMergeTvlVectType>::const_iterator pMTvlVectIter =
          restrictions.begin();
      ++pMTvlVectIter;
      for (; pMTvlVectIter != restrictions.end(); ++pMTvlVectIter)
      {
        if (pMTvlVectIter->empty())
          return;

        found = false;
        PortionMergeTvlVectType::const_iterator tvlVectIter =
            pMTvlVectIter->begin();
        for (; tvlVectIter != pMTvlVectIter->end(); ++tvlVectIter)
        {
          if (*tvlVectIter == *firstElemIter)
          {
            found = true;
            break;
          }
        }
        if (!found)
          break;
      }
      if (found)
        mergedRestrictions.push_back(*firstElemIter);
    }
  }

  if (dc)
  {
    if (mergedRestrictions.empty())
    {
      *dc << "\n     NO MERGED PORTION RESTRICTIONS";
    }
    else
    {
      *dc << "\n     AFTER MERGE: \n";
      *dc << "      UNCHANGABLE SEGS POS: ";
      PortionMergeTvlVectType::const_iterator tvlVectIter =
          mergedRestrictions.begin();
      for (; tvlVectIter != mergedRestrictions.end(); ++tvlVectIter)
      {
        *dc << *tvlVectIter << " ";
      }
    }
    *dc << std::endl;
  }
}
}
