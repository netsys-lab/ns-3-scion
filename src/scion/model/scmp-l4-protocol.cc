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

#include "scmp-l4-protocol.h"

#include "scion-interface.h"
#include "scion-l3-protocol.h"
#include "scion-raw-socket-factory-impl.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/scion-route.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScmpL4Protocol");

NS_OBJECT_ENSURE_REGISTERED(ScmpL4Protocol);


const uint8_t ScmpL4Protocol::PROT_NUMBER = 1;

TypeId
ScmpL4Protocol::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ScmpL4Protocol")
                            .SetParent<SCIONL4Protocol>()
                            .SetGroupName("Internet")
                            .AddConstructor<ScmpL4Protocol>();
    return tid;
}

ScmpL4Protocol::ScmpL4Protocol()
    : m_node(nullptr)
{
    NS_LOG_FUNCTION(this);
}

ScmpL4Protocol::~ScmpL4Protocol()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT(!m_node);
}

void
ScmpL4Protocol::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

/*
 * This method is called by AggregateObject and completes the aggregation
 * by setting the node in the ICMP stack and adding ICMP factory to
 * IPv4 stack connected to the node
 */
void
ScmpL4Protocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);
    if (!m_node)
    {
        Ptr<Node> node = this->GetObject<Node>();
        if (node)
        {
            Ptr<SCION> scion = this->GetObject<SCION>();
            if (scion && m_downTarget.IsNull())
            {
                this->SetNode(node);
                scion->Insert(this);
                Ptr<SCIONRawSocketFactoryImpl> rawFactory = CreateObject<SCIONRawSocketFactoryImpl>();
                scion->AggregateObject(rawFactory);
                this->SetDownTarget(MakeCallback(&SCION::Send, scion));
            }
        }
    }
    IpL4Protocol::NotifyNewAggregate();
}

uint16_t
ScmpL4Protocol::GetStaticProtocolNumber()
{
    NS_LOG_FUNCTION_NOARGS();
    return PROT_NUMBER;
}

int
ScmpL4Protocol::GetProtocolNumber() const
{
    NS_LOG_FUNCTION(this);
    return PROT_NUMBER;
}

void
ScmpL4Protocol::SendMessage(Ptr<Packet> packet, SCIONAddress dest, uint8_t type, uint8_t code)
{
    NS_LOG_FUNCTION(this << packet << dest << static_cast<uint32_t>(type)
                         << static_cast<uint32_t>(code));
    auto scion = m_node->GetObject<SCION>();
    NS_ASSERT(scion && scion->GetRoutingProtocol());
    SCIONHeader header;
    header.SetDstAddress(dest);
    header.SetProtocol(PROT_NUMBER);
    Socket::SocketErrno errno_;
    Ptr<SCIONRoute> route;
    Ptr<NetDevice> oif(nullptr); // specify non-zero if bound to a source address
    route = scion->GetRoutingProtocol()->RouteOutput(packet, header, oif, errno_);
    if (route)
    {
        NS_LOG_LOGIC("Route exists");
        SCIONAddress source = route->GetSource();
        SendMessage(packet, source, dest, type, code, route);
    }
    else
    {
        NS_LOG_WARN("drop icmp message");
    }
}

void
ScmpL4Protocol::SendMessage(Ptr<Packet> packet,
                              SCIONAddress source,
                              SCIONAddress dest,
                              uint8_t type,
                              uint8_t code,
                              Ptr<SCIONRoute> route)
{
    NS_LOG_FUNCTION(this << packet << source << dest << static_cast<uint32_t>(type)
                         << static_cast<uint32_t>(code) << route);
    ScmpHeader icmp;
    icmp.SetType(type);
    icmp.SetCode(code);
    if (Node::ChecksumEnabled())
    {
        icmp.EnableChecksum();
    }
    packet->AddHeader(icmp);

    m_downTarget(packet, source, dest, PROT_NUMBER, route);
}

