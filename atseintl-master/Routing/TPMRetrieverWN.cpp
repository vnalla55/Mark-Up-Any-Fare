#include "Routing/TPMRetrieverWN.h"

#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"
#include "Routing/GlobalDirectionRetriever.h"
#include "Routing/MileageRetriever.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MileageSubstitutionRetriever.h"
#include "Routing/TPMConstructor.h"

#include <numeric>

namespace tse
{
TPMRetrieverWN::~TPMRetrieverWN() {}

/* Algorithm of TPM retrieval for WN. Try to retrieve mileage data from subsequent data sources
   until successful. */
bool
TPMRetrieverWN::retrieve(MileageRouteItem& mileageRouteItem,
                         DataHandle& dataHandle,
                         GDPrompt*& gdPrompt) const
{
  // try to retrieve data directly from Mileage table. If successful, we are done.
  if (tpmGlobalDirection(mileageRouteItem, dataHandle, gdPrompt))
    return true;
  else if (gdPrompt != nullptr)
    return false;
  // try to substitute input (mainly the market) for Mileage table from subsequent tables.
  // if managed to retrieve data from Mileage table for substituted input, we are done.
  // to avoid modification of the original mileageRouteItem, work on a deep copy
  MileageRouteItem itemCopy;
  mileageRouteItem.clone(dataHandle, itemCopy);
  //    bool marketSubstituted;

  if (getMileageSubstitution(itemCopy, dataHandle) &&
      tpmGlobalDirection(itemCopy, dataHandle, gdPrompt))
  {
    // if successful for substituted market, copy the result to the original mileageRouteItem
    mileageRouteItem.tpm() = itemCopy.tpm();
    mileageRouteItem.tpmGlobalDirection() = itemCopy.tpmGlobalDirection();
    return true;
  }
  else if (gdPrompt != nullptr)
    return false;
  // if unsuccessful at retrieving TPM from Mileage, try to construct it
  return mpmBasedGD(mileageRouteItem, dataHandle, gdPrompt);
}

namespace
{
struct LessByGD : public std::binary_function<Mileage, Mileage, bool>
{
  bool operator()(const Mileage* l, const Mileage* r) const
  {
    return l->globaldir() < r->globaldir();
  }

  // lint --e{1509}
} lessByGD;

struct LessByVenderCode : public std::binary_function<Mileage, Mileage, bool>
{
  bool operator()(const Mileage* l, const Mileage* r) const { return l->vendor() < r->vendor(); }

  // lint --e{1509}
} lessByVendorCode;

struct EqualByGD : public std::binary_function<Mileage, Mileage, bool>
{
  bool operator()(const Mileage* l, const Mileage* r) const
  {
    return l->globaldir() == r->globaldir();
  }

  // lint --e{1509}
} equalByGD;

struct GatherGD : public std::binary_function<std::vector<GlobalDirection>,
                                              Mileage,
                                              std::vector<GlobalDirection> >
{
  std::vector<GlobalDirection>& operator()(std::vector<GlobalDirection>& l, const Mileage* m) const
  {
    l.push_back(m->globaldir());
    return l;
  }

  // lint --e{1509}
} gatherGD;

class EqualsByGD : public std::unary_function<Mileage, bool>
{
public:
  EqualsByGD(GlobalDirection gd) : _gd(gd) {}
  bool operator()(const Mileage* m) const { return m->globaldir() == _gd; }

private:
  GlobalDirection _gd;

