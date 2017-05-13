//-------------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
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
class ReissueSequence;
class DeleteList;

typedef HashKey<VendorCode, int> ReissueSequenceKey;

class ReissueSequenceDAO
    : public DataAccessObject<ReissueSequenceKey, std::vector<ReissueSequence*> >
{
public:
  static ReissueSequenceDAO& instance();
  std::vector<ReissueSequence*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ReissueSequenceKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ReissueSequenceDAO>;
  static DAOHelper<ReissueSequenceDAO> _helper;
  ReissueSequenceDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ReissueSequenceKey, std::vector<ReissueSequence*> >(cacheSize, cacheType, 4)
  {
  }
  std::vector<ReissueSequence*>* create(ReissueSequenceKey key) override;
  void destroy(ReissueSequenceKey key, std::vector<ReissueSequence*>* t) override;

private:
  static ReissueSequenceDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class ReissueSequenceDAO

// --------------------------------------------------
// Historical DAO: ReissueSequenceHistoricalDAO
// --------------------------------------------------
typedef HashKey<VendorCode, int, DateTime, DateTime> ReissueSequenceHistoricalKey;

class ReissueSequenceHistoricalDAO
    : public HistoricalDataAccessObject<ReissueSequenceHistoricalKey,
                                        std::vector<ReissueSequence*> >
{
public:
  static ReissueSequenceHistoricalDAO& instance();
  std::vector<ReissueSequence*>&
  get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, ReissueSequenceHistoricalKey& key) const override
  {
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
               objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
  }

  bool translateKey(const ObjectKey& objectKey,
                    ReissueSequenceHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
  }

  void setKeyDateRange(ReissueSequenceHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<ReissueSequenceHistoricalDAO>;
  static DAOHelper<ReissueSequenceHistoricalDAO> _helper;
  ReissueSequenceHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<ReissueSequenceHistoricalKey, std::vector<ReissueSequence*> >(
          cacheSize, cacheType, 4)
  {
  }
  std::vector<ReissueSequence*>* create(ReissueSequenceHistoricalKey key) override;
  void destroy(ReissueSequenceHistoricalKey key, std::vector<ReissueSequence*>* t) override;

private:
  static ReissueSequenceHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
}; // class ReissueSequenceHistoricalDAO
} // namespace tse
