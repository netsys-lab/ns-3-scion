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

#include "scion-interface-address.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONInterfaceAddress");

SCIONInterfaceAddress::SCIONInterfaceAddress()
    : m_scope(GLOBAL)
{
    NS_LOG_FUNCTION(this);
}

SCIONInterfaceAddress::SCIONInterfaceAddress(SCIONAddress local)
    : m_scope(GLOBAL)
      
{
    NS_LOG_FUNCTION(this << local);
    m_local = local;
    if (m_local == SCIONAddress::GetLoopback())
    {
        m_scope = HOST;
    }
}

SCIONInterfaceAddress::SCIONInterfaceAddress(const SCIONInterfaceAddress& o)
    : m_local(o.m_local),  
      m_scope(o.m_scope)
{
    NS_LOG_FUNCTION(this << &o);
}

void
SCIONInterfaceAddress::SetAddress(SCIONAddress address)
{
    NS_LOG_FUNCTION(this << address);
    m_local = address;
}


SCIONAddress
SCIONInterfaceAddress::GetAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_local;
}


void
SCIONInterfaceAddress::SetScope(SCIONInterfaceAddress::InterfaceAddressScope_e scope)
{
    NS_LOG_FUNCTION(this << scope);
    m_scope = scope;
}

SCIONInterfaceAddress::InterfaceAddressScope_e
SCIONInterfaceAddress::GetScope() const
{
    NS_LOG_FUNCTION(this);
    return m_scope;
}

bool
SCIONInterfaceAddress::IsInSameSubnet(const SCIONAddress b) const
{
    // TODO compare host address types and value
    return m_local.GetIA()==b.GetIA();
}


std::ostream&
operator<<(std::ostream& os, const SCIONInterfaceAddress& addr)
{
    os << "m_local=" << addr.GetAddress() <<
       "; m_scope=" << addr.GetScope();
    return os;
}

} // namespace ns3