  // lint --e{1509}
};
}

bool
TPMRetrieverWN::tpmGlobalDirection(MileageRouteItem& item,
                                   DataHandle& dataHandle,
                                   GDPrompt*& gdPrompt) const
{
  bool result(false);
  std::vector<Mileage*> tpms =
      dataHandle.getMileage(item.city1()->loc(), item.city2()->loc(), item.travelDate());

  // Get Unique data
  std::vector<Mileage*> alternateGDs(tpms);
  std::sort(alternateGDs.begin(), alternateGDs.end(), lessByGD);
  alternateGDs.erase(std::unique(alternateGDs.begin(), alternateGDs.end(), equalByGD),
                     alternateGDs.end());

  if (item.tpmGlobalDirection() == GlobalDirection::ZZ)
  { // user did not enter GI
    switch (alternateGDs.size())
    {
    case 1: // there is one published TPM - use it
      // There is one Golbal direction but can be different vendors.  Sort
      std::sort(tpms.begin(), tpms.end(), lessByVendorCode);
      item.tpmGlobalDirection() = tpms.front()->globaldir();
      item.tpm() = tpms.front()->mileage();
      result = true;
      break;
    case 0: // no published TPM - need to check MPM global
      result = false;
      break;
    default: // multiple published TPMs - ask user to choose
      dataHandle.get(gdPrompt);
      gdPrompt->alternateGDs() = std::accumulate(
          alternateGDs.begin(), alternateGDs.end(), gdPrompt->alternateGDs(), gatherGD);
      gdPrompt->origin() = item.city1()->loc();
      gdPrompt->destination() = item.city2()->loc();
      gdPrompt->currentGD() = item.tpmGlobalDirection();
      result = false;
    }
  }
  else
  { // user entered GI
    if (tpms.empty())
    { // no published TPM - need to check MPM global
      result = false;
    }
    else
    {
      std::vector<Mileage*>::const_iterator found =
          std::find_if(tpms.begin(), tpms.end(), EqualsByGD(item.tpmGlobalDirection()));
      if (found == tpms.end())
      { // user entered wrong global - return hint
        dataHandle.get(gdPrompt);
        gdPrompt->alternateGDs() = std::accumulate(
            alternateGDs.begin(), alternateGDs.end(), gdPrompt->alternateGDs(), gatherGD);
        gdPrompt->origin() = item.city1()->loc();
        gdPrompt->destination() = item.city2()->loc();
        gdPrompt->currentGD() = item.tpmGlobalDirection();
        result = false;
      }
      else
      { // user entered correct global - use the mileage
        item.tpm() = (*found)->mileage();
        result = true;
      }
    }
  }
  return result;
}

bool
TPMRetrieverWN::mpmBasedGD(MileageRouteItem& item, DataHandle& dataHandle, GDPrompt*& gdPrompt)
    const
{
  bool result(false);
  const std::vector<Mileage*> mpms =
      dataHandle.getMileage(item.city1()->loc(), item.city2()->loc(), item.travelDate(), MPM);
  if (item.tpmGlobalDirection() == GlobalDirection::ZZ)
  { // user did not enter GI
    const MileageDataRetriever& gdRetriever(tse::Singleton<GlobalDirectionRetriever>::instance());
    switch (mpms.size())
    {
    case 1: // single MPM published - use it
      item.tpmGlobalDirection() = mpms.front()->globaldir();
      item.tpm() = static_cast<uint16_t>(TseUtil::getTPMFromMPM(mpms.front()->mileage()));
      break;
    case 0
        : // no MPM published - need to retrieve from GlobalDirection and calculate mileage from GCM
      gdRetriever.retrieve(item, dataHandle);
      if (item.city1()->loc() != "IAT" && item.city2()->loc() != "IAT")
        item.tpm() = static_cast<uint16_t>(TseUtil::greatCircleMiles(*item.city1(), *item.city2()));
      break;
    default: // multiple MPM published - need to verify with GlobalDirection
      gdRetriever.retrieve(item, dataHandle);
      std::vector<Mileage*>::const_iterator found =
          std::find_if(mpms.begin(), mpms.end(), EqualsByGD(item.tpmGlobalDirection()));
      if (found != mpms.end())
        item.tpm() = static_cast<uint16_t>(TseUtil::getTPMFromMPM((*found)->mileage()));
      else
        item.tpm() = static_cast<uint16_t>(TseUtil::greatCircleMiles(*item.city1(), *item.city2()));
      break;
    }
    result = true;
  }
  else
  { // user entered GI
    if (mpms.empty())
    { // no published MPM
      MileageRouteItem itemCopy(item);
      const MileageDataRetriever& gdRetriever(tse::Singleton<GlobalDirectionRetriever>::instance());
      gdRetriever.retrieve(itemCopy, dataHandle);
      if (itemCopy.tpmGlobalDirection() == item.tpmGlobalDirection())
      { // user entered correct GI
        item.tpm() = static_cast<uint16_t>(TseUtil::greatCircleMiles(*item.city1(), *item.city2()));
        result = true;
      }
      else
      { // user entered wrong GI
        dataHandle.get(gdPrompt);
        gdPrompt->alternateGDs().push_back(itemCopy.tpmGlobalDirection());
        gdPrompt->origin() = item.city1()->loc();
        gdPrompt->destination() = item.city2()->loc();
        gdPrompt->currentGD() = item.tpmGlobalDirection();
        result = false;
      }
    }
    else
    { // there are published MPM
      std::vector<Mileage*>::const_iterator found =
          std::find_if(mpms.begin(), mpms.end(), EqualsByGD(item.tpmGlobalDirection()));
      if (found == mpms.end())
      { // user entered wrong global - return hint
        std::vector<Mileage*> alternateGDs(mpms);
        std::sort(alternateGDs.begin(), alternateGDs.end(), lessByGD);
        std::unique(alternateGDs.begin(), alternateGDs.end(), equalByGD);
        dataHandle.get(gdPrompt);
        gdPrompt->alternateGDs() = std::accumulate(
            alternateGDs.begin(), alternateGDs.end(), gdPrompt->alternateGDs(), gatherGD);
        gdPrompt->origin() = item.city1()->loc();
        gdPrompt->destination() = item.city2()->loc();
        gdPrompt->currentGD() = item.tpmGlobalDirection();
        result = false;
      }
      else
      { // user entered correct global - use the mileage
        item.tpm() = static_cast<uint16_t>(TseUtil::getTPMFromMPM((*found)->mileage()));
        result = true;
      }
    }
  }
  item.isConstructed() = true;
  return result;
}

bool
TPMRetrieverWN::getMileage(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& mileageRetriever(tse::Singleton<MileageRetriever>::instance());
  return mileageRetriever.retrieve(item, dataHandle, TPM);
}

bool
TPMRetrieverWN::getMileageSubstitution(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& mileageSubstitution(
      tse::Singleton<MileageSubstitutionRetriever>::instance());
  return mileageSubstitution.retrieve(item, dataHandle);
}

bool
TPMRetrieverWN::getConstructed(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const MileageDataRetriever& tpmConstructor(tse::Singleton<TPMConstructor>::instance());
  return tpmConstructor.retrieve(item, dataHandle);
}
}
