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

#include "scion-udp-l4-protocol.h"

#include "scion-end-point-demux.h"
#include "scion-end-point.h"
#include "ns3/scion.h"
#include "scion-l3-protocol.h"

#include "udp-header.h"
#include "udp-socket-factory-impl.h"
#include "udp-socket-impl.h"

#include "ns3/assert.h"
#include "ns3/boolean.h"
#include "ns3/scion-route.h"
#include "ns3/scion-header.h"
#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScionUdpL4Protocol");

NS_OBJECT_ENSURE_REGISTERED(ScionUdpL4Protocol);



TypeId
ScionUdpL4Protocol::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ScionUdpL4Protocol")
                            .SetParent<IpL4Protocol>()
                            .SetGroupName("Internet")
                            .AddConstructor<ScionUdpL4Protocol>()
                            .AddAttribute("SocketList",
                                          "The list of sockets associated to this protocol.",
                                          ObjectVectorValue(),
                                          MakeObjectVectorAccessor(&ScionUdpL4Protocol::m_sockets),
                                          MakeObjectVectorChecker<UdpSocketImpl>());
    return tid;
}

ScionUdpL4Protocol::ScionUdpL4Protocol()
    : m_endPoints(new SCIONEndPointDemux()),
{
    NS_LOG_FUNCTION(this);
}

ScionUdpL4Protocol::~ScionUdpL4Protocol()
{
    NS_LOG_FUNCTION(this);
}


/**
 * This method is called by AggregateObject and completes the aggregation
 * by setting the node in the udp stack and link it to the ipv4 object
 * present in the node along with the socket factory
 * 
 * \note since SCION UDP L4 protocol is a drop-in substitute/surrogate for UdpL4Protocol
 *      (and not an extension added on the side) we need to setup IPv4/v6 UDP (the base class) here as well,
 *      not only scion
 */
void
ScionUdpL4Protocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);

    this->UdpL4Protocol::NotifyNewAggregate();

    Ptr<Node> node = this->GetObject<Node>();
    Ptr<SCION> scion = this->GetObject<SCION>();

    if (!m_node)
    {
        if (node && scion)
        {
            this->SetNode(node);
            Ptr<UdpSocketFactoryImpl> udpFactory = CreateObject<UdpSocketFactoryImpl>();
            udpFactory->SetUdp(this);
            node->AggregateObject(udpFactory);
        }
    }
    if ( scion && m_downTarget.IsNull())
    {
        scion->Insert(this);
        this->SetDownTargetSCION(MakeCallback(&SCION::Send, scion));
    }

    /*
    Ptr<Node> node = this->GetObject<Node>();
    Ptr<Ipv4> ipv4 = this->GetObject<Ipv4>();
    Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();

    if (!m_node)
    {
        if (node && (ipv4 || ipv6))
        {
            this->SetNode(node);
            Ptr<UdpSocketFactoryImpl> udpFactory = CreateObject<UdpSocketFactoryImpl>();
            udpFactory->SetUdp(this);
            node->AggregateObject(udpFactory);
        }
    }

    // We set at least one of our 2 down targets to the IPv4/IPv6 send
    // functions.  Since these functions have different prototypes, we
    // need to keep track of whether we are connected to an IPv4 or
    // IPv6 lower layer and call the appropriate one.

    if (ipv4 && m_downTarget.IsNull())
    {
        ipv4->Insert(this);
        this->SetDownTarget(MakeCallback(&Ipv4::Send, ipv4));
    }
    if (ipv6 && m_downTarget6.IsNull())
    {
        ipv6->Insert(this);
        this->SetDownTarget6(MakeCallback(&Ipv6::Send, ipv6));
    }
    IpL4Protocol::NotifyNewAggregate();
    */
}

void
ScionUdpL4Protocol::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (std::vector<Ptr<UdpSocketImpl>>::iterator i = m_sockets.begin(); i != m_sockets.end(); i++)
    {
        *i = nullptr;
    }
    m_sockets.clear();

    if (m_endPoints != nullptr)
    {
        delete m_endPoints;
        m_endPoints = nullptr;
    }

    m_downTarget.Nullify();

    /*
     = MakeNullCallback<void,Ptr<Packet>, SCIONAddress, SCIONAddress, uint8_t, Ptr<Ipv4Route> > ();
    */
    UdpL4Protocol::DoDispose();
}

Ptr<Socket>
ScionUdpL4Protocol::CreateSocket() // does this even need overriding ?! currently its identical to UdpL4Protocol's impl.
{
    NS_LOG_FUNCTION(this);
    Ptr<UdpSocketImpl> socket = CreateObject<UdpSocketImpl>();
    socket->SetNode(m_node);
    socket->SetUdp(this);
    m_sockets.push_back(socket);
    return socket;
}

SCIONEndPoint*
ScionUdpL4Protocol::Allocate()
{
    NS_LOG_FUNCTION(this);
    return m_endPoints->Allocate();
}

SCIONEndPoint*
ScionUdpL4Protocol::Allocate(SCIONAddress address)
{
    NS_LOG_FUNCTION(this << address);
    return m_endPoints->Allocate(address);
}

SCIONEndPoint*
ScionUdpL4Protocol::Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port)
{
    NS_LOG_FUNCTION(this << boundNetDevice << port);
    return m_endPoints->Allocate(boundNetDevice, port);
}

SCIONEndPoint*
ScionUdpL4Protocol::Allocate(Ptr<NetDevice> boundNetDevice, SCIONAddress address, uint16_t port)
{
    NS_LOG_FUNCTION(this << boundNetDevice << address << port);
    return m_endPoints->Allocate(boundNetDevice, address, port);
}

SCIONEndPoint*
ScionUdpL4Protocol::Allocate(Ptr<NetDevice> boundNetDevice,
                        SCIONAddress localAddress,
                        uint16_t localPort,
                        SCIONAddress peerAddress,
                        uint16_t peerPort)
{
    NS_LOG_FUNCTION(this << boundNetDevice << localAddress << localPort << peerAddress << peerPort);
    return m_endPoints->Allocate(boundNetDevice, localAddress, localPort, peerAddress, peerPort);
}

void
ScionUdpL4Protocol::DeAllocate(SCIONEndPoint* endPoint)
{
    NS_LOG_FUNCTION(this << endPoint);
    m_endPoints->DeAllocate(endPoint);
}

void
ScionUdpL4Protocol::ReceiveScmp(SCIONAddress scmpSource,
                           uint8_t scmpTtl,
                           uint8_t scmpType,
                           uint8_t scmpCode,
                           uint32_t scmpInfo,
                           SCIONAddress payloadSource,
                           SCIONAddress payloadDestination,
                           const uint8_t payload[8])
{
    NS_LOG_FUNCTION(this << scmpSource << scmpTtl << scmpType << scmpCode << scmpInfo
                         << payloadSource << payloadDestination);
    uint16_t src; // source port
    uint16_t dst; // destination port
    src = payload[0] << 8;
    src |= payload[1];
    dst = payload[2] << 8;
    dst |= payload[3];

    SCIONEndPoint* endPoint = m_endPoints->SimpleLookup(payloadSource, src, payloadDestination, dst);
    if (endPoint != nullptr)
    {
        endPoint->ForwardScmp(scmpSource, scmpTtl, scmpType, scmpCode, scmpInfo);
    }
    else
    {
        NS_LOG_DEBUG("no endpoint found source=" << payloadSource
                                                 << ", destination=" << payloadDestination
                                                 << ", src=" << src << ", dst=" << dst);
    }
}


enum IpL4Protocol::RxStatus
ScionUdpL4Protocol::Receive(Ptr<Packet> packet, const SCIONHeader& header, Ptr<SCIONInterface> interface)
{
    NS_LOG_FUNCTION(this << packet << header);
    UdpHeader udpHeader;
    if (Node::ChecksumEnabled())
    {
        udpHeader.EnableChecksums();
    }

    udpHeader.InitializeChecksum(header.GetSource(), header.GetDestination(), UdpL4Protocol::PROT_NUMBER);

    packet->PeekHeader(udpHeader);

    if (!udpHeader.IsChecksumOk())
    {
        NS_LOG_INFO("Bad checksum : dropping packet!");
        return IpL4Protocol::RX_CSUM_FAILED;
    }

    NS_LOG_DEBUG("Looking up dst " << header.GetDestination() << " port "
                                   << udpHeader.GetDestinationPort());
    SCIONEndPointDemux::EndPoints endPoints = m_endPoints->Lookup(header.GetDestination(),
                                                                 udpHeader.GetDestinationPort(),
                                                                 header.GetSource(),
                                                                 udpHeader.GetSourcePort(),
                                                                 interface);
    if (endPoints.empty())
    {
        NS_LOG_LOGIC("RX_ENDPOINT_UNREACH");
        return IpL4Protocol::RX_ENDPOINT_UNREACH;
    }

    packet->RemoveHeader(udpHeader);
    for (SCIONEndPointDemux::EndPointsI endPoint = endPoints.begin(); endPoint != endPoints.end();
         endPoint++)
    {
        (*endPoint)->ForwardUp(packet->Copy(), header, udpHeader.GetSourcePort(), interface);
    }
    return IpL4Protocol::RX_OK;
}


void
ScionUdpL4Protocol::Send(Ptr<Packet> packet,
                    SCIONAddress saddr,
                    SCIONAddress daddr,
                    uint16_t sport,
                    uint16_t dport)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << sport << dport);

    UdpHeader udpHeader;
    if (Node::ChecksumEnabled())
    {
        udpHeader.EnableChecksums();
        udpHeader.InitializeChecksum(saddr, daddr, UdpL4Protocol::PROT_NUMBER);
    }
    udpHeader.SetDestinationPort(dport);
    udpHeader.SetSourcePort(sport);

    packet->AddHeader(udpHeader);

    m_downTarget(packet, saddr, daddr, UdpL4Protocol::PROT_NUMBER, nullptr);
}

void
ScionUdpL4Protocol::Send(Ptr<Packet> packet,
                    SCIONAddress saddr,
                    SCIONAddress daddr,
                    uint16_t sport,
                    uint16_t dport,
                    Ptr<SCIONRoute> route)
{
    NS_LOG_FUNCTION(this << packet << saddr << daddr << sport << dport << route);

    UdpHeader udpHeader;
    if (Node::ChecksumEnabled())
    {
        udpHeader.EnableChecksums();
        udpHeader.InitializeChecksum(saddr, daddr, UdpL4Protocol::PROT_NUMBER);
    }
    udpHeader.SetDestinationPort(dport);
    udpHeader.SetSourcePort(sport);

    packet->AddHeader(udpHeader);

    m_downTarget(packet, saddr, daddr, UdpL4Protocol::PROT_NUMBER, route);
}


void
ScionUdpL4Protocol::SetDownTargetSCION(SCIONL4Protocol::DownTargetCallbackSCION callback)
{
    NS_LOG_FUNCTION(this);
    m_downTarget = callback;
}

SCIONL4Protocol::DownTargetCallbackSCION
ScionUdpL4Protocol::GetDownTargetSCION() const
{
    return m_downTarget;
}


} // namespace ns3
