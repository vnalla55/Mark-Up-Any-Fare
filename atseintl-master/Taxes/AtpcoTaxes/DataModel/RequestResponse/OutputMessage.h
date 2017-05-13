// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <string>
#include <boost/ptr_container/ptr_vector.hpp>

namespace tax
{

class OutputMessage
{
public:
  explicit OutputMessage(const std::string& msg) : _content(msg) {}

  const std::string& content() const { return _content; }
  std::string& content() { return _content; }

private:
  OutputMessage& operator=(const OutputMessage&);

  std::string _content;
};

class MessageContainer
{
public:
  const boost::ptr_vector<OutputMessage>& messages() const { return _messages; }
  boost::ptr_vector<OutputMessage>& messages() { return _messages; }

private:
  MessageContainer& operator==(const MessageContainer&);

  boost::ptr_vector<OutputMessage> _messages;
};

} // namespace tax
