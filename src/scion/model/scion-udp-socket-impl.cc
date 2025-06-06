/*
 * Copyright (c) 2007 INRIA
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

#include "scion-udp-socket-impl.h"

#include "scion-end-point.h"
#include "udp-l4-protocol.h"

#include "ns3/scion-socket-address.h"

#include "ns3/scion-header.h"

#include "ns3/ipv4.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"

#include <limits>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScionUdpSocketImpl");

NS_OBJECT_ENSURE_REGISTERED(ScionUdpSocketImpl);


// Add attributes generic to all UdpSockets to base class UdpSocket
TypeId
ScionUdpSocketImpl::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ScionUdpSocketImpl")
            .SetParent<UdpSocketImpl>()
            .SetGroupName("Internet")
            .AddConstructor<ScionUdpSocketImpl>()
            .AddAttribute("ScmpCallback",
                          "Callback invoked whenever an SCMP error is received on this socket.",
                          CallbackValue(),
                          MakeCallbackAccessor(&ScionUdpSocketImpl::m_scmpCallback),
                          MakeCallbackChecker());
    return tid;
}

ScionUdpSocketImpl::ScionUdpSocketImpl()
    : UdpSocketImpl()
    
{
    NS_LOG_FUNCTION(this);
    m_endPoint = nullptr;
}

ScionUdpSocketImpl::~ScionUdpSocketImpl()
{
    NS_LOG_FUNCTION(this);

    
    /**
     * Note: actually this function is called AFTER
     * ScionUdpSocketImpl::Destroy or ScionUdpSocketImpl::Destroy6
     * so the code below is unnecessary in normal operations
     */
    if (m_endPoint != nullptr)
    {
        NS_ASSERT(m_udp);
        /**
         * Note that this piece of code is a bit tricky:
         * when DeAllocate is called, it will call into
         * Ipv4EndPointDemux::Deallocate which triggers
         * a delete of the associated endPoint which triggers
         * in turn a call to the method ScionUdpSocketImpl::Destroy below
         * will will zero the m_endPoint field.
         */
        NS_ASSERT(m_endPoint != nullptr);
        m_udp->DeAllocate(m_endPoint);
        NS_ASSERT(m_endPoint == nullptr);
    }
   
    m_udp = nullptr;
    this->UdpSocketImpl::~UdpSocketImpl();
}


void
ScionUdpSocketImpl::DestroySCION()
{
    NS_LOG_FUNCTION(this);
    m_endPoint = nullptr;
}

/* Deallocate the end point and cancel all the timers */
void
ScionUdpSocketImpl::DeallocateEndPoint()
{

    // deallocate IPv4/v6 EndPoints
    this->UdpSocketImpl::DeallocateEndPoint();

    if (m_endPoint != nullptr)
    {
        m_endPoint->SetDestroyCallback(MakeNullCallback<void>());
        m_udp->DeAllocate(m_endPoint);
        m_endPoint = nullptr;
    }

}

int
ScionUdpSocketImpl::FinishBind()
{
    NS_LOG_FUNCTION(this);
    bool done = false;
    if (m_endPoint != nullptr)
    {
        m_endPoint->SetRxCallback(
            MakeCallback(&ScionUdpSocketImpl::ForwardUp, Ptr<ScionUdpSocketImpl>(this)));
        m_endPoint->SetScmpCallback(
            MakeCallback(&ScionUdpSocketImpl::ForwardScmp, Ptr<ScionUdpSocketImpl>(this)));
        m_endPoint->SetDestroyCallback(
            MakeCallback(&ScionUdpSocketImpl::DestroySCION, Ptr<ScionUdpSocketImpl>(this)));
        done = true;
    }

    bool ip_done = this->UdpSocketImpl::FinishBind();

    if (done || ip_done)
    {
        m_shutdownRecv = false;
        m_shutdownSend = false;
        return 0;
    }
    return -1;
}

int
ScionUdpSocketImpl::BindSCION()
{
    NS_LOG_FUNCTION(this);
    m_endPoint = m_udp->Allocate();
    if (m_boundnetdevice)
    {
        m_endPoint->BindToNetDevice(m_boundnetdevice);
    }
    return FinishBind();
}


