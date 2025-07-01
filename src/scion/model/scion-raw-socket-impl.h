#ifndef SCION_RAW_SOCKET_IMPL_H
#define SCION_RAW_SOCKET_IMPL_H

#include "ns3/scion-header.h"
#include "ns3/scion-interface.h"
//#include "ns3/scion-route.h"
#include "ns3/socket.h"

#include <list>

namespace ns3
{

class NetDevice;
class Node;

/**
 * \ingroup socket
 * \ingroup scion
 *
 * \brief SCION raw socket.
 *
 * A RAW Socket typically is used to access specific packet layers not usually
 * available through L4 sockets, e.g., SCMP. The implementer should take
 * particular care to define the SCIONRawSocketImpl Attributes, and in
 * particular the Protocol attribute.
 * 
 *  Raw sockets bypass the transport layer (TCP or UDP).  With IPv4, raw
   sockets are used to access ICMPv4, IGMPv4, and to read and write IPv4
   datagrams containing a protocol field that the kernel does not
   process.  An example of the latter is a routing daemon for OSPF,
   since it uses IPv4 protocol field 89.
    All data sent via raw sockets must be in network byte order and all
   data received via raw sockets will be in network byte order.

   When writing to a raw socket the kernel will automatically fragment
   the packet if its size exceeds the path MTU, inserting the required
   fragment headers.  On input the kernel reassembles received
   fragments, so the reader of a raw socket never sees any fragment
   headers.
 * 
 * IP_HDRINCL is a socket option that enables the inclusion of the IP header
 *  in packets sent through a raw socket. When this option is enabled,
 *  the packet must contain an IP header, and the kernel will not generate one automatically.
 * 
 * After enabling IP_HDRINCL, the application is responsible for constructing the entire IP header, 
 * including fields such as the source address, destination address, and checksum.
 * However, the kernel will fill in certain fields, such as the source address and packet ID, if they are left as zero.

It's important to note that enabling IP_HDRINCL is necessary when you want to have full control over the SCION header fields,
 such as Path or Extension Header.

When receiving packets, the IP header is always included regardless of whether IP_HDRINCL is enabled or not
 */
class SCIONRawSocketImpl : public Socket
{
  public:
    /**
     * \brief Get the type ID of this class.
     * \return type ID
     */
    static TypeId GetTypeId();

    SCIONRawSocketImpl();

    /**
     * \brief Set the node associated with this socket.
     * \param node node to set
     */
    void SetNode(Ptr<Node> node);

    enum Socket::SocketErrno GetErrno() const override;

    /**
     * \brief Get socket type (NS3_SOCK_RAW)
     * \return socket type
     */
    enum Socket::SocketType GetSocketType() const override;

    Ptr<Node> GetNode() const override;
    int Bind(const Address& address) override;
    int Bind() override;
    int Bind6() override;
    int BindSCION() override;
    int GetSockName(Address& address) const override;
    int GetPeerName(Address& address) const override;
    int Close() override;
    int ShutdownSend() override;
    int ShutdownRecv() override;
    int Connect(const Address& address) override;
    int Listen() override;
    uint32_t GetTxAvailable() const override;
    int Send(Ptr<Packet> p, uint32_t flags) override;
    int SendTo(Ptr<Packet> p, uint32_t flags, const Address& toAddress) override;
    uint32_t GetRxAvailable() const override;
    Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
    Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress) override;

    /**
     * \brief Set protocol field.
     * \param protocol protocol to set
     */
    void SetProtocol(uint16_t protocol);

    /**
     * \brief Forward up to receive method.
     * \param p packet
     * \param header SCION header
     * \param incomingInterface incoming interface
     * \return true if forwarded, false otherwise
     */
    bool ForwardUp(Ptr<const Packet> p, SCIONHeader ipHeader, Ptr<SCIONInterface> incomingInterface);
    bool SetAllowBroadcast(bool allowBroadcast) override;
    bool GetAllowBroadcast() const override;

  private:
    void DoDispose() override;

    /**
     * \struct Data
     * \brief IPv4 raw data and additional information.
     */
    struct Data
    {
        Ptr<Packet> packet;    /**< Packet data */
        SCIONAddress fromIp;    /**< Source address */
        uint16_t fromProtocol; /**< Protocol used */
    };

    mutable enum Socket::SocketErrno m_err; //!< Last error number.
    Ptr<Node> m_node;                       //!< Node
    SCIONAddress m_src;                      //!< Source address.
    SCIONAddress m_dst;                      //!< Destination address.
    uint16_t m_protocol;                    //!< Protocol.
    std::list<struct Data> m_recv;          //!< Packet waiting to be processed.
    bool m_shutdownSend;                    //!< Flag to shutdown send capability.
    bool m_shutdownRecv;                    //!< Flag to shutdown receive capability.
    uint32_t m_scmpFilter;                  //!< ICMPv4 filter specification
    bool m_iphdrincl; //!< Include IP Header information (a.k.a setsockopt (IP_HDRINCL))
};

} // namespace ns3

#endif /* SCION_RAW_SOCKET_IMPL_H */