void
ScmpL4Protocol::SendDestUnreachFragNeeded(SCIONHeader header,
                                            Ptr<const Packet> orgData,
                                            uint16_t nextHopMtu)
{
    NS_LOG_FUNCTION(this << header << *orgData << nextHopMtu);
    SendDestUnreach(header, orgData, Icmpv4DestinationUnreachable::ICMPV4_FRAG_NEEDED, nextHopMtu);
}

void
ScmpL4Protocol::SendDestUnreachPort(SCIONHeader header, Ptr<const Packet> orgData)
{
    NS_LOG_FUNCTION(this << header << *orgData);
    SendDestUnreach(header, orgData, Icmpv4DestinationUnreachable::ICMPV4_PORT_UNREACHABLE, 0);
}

void
ScmpL4Protocol::SendDestUnreach(SCIONHeader header,
                                  Ptr<const Packet> orgData,
                                  uint8_t code,
                                  uint16_t nextHopMtu)
{
    NS_LOG_FUNCTION(this << header << *orgData << (uint32_t)code << nextHopMtu);
    Ptr<Packet> p = Create<Packet>();
    Icmpv4DestinationUnreachable unreach;
    unreach.SetNextHopMtu(nextHopMtu);
    unreach.SetHeader(header);
    unreach.SetData(orgData);
    p->AddHeader(unreach);
    SendMessage(p, header.GetSource(), Icmpv4Header::ICMPV4_DEST_UNREACH, code);
}

void
ScmpL4Protocol::SendTimeExceededTtl(SCIONHeader header, Ptr<const Packet> orgData, bool isFragment)
{
    NS_LOG_FUNCTION(this << header << *orgData);
    Ptr<Packet> p = Create<Packet>();
    Icmpv4TimeExceeded time;
    time.SetHeader(header);
    time.SetData(orgData);
    p->AddHeader(time);
    if (!isFragment)
    {
        SendMessage(p,
                    header.GetSource(),
                    Icmpv4Header::ICMPV4_TIME_EXCEEDED,
                    Icmpv4TimeExceeded::ICMPV4_TIME_TO_LIVE);
    }
    else
    {
        SendMessage(p,
                    header.GetSource(),
                    Icmpv4Header::ICMPV4_TIME_EXCEEDED,
                    Icmpv4TimeExceeded::ICMPV4_FRAGMENT_REASSEMBLY);
    }
}

void
ScmpL4Protocol::HandleEcho(Ptr<Packet> p,
                             Icmpv4Header header,
                             SCIONAddress source,
                             SCIONAddress destination)
{
    NS_LOG_FUNCTION(this << p << header << source << destination);

    Ptr<Packet> reply = Create<Packet>();
    Icmpv4Echo echo;
    p->RemoveHeader(echo);
    reply->AddHeader(echo);
    SendMessage(reply, destination, source, Icmpv4Header::ICMPV4_ECHO_REPLY, 0, nullptr);
}

void
ScmpL4Protocol::Forward(SCIONAddress source,
                          Icmpv4Header icmp,
                          uint32_t info,
                          SCIONHeader ipHeader,
                          const uint8_t payload[8])
{
    NS_LOG_FUNCTION(this << source << icmp << info << ipHeader << payload);

    Ptr<Ipv4> ipv4 = m_node->GetObject<Ipv4>();
    Ptr<IpL4Protocol> l4 = ipv4->GetProtocol(ipHeader.GetProtocol());
    if (l4)
    {
        l4->ReceiveIcmp(source,
                        ipHeader.GetTtl(),
                        icmp.GetType(),
                        icmp.GetCode(),
                        info,
                        ipHeader.GetSource(),
                        ipHeader.GetDestination(),
                        payload);
    }
}

void
ScmpL4Protocol::HandleDestUnreach(Ptr<Packet> p,
                                    Icmpv4Header icmp,
                                    SCIONAddress source,
                                    SCIONAddress destination)
{
    NS_LOG_FUNCTION(this << p << icmp << source << destination);

    Icmpv4DestinationUnreachable unreach;
    p->PeekHeader(unreach);
    uint8_t payload[8];
    unreach.GetData(payload);
    SCIONHeader ipHeader = unreach.GetHeader();
    Forward(source, icmp, unreach.GetNextHopMtu(), ipHeader, payload);
}

