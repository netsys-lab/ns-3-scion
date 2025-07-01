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

#include "scion-interface.h"
#include "ns3/scion-address.h"
#include "ns3/scion-header.h"
#include "scion-l3-protocol.h"
#include "scion-queue-disc-item.h"
#include "ns3/loopback-net-device.h"

#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/pointer.h"
#include "ns3/traffic-control-layer.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONInterface");

NS_OBJECT_ENSURE_REGISTERED(SCIONInterface);

TypeId
SCIONInterface::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCIONInterface")
                            .SetParent<Object>()
                            .SetGroupName("Internet");
    ;
    return tid;
}

/**
 * By default, Ipv4 interface are created in the "down" state
 *  with no IP addresses.  Before becoming usable, the user must
 * invoke SetUp on them once an Ipv4 address and mask have been set.
 */
SCIONInterface::SCIONInterface()
    : m_ifup(false),
      m_forwarding(true),
      m_metric(1),
      m_node(nullptr),
      m_device(nullptr),
      m_tc(nullptr)
{
    NS_LOG_FUNCTION(this);
}

SCIONInterface::~SCIONInterface()
{
    NS_LOG_FUNCTION(this);
}

void
SCIONInterface::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_node = nullptr;
    m_device = nullptr;
    m_tc = nullptr;
    Object::DoDispose();
}

void
SCIONInterface::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
    DoSetup();
}

void
SCIONInterface::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
    DoSetup();
}

void
SCIONInterface::SetTrafficControl(Ptr<TrafficControlLayer> tc)
{
    NS_LOG_FUNCTION(this << tc);
    m_tc = tc;
}

void
SCIONInterface::DoSetup()
{
    NS_LOG_FUNCTION(this);
    if (!m_node || !m_device)
    {
        return;
    }
    if (!m_device->NeedsArp())
    {
        return;
    }    
}

Ptr<NetDevice>
SCIONInterface::GetDevice() const
{
    NS_LOG_FUNCTION(this);
    return m_device;
}

/**
 * These are IP interface states and may be distinct from
 * NetDevice states, such as found in real implementations
 * (where the device may be down but IP interface state is still up).
 */
bool
SCIONInterface::IsUp() const
{
    NS_LOG_FUNCTION(this);
    return m_ifup;
}

bool
SCIONInterface::IsDown() const
{
    NS_LOG_FUNCTION(this);
    return !m_ifup;
}

void
SCIONInterface::SetUp()
{
    NS_LOG_FUNCTION(this);
    m_ifup = true;
}

void
SCIONInterface::SetDown()
{
    NS_LOG_FUNCTION(this);
    m_ifup = false;
}

bool
SCIONInterface::IsForwarding() const
{
    NS_LOG_FUNCTION(this);
    return m_forwarding;
}

void
SCIONInterface::SetForwarding(bool val)
{
    NS_LOG_FUNCTION(this << val);
    m_forwarding = val;
}

/**
 * sending through an interface (L3) eventually hands the packet over to the TrafficControlLayer (L2)
 * which acts as a gate keeper for the NetDevices
 */
void
SCIONInterface::Send(Ptr<Packet> p, const SCIONHeader& hdr, SCIONAddress dest)
{
    NS_LOG_FUNCTION(this << *p << dest);
    if (!IsUp())
    {
        return;
    }

    // Check for a loopback device, if it's the case we don't pass through
    // traffic control layer
    if (DynamicCast<LoopbackNetDevice>(m_device))
    {
        /// \todo additional checks needed here (such as whether multicast
        /// goes to loopback)?
        p->AddHeader(hdr);
        m_device->Send(p, m_device->GetBroadcast(), SCIONL3Protocol::PROT_NUMBER);
        return;
    }

    NS_ASSERT(m_tc);

    // is this packet aimed at a local interface ?
    for (SCIONInterfaceAddressListCI i = m_ifaddrs.begin(); i != m_ifaddrs.end(); ++i)
    {
        if (dest == (*i).GetAddress())
        {
            p->AddHeader(hdr);
            m_tc->Receive(m_device,
                          p,
                          SCIONL3Protocol::PROT_NUMBER,
                          m_device->GetBroadcast(),
                          m_device->GetBroadcast(),
                          NetDevice::PACKET_HOST);
            return;
        }
    }

        NS_LOG_LOGIC("Doesn't need ARP");
        m_tc->Send(m_device,
                   Create<SCIONQueueDiscItem>(p,
                                             m_device->GetBroadcast(),
                                             SCIONL3Protocol::PROT_NUMBER,
                                             hdr));
    
}

