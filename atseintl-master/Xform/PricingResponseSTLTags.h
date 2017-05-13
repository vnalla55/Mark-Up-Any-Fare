//-------------------------------------------------------------------
//
//  File:        PricingResponseSTLTags.h
//  Created:     August 8, 2013
//  Authors:     Steven Schronk
//
//  Description: All known STL tags
//
//  Updates:
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

#include <string>

namespace tse
{
namespace STL
{

// STL Tags
static const std::string BankIdentityNumber = "BankIdentityNumber";
static const std::string MessageInformation = "Message";
static const std::string TicketingFee = "TicketingFee";
static const std::string PassengerOBFees = "PassengerOBFees";
static const std::string PassengerIdentity = "PassengerIdentity";

static const std::string ValidatingCxrResult = "ValidatingCxrResult";
static const std::string Country = "Country";
static const std::string Carrier = "Carrier";
static const std::string PrimeHost = "PrimeHost";
static const std::string InterlineAgreementDisplay = "InterlineAgreementDisplay";
static const std::string InterlineAgreements = "InterlineAgreements";

static const std::string ValidatingCxrDisplay = "ValidatingCxrDisplay";
static const std::string GeneralSalesAgents = "GeneralSalesAgents";
static const std::string NeutralValidatingCxrs = "NeutralValidatingCxrs";

static const std::string ValidationResult = "ValidationResult";
static const std::string TicketType = "TicketType";
static const std::string ValidatingCxr = "ValidatingCxr";
static const std::string ValidatingCxrs = "ValidatingCxrs";
static const std::string ParticipatingCxr = "ParticipatingCxr";


// STL Attributes
static const std::string MessageCode = "code";
static const std::string MessageType = "type";

static const std::string code = "code";
static const std::string showNoOBFeeInd = "showNoOBFeeInd";

static const std::string typeCode = "typeCode";
static const std::string feeAmount = "feeAmount";
static const std::string maxAmount = "maxAmount";
static const std::string noChargeInd = "noChargeInd";
static const std::string iataIndicators = "iataIndicators";
static const std::string serviceFeePercentage = "serviceFeePercentage";
static const std::string binNumber = "binNumber";
static const std::string requestedBinNumber = "requestedBinNumber";
static const std::string chargeAmount = "chargeAmount";

static const std::string feeApplyInd = "feeApplyInd";
static const std::string passengerTypeCode = "passengerTypeCode";
static const std::string totalAmountWithOBFee = "totalAmountWithOBFee";
static const std::string totalPriceAmount = "totalPriceAmount";
static const std::string paymentCurrency = "paymentCurrency";

static const std::string firstNameNumber = "firstNameNumber";
static const std::string surNameNumber = "surNameNumber";
static const std::string pnrNameNumber = "pnrNameNumber";

static const std::string agreementType = "agreementType";
static const std::string iatLevel = "iatLevel";
static const std::string carrierName = "carrierName";
static const std::string carrier = "carrier";
static const std::string settlementPlanCode = "settlementPlanCode";
static const std::string settlementPlanName ="settlementPlanName";
static const std::string ticketType = "ticketType";

} // end namespace xml2
} // end namespace tse