int
ScionUdpSocketImpl::Bind(const Address& address)
{
    NS_LOG_FUNCTION(this << address);

    if (SCIONSocketAddress::IsMatchingType(address))
    {
        NS_ASSERT_MSG(m_endPoint == nullptr, "Endpoint already allocated.");

        SCIONSocketAddress transport = SCIONSocketAddress::ConvertFrom(address);
        SCIONAddress ipv4 = transport.GetIpv4();
        uint16_t port = transport.GetPort();
        SetIpTos(transport.GetTos());
        if (ipv4 == Ipv4Address::GetAny() && port == 0)
        {
            m_endPoint = m_udp->Allocate();
        }
        else if (ipv4 == Ipv4Address::GetAny() && port != 0)
        {
            m_endPoint = m_udp->Allocate(GetBoundNetDevice(), port);
        }
        else if (ipv4 != Ipv4Address::GetAny() && port == 0)
        {
            m_endPoint = m_udp->Allocate(ipv4);
        }
        else if (ipv4 != Ipv4Address::GetAny() && port != 0)
        {
            m_endPoint = m_udp->Allocate(GetBoundNetDevice(), ipv4, port);
        }
        if (nullptr == m_endPoint)
        {
            m_errno = port ? ERROR_ADDRINUSE : ERROR_ADDRNOTAVAIL;
            return -1;
        }
        if (m_boundnetdevice)
        {
            m_endPoint->BindToNetDevice(m_boundnetdevice);
        }
    }
    else 
    {
        return this->UdpSocketImpl::Bind(address);
    }
}


int
ScionUdpSocketImpl::Close()
{
    NS_LOG_FUNCTION(this);
    if (m_shutdownRecv == true && m_shutdownSend == true)
    {
        m_errno = Socket::ERROR_BADF;
        return -1;
    }
    Ipv6LeaveGroup();
    m_shutdownRecv = true;
    m_shutdownSend = true;
    DeallocateEndPoint();
    return 0;
}

int
ScionUdpSocketImpl::Connect(const Address& address)
{
    NS_LOG_FUNCTION(this << address);
    if (SCIONSocketAddress::IsMatchingType(address) == true)
    {
        InetSocketAddress transport = InetSocketAddress::ConvertFrom(address);
        m_defaultAddress = Address(transport.GetIpv4());
        m_defaultPort = transport.GetPort();
        SetIpTos(transport.GetTos());
        m_connected = true;
        NotifyConnectionSucceeded();
    }
    else 
    {
        return this->UdpSocketImpl::Connect(address);
    }

}


int
ScionUdpSocketImpl::Send(Ptr<Packet> p, uint32_t flags)
{
    NS_LOG_FUNCTION(this << p << flags);

    if (!m_connected)
    {
        m_errno = ERROR_NOTCONN;
        return -1;
    }

    return DoSend(p);
}

int
ScionUdpSocketImpl::DoSend(Ptr<Packet> p)
{
    NS_LOG_FUNCTION(this << p);
    // what does this mean ?! we were Connect()ed and thus have a defaultAddress,
    // but then in between Close()'d again ?!
    if ((m_endPoint == nullptr) && (SCIONAddress::IsMatchingType(GetDefaultAddress()) == true))
    {
        if (BindSCION() == -1)
        {
            NS_ASSERT(m_endPoint == nullptr);
            return -1;
        }
        NS_ASSERT(m_endPoint != nullptr);
    }

    if (GetShutdownSend())
    {
        SetErrno( Socket::ERROR_SHUTDOWN);
        return -1;
    }

    if (auto default_addr{GetDefaultAddress()}; SCIONAddress::IsMatchingType(default_addr))
    {
        return DoSendTo(p, SCIONAddress::ConvertFrom(default_addr), m_defaultPort, GetIpTos());
    }
    else 
    {
        return this->UdpSocketImpl::DoSend(p);
    }
}

/**
 * \param port destination port
 */
