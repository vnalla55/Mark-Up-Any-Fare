/*
 *
 * Cat05OverrideCarrierDAO.h
 *
 *  Author:sg216934
 */
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class Cat05OverrideCarrier;
class DeleteList;

typedef HashKey<PseudoCityCode> Cat05OverrideKey;

class Cat05OverrideCarrierDAO
    : public DataAccessObject<Cat05OverrideKey, std::vector<Cat05OverrideCarrier*> >
{
public:
  static Cat05OverrideCarrierDAO& instance();

  const std::vector<Cat05OverrideCarrier*>& get(DeleteList& del, const PseudoCityCode& pcc);

  bool translateKey(const ObjectKey& objectKey, Cat05OverrideKey& key) const override;

  Cat05OverrideKey createKey(const Cat05OverrideCarrier* info);
  void translateKey(const Cat05OverrideKey& key, ObjectKey& objectKey) const override;
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<Cat05OverrideCarrierDAO>;
  static DAOHelper<Cat05OverrideCarrierDAO> _helper;
  Cat05OverrideCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<Cat05OverrideKey, std::vector<Cat05OverrideCarrier*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<Cat05OverrideCarrier*>* create(Cat05OverrideKey key) override;
  void destroy(Cat05OverrideKey key, std::vector<Cat05OverrideCarrier*>* recs) override;
  void load() override;
  virtual size_t clear() override;

private:
  static Cat05OverrideCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // tse
