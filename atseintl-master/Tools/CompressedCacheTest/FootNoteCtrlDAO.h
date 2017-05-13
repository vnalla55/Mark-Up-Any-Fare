//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#ifndef FOOTNOTE_CTRL_DAO_H
#define FOOTNOTE_CTRL_DAO_H

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
class FootNoteCtrlInfo;
class DeleteList;

typedef HashKey<int> FootNoteCtrlKey;

class FootNoteCtrlDAO 
  : public DataAccessObject<FootNoteCtrlKey, std::vector<FootNoteCtrlInfo*>>
{
public:
  static FootNoteCtrlDAO& instance();

  FootNoteCtrlDAO(int cacheSize = 0, const std::string& cacheType="")
  {}
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

  FootNoteCtrlKey createKey(FootNoteCtrlInfo* info);

  const std::string& cacheClass() { return _cacheClass; }

  virtual sfc::CompressedData *
    compress (const std::vector<FootNoteCtrlInfo*> *vect) const;

  virtual std::vector<FootNoteCtrlInfo*> *
    uncompress (const sfc::CompressedData &compressed) const;
protected:

  std::vector<FootNoteCtrlInfo*>* create(FootNoteCtrlKey key);
  void destroy(FootNoteCtrlKey key, std::vector<FootNoteCtrlInfo*>* recs);

  void load();

  static std::string _name;
  static std::string _cacheClass;

private:
  template<typename T, typename P>
  const std::vector<FootNoteCtrlInfo*>& getImpl(DeleteList& del,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const TariffNumber& ruleTariff,
                                                const Footnote& footnote,
                                                const int category,
                                                const T& effectiveFilter,
                                                P inhibitFilter);
  
  static FootNoteCtrlDAO* _instance;
};

} // namespace tse
#endif // FOOTNOTE_CTRL_DAO_H
