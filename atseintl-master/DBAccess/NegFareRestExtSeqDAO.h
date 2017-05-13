//-------------------------------------------------------------------------------
// @ 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

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
class NegFareRestExtSeq;
class DeleteList;

typedef HashKey<VendorCode, int> NegFareRestExtSeqKey;

class NegFareRestExtSeqDAO
    : public DataAccessObject<NegFareRestExtSeqKey, std::vector<NegFareRestExtSeq*> >
{
public:
  static NegFareRestExtSeqDAO& instance();
  std::vector<NegFareRestExtSeq*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareRestExtSeqKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareRestExtSeqDAO>;
  static DAOHelper<NegFareRestExtSeqDAO> _helper;
  NegFareRestExtSeqDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<NegFareRestExtSeqKey, std::vector<NegFareRestExtSeq*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<NegFareRestExtSeq*>* create(NegFareRestExtSeqKey key) override;
  void destroy(NegFareRestExtSeqKey key, std::vector<NegFareRestExtSeq*>* t) override;

  virtual sfc::CompressedData* compress(const std::vector<NegFareRestExtSeq*>* vect) const override;

  virtual std::vector<NegFareRestExtSeq*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static NegFareRestExtSeqDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NegFareRestExtSeqDAO

// --------------------------------------------------
// Historical DAO: NegFareRestExtSeqHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> NegFareRestExtSeqHistoricalKey;

class NegFareRestExtSeqHistoricalDAO
    : public HistoricalDataAccessObject<NegFareRestExtSeqHistoricalKey,
                                        std::vector<NegFareRestExtSeq*> >
{
public:
  static NegFareRestExtSeqHistoricalDAO& instance();
  std::vector<NegFareRestExtSeq*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, NegFareRestExtSeqHistoricalKey& key) const override
  {
    objectKey.getValue("STARTDATE", key._c);
    objectKey.getValue("ENDDATE", key._d);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  bool translateKey(const ObjectKey& objectKey,
                    NegFareRestExtSeqHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void
  setKeyDateRange(NegFareRestExtSeqHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<NegFareRestExtSeqHistoricalDAO>;
  static DAOHelper<NegFareRestExtSeqHistoricalDAO> _helper;
  NegFareRestExtSeqHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<NegFareRestExtSeqHistoricalKey, std::vector<NegFareRestExtSeq*> >(
          cacheSize, cacheType, 2)
  {
  }
  std::vector<NegFareRestExtSeq*>* create(NegFareRestExtSeqHistoricalKey key) override;
  void destroy(NegFareRestExtSeqHistoricalKey key, std::vector<NegFareRestExtSeq*>* t) override;

  virtual sfc::CompressedData* compress(const std::vector<NegFareRestExtSeq*>* vect) const override;

  virtual std::vector<NegFareRestExtSeq*>*
  uncompress(const sfc::CompressedData& compressed) const override;

private:
  static NegFareRestExtSeqHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class NegFareRestExtSeqHistoricalDAO
} // namespace tse
