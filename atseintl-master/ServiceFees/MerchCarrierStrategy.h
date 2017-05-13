/*
 * MerchCarrierStrategy.h
 *
 *  Created on: 12-12-2011
 *      Author: SG0926655
 */

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/SubCodeInfo.h"

#include <boost/bind.hpp>

namespace tse
{

class MerchCarrierStrategy
{
public:
  MerchCarrierStrategy() {}
  virtual ~MerchCarrierStrategy() {}
  virtual void setPreferedVendor(VendorCode& vendor) { _preferedVendor = vendor; }
  VendorCode getPreferedVendor() const { return _preferedVendor; }
  virtual void getSubCode(DataHandle& dataHandle,
                          std::vector<SubCodeInfo*>& subCodes,
                          const CarrierCode& carrier,
                          const ServiceTypeCode& srvTypeCode,
                          const ServiceGroup& groupCode,
                          const DateTime& travelDate) const = 0;

  virtual const std::vector<OptionalServicesInfo*>&
  getOptionalServicesInfo(DataHandle& dataHandle,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const LocCode& loc1,
                          const LocCode& loc2,
                          const ServiceTypeCode& serviceTypeCode,
                          const ServiceSubTypeCode& serviceSubTypeCode,
                          Indicator fltTktMerchInd,
                          const DateTime& date) const = 0;

protected:
  VendorCode _preferedVendor;
};

class SingleSegmentStrategy : public MerchCarrierStrategy
{
public:
  SingleSegmentStrategy() {}
  ~SingleSegmentStrategy() {}

  void getSubCode(DataHandle& dataHandle,
                  std::vector<SubCodeInfo*>& subCodes,
                  const CarrierCode& carrier,
                  const ServiceTypeCode& srvTypeCode,
                  const ServiceGroup& groupCode,
                  const DateTime& travelDate) const override
  {
    subCodes = dataHandle.getSubCode(_preferedVendor, carrier, srvTypeCode, groupCode, travelDate);
  }

  const std::vector<OptionalServicesInfo*>&
  getOptionalServicesInfo(DataHandle& dataHandle,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const LocCode& loc1,
                          const LocCode& loc2,
                          const ServiceTypeCode& serviceTypeCode,
                          const ServiceSubTypeCode& serviceSubTypeCode,
                          Indicator fltTktMerchInd,
                          const DateTime& date) const override
  {
    return dataHandle.getOptionalServicesMktInfo(
        vendor, carrier, loc1, loc2, serviceTypeCode, serviceSubTypeCode, fltTktMerchInd, date);
  }
};

class MultipleSegmentStrategy : public MerchCarrierStrategy
{
protected:
  static void copyUnique(std::vector<SubCodeInfo*>* subCodes, SubCodeInfo* newElement)
  {
    std::vector<SubCodeInfo*>::iterator i = subCodes->begin();
    std::vector<SubCodeInfo*>::iterator ie = subCodes->end();

    for (; i != ie; i++)
    {
      if ((*i)->serviceSubTypeCode() == newElement->serviceSubTypeCode() &&
          (*i)->fltTktMerchInd() == newElement->fltTktMerchInd())
      {
        return;
      }
    }
    subCodes->push_back(newElement);
  }

public:
  friend class ServiceFeesGroupTest;
  MultipleSegmentStrategy() {}
  ~MultipleSegmentStrategy() {}

  void getSubCode(DataHandle& dataHandle,
                  std::vector<SubCodeInfo*>& subCodes,
                  const CarrierCode& carrier,
                  const ServiceTypeCode& srvTypeCode,
                  const ServiceGroup& groupCode,
                  const DateTime& travelDate) const override
  {
    subCodes =
        dataHandle.getSubCode(ATPCO_VENDOR_CODE, carrier, srvTypeCode, groupCode, travelDate);
    const std::vector<SubCodeInfo*>& mmSubCodes = dataHandle.getSubCode(
        MERCH_MANAGER_VENDOR_CODE, carrier, srvTypeCode, groupCode, travelDate);
    for_each(mmSubCodes.begin(),
             mmSubCodes.end(),
             boost::bind<void>(&MultipleSegmentStrategy::copyUnique, &subCodes, _1));
  }

  const std::vector<OptionalServicesInfo*>&
  getOptionalServicesInfo(DataHandle& dataHandle,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const LocCode& loc1,
                          const LocCode& loc2,
                          const ServiceTypeCode& serviceTypeCode,
                          const ServiceSubTypeCode& serviceSubTypeCode,
                          Indicator fltTktMerchInd,
                          const DateTime& date) const override
  {
    return dataHandle.getOptionalServicesInfo(
        vendor, carrier, serviceTypeCode, serviceSubTypeCode, fltTktMerchInd, date);
  }
};
}

