//-----------------------------------------------------------------------------
//  File:        MultiTicketUtil.h
//  Created:     2014
//
//  Description: Support Multi-ticket project project.
//
//  Copyright Sabre 2014
//
//               The copyright to the computer program(s) herein
//               is the property of Sabre.
//               The program(s) may be used and/or copied only with
//               the written permission of Sabre or in accordance
//               with the terms and conditions stipulated in the
//               agreement/contract under which the program(s)
//               have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "Common/Money.h"


#include <vector>

#include <stdint.h>

namespace tse
{
class Itin;
class TravelSeg;
class PricingTrx;
class Diag676Collector;

class MultiTicketUtil
{
  friend class MultiTicketUtilTest;

public:
  static void createMultiTicketItins(PricingTrx& trx);
  static void processMultiTicketItins(PricingTrx& trx);
  static bool isMultiTicketSolutionFound(PricingTrx& trx);
  static void validateTicketingAgreement(PricingTrx& trx);

  enum TicketSolution
  {
    MULTITKT_NOT_APPLICABLE,
    MULTITKT_NOT_FOUND,
    SINGLETKT_LOWER_THAN_MULTITKT,
    SINGLETKT_SAME_AS_MULTITKT,
    MULTITKT_LOWER_THAN_SINGLETKT,
    SINGLETKT_NOT_APPLICABLE,
    NO_SOLUTION_FOUND
  };
  static TicketSolution getTicketSolution(PricingTrx& trx);
  static Money getSingleTicketTotal() { return _singleTicketTotal; }
  static Money getMultiTicketTotal()  { return _multiTicketTotal;  }
  static Itin* getMultiTicketItin(PricingTrx& trx, const uint16_t& itinMultiTktOrderNum);
  static void  setRespondMsg(uint16_t position, const std::string& msg);
  static std::vector<std::string>& getAllRespondMsgs();
  static void  cleanUpMultiTickets(PricingTrx& trx);

private:
  MultiTicketUtil(PricingTrx& trx);
  void createSubItins();
  void findStopOvers(std::vector<TravelSeg*> tvlSegs,
                     size_t& count,
                     TravelSeg*& tvl);
  bool isMarriedSeg(TravelSeg* tvl);
  void createSubItins(Itin& itin, TravelSeg* tvl, std::vector<Itin*>& subItins);
  Itin* createSubItin(Itin& itin, uint16_t num, const std::vector<TravelSeg*>& tvl);
  void createDiag676();
  void finishDiag676();
  void validateMultiTicketItins();
  void cleanUpSubItins();
  void createDiag676ForTotal();
  void finishDiag676ForTotal();
  bool getFareTotal(Itin* itin, Money& amount);
  Itin* getMultiTicketItin(const uint16_t& itinMultiTktOrderNum);
  bool isSolutionFound();
  void displayPricingSolutionMsg(Itin* itin);
  void validateValidatingCxrData();
  void displayValidatingCarriersDataMsg(Itin* itin);

  PricingTrx& trx() { return _trx; }
  const PricingTrx& trx() const { return _trx; }
  static Money _singleTicketTotal;
  static Money _multiTicketTotal;
  static std::vector<std::string> _respondMsg;

  PricingTrx& _trx;
  Diag676Collector* _diag676;
};

} // namespace tse

