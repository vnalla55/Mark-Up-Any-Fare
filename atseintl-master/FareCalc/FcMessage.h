#pragma once

#include "Common/Message.h"

#include <string>

namespace tse
{

class FcMessage : public Message
{
public:
  enum MessageType
  {
    GENERAL = TYPE_GENERAL,
    ERROR = TYPE_ERROR,
    WARNING = TYPE_WARNING,
    ENDORSEMENT = TYPE_ENDORSE,
    RESTRICTION = TYPE_RESTRICTION,
    DISPLAY = TYPE_DISPLAY,
    CURRENCY = TYPE_CURRENCY,
    TFR_ERROR = TYPE_TICKETING_WARNING,
    BAGGAGE = TYPE_BAGGAGE,
    TRAILER = 'T', // special PSS message type,
    NOPNR_RULE_WARNING = 'Z',
    SEGMENT_FEE = 'H',
    VCX_SINGLE_GSA_SWAP = 'S', //Tkting to distinguish single gsa swap msg and other msgs
    AGENCY_COMM_MSG = TYPE_AGENCY_COMM_MSG
  };

  enum MessageSubType
  {
    SUBUNSPECIFIED,
    VALIDATINGCXR = 'D',
    SMPPART1,
    SMPPART2,
    SMPPART3
  };

  enum WQMessageContentType
  {
    UNSPECIFIED,
    RULE_WARNING,
    REMAINING_WARNING_MSG,
    CARRIER_MESSAGE,
    SERVICE_FEE_TEMPLATE,
    RETAILER_RULE_MESSAGE,
    TRUE_PTC_MESSAGE,
    MATCHED_ACCOUNT_MESSAGE,
    PVT_INDICATOR,
    INTERLINE_MESSAGE,
  };

  typedef uint16_t ErrorCode;

  static const ErrorCode VCX_WARNINGCODE = 9999;

  FcMessage(MessageType type,
            ErrorCode code,
            const std::string& msg,
            bool prefix = true,
            WQMessageContentType content = UNSPECIFIED,
            MessageSubType subType = SUBUNSPECIFIED)
    : msgType(type),
      msgCode(code),
      msgAirlineCode(""),
      msgText(msg),
      msgPrefix(prefix),
      msgContent(content),
      msgSubType(subType)
  {
  }

  FcMessage(MessageType type,
            ErrorCode code,
            const std::string& alCode,
            const std::string& msg,
            bool prefix = true,
            WQMessageContentType content = UNSPECIFIED,
            MessageSubType subType = SUBUNSPECIFIED)
    : msgType(type),
      msgCode(code),
      msgAirlineCode(alCode),
      msgText(msg),
      msgPrefix(prefix),
      msgContent(content),
      msgSubType(subType)
  {
  }

  FcMessage(const FcMessage& rhs)
    : msgType(rhs.messageType()),
      msgCode(rhs.messageCode()),
      msgAirlineCode(rhs.airlineCode()),
      msgText(rhs.messageText()),
      msgPrefix(rhs.messagePrefix()),
      msgContent(rhs.messageContent()),
      msgSubType(rhs.messageSubType())
  {
  }

  const FcMessage& operator=(const FcMessage& rhs)
  {
    msgType = rhs.messageType();
    msgCode = rhs.messageCode();
    msgAirlineCode = rhs.airlineCode();
    msgText = rhs.messageText();
    msgPrefix = rhs.messagePrefix();
    msgSubType = rhs.messageSubType();
    msgContent = rhs.msgContent;
    return *this;
  }

  MessageType messageType() const { return msgType; }
  ErrorCode messageCode() const { return msgCode; }
  const std::string& airlineCode() const { return msgAirlineCode; }
  const std::string& messageText() const { return msgText; }
  bool messagePrefix() const { return msgPrefix; }
  MessageSubType messageSubType() const { return msgSubType; }
  WQMessageContentType messageContent() const { return msgContent; }

private:
  MessageType msgType;
  ErrorCode msgCode;
  std::string msgAirlineCode;
  std::string msgText;
  bool msgPrefix;
  WQMessageContentType msgContent;
  MessageSubType msgSubType;
};
}

