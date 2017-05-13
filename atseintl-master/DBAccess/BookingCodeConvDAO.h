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

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class BookingCodeConv;
class DeleteList;

typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, Indicator> BookingCodeConvKey;

class BookingCodeConvDAO
    : public DataAccessObject<BookingCodeConvKey, std::vector<BookingCodeConv*>, false>
{
public:
  static BookingCodeConvDAO& instance();

  const std::vector<BookingCodeConv*>& get(DeleteList& del,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const TariffNumber& ruleTariff,
                                           const RuleNumber& rule,
                                           Indicator conventionNo,
                                           bool& isRuleZero,
                                           const DateTime& ticketDate,
                                           const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, BookingCodeConvKey& key) const override
  {
    if (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
        objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d))
    {
      if (key._c == 0)
      {
        key._e = '1';
      }
      else
      {
        key._e = '2';
      }
      key.initialized = true;
    }
    else
    {
      key.initialized = false;
    }

    return key.initialized;
  }

  BookingCodeConvKey createKey(BookingCodeConv* info);

  void translateKey(const BookingCodeConvKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
    objectKey.setValue("CARRIER", key._b);
    objectKey.setValue("RULETARIFF", key._c);
    objectKey.setValue("RULE", key._d);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<BookingCodeConv, BookingCodeConvDAO>(flatKey, objectKey).success();
  }

  const std::string& cacheClass() override { return _cacheClass; }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  virtual sfc::CompressedData* compress(const std::vector<BookingCodeConv*>* vect) const override;

  virtual std::vector<BookingCodeConv*>*
  uncompress(const sfc::CompressedData& compressed) const override;

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BookingCodeConvDAO>;
  static DAOHelper<BookingCodeConvDAO> _helper;

  BookingCodeConvDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<BookingCodeConvKey, std::vector<BookingCodeConv*>, false>(cacheSize,
                                                                                 cacheType)
  {
  }

  virtual void load() override;

  std::vector<BookingCodeConv*>* create(BookingCodeConvKey key) override;

  void destroy(BookingCodeConvKey key, std::vector<BookingCodeConv*>* t) override;

private:
  static BookingCodeConvDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class BookingCodeConvDAO

// Historical Stuff /////////////////////////////////////////////////////////////////////
typedef HashKey<VendorCode, CarrierCode, TariffNumber, RuleNumber, Indicator, DateTime, DateTime>
BookingCodeConvHistoricalKey;

class BookingCodeConvHistoricalDAO
    : public HistoricalDataAccessObject<BookingCodeConvHistoricalKey,
                                        std::vector<BookingCodeConv*> >
{
public:
  static BookingCodeConvHistoricalDAO& instance();

  const std::vector<BookingCodeConv*>& get(DeleteList& del,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const TariffNumber& ruleTariff,
                                           const RuleNumber& rule,
                                           Indicator conventionNo,
                                           bool& isRuleZero,
                                           const DateTime& ticketDate,
                                           const DateTime& date);

  bool translateKey(const ObjectKey& objectKey, BookingCodeConvHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._f);
    objectKey.getValue("ENDDATE", key._g);
    if (objectKey.getValue("VENDOR", key._a)
        && objectKey.getValue("CARRIER", key._b)
        && objectKey.getValue("RULETARIFF", key._c)
        && objectKey.getValue("RULE", key._d))
    {
      key._e = (key._c == 0) ? '1' : '2';
      return key.initialized = true;
    }
    else
      return key.initialized = false;
  }

  bool translateKey(const ObjectKey& objectKey,
                    BookingCodeConvHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
    if (objectKey.getValue("VENDOR", key._a) && objectKey.getValue("CARRIER", key._b) &&
        objectKey.getValue("RULETARIFF", key._c) && objectKey.getValue("RULE", key._d))
    {
      key._e = (key._c == 0) ? '1' : '2';
      return key.initialized = true;
    }
    else
      return key.initialized = false;
  }

  void setKeyDateRange(BookingCodeConvHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<BookingCodeConvHistoricalDAO>;
  static DAOHelper<BookingCodeConvHistoricalDAO> _helper;

  BookingCodeConvHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<BookingCodeConvHistoricalKey, std::vector<BookingCodeConv*> >(
          cacheSize, cacheType)
  {
  }

  std::vector<BookingCodeConv*>* create(BookingCodeConvHistoricalKey key) override;
  void destroy(BookingCodeConvHistoricalKey key, std::vector<BookingCodeConv*>* t) override;

  virtual sfc::CompressedData* compress(const std::vector<BookingCodeConv*>* vect) const override;

  virtual std::vector<BookingCodeConv*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static BookingCodeConvHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;

}; // class BookingCodeConvHistoricalDAO

} // namespace tse

