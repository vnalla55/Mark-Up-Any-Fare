//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
typedef HashKey<int> NegFareRestKey;
class NegFareRest;
class DeleteList;

class NegFareRestDAO : public DataAccessObject<NegFareRestKey, std::vector<NegFareRest*>>
{
public:
    static NegFareRestDAO& instance();
    NegFareRestDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }
    const NegFareRest* get(DeleteList& del,
                const VendorCode& vendor,
               int itemNo,
               const DateTime& ticketDate);

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData *
      compress (const std::vector<NegFareRest*> *vect) const;

    virtual std::vector<NegFareRest*> *
      uncompress (const sfc::CompressedData &compressed) const;

protected:
    static std::string _name;
    static std::string _cacheClass;
    void load() ;
    std::vector<NegFareRest*>* create(NegFareRestKey key);
    void destroy(NegFareRestKey key, std::vector<NegFareRest*>* t);

private:
    static NegFareRestDAO* _instance;
};

} // namespace tse
