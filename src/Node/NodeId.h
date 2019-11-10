#ifndef _NODE_ID_H
#define _NODE_ID_H

#include <FixedString.h>
#include "ThingifyConstants.h"


class NodeId
{
public:
	FixedString<ThingifyConstants::MaxDeviceNameLength> DeviceId;
	FixedString<ThingifyConstants::MaxNodeNameLength> NodeName;
};

#endif


