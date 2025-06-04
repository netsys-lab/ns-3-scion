/*
 * Copyright (c) 2008 INRIA
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

#include "scion-interface-container.h"

#include "ns3/names.h"
#include "ns3/node-list.h"

namespace ns3
{

SCIONInterfaceContainer::SCIONInterfaceContainer()
{
}

void
SCIONInterfaceContainer::Add(const SCIONInterfaceContainer& other)
{
    for (InterfaceVector::const_iterator i = other.m_interfaces.begin();
         i != other.m_interfaces.end();
         i++)
    {
        m_interfaces.push_back(*i);
    }
}

SCIONInterfaceContainer::Iterator
SCIONInterfaceContainer::Begin() const
{
    return m_interfaces.begin();
}

SCIONInterfaceContainer::Iterator
SCIONInterfaceContainer::End() const
{
    return m_interfaces.end();
}

uint32_t
SCIONInterfaceContainer::GetN() const
{
    return m_interfaces.size();
}

SCIONAddress
SCIONInterfaceContainer::GetAddress(uint32_t i, uint32_t j) const
{
    Ptr<SCION> ipv4 = m_interfaces[i].first;
    uint32_t interface = m_interfaces[i].second;
    return ipv4->GetAddress(interface, j).GetLocal();
}

void
SCIONInterfaceContainer::SetMetric(uint32_t i, uint16_t metric)
{
    Ptr<SCION> ipv4 = m_interfaces[i].first;
    uint32_t interface = m_interfaces[i].second;
    ipv4->SetMetric(interface, metric);
}

void
SCIONInterfaceContainer::Add(Ptr<SCION> ipv4, uint32_t interface)
{
    m_interfaces.emplace_back(ipv4, interface);
}

void
SCIONInterfaceContainer::Add(std::pair<Ptr<SCION>, uint32_t> a)
{
    Add(a.first, a.second);
}

void
SCIONInterfaceContainer::Add(std::string ipv4Name, uint32_t interface)
{
    Ptr<SCION> ipv4 = Names::Find<SCION>(ipv4Name);
    m_interfaces.emplace_back(ipv4, interface);
}

std::pair<Ptr<SCION>, uint32_t>
SCIONInterfaceContainer::Get(uint32_t i) const
{
    return m_interfaces[i];
}

} // namespace ns3
