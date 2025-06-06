#include "scion-raw-socket-impl.h"

#include "scmp.h"
#include "scion-l3-protocol.h"

#include "ns3/boolean.h"
#include "ns3/scion-socket-address.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#ifdef __WIN32__
#include "ns3/win32-internet.h"
#else
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONRawSocketImpl");

NS_OBJECT_ENSURE_REGISTERED(SCIONRawSocketImpl);

TypeId
SCIONRawSocketImpl::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SCIONRawSocketImpl")
            .SetParent<Socket>()
            .SetGroupName("Internet")
            .AddAttribute("Protocol",
                          "Protocol number to match.",
                          UintegerValue(0),
                          MakeUintegerAccessor(&SCIONRawSocketImpl::m_protocol),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("ScmpFilter",
                          "Any icmp header whose type field matches a bit in this filter is "
                          "dropped. Type must be less than 32.",
                          UintegerValue(0),
                          MakeUintegerAccessor(&SCIONRawSocketImpl::m_icmpFilter),
                          MakeUintegerChecker<uint32_t>())
            //
            //  from raw (7), linux, returned length of Send/Recv should be
            //
            //            | IP_HDRINC on  |      off    |
            //  ----------+---------------+-------------+-
            //  Send(Ipv4)| hdr + payload | payload     |
            //  Recv(Ipv4)| hdr + payload | hdr+payload |
            //  ----------+---------------+-------------+-
            .AddAttribute("IpHeaderInclude",
                          "Include IP Header information (a.k.a setsockopt (IP_HDRINCL)).",
                          BooleanValue(false),
                          MakeBooleanAccessor(&SCIONRawSocketImpl::m_iphdrincl),
                          MakeBooleanChecker());
    return tid;
}

SCIONRawSocketImpl::SCIONRawSocketImpl()
{
    NS_LOG_FUNCTION(this);
    m_err = Socket::ERROR_NOTERROR;
    m_node = nullptr;
    m_src = Ipv4Address::GetAny();
    m_dst = Ipv4Address::GetAny();
    m_protocol = 0;
    m_shutdownSend = false;
    m_shutdownRecv = false;
}

void
SCIONRawSocketImpl::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

void
SCIONRawSocketImpl::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_node = nullptr;
    Socket::DoDispose();
}

enum Socket::SocketErrno
SCIONRawSocketImpl::GetErrno() const
{
    NS_LOG_FUNCTION(this);
    return m_err;
}

enum Socket::SocketType
SCIONRawSocketImpl::GetSocketType() const
{
    NS_LOG_FUNCTION(this);
    return NS3_SOCK_RAW;
}

Ptr<Node>
SCIONRawSocketImpl::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

int
SCIONRawSocketImpl::Bind6()
{
    NS_LOG_FUNCTION(this);
    return (-1);
}

int
SCIONRawSocketImpl::Bind(const Address& address)
{
    NS_LOG_FUNCTION(this << address);
    if (!SCIONSocketAddress::IsMatchingType(address))
    {
        m_err = Socket::ERROR_INVAL;
        return -1;
    }
    auto ad = SCIONSocketAddress::ConvertFrom(address);
    m_src = ad.GetSCION();
    return 0;
}

int
SCIONRawSocketImpl::Bind()
{   NS_LOG_FUNCTION(this);
    return -1;
}

int SCIONRawSocketImpl::BindSCION() {
    NS_LOG_FUNCTION(this);
    m_src = SCIONAddress::GetAny();
    return 0;
}


int
SCIONRawSocketImpl::GetSockName(Address& address) const
{
    NS_LOG_FUNCTION(this << address);
    address = SCIONSocketAddress(m_src, 0);
    return 0;
}

int
SCIONRawSocketImpl::GetPeerName(Address& address) const
{
    NS_LOG_FUNCTION(this << address);

    if (m_dst == SCIONAddress::GetAny())
    {
        m_err = ERROR_NOTCONN;
        return -1;
    }

    address = SCIONSocketAddress(m_dst, 0);

    return 0;
}

int
SCIONRawSocketImpl::Close()
{
    NS_LOG_FUNCTION(this);
    auto ipv4 = m_node->GetObject<SCION>();
    if (ipv4)
    {
        ipv4->DeleteRawSocket(this);
    }
    return 0;
}

int
SCIONRawSocketImpl::ShutdownSend()
{
    NS_LOG_FUNCTION(this);
    m_shutdownSend = true;
    return 0;
}

int
SCIONRawSocketImpl::ShutdownRecv()
{
    NS_LOG_FUNCTION(this);
    m_shutdownRecv = true;
    return 0;
}