uint32_t
SCIONInterface::GetNAddresses() const
{
    NS_LOG_FUNCTION(this);
    return m_ifaddrs.size();
}

bool
SCIONInterface::AddAddress(SCIONInterfaceAddress addr)
{
    NS_LOG_FUNCTION(this << addr);
    m_ifaddrs.push_back(addr);
    if (!m_addAddressCallback.IsNull())
    {
        m_addAddressCallback(this, addr);
    }
    return true;
}

SCIONInterfaceAddress
SCIONInterface::GetAddress(uint32_t index) const
{
    NS_LOG_FUNCTION(this << index);
    if (index < m_ifaddrs.size())
    {
        uint32_t tmp = 0;
        for (SCIONInterfaceAddressListCI i = m_ifaddrs.begin(); i != m_ifaddrs.end(); i++)
        {
            if (tmp == index)
            {
                return *i;
            }
            ++tmp;
        }
    }
    else
    {
        NS_FATAL_ERROR("index " << index << " out of bounds");
    }
    SCIONInterfaceAddress addr;
    return (addr); // quiet compiler
}

SCIONInterfaceAddress
SCIONInterface::RemoveAddress(uint32_t index)
{
    NS_LOG_FUNCTION(this << index);
    if (index >= m_ifaddrs.size())
    {
        NS_FATAL_ERROR("Bug in SCIONInterface::RemoveAddress");
    }
    SCIONInterfaceAddressListI i = m_ifaddrs.begin();
    uint32_t tmp = 0;
    while (i != m_ifaddrs.end())
    {
        if (tmp == index)
        {
            SCIONInterfaceAddress addr = *i;
            m_ifaddrs.erase(i);
            if (!m_removeAddressCallback.IsNull())
            {
                m_removeAddressCallback(this, addr);
            }
            return addr;
        }
        ++tmp;
        ++i;
    }
    NS_FATAL_ERROR("Address " << index << " not found");
    SCIONInterfaceAddress addr;
    return (addr); // quiet compiler
}

SCIONInterfaceAddress
SCIONInterface::RemoveAddress(SCIONAddress address)
{
    NS_LOG_FUNCTION(this << address);

    if (address == SCIONAddress::GetLoopback())
    {
        NS_LOG_WARN("Cannot remove loopback address.");
        return SCIONInterfaceAddress();
    }

    for (SCIONInterfaceAddressListI it = m_ifaddrs.begin(); it != m_ifaddrs.end(); it++)
    {
        if ((*it).GetAddress() == address)
        {
            SCIONInterfaceAddress ifAddr = *it;
            m_ifaddrs.erase(it);
            if (!m_removeAddressCallback.IsNull())
            {
                m_removeAddressCallback(this, ifAddr);
            }
            return ifAddr;
        }
    }
    return SCIONInterfaceAddress();
}

void
SCIONInterface::RemoveAddressCallback(
    Callback<void, Ptr<SCIONInterface>, SCIONInterfaceAddress> removeAddressCallback)
{
    NS_LOG_FUNCTION(this << &removeAddressCallback);
    m_removeAddressCallback = removeAddressCallback;
}

void
SCIONInterface::AddAddressCallback(
    Callback<void, Ptr<SCIONInterface>, SCIONInterfaceAddress> addAddressCallback)
{
    NS_LOG_FUNCTION(this << &addAddressCallback);
    m_addAddressCallback = addAddressCallback;
}

} // namespace ns3
