//----------------------------------------------------------------------------
//  File:        BoundFareDAO.h
//  Created:     2009-01-01
//
//  Description: Bound Fare DAO class
//
//  Updates:
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

#pragma once

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/FareInfo.h"

namespace tse
{
class DeleteList;

typedef HashKey<LocCode, LocCode, CarrierCode> FareKey;
typedef std::vector<const FareInfo*> FareInfoVec;
typedef sfc::Cache<FareKey, FareInfoVec> BoundFareCache;

class BoundFareDAO : public DataAccessObject<FareKey, std::vector<const FareInfo*> >
{
  friend class DAOHelper<BoundFareDAO>;

public:
  virtual ~BoundFareDAO();

  const std::string& cacheClass() override;

  void load() override;

  bool translateKey(const ObjectKey& objectKey, FareKey& key) const override;

  FareKey createKey(const FareInfo* info);

  void translateKey(const FareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("MARKET1", key._a);
    objectKey.setValue("MARKET2", key._b);
    objectKey.setValue("CARRIER", key._c);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FareInfo, BoundFareDAO>(flatKey, objectKey).success();
  }

  const std::vector<const FareInfo*>& get(DeleteList& del,
                                          const LocCode& market1,
                                          const LocCode& market2,
                                          const CarrierCode& cxr,
                                          const DateTime& startDate,
                                          const DateTime& endDate,
                                          const DateTime& ticketDate,
                                          bool fareDisplay = false);

  static BoundFareDAO& instance();

  static void checkForUpdates();

private:
  BoundFareDAO(int cacheSize = 0, const std::string& cacheType = "");

  std::vector<const FareInfo*>* create(FareKey key) override;

  void destroy(FareKey key, std::vector<const FareInfo*>* t) override;

  static log4cxx::LoggerPtr _logger;
  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<BoundFareDAO> _helper;
  static BoundFareDAO* _instance;
  static std::string _dataDir;
  static int32_t _latestGeneration;
};

} // namespace tse

