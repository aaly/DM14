#ifndef __MESSAGEHPP
#define __MESSAGEHPP

#include "Array.hpp"
#include "Thread.hpp"
#include "common.hpp"
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string> // For string
#include <vector>

class nodeMessage {
public:
  nodeMessage(const string &constructMessage, int node = -1);
  nodeMessage();
  ~nodeMessage();

  int pushToken(const string messagePart);

  int getNodeIndex();
  int getType();
  const string &getMessage();
  string getPackedMessage();

  int setType(const int messageType);
  int setNodeIndex(const int node);
  string getToken(int pos, unsigned long size = 0);

  static const char separatorByte = ';';

private:
  long getPosition(int order);

  int type;
  string message;
  int nodeIndex;
};

#endif /* __MESSAGEHPP */
