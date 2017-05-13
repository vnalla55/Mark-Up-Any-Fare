//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/VendorCrossRef.h"

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DeleteList;

typedef HashKey<VendorCode> VendorCrossRefKey;

class VendorCrossRefDAO
    : public DataAccessObject<VendorCrossRefKey, std::vector<VendorCrossRef*>, false>
{
public:
  static VendorCrossRefDAO& instance();
  const VendorCrossRef*
  get(DeleteList& del, const VendorCode& vendor, const DateTime& ticketDate, bool isHistorical);

  bool translateKey(const ObjectKey& objectKey, VendorCrossRefKey& key) const override
  {
    return key.initialized = objectKey.getValue("VENDOR", key._a);
  }

  VendorCrossRefKey createKey(VendorCrossRef* info);

  void translateKey(const VendorCrossRefKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("VENDOR", key._a);
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    return DummyObjectInserter<VendorCrossRef, VendorCrossRefDAO>(flatKey, objectKey).success();
  }

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

protected:
  static std::string _name;
  friend class DAOHelper<VendorCrossRefDAO>;
  static DAOHelper<VendorCrossRefDAO> _helper;
  VendorCrossRefDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<VendorCrossRefKey, std::vector<VendorCrossRef*>, false>(cacheSize, cacheType)
  {
  }
  virtual void load() override;
  std::vector<VendorCrossRef*>* create(VendorCrossRefKey key) override;
  void destroy(VendorCrossRefKey key, std::vector<VendorCrossRef*>* t) override;

private:
  static VendorCrossRefDAO* _instance;
  static log4cxx::LoggerPtr _logger;
  static VendorCrossRef* ATPVendor;
  static VendorCrossRef* SITAVendor;
};

namespace
{
const VendorCode VENDOR_ATP = "ATP";
const VendorCode VENDOR_SITA = "SITA";
}

inline VendorCode
getRCVendorData(const VendorCode& vendor,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (vendor == VENDOR_ATP || vendor == VENDOR_SITA)
  {
    return vendor;
  }

  VendorCrossRefDAO& dao = VendorCrossRefDAO::instance();
  const VendorCrossRef* vxr = dao.get(deleteList, vendor, ticketDate, isHistorical);

  return vxr ? vxr->ruleCategoryVendor() : vendor;
}

inline VendorCode
getTCRVendorData(const VendorCode& vendor,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (vendor == VENDOR_ATP || vendor == VENDOR_SITA)
  {
    return vendor;
  }

  VendorCrossRefDAO& dao = VendorCrossRefDAO::instance();
  const VendorCrossRef* vxr = dao.get(deleteList, vendor, ticketDate, isHistorical);

  return vxr ? vxr->tariffCrossRefVendor() : vendor;
}

inline char
getVendorTypeData(const VendorCode& vendor,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (vendor == VENDOR_ATP || vendor == VENDOR_SITA)
  {
    return 'P';
  }

  VendorCrossRefDAO& dao = VendorCrossRefDAO::instance();
  const VendorCrossRef* vxr = dao.get(deleteList, vendor, ticketDate, isHistorical);
  if (vxr == nullptr)
    return '\0';

  return vxr->vendorType();
}

} // namespace tse
