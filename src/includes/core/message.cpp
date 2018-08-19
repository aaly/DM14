#include "message.hpp"



nodeMessage::nodeMessage(const string& constructMessage, int node) 
{
	if(constructMessage.size())
	{
		type = (int)constructMessage.at(0);
		message = constructMessage.substr(1);
		nodeIndex = node;
	}
}

nodeMessage::~nodeMessage()
{
	;
}

int nodeMessage::getType()
{
	return type;
};

int nodeMessage::pushToken(const string messagePart)
{
	if(message.size())
	{
		message += separatorByte;
	}
	message += messagePart;
	return message.size();
}

string nodeMessage::getPackedMessage()
{
	string msg;
	msg = (char)type;
	
	if(message.size())
	{
		msg +=  message;
	}
	
	return msg;
};

const string& nodeMessage::getMessage()
{
	return message;
};


int nodeMessage::getNodeIndex()
{
	return nodeIndex;
};



int nodeMessage::setType(const int messageType)
{
	type = messageType;
	return type;
}

int nodeMessage::setNodeIndex(const int node)
{
	nodeIndex = node;
	return nodeIndex;
}


nodeMessage::nodeMessage()
{
	type = 0;
	message = "";
}


long nodeMessage::getPosition(int order)
{
	if (order == 0)
	{
		return 0;
	}
	
	int tmp = 0;
	string const& msg = message;
	unsigned int size =  msg.length();
	for (unsigned int i =0; i <size; i++)
	{
		if(msg.at(i) == separatorByte)
		{
			tmp++;
		}
		
		if(tmp == order)
		{
			return i+1;
		}
	}
	return -1;
}

string nodeMessage::getToken(int pos, unsigned long size)
{
	long from = getPosition(pos);
	
	long to;
	
	if(size)
	{
		to = size;
	}
	else
	{
		to  = getPosition(pos+1)-getPosition(pos)-1;
	}
	
	
	if(from != -1 && to != -1)
	{
		return message.substr(from, to);
	}
	
	return "";
}


