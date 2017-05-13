//-------------------------------------------------------------------
//
//  File:        Billing.h
//  Created:     March 31, 2004
//  Authors:
//
//  Description: Billing information
//
//  Updates:
//          03/31/04 - VN - file created.
//          June 4 2007 - Darek Kubiczek - updated transaction ids
//                            and service names
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{

class Billing
{
public:
  enum Service
  {
    SVC_CURRENCY = 0,
    SVC_PRICING_DETAIL,
    SVC_TAX,
    SVC_FARE_DISPLAY,
    SVC_MILEAGE,
    SVC_SHOPPING,
    SVC_PRICING,
    SVC_MULTIPLE, // deprecated
    SVC_PRICING_DISPLAY, // deprecated
    SVC_HISTORICAL,
    SVC_ENHANCED_RULE_DISPLAY,
    SVC_TICKETINGCXR, // 11
    SVC_TICKETINGCXR_DISP,
    MAX_SVC_IND // Add all new services before this enumeration
  };

  Billing();

  friend std::ostream& operator<<(std::ostream& os, const Billing& bill);

  std::string& userPseudoCityCode() { return _userPseudoCityCode; }
  const std::string& userPseudoCityCode() const { return _userPseudoCityCode; }

  std::string& userStation() { return _userStation; }
  const std::string& userStation() const { return _userStation; }

  std::string& userBranch() { return _userBranch; }
  const std::string& userBranch() const { return _userBranch; }

  std::string& partitionID() { return _partitionID; }
  const std::string& partitionID() const { return _partitionID; }

  std::string& validatingCarrier() { return _validatingCarrier; }
  const std::string& validatingCarrier() const { return _validatingCarrier; }

  std::string& userSetAddress() { return _userSetAddress; }
  const std::string& userSetAddress() const { return _userSetAddress; }

  std::string& serviceName() { return _serviceName; }
  const std::string& serviceName() const { return _serviceName; }

  std::string& aaaCity() { return _aaaCity; }
  const std::string& aaaCity() const { return _aaaCity; }

  std::string& aaaSine() { return _aaaSine; }
  const std::string& aaaSine() const { return _aaaSine; }

  std::string& actionCode() { return _actionCode; }
  const std::string& actionCode() const { return _actionCode; }

  uint64_t& transactionID() { return _transactionID; }
  uint64_t transactionID() const { return _transactionID; }

  uint64_t& clientTransactionID() { return _clientTranID; }
  uint64_t clientTransactionID() const { return _clientTranID; }

  uint64_t& parentTransactionID() { return _parentTranID; }
  uint64_t parentTransactionID() const { return _parentTranID; }

  std::string& clientServiceName() { return _clientServiceName; }
  const std::string& clientServiceName() const { return _clientServiceName; }

  std::string& parentServiceName() { return _parentServiceName; }
  const std::string& parentServiceName() const { return _parentServiceName; }

  std::string& requestPath() { return _requestPath; }
  const std::string& requestPath() const { return _requestPath; }

  /**
   *  @brief return internal service name using service
   *
   *
   *  @param [in] service    service
   *
   *  @return char*
   *
   */
  const char* getServiceName(Service service);
  const char* getBusinessFunction(Service service);

  //--------------------------------------------------------------------------

  /**
   *  @brief Updates internal, client and parent transaction id
   *
   *  Client, parent and internal transaction ids are updated only in case
   *  when they were not set during processing the input request
   *
   *  @param [in] transactionID    locally generated transaction id
   *
   *  @pre transactionId != 0
   *
   *  @post _transactionId != 0, _clientTranID != 0, _parentTranID != 0
   */
  void updateTransactionIds(uint64_t transactionID);

  /**
   *  @brief Updates internal, client and parent service name
   *
   *  Client, parent and internal service names are updated only in case
   *  when they were not set during processing input request
   *
   *  @param [in] service    service
   *
   *  @post ! serviceName().empty() and
   *        ! clientServiceName().empty() and
   *        ! parentServiceName().empty
   */
  void updateServiceNames(Service service);

  /**
   *  @brief Converts transaction id string to unsigned long long number
   *
   *  @param [in] cstr    c-string transaction id
   *  @return unsigned long long number
   *
   *  @pre cstr != 0
   */

  static uint64_t string2transactionId(const char* cstr);

private:
  std::string _userPseudoCityCode; // A20 (formerly UCD)
  std::string _userStation; // Q03 (formerly UST)
  std::string _userBranch; // Q02 (formerly UBR)
  std::string _partitionID; // AE0 (formerly PID)
  std::string _validatingCarrier; // B05
  std::string _userSetAddress; // AD0 (formerly USA)
  std::string _serviceName; // C20 (formerly CSV)
  std::string _aaaCity; // A22 (formerly AAA)
  std::string _aaaSine; // AA0 (formerly ASI)
  std::string _actionCode; // AKD (formerly AKD)
  uint64_t _transactionID; // C00
  uint64_t _clientTranID; // C01
  uint64_t _parentTranID; // C02
  std::string _clientServiceName; // C21
  std::string _parentServiceName; // C22
  std::string _requestPath; // S0R

  // Note: Q0S and Q1L (number of soulutions requested and produced) are
  // directly inserted into billing and instrumentation record before sending
  bool isHistoricalServer();
  void updateHistoricalServiceNames(Service service);
};

inline Billing::Billing()
  : _transactionID(), _clientTranID(), _parentTranID(), _requestPath(UNKNOWN_PATH)
{
}

} // tse namespace