int
ScionUdpSocketImpl::DoSendToSCION(Ptr<Packet> p, SCIONAddress dest, uint16_t port, uint8_t tos)
{
    NS_LOG_FUNCTION(this << p << dest << port << (uint16_t)tos);
    if (m_boundnetdevice)
    {
        NS_LOG_LOGIC("Bound interface number " << m_boundnetdevice->GetIfIndex());
    }
    if (m_endPoint == nullptr)
    {
        if (Bind() == -1)
        {
            NS_ASSERT(m_endPoint == nullptr);
            return -1;
        }
        NS_ASSERT(m_endPoint != nullptr);
    }
    if (m_shutdownSend)
    {
        m_errno = ERROR_SHUTDOWN;
        return -1;
    }

    if (p->GetSize() > GetTxAvailable())
    {
        m_errno = ERROR_MSGSIZE;
        return -1;
    }

    uint8_t priority = GetPriority();
    if (tos)
    {
        SocketIpTosTag ipTosTag;
        ipTosTag.SetTos(tos);
        // This packet may already have a SocketIpTosTag (see BUG 2440)
        p->ReplacePacketTag(ipTosTag);
        priority = IpTos2Priority(tos);
    }

    if (priority)
    {
        SocketPriorityTag priorityTag;
        priorityTag.SetPriority(priority);
        p->ReplacePacketTag(priorityTag);
    }

    /*
    {
        SocketSetDontFragmentTag tag;
        bool found = p->RemovePacketTag(tag);
        if (!found)
        {
            if (m_mtuDiscover)
            {
                tag.Enable();
            }
            else
            {
                tag.Disable();
            }
            p->AddPacketTag(tag);
        }
    }

    // Note that some systems will only send limited broadcast packets
    // out of the "default" interface; here we send it out all interfaces
    if (dest.IsBroadcast())
    {
        if (!m_allowBroadcast)
        {
            m_errno = ERROR_OPNOTSUPP;
            return -1;
        }
        NS_LOG_LOGIC("Limited broadcast start.");
        for (uint32_t i = 0; i < ipv4->GetNInterfaces(); i++)
        {
            // Get the primary address
            Ipv4InterfaceAddress iaddr = ipv4->GetAddress(i, 0);
            Ipv4Address addri = iaddr.GetLocal();
            if (addri == Ipv4Address("127.0.0.1"))
            {
                continue;
            }
            // Check if interface-bound socket
            if (m_boundnetdevice)
            {
                if (ipv4->GetNetDevice(i) != m_boundnetdevice)
                {
                    continue;
                }
            }
            NS_LOG_LOGIC("Sending one copy from " << addri << " to " << dest);
            m_udp->Send(p->Copy(), addri, dest, m_endPoint->GetLocalPort(), port);
            NotifyDataSent(p->GetSize());
            NotifySend(GetTxAvailable());
        }
        NS_LOG_LOGIC("Limited broadcast end.");
        return p->GetSize();
    }
    else if (m_endPoint->GetLocalAddress() != Ipv4Address::GetAny())
    */
  // TODO choose an adequate source host address if none is specified (any-source address)
    {
        m_udp->Send(p->Copy(),
                    m_endPoint->GetLocalAddress(),
                    dest,
                    m_endPoint->GetLocalPort(),
                    port,
                    nullptr);
        NotifyDataSent(p->GetSize());
        NotifySend(GetTxAvailable());
        return p->GetSize();
    }
    /*
    else if (ipv4->GetRoutingProtocol())
    {
        Ipv4Header header;
        header.SetDestination(dest);
        header.SetProtocol(UdpL4Protocol::PROT_NUMBER);
        Socket::SocketErrno errno_;
        Ptr<Ipv4Route> route;
        Ptr<NetDevice> oif = m_boundnetdevice; // specify non-zero if bound to a specific device
        // TBD-- we could cache the route and just check its validity
        route = ipv4->GetRoutingProtocol()->RouteOutput(p, header, oif, errno_);
        if (route)
        {
            NS_LOG_LOGIC("Route exists");
            if (!m_allowBroadcast)
            {
                // Here we try to route subnet-directed broadcasts
                uint32_t outputIfIndex = ipv4->GetInterfaceForDevice(route->GetOutputDevice());
                uint32_t ifNAddr = ipv4->GetNAddresses(outputIfIndex);
                for (uint32_t addrI = 0; addrI < ifNAddr; ++addrI)
                {
                    Ipv4InterfaceAddress ifAddr = ipv4->GetAddress(outputIfIndex, addrI);
                    if (dest == ifAddr.GetBroadcast())
                    {
                        m_errno = ERROR_OPNOTSUPP;
                        return -1;
                    }
                }
            }

            header.SetSource(route->GetSource());
            m_udp->Send(p->Copy(),
                        header.GetSource(),
                        header.GetDestination(),
                        m_endPoint->GetLocalPort(),
                        port,
                        route);
            NotifyDataSent(p->GetSize());
            return p->GetSize();
        }
        else
        {
            NS_LOG_LOGIC("No route to destination");
            NS_LOG_ERROR(errno_);
            m_errno = errno_;
            return -1;
        }
    }
    else
    {
        NS_LOG_ERROR("ERROR_NOROUTETOHOST");
        m_errno = ERROR_NOROUTETOHOST;
        return -1;
    }
    */

    
}


int
ScionUdpSocketImpl::SendTo(Ptr<Packet> p, uint32_t flags, const Address& address)
{
    NS_LOG_FUNCTION(this << p << flags << address);
    if (SCIONSocketAddress::IsMatchingType(address))
    {
        auto transport = SCIONSocketAddress::ConvertFrom(address);
        SCIONAddress addr = transport.GetIpv4();
        uint16_t port = transport.GetPort();
        uint8_t tos = transport.GetTos();
        return DoSendToSCION(p, addr, port, tos);
    }
    else 
    {
        this->UdpSocketImpl::SendTo(p,flags,address);
    }
}


