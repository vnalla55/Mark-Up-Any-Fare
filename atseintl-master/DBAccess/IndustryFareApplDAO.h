//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class IndustryFareAppl;
class DeleteList;

typedef HashKey<Indicator, CarrierCode> IndustryFareApplKey;

class IndustryFareApplDAO
    : public DataAccessObject<IndustryFareApplKey, std::vector<IndustryFareAppl*>, false>
{
public:
  static IndustryFareApplDAO& instance();
  const std::vector<const IndustryFareAppl*>& get(DeleteList& del,
                                                  const Indicator& selectionType,
                                                  const CarrierCode& carrier,
                                                  const DateTime& date,
                                                  const DateTime& ticketDate);
  const std::vector<const IndustryFareAppl*>& get(DeleteList& del,
                                                  const Indicator& selectionType,
                                                  const CarrierCode& carrier,
                                                  const DateTime& startDate,
                                                  const DateTime& endDate,
                                                  const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IndustryFareApplKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("SELECTIONTYPE", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  IndustryFareApplKey createKey(IndustryFareAppl* info);

  void translateKey(const IndustryFareApplKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("SELECTIONTYPE", key._a);
    objectKey.setValue("CARRIER", key._b);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<IndustryFareAppl, IndustryFareApplDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<IndustryFareAppl*>* vect) const override;

  virtual std::vector<IndustryFareAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IndustryFareApplDAO>;
  static DAOHelper<IndustryFareApplDAO> _helper;
  IndustryFareApplDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<IndustryFareApplKey, std::vector<IndustryFareAppl*>, false>(
          cacheSize, cacheType, 2)
  {
  }

  std::vector<IndustryFareAppl*>* create(IndustryFareApplKey key) override;
  void destroy(IndustryFareApplKey key, std::vector<IndustryFareAppl*>* recs) override;

  virtual void load() override;

private:
  static IndustryFareApplDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: IndustryFareApplHistoricalDAO
// --------------------------------------------------
typedef HashKey<Indicator, CarrierCode, DateTime, DateTime> IndustryFareApplHistoricalKey;

class IndustryFareApplHistoricalDAO
    : public HistoricalDataAccessObject<IndustryFareApplHistoricalKey,
                                        std::vector<IndustryFareAppl*>,
                                        false>
{
public:
  static IndustryFareApplHistoricalDAO& instance();
  const std::vector<const IndustryFareAppl*>& get(DeleteList& del,
                                                  const Indicator& selectionType,
                                                  const CarrierCode& carrier,
                                                  const DateTime& date,
                                                  const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, IndustryFareApplHistoricalKey& key) const override
  {
    return key.initialized = objectKey.getValue("SELECTIONTYPE", key._a) &&
                             objectKey.getValue("CARRIER", key._b) &&
                             objectKey.getValue("STARTDATE", key._c) &&
                             objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    IndustryFareApplHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("SELECTIONTYPE", key._a) && objectKey.getValue("CARRIER", key._b);
  }

  void setKeyDateRange(IndustryFareApplHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<IndustryFareAppl*>* vect) const override;

  virtual std::vector<IndustryFareAppl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<IndustryFareApplHistoricalDAO>;
  static DAOHelper<IndustryFareApplHistoricalDAO> _helper;
  IndustryFareApplHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<IndustryFareApplHistoricalKey,
                                 std::vector<IndustryFareAppl*>,
                                 false>(cacheSize, cacheType, 2)
  {
  }

  std::vector<IndustryFareAppl*>* create(IndustryFareApplHistoricalKey key) override;
  void destroy(IndustryFareApplHistoricalKey key, std::vector<IndustryFareAppl*>* recs) override;

private:
  static IndustryFareApplHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
