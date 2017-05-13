//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/FootNoteCtrlInfo.h"

namespace tse
{
class FootNoteCtrlInfo;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, Footnote, int> FootNoteCtrlKey;

class FootNoteCtrlDAO : public DataAccessObject<FootNoteCtrlKey, std::vector<FootNoteCtrlInfo*> >
{
  friend class DAOHelper<FootNoteCtrlDAO>;

public:
  static FootNoteCtrlDAO& instance();

  const std::vector<FootNoteCtrlInfo*>& getAll(const VendorCode vendor,
                                               const CarrierCode carrier,
                                               const TariffNumber tariffNumber,
                                               const Footnote footnote,
                                               int category)
  {
    // Track calls for code coverage
    _codeCoverageGetCallCount++;
    return *cache().get(FootNoteCtrlKey(vendor, carrier, tariffNumber, footnote, category));
  }

  const std::vector<FootNoteCtrlInfo*>& get(DeleteList& del,
                                            const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& ruleTariff,
                                            const Footnote& footnote,
                                            const int category,
                                            const DateTime& date,
                                            const DateTime& ticketDate);

  const std::vector<FootNoteCtrlInfo*>& getForFD(DeleteList& del,
                                                 const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber& ruleTariff,
                                                 const Footnote& footnote,
                                                 const int category,
                                                 const DateTime& date,
                                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FootNoteCtrlKey& key) const override
  {
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("FARETARIFF", key._c) &&
                objectKey.getValue("FOOTNOTE", key._d) && objectKey.getValue("CATEGORY", key._e));
  }

  FootNoteCtrlKey createKey(FootNoteCtrlInfo* info);

  void translateKey(const FootNoteCtrlKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("FARETARIFF", key._c);
    objectKey.setValue("FOOTNOTE", key._d);
    objectKey.setValue("CATEGORY", key._e);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<FootNoteCtrlInfo, FootNoteCtrlDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<FootNoteCtrlInfo*>* vect) const override;

  virtual std::vector<FootNoteCtrlInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  FootNoteCtrlDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FootNoteCtrlKey, std::vector<FootNoteCtrlInfo*> >(cacheSize, cacheType, 5)
  {
  }

  std::vector<FootNoteCtrlInfo*>* create(FootNoteCtrlKey key) override;
  void destroy(FootNoteCtrlKey key, std::vector<FootNoteCtrlInfo*>* recs) override;

  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<FootNoteCtrlDAO> _helper;

private:
  static FootNoteCtrlDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode, CarrierCode, TariffNumber, Footnote, int, DateTime, DateTime>
FootNoteCtrlHistoricalKey;

class FootNoteCtrlHistoricalDAO
    : public HistoricalDataAccessObject<FootNoteCtrlHistoricalKey, std::vector<FootNoteCtrlInfo*> >
{
  friend class DAOHelper<FootNoteCtrlHistoricalDAO>;

public:
  static FootNoteCtrlHistoricalDAO& instance();

  const std::vector<FootNoteCtrlInfo*>& get(DeleteList& del,
                                            const VendorCode& vendor,
                                            const CarrierCode& carrier,
                                            const TariffNumber& ruleTariff,
                                            const Footnote& footnote,
                                            int category,
                                            const DateTime& date,
                                            const DateTime& ticketDate);

  const std::vector<FootNoteCtrlInfo*>& getForFD(DeleteList& del,
                                                 const VendorCode& vendor,
                                                 const CarrierCode& carrier,
                                                 const TariffNumber& ruleTariff,
                                                 const Footnote& footnote,
                                                 int category,
                                                 const DateTime& date,
                                                 const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, FootNoteCtrlHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._f);
    objectKey.getValue("ENDDATE", key._g);
    return key.initialized = objectKey.getValue("VENDOR", key._a)
                             && objectKey.getValue("CARRIER", key._b)
                             && objectKey.getValue("FARETARIFF", key._c)
                             && objectKey.getValue("FOOTNOTE", key._d)
                             && objectKey.getValue("CATEGORY", key._e);
  }

  bool translateKey(const ObjectKey& objectKey,
                    FootNoteCtrlHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
    return key.initialized =
               (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
                objectKey.getValue("FARETARIFF", key._c) &&
                objectKey.getValue("FOOTNOTE", key._d) && objectKey.getValue("CATEGORY", key._e));
  }

  void translateKey(const FootNoteCtrlHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("FARETARIFF", key._c);
    objectKey.setValue("FOOTNOTE", key._d);
    objectKey.setValue("CATEGORY", key._e);
    objectKey.setValue("STARTDATE", key._f);
    objectKey.setValue("ENDDATE", key._g);
  }

  FootNoteCtrlHistoricalKey createKey(const FootNoteCtrlInfo* info,
                                      const DateTime& startDate = DateTime::emptyDate(),
                                      const DateTime& endDate = DateTime::emptyDate());

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<FootNoteCtrlInfo, FootNoteCtrlHistoricalDAO>(
               flatKey, objectKey).success();
  }

  void setKeyDateRange(FootNoteCtrlHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<FootNoteCtrlInfo*>* vect) const override;

  virtual std::vector<FootNoteCtrlInfo*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  FootNoteCtrlHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FootNoteCtrlHistoricalKey, std::vector<FootNoteCtrlInfo*> >(
          cacheSize, cacheType, 5)
  {
  }

  std::vector<FootNoteCtrlInfo*>* create(FootNoteCtrlHistoricalKey key) override;
  void destroy(FootNoteCtrlHistoricalKey key, std::vector<FootNoteCtrlInfo*>* t) override;
  void load() override;

  static std::string _name;
  static std::string _cacheClass;
  static DAOHelper<FootNoteCtrlHistoricalDAO> _helper;

private:
  static FootNoteCtrlHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // namespace tse
