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
 */

#include "scion-route.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/net-device.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONRoute");

SCIONRoute::SCIONRoute()
{
    NS_LOG_FUNCTION(this);
}

void
SCIONRoute::SetDestination(SCIONAddress dest)
{
    NS_LOG_FUNCTION(this << dest);
    m_dest = dest;
}

SCIONAddress
SCIONRoute::GetDestination() const
{
    NS_LOG_FUNCTION(this);
    return m_dest;
}

void
SCIONRoute::SetSource(SCIONAddress src)
{
    NS_LOG_FUNCTION(this << src);
    m_source = src;
}

SCIONAddress
SCIONRoute::GetSource() const
{
    NS_LOG_FUNCTION(this);
    return m_source;
}

void
SCIONRoute::SetGateway(SCIONAddress gw)
{
    NS_LOG_FUNCTION(this << gw);
    m_gateway = gw;
}

SCIONAddress
SCIONRoute::GetGateway() const
{
    NS_LOG_FUNCTION(this);
    return m_gateway;
}

void
SCIONRoute::SetOutputDevice(Ptr<NetDevice> outputDevice)
{
    NS_LOG_FUNCTION(this << outputDevice);
    m_outputDevice = outputDevice;
}

Ptr<NetDevice>
SCIONRoute::GetOutputDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_outputDevice;
}

std::ostream&
operator<<(std::ostream& os, const SCIONRoute& route)
{
    os << "source=" << route.GetSource() << " dest=" << route.GetDestination()
       << " gw=" << route.GetGateway();
    return os;
}

} // namespace ns3
