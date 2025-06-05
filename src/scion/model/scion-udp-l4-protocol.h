/*
 * Copyright (c) 2005,2006,2007 INRIA
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

#ifndef SCION_UDP_L4_PROTOCOL_H
#define SCION_UDP_L4_PROTOCOL_H

#include "scion-l4-protocol.h"
#include "ns3/udp-l4-protocol.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"

#include <stdint.h>

namespace ns3
{

class Node;
class Socket;
class SCIONEndPointDemux;
class SCIONEndPoint;
class UdpSocketImpl;
class NetDevice;

/**
 * \ingroup internet
 * \defgroup udp UDP

 */

/**
 * \ingroup udp
 * \brief Implementation of the UDP protocol
 */
class ScionUdpL4Protocol : public SCIONL4Protocol, public UdpL4Protocol
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ScionUdpL4Protocol();
    ~ScionUdpL4Protocol() override;

    // Delete copy constructor and assignment operator to avoid misuse
    ScionUdpL4Protocol(const ScionUdpL4Protocol&) = delete;
    ScionUdpL4Protocol& operator=(const ScionUdpL4Protocol&) = delete;


    int GetProtocolNumber() const override;

    /**
     * \return A smart Socket pointer to a UdpSocket, allocated by this instance
     * of the UDP protocol
     */
    Ptr<Socket> CreateSocket();

    /**
     * \brief Allocate an IPv4 Endpoint
     * \return the Endpoint
     */
    SCIONEndPoint* Allocate();
    /**
     * \brief Allocate an IPv4 Endpoint
     * \param address address to use
     * \return the Endpoint
     */
    SCIONEndPoint* Allocate(SCIONAddress address);
    /**
     * \brief Allocate an IPv4 Endpoint
     * \param boundNetDevice Bound NetDevice (if any)
     * \param port port to use
     * \return the Endpoint
     */
    SCIONEndPoint* Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port);
    /**
     * \brief Allocate an IPv4 Endpoint
     * \param boundNetDevice Bound NetDevice (if any)
     * \param address address to use
     * \param port port to use
     * \return the Endpoint
     */
    SCIONEndPoint* Allocate(Ptr<NetDevice> boundNetDevice, SCIONAddress address, uint16_t port);
    /**
     * \brief Allocate an IPv4 Endpoint
     * \param boundNetDevice Bound NetDevice (if any)
     * \param localAddress local address to use
     * \param localPort local port to use
     * \param peerAddress remote address to use
     * \param peerPort remote port to use
     * \return the Endpoint
     */
    SCIONEndPoint* Allocate(Ptr<NetDevice> boundNetDevice,
                           SCIONAddress localAddress,
                           uint16_t localPort,
                           SCIONAddress peerAddress,
                           uint16_t peerPort);

   
    /**
     * \brief Remove an Endpoint.
     * \param endPoint the end point to remove
     */
    void DeAllocate(SCIONEndPoint* endPoint);

    // called by UdpSocket.
    /**
     * \brief Send a packet via UDP (IPv4)
     * \param packet The packet to send
     * \param saddr The source SCIONAddress
     * \param daddr The destination SCIONAddress
     * \param sport The source port number
     * \param dport The destination port number
     */
    void Send(Ptr<Packet> packet,
              SCIONAddress saddr,
              SCIONAddress daddr,
              uint16_t sport,
              uint16_t dport);
    /**
     * \brief Send a packet via UDP (IPv4)
     * \param packet The packet to send
     * \param saddr The source SCIONAddress
     * \param daddr The destination SCIONAddress
     * \param sport The source port number
     * \param dport The destination port number
     * \param route The route
     */
    void Send(Ptr<Packet> packet,
              SCIONAddress saddr,
              SCIONAddress daddr,
              uint16_t sport,
              uint16_t dport,
              Ptr<SCIONRoute> route);

    enum IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                        const SCIONHeader& header,
                                        Ptr<SCIONInterface> interface) override;


    void ReceiveScmp(SCIONAddress icmpSource,
                     uint8_t icmpTtl,
                     uint8_t icmpType,
                     uint8_t icmpCode,
                     uint32_t icmpInfo,
                     SCIONAddress payloadSource,
                     SCIONAddress payloadDestination,
                     const uint8_t payload[8]) override;

    void SetDownTargetSCION(SCIONL4Protocol::DownTargetCallbackSCION cb) override;
    
    SCIONL4Protocol::DownTargetCallbackSCION GetDownTargetSCION() const override;
    

  protected:
    void DoDispose() override;
    /*
     * This function will notify other components connected to the node that a new stack member is
     * now connected This will be used to notify Layer 3 protocol of layer 4 protocol stack to
     * connect them together.
     */
    void NotifyNewAggregate() override;

  private:
    //Ptr<Node> m_node;                //!< the node this stack is associated with
    SCIONEndPointDemux* m_endPoints;  //!< A list of SCION end points.

    // std::vector<Ptr<UdpSocketImpl>> m_sockets;       //!< list of sockets
    SCIONL4Protocol::DownTargetCallbackSCION m_downTarget;   //!< Callback to send packets over SCION
    
};

} // namespace ns3

#endif /* SCION_UDP_L4_PROTOCOL_H */