int
SCIONRawSocketImpl::Connect(const Address& address)
{
    NS_LOG_FUNCTION(this << address);
    if (!SCIONSocketAddress::IsMatchingType(address))
    {
        m_err = Socket::ERROR_INVAL;
        NotifyConnectionFailed();
        return -1;
    }
    SCIONSocketAddress ad = SCIONSocketAddress::ConvertFrom(address);
    m_dst = ad.GetSCION();
    SetIpTos(ad.GetTos());
    NotifyConnectionSucceeded();

    return 0;
}

int
SCIONRawSocketImpl::Listen()
{
    NS_LOG_FUNCTION(this);
    m_err = Socket::ERROR_OPNOTSUPP;
    return -1;
}

uint32_t
SCIONRawSocketImpl::GetTxAvailable() const
{
    NS_LOG_FUNCTION(this);
    return 0xffffffff;
}

int
SCIONRawSocketImpl::Send(Ptr<Packet> p, uint32_t flags)
{
    NS_LOG_FUNCTION(this << p << flags);
    SCIONSocketAddress to = SCIONSocketAddress(m_dst, m_protocol);
    to.SetTos(GetIpTos());
    return SendTo(p, flags, to);
}

int
SCIONRawSocketImpl::SendTo(Ptr<Packet> p, uint32_t flags, const Address& toAddress)
{
    NS_LOG_FUNCTION(this << p << flags << toAddress);
    if (!SCIONSocketAddress::IsMatchingType(toAddress))
    {
        m_err = Socket::ERROR_INVAL;
        return -1;
    }
    if (m_shutdownSend)
    {
        return 0;
    }

    SCIONSocketAddress ad = SCIONSocketAddress::ConvertFrom(toAddress);
    Ptr<Ipv4> ipv4 = m_node->GetObject<SCION>();
    SCIONAddress dst = ad.GetSCION();
    SCIONAddress src = m_src;
    uint8_t tos = ad.GetTos();

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

    Ptr<NetDevice> boundNetDevice = m_boundnetdevice;

    if (!m_src.IsAny())
    {
        int32_t index = ipv4->GetInterfaceForAddress(m_src);
        NS_ASSERT(index >= 0);
        boundNetDevice = ipv4->GetNetDevice(index);
    }

    bool subnetDirectedBroadcast = false;
    if (boundNetDevice)
    {
        uint32_t iif = ipv4->GetInterfaceForDevice(boundNetDevice);
        for (uint32_t j = 0; j < ipv4->GetNAddresses(iif); j++)
        {
            Ipv4InterfaceAddress ifAddr = ipv4->GetAddress(iif, j);
            if (dst.IsSubnetDirectedBroadcast(ifAddr.GetMask()))
            {
                subnetDirectedBroadcast = true;
            }
        }
    }

    if (dst.IsBroadcast() || subnetDirectedBroadcast)
    {
        if (ipv4->GetNInterfaces() == 1)
        {
            boundNetDevice = ipv4->GetNetDevice(0);
        }
        if (!boundNetDevice)
        {
            NS_LOG_DEBUG("dropped because no outgoing route.");
            return -1;
        }

        Ipv4Header header;
        uint32_t pktSize = p->GetSize();
        if (!m_iphdrincl)
        {
            header.SetDestination(dst);
            header.SetProtocol(m_protocol);
            Ptr<Ipv4Route> route = Create<Ipv4Route>();
            route->SetSource(src);
            route->SetDestination(dst);
            route->SetOutputDevice(boundNetDevice);
            route->SetGateway("0.0.0.0");
            ipv4->Send(p, route->GetSource(), dst, m_protocol, route);
        }
        else
        {
            p->RemoveHeader(header);
            dst = header.GetDestination();
            src = header.GetSource();
            pktSize += header.GetSerializedSize();
            Ptr<Ipv4Route> route = Create<Ipv4Route>();
            route->SetSource(src);
            route->SetDestination(dst);
            route->SetOutputDevice(boundNetDevice);
            route->SetGateway("0.0.0.0");
            ipv4->SendWithHeader(p, header, route);
        }
        NotifyDataSent(pktSize);
        NotifySend(GetTxAvailable());
        return pktSize;
    }

    if (ipv4->GetRoutingProtocol())
    {
        Ipv4Header header;
        if (!m_iphdrincl)
        {
            header.SetDestination(dst);
            header.SetProtocol(m_protocol);
        }
        else
        {
            p->RemoveHeader(header);
            dst = header.GetDestination();
            src = header.GetSource();
        }
        SocketErrno errno_ =
            ERROR_NOTERROR; // do not use errno as it is the standard C last error number
        Ptr<Ipv4Route> route;
        Ptr<NetDevice> oif = m_boundnetdevice; // specify non-zero if bound to a source address
        if (!oif && src != Ipv4Address::GetAny())
        {
            int32_t index = ipv4->GetInterfaceForAddress(src);
            NS_ASSERT(index >= 0);
            oif = ipv4->GetNetDevice(index);
            NS_LOG_LOGIC("Set index " << oif << "from source " << src);
        }

        // TBD-- we could cache the route and just check its validity
        route = ipv4->GetRoutingProtocol()->RouteOutput(p, header, oif, errno_);
        if (route)
        {
            NS_LOG_LOGIC("Route exists");
            uint32_t pktSize = p->GetSize();
            if (!m_iphdrincl)
            {
                ipv4->Send(p, route->GetSource(), dst, m_protocol, route);
            }
            else
            {
                pktSize += header.GetSerializedSize();
                ipv4->SendWithHeader(p, header, route);
            }
            NotifyDataSent(pktSize);
            NotifySend(GetTxAvailable());
            return pktSize;
        }
        else
        {
            NS_LOG_DEBUG("dropped because no outgoing route.");
            return -1;
        }
    }
    return 0;
}