Ptr<Packet>
ScionUdpSocketImpl::Recv(uint32_t maxSize, uint32_t flags)
{
    NS_LOG_FUNCTION(this << maxSize << flags);

    Address fromAddress;
    Ptr<Packet> packet = RecvFrom(maxSize, flags, fromAddress);
    return packet;
}

/**
 * dequeue (packet|address) pair from the internal m_deliveryQueue and return to the Application.
 * It was previously enqueued in ForwardUp()
 *
Ptr<Packet>
ScionUdpSocketImpl::RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress)
{
    NS_LOG_FUNCTION(this << maxSize << flags);

    if (m_deliveryQueue.empty())
    {
        m_errno = ERROR_AGAIN;
        return nullptr;
    }
    Ptr<Packet> p = m_deliveryQueue.front().first;
    fromAddress = m_deliveryQueue.front().second;

    if (p->GetSize() <= maxSize)
    {
        m_deliveryQueue.pop();
        m_rxAvailable -= p->GetSize();
    }
    else
    {
        p = nullptr;
    }
    return p;
}*/

// return this socket's local listen address
int
ScionUdpSocketImpl::GetSockName(Address& address) const
{
    NS_LOG_FUNCTION(this << address);
    if (m_endPoint != nullptr)
    {
        address = SCIONSocketAddress(m_endPoint->GetLocalAddress(), m_endPoint->GetLocalPort());
        return 0;
    }
    else 
    {
        return this->UdpSocketImpl::GetSockName(address);
    }
}

int
ScionUdpSocketImpl::GetPeerName(Address& address) const
{
    NS_LOG_FUNCTION(this << address);

    if (!m_connected)
    {
       SetErrno(Socket::ERROR_NOTCONN);
        return -1;
    }

    if (auto default_addr = GetDefaultAddress(); SCIONAddress::IsMatchingType(default_addr))
    {
        auto addr = SCIONAddress::ConvertFrom(default_address);
        SCIONSocketAddress inet(addr, m_defaultPort);
        inet.SetTos(GetIpTos());
        address = inet;
        return 0;
        
    }
    else 
    {
        return this->UdpSocketImpl::GetPeerName(address);
    }
}


void
ScionUdpSocketImpl::BindToNetDevice(Ptr<NetDevice> netdevice)
{
    NS_LOG_FUNCTION(netdevice);

    if (m_endPoint != nullptr)
    {
        m_endPoint->BindToNetDevice(netdevice);
    }
    this->UdpSocketImpl::BindToNetDevice(netdevice);
}

void
ScionUdpSocketImpl::ForwardUp(Ptr<Packet> packet,
                         SCIONHeader header,
                         uint16_t port,
                         Ptr<SCIONInterface> incomingInterface)
{
    NS_LOG_FUNCTION(this << packet << header << port);

    if (GetShutdownRecv())
    {
        return;
    }
    //TODO: handle packet tags here.. .
    /*
    the SCIONHeader is dropped here, but meta information such as
    i.e. over which Path the packet came, could be tagged onto the Packet,
    which is eventually yielded to the Application if it calls RecvFrom().
     */

    /*  
    // Should check via getsockopt ()..
    if (IsRecvPktInfo())
    {
        Ipv4PacketInfoTag tag;
        packet->RemovePacketTag(tag);
        tag.SetAddress(header.GetDestination());
        tag.SetTtl(header.GetTtl());
        tag.SetRecvIf(incomingInterface->GetDevice()->GetIfIndex());
        packet->AddPacketTag(tag);
    }

    // Check only version 4 options
    if (IsIpRecvTos())
    {
        SocketIpTosTag ipTosTag;
        ipTosTag.SetTos(header.GetTos());
        packet->AddPacketTag(ipTosTag);
    }
    
    */

    // in case the packet still has a priority tag attached, remove it
    SocketPriorityTag priorityTag;
    packet->RemovePacketTag(priorityTag);

    if ((GetRxAvailable() + packet->GetSize()) <= GetRcvBufSize())
    {
        Address address = SCIONSocketAddress(header.GetSource(), port);
        EnqueueForDeliver(packet, address);
        AddRxAvailable(packet->GetSize());
        NotifyDataRecv();
    }
    else
    {
        // In general, this case should not occur unless the
        // receiving application reads data from this socket slowly
        // in comparison to the arrival rate
        //
        // drop and trace packet
        NS_LOG_WARN("No receive buffer space available.  Drop.");
        m_dropTrace(packet);
    }
}


void
ScionUdpSocketImpl::ForwardScmp(SCIONAddress scmpSource,                           
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

} // namespace ns3
