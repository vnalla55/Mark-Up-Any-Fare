#pragma once

#include "Common/TseObjectPool.h"
#include "Pricing/Shopping/PQ/CommonSoloPQItem.h"
#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"
#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Pricing/Shopping/PQ/FarePathFactoryPQItem.h"
#include "Pricing/Shopping/PQ/SolutionPatternPQItem.h"
#include "Util/BranchPrediction.h"

namespace tse
{
namespace shpq
{

class SoloPQItemManager;

namespace details
{

template <typename T>
struct SoloPQItemDeleter
{
  SoloPQItemDeleter(SoloPQItemManager& pqItemMgr) : _pqItemMgr(pqItemMgr) {}

  void operator()(T* ptr);

protected:
  SoloPQItemManager& _pqItemMgr;
};

} // namespace details

class SoloPQItemManager
{
public:
  typedef TseObjectPool<SolutionPatternPQItem> SPPQItemPool;
  typedef TseObjectPool<ConxRoutePQItem> CRPQItemPool;
  typedef TseObjectPool<ConxRouteCxrPQItem> CRCPQItemPool;
  typedef TseObjectPool<FarePathFactoryPQItem> FPFPQItemPool;

  typedef CommonSoloPQItem::LegPosition LegPosition;
  typedef FarePathFactoryPQItem::PUFBucketVector PUFBucketVector;
  typedef FarePathFactoryPQItem::PUStruct PUStruct;

  SoloPQItemManager() {}

  SolutionPatternPQItemPtr constructSPPQItem(const SolutionPattern& solutionPattern,
                                             const DirFMPathListPtr& outboundDFm,
                                             const DirFMPathListPtr& inboundDFm);
  SolutionPatternPQItemPtr
  constructSPPQItem(const SolutionPatternPQItem& other, const LegPosition expandedLegPos);

  ConxRoutePQItemPtr constructCRPQItem(const SolutionPattern& solutionPattern,
                                       const DirFMPathPtr& outboundDFm,
                                       const DirFMPathPtr& inboundDFm);
  ConxRoutePQItemPtr
  constructCRPQItem(const ConxRoutePQItem& other, const LegPosition expandedLegP);

  ConxRouteCxrPQItemPtr constructCRCPQItem(const ConxRoutePQItem& crPQItem,
                                           SoloTrxData& soloTrxData);

  FarePathFactoryPQItemPtr constructFPFPQItem(const ConxRouteCxrPQItem& crcPQItem,
                                              SoloTrxData& soloTrxData,
                                              PUStruct& puStruct);
  FarePathFactoryPQItemPtr
  constructFPFPQItem(const FarePathFactoryPQItem& other, DiagCollector& diag);

  void destroy(SolutionPatternPQItem* item) { _spPQItemPool.destroy(item); }

  void destroy(ConxRoutePQItem* item) { _crPQItemPool.destroy(item); }
  void destroy(ConxRouteCxrPQItem* item) { _crcPQItemPool.destroy(item); }
  void destroy(FarePathFactoryPQItem* item) { _fpfPQItemPool.destroy(item); }

private:
  SPPQItemPool _spPQItemPool;
  CRPQItemPool _crPQItemPool;
  CRCPQItemPool _crcPQItemPool;
  FPFPQItemPool _fpfPQItemPool;
};

namespace details
{
template <typename T>
inline void
SoloPQItemDeleter<T>::
operator()(T* ptr)
{
  _pqItemMgr.destroy(ptr);
}
}

inline SolutionPatternPQItemPtr
SoloPQItemManager::constructSPPQItem(const SolutionPattern& solutionPattern,
                                     const DirFMPathListPtr& outboundDFm,
                                     const DirFMPathListPtr& inboundDFm)
{
  return SolutionPatternPQItemPtr(_spPQItemPool.construct(solutionPattern, outboundDFm, inboundDFm),
                                  details::SoloPQItemDeleter<SolutionPatternPQItem>(*this));
}

inline SolutionPatternPQItemPtr
SoloPQItemManager::constructSPPQItem(const SolutionPatternPQItem& other,
                                     const LegPosition expandedLegPos)
{
  return SolutionPatternPQItemPtr(_spPQItemPool.construct(other, expandedLegPos),
                                  details::SoloPQItemDeleter<SolutionPatternPQItem>(*this));
}

inline ConxRoutePQItemPtr
SoloPQItemManager::constructCRPQItem(const SolutionPattern& solutionPattern,
                                     const DirFMPathPtr& outboundDFm,
                                     const DirFMPathPtr& inboundDFm)
{
  return ConxRoutePQItemPtr(_crPQItemPool.construct(solutionPattern, outboundDFm, inboundDFm),
                            details::SoloPQItemDeleter<ConxRoutePQItem>(*this));
}

inline ConxRoutePQItemPtr
SoloPQItemManager::constructCRPQItem(const ConxRoutePQItem& other, const LegPosition expandedLegP)
{
  return ConxRoutePQItemPtr(_crPQItemPool.construct(other, expandedLegP),
                            details::SoloPQItemDeleter<ConxRoutePQItem>(*this));
}

inline ConxRouteCxrPQItemPtr
SoloPQItemManager::constructCRCPQItem(const ConxRoutePQItem& crPQItem,
                                      SoloTrxData& soloTrxData)
{
  return ConxRouteCxrPQItemPtr(_crcPQItemPool.construct(crPQItem, soloTrxData),
                               details::SoloPQItemDeleter<ConxRouteCxrPQItem>(*this));
}

inline FarePathFactoryPQItemPtr
SoloPQItemManager::constructFPFPQItem(const ConxRouteCxrPQItem& crcPQItem,
                                      SoloTrxData& soloTrxData,
                                      PUStruct& puStruct)
{
  return FarePathFactoryPQItemPtr(_fpfPQItemPool.construct(crcPQItem, soloTrxData, puStruct),
                                  details::SoloPQItemDeleter<FarePathFactoryPQItem>(*this));
}

inline FarePathFactoryPQItemPtr
SoloPQItemManager::constructFPFPQItem(const FarePathFactoryPQItem& other, DiagCollector& diag)
{
  return FarePathFactoryPQItemPtr(_fpfPQItemPool.construct(other, diag),
                                  details::SoloPQItemDeleter<FarePathFactoryPQItem>(*this));
}
}
} // namespace tse::shpq

