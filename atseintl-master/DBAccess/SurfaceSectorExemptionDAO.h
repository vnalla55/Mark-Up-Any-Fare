#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/SurfaceSectorExemptionInfo.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

/*
 * DAO for SurfaceSectorExemptionInfo.
 */
class SurfaceSectorExemptionDAO
    : public DataAccessObject<CarrierKey, std::vector<SurfaceSectorExemptionInfo*>, false>
{
public:
  static SurfaceSectorExemptionDAO& instance()
  {
    if (_instance == nullptr)
    {
      _helper.init();
    }
    return *_instance;
  }

  const std::vector<SurfaceSectorExemptionInfo*>&
  get(DeleteList& del, const CarrierCode& validatingCarrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return key.initialized = objectKey.getValue("VALIDATINGCARRIER", key._a);
  }

  void translateKey(const CarrierKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VALIDATINGCARRIER", key._a);
  }

  const std::string& name() const override { return _name; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<SurfaceSectorExemptionDAO>;

  static DAOHelper<SurfaceSectorExemptionDAO> _helper;

  SurfaceSectorExemptionDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CarrierKey, std::vector<SurfaceSectorExemptionInfo*>, false>(cacheSize,
                                                                                    cacheType)
  {
  }

  std::vector<SurfaceSectorExemptionInfo*>* create(CarrierKey key) override;

  void destroy(CarrierKey key, std::vector<SurfaceSectorExemptionInfo*>* recs) override;

private:
  static SurfaceSectorExemptionDAO* _instance;

  static log4cxx::LoggerPtr _logger;
};

/*
 * DAO for historical SurfaceSectorExceptionInfo.
 */
class SurfaceSectorExemptionHistoricalDAO
    : public HistoricalDataAccessObject<CarrierKey, std::vector<SurfaceSectorExemptionInfo*>, false>
{
public:
  static SurfaceSectorExemptionHistoricalDAO& instance()
  {
    if (_instance == nullptr)
    {
      _helper.init();
    }
    return *_instance;
  }

  const std::vector<SurfaceSectorExemptionInfo*>&
  get(DeleteList& del, const CarrierCode& validatingCarrier, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, CarrierKey& key) const override
  {
    return objectKey.getValue("VALIDATINGCARRIER", key._a);
  }

  const std::string& name() const override { return _name; }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<SurfaceSectorExemptionHistoricalDAO>;

  static DAOHelper<SurfaceSectorExemptionHistoricalDAO> _helper;

  SurfaceSectorExemptionHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CarrierKey, std::vector<SurfaceSectorExemptionInfo*>, false>(
          cacheSize, cacheType)
  {
  }

  std::vector<SurfaceSectorExemptionInfo*>* create(CarrierKey key) override;

  void destroy(CarrierKey key, std::vector<SurfaceSectorExemptionInfo*>* recs) override;

private:
  static SurfaceSectorExemptionHistoricalDAO* _instance;

  static log4cxx::LoggerPtr _logger;
};
}
