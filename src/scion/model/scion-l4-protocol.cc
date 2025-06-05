//
// Copyright (c) 2006 Georgia Tech Research Corporation
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation;
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Author: 
#include "scion-l4-protocol.h"

#include "ns3/integer.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONProtocol");

NS_OBJECT_ENSURE_REGISTERED(SCIONL4Protocol);

TypeId
SCIONL4Protocol::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCIONL4Protocol")
                            .SetParent<IpL4Protocol>()
                            .SetGroupName("Internet")
                            .AddAttribute("ProtocolNumber",
                                          "The IP protocol number.",
                                          TypeId::ATTR_GET,
                                          IntegerValue(0),
                                          MakeIntegerAccessor(&SCIONL4Protocol::GetProtocolNumber),
                                          MakeIntegerChecker<int>(0, 255));
    return tid;
}

SCIONL4Protocol::~SCIONL4Protocol()
{
    NS_LOG_FUNCTION(this);
}

void
SCIONL4Protocol::ReceiveScmp(SCIONAddress icmpSource,
                          uint8_t icmpTtl,
                          uint8_t icmpType,
                          uint8_t icmpCode,
                          uint32_t icmpInfo,
                          SCIONAddress payloadSource,
                          SCIONAddress payloadDestination,
                          const uint8_t payload[8])
{
    NS_LOG_FUNCTION(this << icmpSource << static_cast<uint32_t>(icmpTtl)
                         << static_cast<uint32_t>(icmpType) << static_cast<uint32_t>(icmpCode)
                         << icmpInfo << payloadSource << payloadDestination << payload);
}


} // namespace ns3
