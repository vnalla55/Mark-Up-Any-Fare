#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

class CustomerSecurityHandshakeInfo;
class DeleteList;

typedef HashKey<PseudoCityCode, Code<8> > CustomerSecurityHandshakeKey;

class CustomerSecurityHandshakeDAO
    : public DataAccessObject<CustomerSecurityHandshakeKey, std::vector<CustomerSecurityHandshakeInfo*> >
{
public:
  static CustomerSecurityHandshakeDAO& instance();

  const std::vector<CustomerSecurityHandshakeInfo*>&
  get(DeleteList& del,
      const PseudoCityCode& pcc,
      const Code<8>& productCD,
      const DateTime& dateTime);

  bool translateKey(const ObjectKey& objectKey, CustomerSecurityHandshakeKey& key) const override
  {
    return key.initialized = objectKey.getValue("SECURITYSOURCEPCC", key._a)
                             && objectKey.getValue("PRODUCTCD", key._b);
  }

  CustomerSecurityHandshakeKey createKey(const CustomerSecurityHandshakeInfo* info);

  void translateKey(const CustomerSecurityHandshakeKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SECURITYSOURCEPCC", key._a);
    objectKey.setValue("PRODUCTCD", key._b);
  }

  virtual sfc::CompressedData*
  compress(const std::vector<CustomerSecurityHandshakeInfo*>* vect) const override;

  virtual std::vector<CustomerSecurityHandshakeInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CustomerSecurityHandshakeDAO>;
  static DAOHelper<CustomerSecurityHandshakeDAO> _helper;
  CustomerSecurityHandshakeDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<CustomerSecurityHandshakeKey, std::vector<CustomerSecurityHandshakeInfo*> >(
          cacheSize, cacheType)
  {
  }
  virtual std::vector<CustomerSecurityHandshakeInfo*>*
  create(CustomerSecurityHandshakeKey key) override;
  void destroy(CustomerSecurityHandshakeKey key,
               std::vector<CustomerSecurityHandshakeInfo*>* recs) override;

private:
  static CustomerSecurityHandshakeDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical
typedef HashKey<PseudoCityCode, Code<8>, DateTime, DateTime> CustomerSecurityHandshakeHistoricalKey;

class CustomerSecurityHandshakeHistoricalDAO
    : public HistoricalDataAccessObject<CustomerSecurityHandshakeHistoricalKey,
                                        std::vector<CustomerSecurityHandshakeInfo*> >
{
public:
  static CustomerSecurityHandshakeHistoricalDAO& instance();

  const std::vector<CustomerSecurityHandshakeInfo*>&
  get(DeleteList& del,
      const PseudoCityCode& pcc,
      const Code<8>& productCD,
      const DateTime& dateTime);

  bool translateKey(const ObjectKey& objectKey, CustomerSecurityHandshakeHistoricalKey& key) const
      override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized = objectKey.getValue("SECURITYSOURCEPCC", key._a)
                             && objectKey.getValue("PRODUCTCD", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    CustomerSecurityHandshakeHistoricalKey& key,
                    const DateTime dateTime) const override
  {
    DAOUtils::getDateRange(dateTime, key._c, key._d, _cacheBy);
    return key.initialized = objectKey.getValue("SECURITYSOURCEPCC", key._a)
                             && objectKey.getValue("PRODUCTCD", key._b);
  }

  void translateKey(const CustomerSecurityHandshakeHistoricalKey& key, ObjectKey& objectKey) const
      override
  {
    objectKey.setValue("SECURITYSOURCEPCC", key._a);
    objectKey.setValue("PRODUCTCD", key._b);
    objectKey.setValue("STARTDATE", key._c);
    objectKey.setValue("ENDDATE", key._d);
  }

  void setKeyDateRange(CustomerSecurityHandshakeHistoricalKey& key, const DateTime dateTime) const
      override
  {
    DAOUtils::getDateRange(dateTime, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  CustomerSecurityHandshakeHistoricalKey createKey(const CustomerSecurityHandshakeInfo* info,
                                                   const DateTime& startDate = DateTime::emptyDate(),
                                                   const DateTime& endDate = DateTime::emptyDate());
  virtual sfc::CompressedData*
  compress(const std::vector<CustomerSecurityHandshakeInfo*>* vect) const override;

  virtual std::vector<CustomerSecurityHandshakeInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<CustomerSecurityHandshakeHistoricalDAO>;
  static DAOHelper<CustomerSecurityHandshakeHistoricalDAO> _helper;

  CustomerSecurityHandshakeHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<CustomerSecurityHandshakeHistoricalKey,
                                 std::vector<CustomerSecurityHandshakeInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<CustomerSecurityHandshakeInfo*>*
  create(CustomerSecurityHandshakeHistoricalKey key) override;
  void destroy(CustomerSecurityHandshakeHistoricalKey key,
               std::vector<CustomerSecurityHandshakeInfo*>* recs) override;

private:
  static CustomerSecurityHandshakeHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
