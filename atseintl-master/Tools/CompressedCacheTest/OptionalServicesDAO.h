//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include <vector>
#include "TseTypes.h"
#include "HashKey.h"
#include "DataAccessObject.h"

namespace tse
{
  typedef HashKey<int> OptionalServicesKey;

  class OptionalServicesInfo;
  class DeleteList;

  class OptionalServicesDAO : public DataAccessObject<OptionalServicesKey, std::vector<OptionalServicesInfo*>>
  {
  public:
    OptionalServicesDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }
    static OptionalServicesDAO& instance();

    OptionalServicesKey createKey(OptionalServicesInfo * info);

    const std::string& cacheClass()
    {
      return _cacheClass;
    }

  virtual sfc::CompressedData *
    compress (const std::vector<OptionalServicesInfo*> *vect) const;

  virtual std::vector<OptionalServicesInfo*> *
    uncompress (const sfc::CompressedData &compressed) const;

  private:

    std::vector<OptionalServicesInfo*>* create(OptionalServicesKey key);
    void destroy(OptionalServicesKey key, std::vector<OptionalServicesInfo*>* recs);
    void load();

    static std::string _name;
    static std::string _cacheClass;
    static OptionalServicesDAO* _instance;
  };
} // tse
