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
#ifndef SCION_UDP_SOCKET_IMPL_H
#define SCION_UDP_SOCKET_IMPL_H

#include "ns3/scion-scmp.h"

#include "ns3/callback.h"
#include "ns3/scion-address.h"
#include "ns3/scion-interface.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-socket.h"

#include <queue>
#include <stdint.h>

namespace ns3
{

class SCIONEndPoint;

class Node;
class Packet;
class UdpL4Protocol;
class SCIONHeader;
class SCIONInterface;

/**
 * \ingroup socket
 * \ingroup udp
 *
 * \brief the SCION capable UDP socket implementation,
 *  created by ScionUdpL4Protocol.
 *  The ScionStackHelper aggregates ScionUdpL4Protocol onto Nodes,
 *   and thus allows Applications on this Node to listen on and dial SCION  Addresses,
 *   which is impossible with UdpL4Protocol installed by the InternetStackHelper.
 */

class ScionScionUdpSocketImpl : public ScionUdpSocketImpl
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    /**
     * Create an unbound udp socket.
     */
    ScionUdpSocketImpl();
    ~ScionUdpSocketImpl() override;


    int BindSCION();
    int Bind(const Address& address) override;
    int Close() override; // DeallocateEndpoint()
    int Connect(const Address& address) override;
    
    // int Send(Ptr<Packet> p, uint32_t flags) override; just calls DoSend()
    int SendTo(Ptr<Packet> p, uint32_t flags, const Address& address) override; // calls DoSendTo()
    // getter for m_rxAvailable which is increased in ForwardUp()[enqueue] and decreased in RecvFrom() [dequeue]
    //uint32_t GetRxAvailable() const override; 
     // just calls RecvFrom() and discards the address
    // Ptr<Packet> Recv(uint32_t maxSize, uint32_t flags) override;
    // simply dequeues (address|packet) pair from deliveryQueue
    Ptr<Packet> RecvFrom(uint32_t maxSize, uint32_t flags, Address& fromAddress) override;

    int GetSockName(Address& address) const override;
    // return the remote address, this socket's endpoint is connected to
    int GetPeerName(Address& address) const override;
    void BindToNetDevice(Ptr<NetDevice> netdevice) override;

  protected:

    /**
     * \brief UdpSocketFactory friend class.
     * \relates UdpSocketFactory
     */
    friend class UdpSocketFactory;
    // invoked by Udp class

    /**
     * Finish the binding process
     * \returns 0 on success, -1 on failure
     */
    virtual int FinishBind()override;

    /**
     * \brief Called by the L3 protocol when it received a packet to pass on to TCP.
     *
     * \param packet the incoming packet
     * \param header the packet's SCION. It is stripped from the packet.
     * \param port the remote port
     * \param incomingInterface the incoming interface
     */
    void ForwardUp(Ptr<Packet> packet,
                   SCIONHeader header,
                   uint16_t port,
                   Ptr<SCIONInterface> incomingInterface);


    /**
     * \brief Kill this socket by zeroing its attributes 
     *
     * This is a callback function configured to m_endpoint in
     * SetupCallback(), invoked when the endpoint is destroyed.
     * 
     * installed by FinishBind()
     */
    void DestroySCION();



    /**
     * \brief Deallocate EndPoints
     */
    virtual void DeallocateEndPoint() override;

    /**
     * \brief Send a packet
     * called from int Send(Ptr<Packet> p, uint32_t flags) 
     * \param p packet
     * \returns 0 on success, -1 on failure
     */
    virtual int DoSend(Ptr<Packet> p) override;
    /**
     * \brief Send a packet to a specific destination and port (IPv4)
     * \param p packet
     * \param daddr destination address
     * \param dport destination port
     * \param tos ToS
     * \returns 0 on success, -1 on failure
     */
    int DoSendToSCION(Ptr<Packet> p, SCIONAddress daddr, uint16_t dport, uint8_t tos);
    

    /**
     * \brief Called by the L3 protocol when it received an SCMP packet to pass on to L4.
     *
     * \param scmpSource the SCMP source address     
     * \param scmpType the SCMP Type
     * \param scmpCode the SCMP Code
     * \param scmpInfo the SCMP Info
     */
    void ForwardScmp(SCIONAddress scmpSource,                     
                     uint8_t scmpType,
                     uint8_t scmpCode,
                     uint32_t scmpInfo);


    // Connections to other layers of TCP/IP
    SCIONEndPoint* m_endPoint;  //!< the SCION endpoint

    Callback<void, SCIONAddress,, uint8_t, uint8_t, uint32_t>
        m_scmpCallback; //!< SCMP callback


    //Address m_defaultAddress;                      //!< Default L3 address (Ipv4Address|Ipv6Address) set to remote address on Connect()
    // uint16_t m_defaultPort;                        //!< Default port set to remote port on Connect()
    
    
};

} // namespace ns3

#endif /* SCION_UDP_SOCKET_IMPL_H */
