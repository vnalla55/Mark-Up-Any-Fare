//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "AddonConstruction/ConstructedCacheDataWrapper.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "Common/Logger.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/LDCHelper.h"


#include <memory>

namespace tse
{

struct CacheStats;

class AddOnCacheControl : public CacheControl
{
public:
  AddOnCacheControl();

  virtual std::string getCacheType() const override;

  virtual const std::string& getID() const override;

  virtual bool
  compareMemCacheToLDC(std::vector<std::string>& results, uint32_t& numCompared) override;

  virtual void getAllFlatKeys(std::set<std::string>& list, bool inclValues = false) override;

  virtual void invalidate(const std::string& flatKey) override;

  virtual bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  virtual bool objectExistsInMem(const std::string& flatKey) override;

  virtual bool objectExistsInLDC(const std::string& flatKey, time_t& timestamp) override;

  virtual void emptyTrash() override;

  static void classInit();

  void init(bool preLoad = true) override;

  void store(const ObjectKey& key) override{/*unused API*/
  };

  virtual size_t invalidate(const ObjectKey& key) override;

  virtual size_t clear() override;

  uint64_t accessCount() override;

  uint64_t readCount() override;

  uint64_t cacheMax() override;

  uint64_t cacheSize() override;

  virtual CacheStats* cacheStats() override;

  virtual size_t getCacheMemory(size_t& used, size_t& indirect, size_t& item_size) override;

  virtual uint32_t tableVersion() const override;

  virtual size_t actionQueueSize() const override;

  virtual bool processNextAction(LDCOperationCounts& counts) override;

  virtual bool ldcEnabled() const override;

  virtual void getMemObjectAsFlat(const std::string& flatKey, std::string& result) override;

  virtual void getLDCObjectAsFlat(const std::string& flatKey, std::string& result) override;

  log4cxx::LoggerPtr& logger();

private:
  std::string _id;
  LDCHelper<ConstructedCacheManager,
            CacheKey,
            ConstructedCacheDataWrapper,
            std::shared_ptr<ConstructedCacheDataWrapper>> _ldcHelper;
};

} // namespace tse
