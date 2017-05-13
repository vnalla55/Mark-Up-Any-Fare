//----------------------------------------------------------------------------
//  File: FDHeaderMsgController.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2007
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseStringTypes.h"
#include "Common/FDCustomerRetriever.h"


#include <set>

namespace tse
{

class Loc;
class FareDisplayTrx;
class FDHeaderMsg;
class FDHeaderMsgText;

class FDHeaderMsgController : public FDCustomerRetriever
{
  friend class FDHeaderMsgControllerTest;

public:
  FDHeaderMsgController(FareDisplayTrx& trx, const std::set<CarrierCode>& prefferedCarriers);
  virtual ~FDHeaderMsgController() {}

  // does everything with all needed records
  virtual bool retrieve() override;

  virtual void getHeaderMsgs(std::vector<tse::FDHeaderMsgText*>& headerMsgs);

  std::vector<const FDHeaderMsg*> getFilteredHeaderMsgs();

protected:
  // does all db access for one set of records
  virtual bool retrieveData(const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const TJRGroup& tjrGroup) override;
  virtual bool
  isEliminateOnFareDisplayType(const Indicator& fdTypeEntry, const Indicator& fdTypeData);

  virtual bool isEliminateOnLocation(const tse::FDHeaderMsg* const& fdHeaderMsg,
                                     const Loc& origin,
                                     const Loc& destination);

  virtual bool isEliminateOnCarrier(const tse::FDHeaderMsg* const& fdHeaderMsg);

  virtual bool isEliminateOnRouting(const tse::FDHeaderMsg* const& fdHeaderMsg);

  virtual bool isEliminateOnGlobalDirection(const tse::FDHeaderMsg* const& fdHeaderMsg);

  virtual bool isEliminateOnPosLoc(const tse::FDHeaderMsg* const& fdHeaderMsg);

  virtual bool isEliminateOnCurrency(const tse::FDHeaderMsg* const& fdHeaderMsg);

  virtual bool isEliminateOnInclusionCode(const tse::FDHeaderMsg* const& fdHeaderMsg,
                                          const InclusionCode& inclusionCode);

  virtual bool isEliminateOnBufferZone(const tse::FDHeaderMsg* const& fdHeaderMsg,
                                       const Loc& origin,
                                       const Loc& destination);

  virtual bool eliminateRows(const std::vector<const tse::FDHeaderMsg*>& fdHeaderMsgDataList,
                             std::vector<const tse::FDHeaderMsg*>& fdFilteredHdrMsg);

  virtual void addMsgText(std::vector<const tse::FDHeaderMsg*>& fdHeaderMsgDataList,
                          std::vector<tse::FDHeaderMsgText*>& headerMsgs);

  virtual void addSupplementHdrMessage(std::vector<tse::FDHeaderMsgText*>& headerMsgs);

  virtual void setHdrMsg(std::vector<tse::FDHeaderMsgText*>& headerMsgs);

private:
  static const Indicator SINGLE_CARRIER_ENTRY;
  static const Indicator MULTI_CARRIER_ENTRY;

  std::vector<const FDHeaderMsg*> _fdFilteredHdrMsg;
  std::vector<FDHeaderMsgText*>* _headerMsgs = nullptr;

  const uint32_t _maxAllowedMsg = 5;
  Indicator _fareDisplayType = 0;
  const std::set<CarrierCode>& _preferredCarriers;
  bool _supplementalCarrierHdrMsg = true;

}; // End of Class FDHeaderMsgController

} // end of namespace

