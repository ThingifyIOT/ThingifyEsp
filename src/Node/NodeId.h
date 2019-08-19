#ifndef _NODE_ID_H
#define _NODE_ID_H

#include <FixedString.h>
#include "ContiConstants.h"


class NodeId
{
public:
	FixedString<ContiConstants::MaxDeviceNameLength> DeviceId;
	FixedString<ContiConstants::MaxNodeNameLength> NodeName;
};

#endif