uint32_t
SCIONRawSocketImpl::GetRxAvailable() const
{
    NS_LOG_FUNCTION(this);
    uint32_t rx = 0;
    for (std::list<Data>::const_iterator i = m_recv.begin(); i != m_recv.end(); ++i)
    {
        rx += (i->packet)->GetSize();
    }
    return rx;
}

Ptr<Packet>
SCIONRawSocketImpl::Recv(uint32_t maxSize, uint32_t flags)
{
    NS_LOG_FUNCTION(this << maxSize << flags);
    Address tmp;
    return RecvFrom(maxSize, flags, tmp);
}

Ptr<Packet>
SCIONRawSocketImpl::RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress)
{
    NS_LOG_FUNCTION(this << maxSize << flags << fromAddress);
    if (m_recv.empty())
    {
        return nullptr;
    }
    Data data = m_recv.front();
    m_recv.pop_front();
    SCIONSocketAddress inet = SCIONSocketAddress(data.fromIp, data.fromProtocol);
    fromAddress = inet;
    if (data.packet->GetSize() > maxSize)
    {
        Ptr<Packet> first = data.packet->CreateFragment(0, maxSize);
        if (!(flags & MSG_PEEK))
        {
            data.packet->RemoveAtStart(maxSize);
        }
        m_recv.push_front(data);
        return first;
    }
    return data.packet;
}

void
SCIONRawSocketImpl::SetProtocol(uint16_t protocol)
{
    NS_LOG_FUNCTION(this << protocol);
    m_protocol = protocol;
}

bool
SCIONRawSocketImpl::ForwardUp(Ptr<const Packet> p,
                             SCIONHeader header,
                             Ptr<SCIONInterface> incomingInterface)
{
    NS_LOG_FUNCTION(this << *p << header << incomingInterface);
    if (m_shutdownRecv)
    {
        return false;
    }

    Ptr<NetDevice> boundNetDevice = Socket::GetBoundNetDevice();
    if (boundNetDevice)
    {
        if (boundNetDevice != incomingInterface->GetDevice())
        {
            return false;
        }
    }

    NS_LOG_LOGIC("src = " << m_src << " dst = " << m_dst);
    if ((m_src == Ipv4Address::GetAny() || ipHeader.GetDestination() == m_src) &&
        (m_dst == Ipv4Address::GetAny() || ipHeader.GetSource() == m_dst) &&
        ipHeader.GetProtocol() == m_protocol)
    {
        Ptr<Packet> copy = p->Copy();
        // Should check via getsockopt ()..
        if (IsRecvPktInfo())
        {
            Ipv4PacketInfoTag tag;
            copy->RemovePacketTag(tag);
            tag.SetAddress(ipHeader.GetDestination());
            tag.SetTtl(ipHeader.GetTtl());
            tag.SetRecvIf(incomingInterface->GetDevice()->GetIfIndex());
            copy->AddPacketTag(tag);
        }

        // Check only version 4 options
        if (IsIpRecvTos())
        {
            SocketIpTosTag ipTosTag;
            ipTosTag.SetTos(ipHeader.GetTos());
            copy->AddPacketTag(ipTosTag);
        }

        if (IsIpRecvTtl())
        {
            SocketIpTtlTag ipTtlTag;
            ipTtlTag.SetTtl(ipHeader.GetTtl());
            copy->AddPacketTag(ipTtlTag);
        }

        if (m_protocol == 1)
        {
            ScmpHeader scmpHeader;
            copy->PeekHeader(scmpHeader);
            uint8_t type = scmpHeader.GetType();
            if (type < 32 && ((uint32_t(1) << type) & m_scmpFilter))
            {
                // filter out icmp packet.
                return false;
            }
        }
        copy->AddHeader(header);
        Data data;
        data.packet = copy;
        data.fromIp = header.GetSource();
        data.fromProtocol = header.GetProtocol();
        m_recv.push_back(data);
        NotifyDataRecv();
        return true;
    }
    return false;
}

} // namespace ns3
