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
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/LocKey.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class MarkupControl;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, int, PseudoCityCode>
MarkupControlKey;

class MarkupControlDAO : public DataAccessObject<MarkupControlKey, std::vector<MarkupControl*> >
{
public:
  static MarkupControlDAO& instance();

  const std::vector<MarkupControl*>& get(DeleteList& del,
                                         const PseudoCityCode& pcc,
                                         const PseudoCityCode& homePCC,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         int seqNo,
                                         long secondarySellerId,
                                         const DateTime& ticketDate);

  const std::vector<MarkupControl*>& get(DeleteList& del,
                                         const PseudoCityCode& pcc,
                                         const PseudoCityCode& homePCC,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         int seqNo,
                                         const DateTime& ticketDate);

  const std::vector<MarkupControl*>& get(DeleteList& del,
                                         const PseudoCityCode& pcc,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         int seqNo,
                                         const DateTime& ticketDate);

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  bool translateKey(const ObjectKey& objectKey, MarkupControlKey& key) const override;

  MarkupControlKey createKey(MarkupControl* info);

  void translateKey(const MarkupControlKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("SEQNO", key._e);
    objectKey.setValue("OWNERPSEUDOCITY", key._f);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<MarkupControl, MarkupControlDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MarkupControl*>* vect) const override;

  virtual std::vector<MarkupControl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MarkupControlDAO>;
  static DAOHelper<MarkupControlDAO> _helper;
  MarkupControlDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MarkupControlKey, std::vector<MarkupControl*> >(cacheSize, cacheType, 7)
  {
  }
  void load() override;
  std::vector<MarkupControl*>* create(MarkupControlKey key) override;
  void destroy(MarkupControlKey key, std::vector<MarkupControl*>* t) override;

private:
  struct CheckPCC;
  static MarkupControlDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

typedef HashKey<VendorCode,
                CarrierCode,
                TariffNumber,
                RuleNumber,
                int,
                PseudoCityCode,
                DateTime,
                DateTime> MarkupControlHistoricalKey;

class MarkupControlHistoricalDAO
    : public HistoricalDataAccessObject<MarkupControlHistoricalKey, std::vector<MarkupControl*> >
{
public:
  static MarkupControlHistoricalDAO& instance();

  const std::vector<MarkupControl*>& get(DeleteList& del,
                                         const PseudoCityCode& pcc,
                                         const PseudoCityCode& homePCC,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         int seqNo,
                                         long secondarySellerId,
                                         const DateTime& ticketDate);

  const std::vector<MarkupControl*>& get(DeleteList& del,
                                         const PseudoCityCode& pcc,
                                         const PseudoCityCode& homePCC,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         int seqNo,
                                         const DateTime& ticketDate);

  const std::vector<MarkupControl*>& get(DeleteList& del,
                                         const PseudoCityCode& pcc,
                                         const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& ruleTariff,
                                         const RuleNumber& rule,
                                         int seqNo,
                                         const DateTime& ticketDate);

  virtual size_t invalidate(const ObjectKey& objectKey) override;

  bool translateKey(const ObjectKey& objectKey, MarkupControlHistoricalKey& key) const override;

  bool translateKey(const ObjectKey& objectKey,
                    MarkupControlHistoricalKey& key,
                    const DateTime ticketDate) const override;

  MarkupControlHistoricalKey createKey(MarkupControl* info,
                                       const DateTime& startDate = DateTime::emptyDate(),
                                       const DateTime& endDate = DateTime::emptyDate());

  void translateKey(const MarkupControlHistoricalKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
    objectKey.setValue("SEQNO", key._e);
    objectKey.setValue("OWNERPSEUDOCITY", key._f);
    objectKey.setValue("STARTDATE", key._g);
    objectKey.setValue("ENDDATE", key._h);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserterWithDateRange<MarkupControl, MarkupControlHistoricalDAO>(
               flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  void setKeyDateRange(MarkupControlHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

  virtual sfc::CompressedData* compress(const std::vector<MarkupControl*>* vect) const override;

  virtual std::vector<MarkupControl*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MarkupControlHistoricalDAO>;
  static DAOHelper<MarkupControlHistoricalDAO> _helper;
  MarkupControlHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MarkupControlHistoricalKey, std::vector<MarkupControl*> >(
          cacheSize, cacheType, 7)
  {
  }
  std::vector<MarkupControl*>* create(MarkupControlHistoricalKey key) override;
  void destroy(MarkupControlHistoricalKey key, std::vector<MarkupControl*>* t) override;
  void load() override;

private:
  struct CheckPCC;
  static MarkupControlHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