void
ScmpL4Protocol::HandleTimeExceeded(Ptr<Packet> p,
                                     Icmpv4Header icmp,
                                     SCIONAddress source,
                                     SCIONAddress destination)
{
    NS_LOG_FUNCTION(this << p << icmp << source << destination);

    Icmpv4TimeExceeded time;
    p->PeekHeader(time);
    uint8_t payload[8];
    time.GetData(payload);
    SCIONHeader ipHeader = time.GetHeader();
    // info field is zero for TimeExceeded on linux
    Forward(source, icmp, 0, ipHeader, payload);
}

enum IpL4Protocol::RxStatus
ScmpL4Protocol::Receive(Ptr<Packet> p,
                          const SCIONHeader& header,
                          Ptr<Ipv4Interface> incomingInterface)
{
    NS_LOG_FUNCTION(this << p << header << incomingInterface);

    Icmpv4Header icmp;
    p->RemoveHeader(icmp);
    switch (icmp.GetType())
    {
    case Icmpv4Header::ICMPV4_ECHO: {
        SCIONAddress dst = header.GetDestination();
        // We could have received an Echo request to a broadcast-type address.
        if (dst.IsBroadcast())
        {
            SCIONAddress src = header.GetSource();
            for (uint32_t index = 0; index < incomingInterface->GetNAddresses(); index++)
            {
                Ipv4InterfaceAddress addr = incomingInterface->GetAddress(index);
                if (addr.IsInSameSubnet(src))
                {
                    dst = addr.GetAddress();
                }
            }
        }
        else
        {
            for (uint32_t index = 0; index < incomingInterface->GetNAddresses(); index++)
            {
                Ipv4InterfaceAddress addr = incomingInterface->GetAddress(index);
                if (dst == addr.GetBroadcast())
                {
                    dst = addr.GetAddress();
                }
            }
        }
        HandleEcho(p, icmp, header.GetSource(), dst);
        break;
    }
    case Icmpv4Header::ICMPV4_DEST_UNREACH:
        HandleDestUnreach(p, icmp, header.GetSource(), header.GetDestination());
        break;
    case Icmpv4Header::ICMPV4_TIME_EXCEEDED:
        HandleTimeExceeded(p, icmp, header.GetSource(), header.GetDestination());
        break;
    default:
        NS_LOG_DEBUG(icmp << " " << *p);
        break;
    }
    return IpL4Protocol::RX_OK;
}

enum IpL4Protocol::RxStatus
ScmpL4Protocol::Receive(Ptr<Packet> p,
                          const Ipv6Header& header,
                          Ptr<Ipv6Interface> incomingInterface)
{
    NS_LOG_FUNCTION(this << p << header.GetSource() << header.GetDestination()
                         << incomingInterface);
    return IpL4Protocol::RX_ENDPOINT_UNREACH;
}

void
ScmpL4Protocol::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_node = nullptr;
    m_downTarget.Nullify();
    IpL4Protocol::DoDispose();
}

void
ScmpL4Protocol::SetDownTarget(IpL4Protocol::DownTargetCallback callback)
{
    NS_LOG_FUNCTION(this << &callback);
    m_downTarget = callback;
}

void
ScmpL4Protocol::SetDownTarget6(IpL4Protocol::DownTargetCallback6 callback)
{
    NS_LOG_FUNCTION(this << &callback);
}

IpL4Protocol::DownTargetCallback
ScmpL4Protocol::GetDownTarget() const
{
    NS_LOG_FUNCTION(this);
    return m_downTarget;
}

IpL4Protocol::DownTargetCallback6
ScmpL4Protocol::GetDownTarget6() const
{
    NS_LOG_FUNCTION(this);
    return IpL4Protocol::DownTargetCallback6();
}

} // namespace ns3
