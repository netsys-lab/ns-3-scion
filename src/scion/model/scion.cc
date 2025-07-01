/*
 * Copyright (c) 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author:
 */

#include "ns3/scion.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/log.h"
#include "ns3/node.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCION");

NS_OBJECT_ENSURE_REGISTERED(SCION);

TypeId
SCION::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SCION")
            .SetParent<Object>()
            .SetGroupName("Internet")
            .AddAttribute("WeakEsModel",
                          "RFC1122 term for whether host accepts datagram with a dest. address on "
                          "another interface",
                          BooleanValue(true),
                          MakeBooleanAccessor(&SCION::SetWeakEsModel, &SCION::GetWeakEsModel),
                          MakeBooleanChecker())
#if 0
    .AddAttribute ("MtuDiscover", "If enabled, every outgoing ip packet will have the DF flag set.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&UdpSocket::SetMtuDiscover,
                                        &UdpSocket::GetMtuDiscover),
                   MakeBooleanChecker ())
#endif
        ;
    return tid;
}

SCION::SCION()
{
    NS_LOG_FUNCTION(this);
}

SCION::~SCION()
{
    NS_LOG_FUNCTION(this);
}

} // namespace ns3
