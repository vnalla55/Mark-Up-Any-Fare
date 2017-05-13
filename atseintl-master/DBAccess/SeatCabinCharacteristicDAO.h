/*
 * SeatCabinCharacteristicDAO.h
 *
 *  Created on: Oct 4, 2012
 *    Author: SG0214951
 */

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class SeatCabinCharacteristicInfo;
class DeleteList;

typedef HashKey<CarrierCode, Indicator> SeatCabinCharacteristicKey;

class SeatCabinCharacteristicDAO
    : public DataAccessObject<SeatCabinCharacteristicKey,
                              std::vector<SeatCabinCharacteristicInfo*> >
{
public:
  static SeatCabinCharacteristicDAO& instance();

  const std::vector<SeatCabinCharacteristicInfo*>& get(DeleteList& del,
                                                       const CarrierCode& carrier,
                                                       const Indicator& codeType,
                                                       const DateTime& travelDate,
                                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SeatCabinCharacteristicKey& key) const override;
  SeatCabinCharacteristicKey createKey(const SeatCabinCharacteristicInfo* info);
  void translateKey(const SeatCabinCharacteristicKey& key, ObjectKey& objectKey) const override;
  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;

  const std::string& cacheClass() override { return _cacheClass; }
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<SeatCabinCharacteristicInfo*>* sccInfoList) const override;

  virtual std::vector<SeatCabinCharacteristicInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SeatCabinCharacteristicDAO>;
  static DAOHelper<SeatCabinCharacteristicDAO> _helper;
  SeatCabinCharacteristicDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SeatCabinCharacteristicKey, std::vector<SeatCabinCharacteristicInfo*> >(
          cacheSize, cacheType, 3)
  {
  }
  std::vector<SeatCabinCharacteristicInfo*>* create(SeatCabinCharacteristicKey key) override;
  void destroy(SeatCabinCharacteristicKey key,
               std::vector<SeatCabinCharacteristicInfo*>* sccInfoList) override;

  void load() override;
  virtual size_t clear() override;

private:
  struct isTravelDate;
  static SeatCabinCharacteristicDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// Historical Stuff /////////////////////////////////////////////////////////////
typedef HashKey<CarrierCode, Indicator, DateTime, DateTime> SeatCabinCharacteristicHistoricalKey;

class SeatCabinCharacteristicHistoricalDAO
    : public HistoricalDataAccessObject<SeatCabinCharacteristicHistoricalKey,
                                        std::vector<SeatCabinCharacteristicInfo*> >
{
public:
  static SeatCabinCharacteristicHistoricalDAO& instance();

  const std::vector<SeatCabinCharacteristicInfo*>& get(DeleteList& del,
                                                       const CarrierCode& carrier,
                                                       const Indicator& codeType,
                                                       const DateTime& travelDate,
                                                       const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey,
                    SeatCabinCharacteristicHistoricalKey& key) const override;
  bool translateKey(const ObjectKey& objectKey,
                    SeatCabinCharacteristicHistoricalKey& key,
                    const DateTime ticketDate) const override;
  void translateKey(const SeatCabinCharacteristicHistoricalKey& key,
                    ObjectKey& objectKey) const override;
  void setKeyDateRange(SeatCabinCharacteristicHistoricalKey& key,
                       const DateTime ticketDate) const override;

  const std::string& cacheClass() override { return _cacheClass; }

  SeatCabinCharacteristicHistoricalKey createKey(const SeatCabinCharacteristicInfo* info,
                                                 const DateTime& startDate = DateTime::emptyDate(),
                                                 const DateTime& endDate = DateTime::emptyDate());

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override;
  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData*
  compress(const std::vector<SeatCabinCharacteristicInfo*>* sccInfoList) const override;

  virtual std::vector<SeatCabinCharacteristicInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SeatCabinCharacteristicHistoricalDAO>;
  static DAOHelper<SeatCabinCharacteristicHistoricalDAO> _helper;

  SeatCabinCharacteristicHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<SeatCabinCharacteristicHistoricalKey,
                                 std::vector<SeatCabinCharacteristicInfo*> >(
          cacheSize, cacheType, 3)
  {
  }

  std::vector<SeatCabinCharacteristicInfo*>*
  create(SeatCabinCharacteristicHistoricalKey key) override;
  void destroy(SeatCabinCharacteristicHistoricalKey key,
               std::vector<SeatCabinCharacteristicInfo*>* sccInfoList) override;
  void load() override;

private:
  struct isTravelDate;
  static SeatCabinCharacteristicHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse

