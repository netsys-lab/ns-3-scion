//
// Copyright (c)
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
//

#include "scion-l3-protocol.h"


#include "scmp-l4-protocol.h"
#include "scion-interface.h"
#include "scion-raw-socket-impl.h"
#include "ns3/loopback-net-device.h"

#include "ns3/boolean.h"
#include "ns3/callback.h"
#include "ns3/scion-address.h"
#include "ns3/scion-header.h"
#include "ns3/scion-route.h"

#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/object-vector.h"
#include "ns3/packet.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/uinteger.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONL3Protocol");

const uint16_t SCIONL3Protocol::PROT_NUMBER = 3333; // just any un-assigned number for now

NS_OBJECT_ENSURE_REGISTERED(SCIONL3Protocol);

TypeId
SCIONL3Protocol::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SCIONL3Protocol")
            .SetParent<SCION>()
            .SetGroupName("Internet")
            .AddConstructor<SCIONL3Protocol>()            
            .AddTraceSource("Tx",
                            "Send SCION packet to outgoing interface.",
                            MakeTraceSourceAccessor(&SCIONL3Protocol::m_txTrace),
                            "ns3::SCIONL3Protocol::TxRxTracedCallback")
            .AddTraceSource("Rx",
                            "Receive SCION packet from incoming interface.",
                            MakeTraceSourceAccessor(&SCIONL3Protocol::m_rxTrace),
                            "ns3::SCIONL3Protocol::TxRxTracedCallback")
            .AddTraceSource("Drop",
                            "Drop SCION packet",
                            MakeTraceSourceAccessor(&SCIONL3Protocol::m_dropTrace),
                            "ns3::SCIONL3Protocol::DropTracedCallback")
            .AddAttribute("InterfaceList",
                          "The set of SCION interfaces associated to this SCION stack.",
                          ObjectVectorValue(),
                          MakeObjectVectorAccessor(&SCIONL3Protocol::m_interfaces),
                          MakeObjectVectorChecker<SCIONInterface>())

            .AddTraceSource("SendOutgoing",
                            "A newly-generated packet by this node is "
                            "about to be queued for transmission",
                            MakeTraceSourceAccessor(&SCIONL3Protocol::m_sendOutgoingTrace),
                            "ns3::SCIONL3Protocol::SentTracedCallback")          
            .AddTraceSource("LocalDeliver",
                            "A SCION packet was received by/for this node, "
                            "and it is being forward up the stack",
                            MakeTraceSourceAccessor(&SCIONL3Protocol::m_localDeliverTrace),
                            "ns3::SCIONL3Protocol::SentTracedCallback")

        ;
    return tid;
}

SCIONL3Protocol::SCIONL3Protocol()
{
    NS_LOG_FUNCTION(this);
}

SCIONL3Protocol::~SCIONL3Protocol()
{
    NS_LOG_FUNCTION(this);
}

void
SCIONL3Protocol::Insert(Ptr<SCIONL4Protocol> protocol)
{
    NS_LOG_FUNCTION(this << protocol);
    L4ListKey_t key = std::make_pair(protocol->GetProtocolNumber(), -1);
    if (m_protocols.find(key) != m_protocols.end())
    {
        NS_LOG_WARN("Overwriting default protocol " << int(protocol->GetProtocolNumber()));
    }
    m_protocols[key] = protocol;
}

void
SCIONL3Protocol::Insert(Ptr<SCIONL4Protocol> protocol, uint32_t interfaceIndex)
{
    NS_LOG_FUNCTION(this << protocol << interfaceIndex);

    L4ListKey_t key = std::make_pair(protocol->GetProtocolNumber(), interfaceIndex);
    if (m_protocols.find(key) != m_protocols.end())
    {
        NS_LOG_WARN("Overwriting protocol " << int(protocol->GetProtocolNumber())
                                            << " on interface " << int(interfaceIndex));
    }
    m_protocols[key] = protocol;
}

void
SCIONL3Protocol::Remove(Ptr<SCIONL4Protocol> protocol)
{
    NS_LOG_FUNCTION(this << protocol);

    L4ListKey_t key = std::make_pair(protocol->GetProtocolNumber(), -1);
    L4List_t::iterator iter = m_protocols.find(key);
    if (iter == m_protocols.end())
    {
        NS_LOG_WARN("Trying to remove an non-existent default protocol "
                    << int(protocol->GetProtocolNumber()));
    }
    else
    {
        m_protocols.erase(key);
    }
}

void
SCIONL3Protocol::Remove(Ptr<SCIONL4Protocol> protocol, uint32_t interfaceIndex)
{
    NS_LOG_FUNCTION(this << protocol << interfaceIndex);

    L4ListKey_t key = std::make_pair(protocol->GetProtocolNumber(), interfaceIndex);
    L4List_t::iterator iter = m_protocols.find(key);
    if (iter == m_protocols.end())
    {
        NS_LOG_WARN("Trying to remove an non-existent protocol "
                    << int(protocol->GetProtocolNumber()) << " on interface "
                    << int(interfaceIndex));
    }
    else
    {
        m_protocols.erase(key);
    }
}

Ptr<SCIONL4Protocol>
SCIONL3Protocol::GetProtocol(int protocolNumber) const
{
    NS_LOG_FUNCTION(this << protocolNumber);

    return GetProtocol(protocolNumber, -1);
}

Ptr<SCIONL4Protocol>
SCIONL3Protocol::GetProtocol(int protocolNumber, int32_t interfaceIndex) const
{
    NS_LOG_FUNCTION(this << protocolNumber << interfaceIndex);

    L4ListKey_t key;
    L4List_t::const_iterator i;
    if (interfaceIndex >= 0)
    {
        // try the interface-specific protocol.
        key = std::make_pair(protocolNumber, interfaceIndex);
        i = m_protocols.find(key);
        if (i != m_protocols.end())
        {
            return i->second;
        }
    }
    // try the generic protocol.
    key = std::make_pair(protocolNumber, -1);
    i = m_protocols.find(key);
    if (i != m_protocols.end())
    {
        return i->second;
    }

    return nullptr;
}

void
SCIONL3Protocol::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
    // Add a LoopbackNetDevice if needed, and an SCIONInterface on top of it
    SetupLoopback();
}

Ptr<Socket>
SCIONL3Protocol::CreateRawSocket()
{
    NS_LOG_FUNCTION(this);
    Ptr<SCIONRawSocketImpl> socket = CreateObject<SCIONRawSocketImpl>();
    socket->SetNode(m_node);
    m_sockets.push_back(socket);
    return socket;
}

void
SCIONL3Protocol::DeleteRawSocket(Ptr<Socket> socket)
{
    NS_LOG_FUNCTION(this << socket);
    for (SocketList::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
    {
        if ((*i) == socket)
        {
            m_sockets.erase(i);
            return;
        }
    }
}

/*
 * This method is called by AggregateObject and completes the aggregation
 * by setting the node in the ipv4 stack
 */
void
SCIONL3Protocol::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);
    if (!m_node)
    {
        Ptr<Node> node = this->GetObject<Node>();
        // verify that it's a valid node and that
        // the node has not been set before
        if (node)
        {
            this->SetNode(node);
        }
    }
    SCION::NotifyNewAggregate();
}


void
SCIONL3Protocol::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (L4List_t::iterator i = m_protocols.begin(); i != m_protocols.end(); ++i)
    {
        i->second = nullptr;
    }
    m_protocols.clear();

    for (SCIONInterfaceList::iterator i = m_interfaces.begin(); i != m_interfaces.end(); ++i)
    {
        *i = nullptr;
    }
    m_interfaces.clear();
    m_reverseInterfacesContainer.clear();

    m_sockets.clear();
    m_node = nullptr;

    Object::DoDispose();
}

void
SCIONL3Protocol::SetupLoopback()
{
    NS_LOG_FUNCTION(this);

    Ptr<SCIONInterface> interface = CreateObject<SCIONInterface>();
    Ptr<LoopbackNetDevice> device = nullptr;
    // First check whether an existing LoopbackNetDevice exists on the node
    for (uint32_t i = 0; i < m_node->GetNDevices(); i++)
    {
        if ((device = DynamicCast<LoopbackNetDevice>(m_node->GetDevice(i))))
        {
            break;
        }
    }
    if (!device)
    {
        device = CreateObject<LoopbackNetDevice>();
        m_node->AddDevice(device);
    }
    interface->SetDevice(device);
    interface->SetNode(m_node);
    auto ifaceAddr = SCIONInterfaceAddress(SCIONAddress::GetLoopback());
    interface->AddAddress(ifaceAddr);
    uint32_t index = AddSCIONInterface(interface);
    Ptr<Node> node = GetObject<Node>();
    node->RegisterProtocolHandler(MakeCallback(&SCIONL3Protocol::Receive, this),
                                  SCIONL3Protocol::PROT_NUMBER,
                                  device);
    interface->SetUp();
}

/**
 * \param underlay_if an IPv4 interface of the same Node.
 *      The SCION Interface will receive any SCION(over IPv4) packets,
 *      that are LocallyDelivered() by the Ipv4L3Protocol from this Interface.
 *      The HostPortion of the interfaces SCION Address will be inherited from
 *      the underlay interface. If for whatever reason the (outer-) IP packet
 *      is dropped (i.e. because of BadChecksum, TTL_Expired, InterfaceDown)
 *      the SCION interface will never get to see the enclosed SCION packet.
 *      Also potential fragments of an enclosed SCION packet are held back,
 *      until the full packet can be reconstruced. (SCION does no fragmentation on its own)
 */
uint32_t SCIONL3Protocol::AddInterface(Ptr<Ipv4Interface> underlay_if )
{

/*  What the Ipv4L3Protocol did:

    Ptr<TrafficControlLayer> tc = m_node->GetObject<TrafficControlLayer>();
    NS_ASSERT(tc);
    m_node->RegisterProtocolHandler(MakeCallback(&TrafficControlLayer::Receive, tc),
                                    Ipv4L3Protocol::PROT_NUMBER,
                                    device);
    
    tc->RegisterProtocolHandler(MakeCallback(&Ipv4L3Protocol::Receive, this),
                                Ipv4L3Protocol::PROT_NUMBER,
                                device); 
 */


    auto ipv4 =Get<Ipv4>(); // Ipv4L3Protocol
    auto ndev = underlay_if->GetDevice();
    NS_ASSERT(ndev);
    // underlay_if is the 'ifidx'-th IPv4 interface of this node
    auto ifidx = ipv4->GetInterfaceForDevice(ndev);
    ipv4->Insert(this, ifidx);


    Ptr<SCIONInterface> interface = CreateObject<SCIONInterface>();
    interface->SetNode(m_node);
    interface->SetDevice(ndev);
    //for IN direction this wont have any effect, because this IF never receives from L2 (TrafficControl) directly,
    // but through the indirection of the underlay protocol. Which however though is policed by the TC layer.
    interface->SetTrafficControl(tc);
    interface->SetForwarding(m_scionForward);
    return AddSCIONInterface(interface);

}

    uint32_t SCIONL3Protocol::AddInterface(Ptr<Ipv6Interface> underlay_if) 
    {
        NS_ASSERT_MSG(false, "not implemented yet");
        return 0;
    }

uint32_t
SCIONL3Protocol::AddInterface(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    NS_ASSERT(m_node);

    Ptr<TrafficControlLayer> tc = m_node->GetObject<TrafficControlLayer>();

    NS_ASSERT(tc);

    m_node->RegisterProtocolHandler(MakeCallback(&TrafficControlLayer::Receive, tc),
                                    SCIONL3Protocol::PROT_NUMBER,
                                    device);
    
    tc->RegisterProtocolHandler(MakeCallback(&SCIONL3Protocol::Receive, this),
                                SCIONL3Protocol::PROT_NUMBER,
                                device);

    Ptr<SCIONInterface> interface = CreateObject<SCIONInterface>();
    interface->SetNode(m_node);
    interface->SetDevice(device);
    interface->SetTrafficControl(tc);
    interface->SetForwarding(m_scionForward);
    return AddSCIONInterface(interface);
}

uint32_t
SCIONL3Protocol::AddSCIONInterface(Ptr<SCIONInterface> interface)
{
    NS_LOG_FUNCTION(this << interface);
    uint32_t index = m_interfaces.size();
    m_interfaces.push_back(interface);
    m_reverseInterfacesContainer[interface->GetDevice()] = index;
    return index;
}

Ptr<SCIONInterface>
SCIONL3Protocol::GetInterface(uint32_t index) const
{
    NS_LOG_FUNCTION(this << index);
    if (index < m_interfaces.size())
    {
        return m_interfaces[index];
    }
    return nullptr;
}

uint32_t
SCIONL3Protocol::GetNInterfaces() const
{
    NS_LOG_FUNCTION(this);
    return m_interfaces.size();
}

int32_t
SCIONL3Protocol::GetInterfaceForAddress(SCIONAddress address) const
{
    NS_LOG_FUNCTION(this << address);
    int32_t interface = 0;
    for (SCIONInterfaceList::const_iterator i = m_interfaces.begin(); i != m_interfaces.end();
         i++, interface++)
    {
        for (uint32_t j = 0; j < (*i)->GetNAddresses(); j++)
        {
            if ((*i)->GetAddress(j).GetLocal() == address)
            {
                return interface;
            }
        }
    }

    return -1;
}


int32_t
SCIONL3Protocol::GetInterfaceForDevice(Ptr<const NetDevice> device) const
{
    NS_LOG_FUNCTION(this << device);

    SCIONInterfaceReverseContainer::const_iterator iter = m_reverseInterfacesContainer.find(device);
    if (iter != m_reverseInterfacesContainer.end())
    {
        return (*iter).second;
    }

    return -1;
}

/**
 * \brief check if a packet with the given destination address 
 *      is meant for delivery(up the stack) on the local node
 */
bool
SCIONL3Protocol::IsDestinationAddress(SCIONAddress address, uint32_t iif) const
{
    NS_LOG_FUNCTION(this << address << iif);
    // First check the incoming interface for a unicast address match
    for (uint32_t i = 0; i < GetNAddresses(iif); i++)
    {
        SCIONInterfaceAddress iaddr = GetAddress(iif, i);
        if (address == iaddr.GetAddress())
        {
            NS_LOG_LOGIC("For me (destination " << address << " match)");
            return true;
        }
        /*
        if (address == iaddr.GetBroadcast())
        {
            NS_LOG_LOGIC("For me (interface broadcast address)");
            return true;
        }*/
    }
    /*
    if (address.IsMulticast())
    {
#ifdef NOTYET
        if (MulticastCheckGroup(iif, address))
#endif
            if (true)
            {
                NS_LOG_LOGIC("For me (Ipv4Addr multicast address)");
                return true;
            }
    }

    if (address.IsBroadcast())
    {
        NS_LOG_LOGIC("For me (Ipv4Addr broadcast address)");
        return true;
    }*/

    if (GetWeakEsModel()) // Check other interfaces
    {
        for (uint32_t j = 0; j < GetNInterfaces(); j++)
        {
            if (j == uint32_t(iif))
            {
                continue;
            }
            for (uint32_t i = 0; i < GetNAddresses(j); i++)
            {
                SCIONInterfaceAddress iaddr = GetAddress(j, i);
                if (address == iaddr.GetAddress())
                {
                    NS_LOG_LOGIC("For me (destination " << address
                                                        << " match) on another interface");
                    return true;
                }
                /*
                //  This is a small corner case:  match another interface's broadcast address
                if (address == iaddr.GetBroadcast())
                {
                    NS_LOG_LOGIC("For me (interface broadcast address on another interface)");
                    return true;
                }
                */
            }
        }
    }
    return false;
}

void
SCIONL3Protocol::Receive(Ptr<NetDevice> device,
                        Ptr<const Packet> p,
                        uint16_t protocol,
                        const Address& from,
                        const Address& to,
                        NetDevice::PacketType packetType)
{
    NS_LOG_FUNCTION(this << device << p << protocol << from << to << packetType);

    NS_LOG_LOGIC("Packet from " << from << " received on node " << m_node->GetId());

    int32_t interface = GetInterfaceForDevice(device);
    NS_ASSERT_MSG(interface != -1, "Received a packet from an interface that is not known to IPv4");

    Ptr<Packet> packet = p->Copy();

    Ptr<SCIONInterface> SCIONInterface = m_interfaces[interface];

    if (SCIONInterface->IsUp())
    {
        m_rxTrace(packet, this, interface);
    }
    else
    {
        NS_LOG_LOGIC("Dropping received packet -- interface is down");
        SCIONHeader ipHeader;
        packet->RemoveHeader(ipHeader);
        m_dropTrace(ipHeader, packet, DROP_INTERFACE_DOWN, this, interface);
        return;
    }

    SCIONHeader ipHeader;
    if (Node::ChecksumEnabled())
    {
        ipHeader.EnableChecksum();
    }
    packet->RemoveHeader(ipHeader);

    // Trim any residual frame padding from underlying devices
    if (ipHeader.GetPayloadSize() < packet->GetSize())
    {
        packet->RemoveAtEnd(packet->GetSize() - ipHeader.GetPayloadSize());
    }

    /*
    // SCION header has no checksum
    if (!ipHeader.IsChecksumOk())
    {
        NS_LOG_LOGIC("Dropping received packet -- checksum not ok");
        m_dropTrace(ipHeader, packet, DROP_BAD_CHECKSUM, this, interface);
        return;
    }

    // the packet is valid, we update the ARP cache entry (if present)
    Ptr<ArpCache> arpCache = SCIONInterface->GetArpCache();
    if (arpCache)
    {
        // case one, it's a a direct routing.
        ArpCache::Entry* entry = arpCache->Lookup(ipHeader.GetSource());
        if (entry)
        {
            if (entry->IsAlive())
            {
                entry->UpdateSeen();
            }
        }
        else
        {
            // It's not in the direct routing, so it's the router, and it could have multiple IP
            // addresses. In doubt, update all of them. Note: it's a confirmed behavior for Linux
            // routers.
            std::list<ArpCache::Entry*> entryList = arpCache->LookupInverse(from);
            std::list<ArpCache::Entry*>::iterator iter;
            for (iter = entryList.begin(); iter != entryList.end(); iter++)
            {
                if ((*iter)->IsAlive())
                {
                    (*iter)->UpdateSeen();
                }
            }
        }
    }
    */

    for (SocketList::iterator i = m_sockets.begin(); i != m_sockets.end(); ++i)
    {
        NS_LOG_LOGIC("Forwarding to raw socket");
        Ptr<SCIONRawSocketImpl> socket = *i;
        socket->ForwardUp(packet, ipHeader, SCIONInterface);
    }

    if (m_enableDpd && ipHeader.GetDestination().IsMulticast() && UpdateDuplicate(packet, ipHeader))
    {
        NS_LOG_LOGIC("Dropping received packet -- duplicate.");
        m_dropTrace(ipHeader, packet, DROP_DUPLICATE, this, interface);
        return;
    }

    NS_ASSERT_MSG(m_routingProtocol, "Need a routing protocol object to process packets");
    if (!m_routingProtocol->RouteInput(packet,
                                       ipHeader,
                                       device,                                     
                                       MakeCallback(&SCIONL3Protocol::LocalDeliver, this),
                                       MakeCallback(&SCIONL3Protocol::RouteInputError, this)))
    {
        NS_LOG_WARN("No route found for forwarding packet.  Drop.");
        m_dropTrace(ipHeader, packet, DROP_NO_ROUTE, this, interface);
    }
}

Ptr<ScmpL4Protocol>
SCIONL3Protocol::GetScmp() const
{
    NS_LOG_FUNCTION(this);
    Ptr<SCIONL4Protocol> prot = GetProtocol(ScmpL4Protocol::GetStaticProtocolNumber());
    if (prot)
    {
        return prot->GetObject<ScmpL4Protocol>();
    }
    else
    {
        return nullptr;
    }
}

void
SCIONL3Protocol::SendWithHeader(Ptr<Packet> packet, SCIONHeader ipHeader, Ptr<SCIONRoute> route)
{
    NS_LOG_FUNCTION(this << packet << ipHeader << route);
    if (Node::ChecksumEnabled())
    {
        ipHeader.EnableChecksum();
    }
    SendRealOut(route, packet, ipHeader);
}

void
SCIONL3Protocol::CallTxTrace(const SCIONHeader& ipHeader,
                            Ptr<Packet> packet,
                            Ptr<SCION> ipv4,
                            uint32_t interface)
{
    if (!m_txTrace.IsEmpty())
    {
        Ptr<Packet> packetCopy = packet->Copy();
        packetCopy->AddHeader(ipHeader);
        m_txTrace(packetCopy, ipv4, interface);
    }
}

/**
 * i.e. called directly by SCIONRawSockets. In this case the route will contain the socket's boundNetDevice as outputDevice,
 *      as well as src and dst addresses (listen/bind address of socket and  either connect/peer address or argument to Socket::SendTo() )
 *     The gateway is set to 0.0.0.0 Any
 * 
 * if called by L4 protocols i.e. UDP the route will be either null or the delegated route argument, 
 * depending on which Send() overload has been called by UdpSocket.
    * \brief Send a packet via UDP (IPv4)
     * \param packet The packet to send
     * \param saddr The source Ipv4Address
     * \param daddr The destination Ipv4Address
     * \param sport The source port number
     * \param dport The destination port number
     *
    void Send(Ptr<Packet> packet,
              Ipv4Address saddr,
              Ipv4Address daddr,
              uint16_t sport,
              uint16_t dport);  // nullptr route
     OR: 
     * \brief Send a packet via UDP (IPv4)
     * \param packet The packet to send
     * \param saddr The source Ipv4Address
     * \param daddr The destination Ipv4Address
     * \param sport The source port number
     * \param dport The destination port number
     * \param route The route
     *
    void Send(Ptr<Packet> packet,
              Ipv4Address saddr,
              Ipv4Address daddr,
              uint16_t sport,
              uint16_t dport,
              Ptr<Ipv4Route> route);

 if the UDPSocket is bound to a listen address, then source address is known, and route can be omitted (be nullptr).
 Otherwise the UDPL4SocketImpl retrieves the routingProtocol from the Ipv4L3 protocol of the node
 and computes the route for the given destination address.
 It provides as input to RoutingProtocol::RouteOutput() the packet, scion header,
  and output netDevice (the device the socket is bound to[might be null if Socket::BindToNetDevice hasn't been called explicitly])
 */
void
SCIONL3Protocol::Send(Ptr<Packet> packet,
                     SCIONAddress source,
                     SCIONAddress destination,
                     uint8_t protocol,
                     Ptr<SCIONRoute> route)
{
    NS_LOG_FUNCTION(this << packet << source << destination << uint32_t(protocol) << route);

    // we need a copy of the packet with its tags in case we need to invoke recursion.
    Ptr<Packet> pktCopyWithTags = packet->Copy();

   
    uint8_t tos = 0;
    SocketIpTosTag ipTosTag;
    bool ipTosTagFound = packet->RemovePacketTag(ipTosTag);
    if (ipTosTagFound)
    {
        tos = ipTosTag.GetTos();
    }

    // can construct the header here
    SCIONHeader ipHeader =
        BuildHeader(source, destination, protocol, packet->GetSize(), ttl, tos);

    // Handle a few cases:
    // 1) packet is passed in with a route entry
    // 1a) packet is passed in with a route entry but route->GetGateway is not set (e.g., on-demand)
    // 1b) packet is passed in with a route entry and valid gateway

    // address 4) packet is passed without a route, packet is not broadcast (e.g., a raw socket
    // call, or ICMP)

    // 1) packet is passed in with route entry
    if (route)
    {
        // 1a) route->GetGateway is not set (e.g., on-demand)
        if (!route->GetGateway().IsInitialized())
        {
            // This could arise because the synchronous RouteOutput() call
            // returned to the transport protocol with a source address but
            // there was no next hop available yet (since a route may need
            // to be queried).
            NS_FATAL_ERROR("SCIONL3Protocol::Send case 1a: packet passed with a route but the "
                           "Gateway address is uninitialized. This case not yet implemented.");
        }

        // 1b) with a valid gateway
        NS_LOG_LOGIC("SCIONL3Protocol::Send case 1b:  passed in with route and valid gateway");
        int32_t interface = GetInterfaceForDevice(route->GetOutputDevice());
        m_sendOutgoingTrace(ipHeader, packet, interface);   
        SendRealOut(route, packet->Copy(), ipHeader);
        return;
    }

    // 4) packet is not broadcast, and route is NULL (e.g., a raw socket call)
    NS_LOG_LOGIC("SCIONL3Protocol::Send case 4:  not broadcast and passed in with no route "
                 << destination);
    Socket::SocketErrno errno_;
    Ptr<NetDevice> oif(nullptr); // unused for now
    Ptr<SCIONRoute> newRoute;
    if (m_routingProtocol)
    {
        newRoute = m_routingProtocol->RouteOutput(pktCopyWithTags, ipHeader, oif, errno_);
    }
    else
    {
        NS_LOG_ERROR("SCIONL3Protocol::Send: m_routingProtocol == 0");
    }
    if (newRoute)
    {     
        Send(pktCopyWithTags, source, destination, protocol, newRoute);
    }
    else
    {
        NS_LOG_WARN("No route to host.  Drop.");
        m_dropTrace(ipHeader, packet, DROP_NO_ROUTE, this, 0);
    }
}


SCIONHeader
SCIONL3Protocol::BuildHeader(SCIONAddress source,
                            SCIONAddress destination,
                            uint8_t protocol,
                            uint16_t payloadSize,                            
                            uint8_t tos
                            )
{
    NS_LOG_FUNCTION(this << source << destination << (uint16_t)protocol << payloadSize << (uint16_t)tos);
    SCIONHeader ipHeader;
    ipHeader.SetSource(source);
    ipHeader.SetDestination(destination);
    ipHeader.SetProtocol(protocol);
    ipHeader.SetPayloadSize(payloadSize);
    
    ipHeader.SetTos(tos);



    return ipHeader;
}

void
SCIONL3Protocol::SendRealOut(Ptr<Ipv4Route> route, Ptr<Packet> packet, const SCIONHeader& ipHeader)
{
    NS_LOG_FUNCTION(this << route << packet << &ipHeader);
    if (!route)
    {
        NS_LOG_WARN("No route to host.  Drop.");
        m_dropTrace(ipHeader, packet, DROP_NO_ROUTE, this, 0);
        return;
    }
    Ptr<NetDevice> outDev = route->GetOutputDevice();
    int32_t interface = GetInterfaceForDevice(outDev);
    NS_ASSERT(interface >= 0);
    Ptr<SCIONInterface> outInterface = GetInterface(interface);
    NS_LOG_LOGIC("Send via NetDevice ifIndex " << outDev->GetIfIndex() << " SCIONInterfaceIndex "
                                               << interface);

    SCIONAddress target;
    std::string targetLabel;
    if (route->GetGateway().IsAny())
    {
        target = ipHeader.GetDestination();
        targetLabel = "destination";
    }
    else
    {
        target = route->GetGateway();
        targetLabel = "gateway";
    }

    if (outInterface->IsUp())
    {
        NS_LOG_LOGIC("Send to " << targetLabel << " " << target);
        NS_ASSERT_MSG(packet->GetSize() + ipHeader.GetSerializedSize() <= outInterface->GetDevice()->GetMtu(),"MTU exceeded");
        
        
        CallTxTrace(ipHeader, packet, this, interface);
        outInterface->Send(packet, ipHeader, target);
        
    }
}

void
SCIONL3Protocol::LocalDeliver(Ptr<const Packet> packet, const SCIONHeader& hdr, uint32_t iif)
{
    NS_LOG_FUNCTION(this << packet << &hdr << iif);
    Ptr<Packet> p = packet->Copy(); // need to pass a non-const packet up
    SCIONHeader header = hdr;

    m_localDeliverTrace(ipHeader, p, iif);

    Ptr<SCIONL4Protocol> protocol = GetProtocol(header.GetProtocol(), iif);
    if (protocol)
    {
        // we need to make a copy in the unlikely event we hit the
        // RX_ENDPOINT_UNREACH codepath
        Ptr<Packet> copy = p->Copy();
        enum IpL4Protocol::RxStatus status = protocol->Receive(p, header, GetInterface(iif));
        switch (status)
        {
        case IpL4Protocol::RX_OK:
        // fall through
        case IpL4Protocol::RX_ENDPOINT_CLOSED:
        // fall through
        case IpL4Protocol::RX_CSUM_FAILED:
            break;
        /*
        case IpL4Protocol::RX_ENDPOINT_UNREACH:
            if (ipHeader.GetDestination().IsBroadcast() == true ||
                ipHeader.GetDestination().IsMulticast() == true)
            {
                break; // Do not reply to broadcast or multicast
            }
            // Another case to suppress SCMP is a subnet-directed broadcast
            bool subnetDirected = false;
            for (uint32_t i = 0; i < GetNAddresses(iif); i++)
            {
                SCIONInterfaceAddress addr = GetAddress(iif, i);
                if (addr.GetLocal().CombineMask(addr.GetMask()) ==
                        ipHeader.GetDestination().CombineMask(addr.GetMask()) &&
                    ipHeader.GetDestination().IsSubnetDirectedBroadcast(addr.GetMask()))
                {
                    subnetDirected = true;
                }
            }
            if (subnetDirected == false)
            {
                GetScmp()->SendDestUnreachPort(ipHeader, copy);
            }
        */
        }
    }
}

bool
SCIONL3Protocol::AddAddress(uint32_t i, SCIONInterfaceAddress address)
{
    NS_LOG_FUNCTION(this << i << address);
    Ptr<SCIONInterface> interface = GetInterface(i);
    bool retVal = interface->AddAddress(address);
    if (m_routingProtocol)
    {
        m_routingProtocol->NotifyAddAddress(i, address);
    }
    return retVal;
}

SCIONInterfaceAddress
SCIONL3Protocol::GetAddress(uint32_t interfaceIndex, uint32_t addressIndex) const
{
    NS_LOG_FUNCTION(this << interfaceIndex << addressIndex);
    Ptr<SCIONInterface> interface = GetInterface(interfaceIndex);
    return interface->GetAddress(addressIndex);
}

uint32_t
SCIONL3Protocol::GetNAddresses(uint32_t interface) const
{
    NS_LOG_FUNCTION(this << interface);
    Ptr<SCIONInterface> iface = GetInterface(interface);
    return iface->GetNAddresses();
}

bool
SCIONL3Protocol::RemoveAddress(uint32_t i, uint32_t addressIndex)
{
    NS_LOG_FUNCTION(this << i << addressIndex);
    Ptr<SCIONInterface> interface = GetInterface(i);
    SCIONInterfaceAddress address = interface->RemoveAddress(addressIndex);
    if (address != SCIONInterfaceAddress())
    {
        if (m_routingProtocol)
        {
            m_routingProtocol->NotifyRemoveAddress(i, address);
        }
        return true;
    }
    return false;
}

bool
SCIONL3Protocol::RemoveAddress(uint32_t i, SCIONAddress address)
{
    NS_LOG_FUNCTION(this << i << address);

    if (address == SCIONAddress::GetLoopback())
    {
        NS_LOG_WARN("Cannot remove loopback address.");
        return false;
    }
    Ptr<SCIONInterface> interface = GetInterface(i);
    SCIONInterfaceAddress ifAddr = interface->RemoveAddress(address);
    if (ifAddr != SCIONInterfaceAddress())
    {
        if (m_routingProtocol)
        {
            m_routingProtocol->NotifyRemoveAddress(i, ifAddr);
        }
        return true;
    }
    return false;
}



uint16_t
SCIONL3Protocol::GetMtu(uint32_t i) const
{
    NS_LOG_FUNCTION(this << i);
    Ptr<SCIONInterface> interface = GetInterface(i);
    return interface->GetDevice()->GetMtu();
}

bool
SCIONL3Protocol::IsUp(uint32_t i) const
{
    NS_LOG_FUNCTION(this << i);
    Ptr<SCIONInterface> interface = GetInterface(i);
    return interface->IsUp();
}

void
SCIONL3Protocol::SetUp(uint32_t i)
{
    NS_LOG_FUNCTION(this << i);
    Ptr<SCIONInterface> interface = GetInterface(i);

    // RFC 791, pg.25:
    //  Every internet module must be able to forward a datagram of 68
    //  octets without further fragmentation.  This is because an internet
    //  header may be up to 60 octets, and the minimum fragment is 8 octets.
    if (interface->GetDevice()->GetMtu() >= 68)
    {
        interface->SetUp();

        if (m_routingProtocol)
        {
            m_routingProtocol->NotifyInterfaceUp(i);
        }
    }
    else
    {
        NS_LOG_LOGIC(
            "Interface "
            << int(i)
            << " is set to be down for IPv4. Reason: not respecting minimum IPv4 MTU (68 octets)");
    }
}

void
SCIONL3Protocol::SetDown(uint32_t ifaceIndex)
{
    NS_LOG_FUNCTION(this << ifaceIndex);
    Ptr<SCIONInterface> interface = GetInterface(ifaceIndex);
    interface->SetDown();

    if (m_routingProtocol)
    {
        m_routingProtocol->NotifyInterfaceDown(ifaceIndex);
    }
}

bool
SCIONL3Protocol::IsForwarding(uint32_t i) const
{
    NS_LOG_FUNCTION(this << i);
    Ptr<SCIONInterface> interface = GetInterface(i);
    NS_LOG_LOGIC("Forwarding state: " << interface->IsForwarding());
    return interface->IsForwarding();
}

void
SCIONL3Protocol::SetForwarding(uint32_t i, bool val)
{
    NS_LOG_FUNCTION(this << i);
    Ptr<SCIONInterface> interface = GetInterface(i);
    interface->SetForwarding(val);
}

Ptr<NetDevice>
SCIONL3Protocol::GetNetDevice(uint32_t i)
{
    NS_LOG_FUNCTION(this << i);
    return GetInterface(i)->GetDevice();
}

void
SCIONL3Protocol::SetSCIONForward(bool forward)
{
    NS_LOG_FUNCTION(this << forward);
    m_scionForward = forward;
    for (SCIONInterfaceList::const_iterator i = m_interfaces.begin(); i != m_interfaces.end(); i++)
    {
        (*i)->SetForwarding(forward);
    }
}

bool
SCIONL3Protocol::GetSCIONForward() const
{
    NS_LOG_FUNCTION(this);
    return m_scionForward;
}

void
SCIONL3Protocol::SetWeakEsModel(bool model)
{
    NS_LOG_FUNCTION(this << model);
    m_weakEsModel = model;
}

bool
SCIONL3Protocol::GetWeakEsModel() const
{
    NS_LOG_FUNCTION(this);
    return m_weakEsModel;
}

void
SCIONL3Protocol::RouteInputError(Ptr<const Packet> p,
                                const SCIONHeader& ipHeader,
                                Socket::SocketErrno sockErrno)
{
    NS_LOG_FUNCTION(this << p << ipHeader << sockErrno);
    NS_LOG_LOGIC("Route input failure-- dropping packet to " << ipHeader << " with errno "
                                                             << sockErrno);
    m_dropTrace(ipHeader, p, DROP_ROUTE_ERROR, this, 0);

    // \todo Send an ICMP no route.
}


bool
SCIONL3Protocol::UpdateDuplicate(Ptr<const Packet> p, const SCIONHeader& header)
{
    NS_LOG_FUNCTION(this << p << header);

    // \todo RFC 6621 mandates SHA-1 hash.  For now ns3 hash should be fine.
    uint8_t proto = header.GetProtocol();
    SCIONAddress src = header.GetSource();
    SCIONAddress dst = header.GetDestination();
    uint64_t id = header.GetIdentification();

    // concat hash value onto id
    uint64_t hash = id << 32;
    if (header.GetFragmentOffset() || !header.IsLastFragment())
    {
        // use I-DPD (RFC 6621, Sec 6.2.1)
        hash |= header.GetFragmentOffset();
    }
    else
    {
        // use H-DPD (RFC 6621, Sec 6.2.2)

        // serialize packet
        Ptr<Packet> pkt = p->Copy();
        pkt->AddHeader(header);

        std::ostringstream oss(std::ios_base::binary);
        pkt->CopyData(&oss, pkt->GetSize());
        std::string bytes = oss.str();

        NS_ASSERT_MSG(bytes.size() >= 20, "Degenerate header serialization");

        // zero out mutable fields
        bytes[1] = 0;                        // DSCP / ECN
        bytes[6] = bytes[7] = 0;             // Flags / Fragment offset
        bytes[8] = 0;                        // TTL
        bytes[10] = bytes[11] = 0;           // Header checksum
        if (header.GetSerializedSize() > 20) // assume options should be 0'd
        {
            std::fill_n(bytes.begin() + 20, header.GetSerializedSize() - 20, 0);
        }

        // concat hash onto ID
        hash |= (uint64_t)Hash32(bytes);
    }

    // set cleanup job for new duplicate entries
    if (!m_cleanDpd.IsRunning() && m_purge.IsStrictlyPositive())
    {
        m_cleanDpd = Simulator::Schedule(m_expire, &SCIONL3Protocol::RemoveDuplicates, this);
    }

    // assume this is a new entry
    DupTuple_t key{hash, proto, src, dst};
    NS_LOG_DEBUG("Packet " << p->GetUid() << " key = (" << std::hex << std::get<0>(key) << ", "
                           << std::dec << +std::get<1>(key) << ", " << std::get<2>(key) << ", "
                           << std::get<3>(key) << ")");

    // place a new entry, on collision the existing entry iterator is returned
    DupMap_t::iterator iter;
    bool inserted;
    bool isDup;
    std::tie(iter, inserted) = m_dups.emplace(key, Seconds(0));
    isDup = !inserted && iter->second > Simulator::Now();

    // set the expiration event
    iter->second = Simulator::Now() + m_expire;
    return isDup;
}

void
SCIONL3Protocol::RemoveDuplicates()
{
    NS_LOG_FUNCTION(this);

    DupMap_t::size_type n = 0;
    Time expire = Simulator::Now();
    auto iter = m_dups.cbegin();
    while (iter != m_dups.cend())
    {
        if (iter->second < expire)
        {
            NS_LOG_LOGIC("Remove key = (" << std::hex << std::get<0>(iter->first) << ", "
                                          << std::dec << +std::get<1>(iter->first) << ", "
                                          << std::get<2>(iter->first) << ", "
                                          << std::get<3>(iter->first) << ")");
            iter = m_dups.erase(iter);
            ++n;
        }
        else
        {
            ++iter;
        }
    }

    NS_LOG_DEBUG("Purged " << n << " expired duplicate entries out of " << (n + m_dups.size()));

    // keep cleaning up if necessary
    if (!m_dups.empty() && m_purge.IsStrictlyPositive())
    {
        m_cleanDpd = Simulator::Schedule(m_purge, &SCIONL3Protocol::RemoveDuplicates, this);
    }
}

} // namespace ns3
