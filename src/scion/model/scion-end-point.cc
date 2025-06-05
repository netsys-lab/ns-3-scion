/*
 * Copyright (c) 2005 INRIA
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
#include "ns3/scion-interface.h"
#include "scion-end-point.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONEndPoint");

SCIONEndPoint::SCIONEndPoint(SCIONAddress address, uint16_t port)
    : m_localAddr(address),
      m_localPort(port),
      m_peerAddr(),
      m_peerPort(0),
      m_rxEnabled(true)
{
    NS_LOG_FUNCTION(this << address << port);
}

SCIONEndPoint::~SCIONEndPoint()
{
    NS_LOG_FUNCTION(this);
    if (!m_destroyCallback.IsNull())
    {
        m_destroyCallback();
    }
    m_rxCallback.Nullify();
    m_scmpCallback.Nullify();
    m_destroyCallback.Nullify();
}

SCIONAddress
SCIONEndPoint::GetLocalAddress()
{
    NS_LOG_FUNCTION(this);
    return m_localAddr;
}

void
SCIONEndPoint::SetLocalAddress(SCIONAddress address)
{
    NS_LOG_FUNCTION(this << address);
    m_localAddr = address;
}

uint16_t
SCIONEndPoint::GetLocalPort()
{
    NS_LOG_FUNCTION(this);
    return m_localPort;
}

SCIONAddress
SCIONEndPoint::GetPeerAddress()
{
    NS_LOG_FUNCTION(this);
    return m_peerAddr;
}

uint16_t
SCIONEndPoint::GetPeerPort()
{
    NS_LOG_FUNCTION(this);
    return m_peerPort;
}

void
SCIONEndPoint::SetPeer(SCIONAddress address, uint16_t port)
{
    NS_LOG_FUNCTION(this << address << port);
    m_peerAddr = address;
    m_peerPort = port;
}

void
SCIONEndPoint::BindToNetDevice(Ptr<NetDevice> netdevice)
{
    NS_LOG_FUNCTION(this << netdevice);
    m_boundnetdevice = netdevice;
}

Ptr<NetDevice>
SCIONEndPoint::GetBoundNetDevice()
{
    NS_LOG_FUNCTION(this);
    return m_boundnetdevice;
}

void
SCIONEndPoint::SetRxCallback(
    Callback<void, Ptr<Packet>, SCIONHeader, uint16_t, Ptr<SCIONInterface>> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_rxCallback = callback;
}

void
SCIONEndPoint::SetScmpCallback(
    Callback<void, SCIONAddress, uint8_t, uint8_t, uint8_t, uint32_t> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_scmpCallback = callback;
}

void
SCIONEndPoint::SetDestroyCallback(Callback<void> callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_destroyCallback = callback;
}

void
SCIONEndPoint::ForwardUp(Ptr<Packet> p,
                        const SCIONHeader& header,
                        uint16_t sport,
                        Ptr<SCIONInterface> incomingInterface)
{
    NS_LOG_FUNCTION(this << p << &header << sport << incomingInterface);

    if (!m_rxCallback.IsNull())
    {
        m_rxCallback(p, header, sport, incomingInterface);
    }
}

void
SCIONEndPoint::ForwardScmp(SCIONAddress scmpSource,                          
                          uint8_t scmpType,
                          uint8_t scmpCode,
                          uint32_t scmpInfo)
{
    NS_LOG_FUNCTION(this << scmpSource << (uint32_t)scmpTtl << (uint32_t)scmpType
                         << (uint32_t)scmpCode << scmpInfo);
    if (!m_scmpCallback.IsNull())
    {
        m_scmpCallback(scmpSource, scmpTtl, scmpType, scmpCode, scmpInfo);
    }
}

void
SCIONEndPoint::SetRxEnabled(bool enabled)
{
    m_rxEnabled = enabled;
}

bool
SCIONEndPoint::IsRxEnabled() const
{
    return m_rxEnabled;
}

} // namespace ns3
