//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Common/TseCodeTypes.h"

// While accumulating booking code statuses, we want to distinguish between
// two scenarios:
// a) "choice" where we can select a solution from
//    several possible solutions
//    In such cases, "not available" wins with "not offered"
//    since it displays the minimal effort that could be made
//    to make selection valid.
// b) "sequence" where there is a vector of statuses
//    that must be evaluated to a single resulting status
//    In such cases, "not offered wins" since we want to
//    return the most severe error that made the sequence
//    invalid.
//    e.g.
//    [NA, NO, NA, NA] evaluates to NO (not offered)

namespace tse
{

struct IbfAvailabilityEnumTools
{
  static bool isBcvsError(BookingCodeValidationStatus stat)
  {
    return (stat >= BOOKING_CODE_NOT_OFFERED) && (stat <= BOOKING_CODE_NOT_AVAILABLE);
  }

  static IbfErrorMessage toIbfErrorMessage(BookingCodeValidationStatus bcStatus)
  {
    TSE_ASSERT(isBcvsError(bcStatus));
    if (bcStatus == BOOKING_CODE_NOT_OFFERED)
    {
      return IbfErrorMessage::IBF_EM_NOT_OFFERED;
    }
    return IbfErrorMessage::IBF_EM_NOT_AVAILABLE;
  }
};

// ------------------------ BookingCodeValidationStatus ------------------------

struct BcvsInitPolicy
{
  static void init(BookingCodeValidationStatus& stat) { stat = BOOKING_CODE_STATUS_NOT_SET; }
};

struct NotAvailableWins
{
  static BookingCodeValidationStatus
  filter(const BookingCodeValidationStatus& oldStatus, const BookingCodeValidationStatus& newStatus)
  {
    TSE_ASSERT(IbfAvailabilityEnumTools::isBcvsError(newStatus));
    if ((oldStatus == BOOKING_CODE_NOT_AVAILABLE) && (newStatus == BOOKING_CODE_NOT_OFFERED))
    {
      return oldStatus;
    }
    return newStatus;
  }
};

struct NotOfferedWins
{
  static BookingCodeValidationStatus
  filter(const BookingCodeValidationStatus& oldStatus, const BookingCodeValidationStatus& newStatus)
  {
    TSE_ASSERT(IbfAvailabilityEnumTools::isBcvsError(newStatus));
    if ((oldStatus == BOOKING_CODE_NOT_OFFERED) && (newStatus == BOOKING_CODE_NOT_AVAILABLE))
    {
      return oldStatus;
    }
    return newStatus;
  }
};

// ------------------------ IbfErrorMessage ------------------------

struct IbfErrorMessageInitPolicy
{
  static void init(IbfErrorMessage& stat) { stat = IbfErrorMessage::IBF_EM_NOT_SET; }
};

struct IbfErrorMessageFmInitPolicy
{
  static void init(IbfErrorMessage& stat)
  {
    // This is the initial state for a fare market
    // We assume that it will be updated to
    // not offered, not available or
    // no fare found later on so that we can
    // set this as default.
    stat = IbfErrorMessage::IBF_EM_NO_FARE_FILED;
  }
};

struct IbfErrorMessagePriorityCalc
{
  static const unsigned int PRIORITY_1 = 1;
  static const unsigned int PRIORITY_2 = 2;
  static const unsigned int PRIORITY_3 = 3;
  static const unsigned int PRIORITY_4 = 4;
  static const unsigned int PRIORITY_5 = 5;
  static const unsigned int PRIORITY_6 = 6;

  // Priority for a choice (from highest to lowest):
  // IbfErrorMessage::IBF_EM_NO_FARE_FOUND
  // IbfErrorMessage::IBF_EM_NOT_AVAILABLE
  // IbfErrorMessage::IBF_EM_NOT_OFFERED
  // IbfErrorMessage::IBF_EM_EARLY_DROP
  // IbfErrorMessage::IBF_EM_NOT_SET
  static unsigned int priorityForChoiceOrder(IbfErrorMessage msg)
  {
    if (IbfErrorMessage::IBF_EM_NOT_SET == msg)
      return PRIORITY_1;
    if (IbfErrorMessage::IBF_EM_NO_FARE_FILED == msg)
      return PRIORITY_2;
    if (IbfErrorMessage::IBF_EM_EARLY_DROP == msg)
      return PRIORITY_3;
    if (IbfErrorMessage::IBF_EM_NOT_OFFERED == msg)
      return PRIORITY_4;
    if (IbfErrorMessage::IBF_EM_NOT_AVAILABLE == msg)
      return PRIORITY_5;
    return PRIORITY_6; // IbfErrorMessage::IBF_EM_NO_FARE_FOUND
  }

  // Priority for a sequence (from highest to lowest):
  // IbfErrorMessage::IBF_EM_EARLY_DROP
  // IbfErrorMessage::IBF_EM_NOT_OFFERED
  // IbfErrorMessage::IBF_EM_NOT_AVAILABLE
  // IbfErrorMessage::IBF_EM_NO_FARE_FOUND
  // IbfErrorMessage::IBF_EM_NOT_SET
  static unsigned int priorityForSequenceOrder(IbfErrorMessage msg)
  {
    if (IbfErrorMessage::IBF_EM_NOT_SET == msg)
      return PRIORITY_1; 
    if (IbfErrorMessage::IBF_EM_NO_FARE_FOUND == msg)
      return PRIORITY_2;
    if (IbfErrorMessage::IBF_EM_NOT_AVAILABLE == msg)
      return PRIORITY_3;
    if (IbfErrorMessage::IBF_EM_NOT_OFFERED == msg)
      return PRIORITY_4;
    if (IbfErrorMessage::IBF_EM_EARLY_DROP == msg)
      return PRIORITY_5;
    return PRIORITY_6; // IbfErrorMessage::IBF_EM_NO_FARE_FILED
  }
};

struct IbfErrorMessageChoiceOrder
{
  static IbfErrorMessage filter(const IbfErrorMessage& oldStatus, const IbfErrorMessage& newStatus)
  {
    if (IbfErrorMessagePriorityCalc::priorityForChoiceOrder(newStatus) >
        IbfErrorMessagePriorityCalc::priorityForChoiceOrder(oldStatus))
    {
      return newStatus;
    }
    return oldStatus;
  }
};

struct IbfErrorMessageSequenceOrder
{
  static IbfErrorMessage filter(const IbfErrorMessage& oldStatus, const IbfErrorMessage& newStatus)
  {
    if (IbfErrorMessagePriorityCalc::priorityForSequenceOrder(newStatus) >
        IbfErrorMessagePriorityCalc::priorityForSequenceOrder(oldStatus))
    {
      return newStatus;
    }
    return oldStatus;
  }
};

} // namespace tse

