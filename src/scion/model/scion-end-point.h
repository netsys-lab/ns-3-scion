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

#ifndef SCION_END_POINT_H
#define SCION_END_POINT_H

#include "ns3/callback.h"
#include "ns3/scion-address.h"
#include "ns3/scion-header.h"

#include "ns3/net-device.h"

#include <stdint.h>

namespace ns3
{

class Header;
class SCIONInterface;
class Packet;

/**
 * \ingroup scion
 *
 * \brief A representation of an internet endpoint/connection
 *
 * This class provides an internet four-tuple (source and destination ports
 * and addresses).  These are used in the ns3::SCIONEndPointDemux as targets
 * of lookups.  The class also has a callback for notification to higher
 * layers that a packet from a lower layer was received.  In the ns3
 * internet-stack, these notifications are automatically registered to be
 * received by the corresponding socket.
 */

class SCIONEndPoint
{
  public:
    /**
     * \brief Constructor.
     * \param address
     * \param port the port
     */
    SCIONEndPoint(SCIONAddress address, uint16_t port);
    ~SCIONEndPoint();

    /**
     * \brief Get the local address.
     * \return the local address
     */
    SCIONAddress GetLocalAddress();

    /**
     * \brief Set the local address.
     * \param address the address to set
     */
    void SetLocalAddress(SCIONAddress address);

    /**
     * \brief Get the local port.
     * \return the local port
     */
    uint16_t GetLocalPort();

    /**
     * \brief Get the peer address.
     * \return the peer address
     */
    SCIONAddress GetPeerAddress();

    /**
     * \brief Get the peer port.
     * \return the peer port
     */
    uint16_t GetPeerPort();

    /**
     * \brief Set the peer information (address and port).
     * \param address peer address
     * \param port peer port
     */
    void SetPeer(SCIONAddress address, uint16_t port);

    /**
     * \brief Bind a socket to specific device.
     *
     * This method corresponds to using setsockopt() SO_BINDTODEVICE
     * of real network or BSD sockets.   If set on a socket, this option will
     * force packets to leave the bound device regardless of the device that
     * IP routing would naturally choose.  In the receive direction, only
     * packets received from the bound interface will be delivered.
     *
     * This option has no particular relationship to binding sockets to
     * an address via Socket::Bind ().  It is possible to bind sockets to a
     * specific IP address on the bound interface by calling both
     * Socket::Bind (address) and Socket::BindToNetDevice (device), but it
     * is also possible to bind to mismatching device and address, even if
     * the socket can not receive any packets as a result.
     *
     * \param netdevice Pointer to Netdevice of desired interface
     */
    void BindToNetDevice(Ptr<NetDevice> netdevice);

    /**
     * \brief Returns socket's bound netdevice, if any.
     *
     * This method corresponds to using getsockopt() SO_BINDTODEVICE
     * of real network or BSD sockets.
     *
     *
     * \returns Pointer to interface.
     */
    Ptr<NetDevice> GetBoundNetDevice();

    /* Called from socket implementations to get notified about important events.
       in UdpSocketImpl::FinishBind():
        m_endPoint->SetRxCallback( MakeCallback(&UdpSocketImpl::ForwardUp, Ptr<UdpSocketImpl>(this)));
    */
    /**
     * \brief Set the reception callback.
     * \param callback callback function
     */
    void SetRxCallback(
        Callback<void, Ptr<Packet>, SCIONHeader, uint16_t, Ptr<SCIONInterface>> callback);
    /**
     * \brief Set the SCMP callback.
     * \param callback callback function
     *  set in UdpSocketImpl::FinishBind() on the Socket's EndPoint
     *    m_endPoint->SetScmpCallback(MakeCallback(&UdpSocketImpl::ForwardScmp, Ptr<UdpSocketImpl>(this)));
     * 
     * which then in turn calls the socket's Scmp callback, which is an attribute of the UdpSocketImpl's TypeId and Null by default
     * unless explicitly set by the user
     */
    void SetScmpCallback(Callback<void, SCIONAddress, uint8_t, uint8_t, uint8_t, uint32_t> callback);
    /**
     * \brief Set the default destroy callback.
     * \param callback callback function
     */
    void SetDestroyCallback(Callback<void> callback);

    /**
     * \brief Forward the packet to the upper level.
     *
     * Called from an L4Protocol implementation to notify an endpoint of a
     * packet reception.
     * \param p the packet
     * \param header the packet header
     * \param sport source port
     * \param incomingInterface incoming interface
     * 
     * calls the RxCallback in turn
     */
    void ForwardUp(Ptr<Packet> p,
                   const SCIONHeader& header,
                   uint16_t sport,
                   Ptr<SCIONInterface> incomingInterface);

    /**
     * \brief Forward the ICMP packet to the upper level.
     *
     * Called from an L4Protocol implementation to notify an endpoint of
     * an SCMP message reception.
     * Eventually calls the SCMP callback
     *
     * \param scmpSource source IP address
     * \param scmpType SCMP type
     * \param scmpCode SCMP code
     * \param scmpInfo SCMP info
     *  
     */
    void ForwardScmp(SCIONAddress icmpSource,              
                     uint8_t scmpType,
                     uint8_t scmpCode,
                     uint32_t scmpInfo);

    /**
     * \brief Enable or Disable the endpoint Rx capability.
     * \param enabled true if Rx is enabled
     */
    void SetRxEnabled(bool enabled);

    /**
     * \brief Checks if the endpoint can receive packets.
     * \returns true if the endpoint can receive packets.
     */
    bool IsRxEnabled() const;

  private:
    /**
     * \brief The local address.
     */
    SCIONAddress m_localAddr;

    /**
     * \brief The local port.
     */
    uint16_t m_localPort;

    /**
     * \brief The peer address.
     */
    SCIONAddress m_peerAddr;

    /**
     * \brief The peer port.
     */
    uint16_t m_peerPort;

    /**
     * \brief The NetDevice the EndPoint is bound to (if any).
     */
    Ptr<NetDevice> m_boundnetdevice;

    /**
     * \brief The RX callback.
     */
    Callback<void, Ptr<Packet>, SCIONHeader, uint16_t, Ptr<SCIONInterface>> m_rxCallback;

    /**
     * \brief invoked by ForwardScmp
     * source address, scmp type,scmp code, info
     */
    Callback<void, SCIONAddress, uint8_t, uint8_t, uint32_t> m_scmpCallback;

    /**
     * \brief The destroy callback.
     */
    Callback<void> m_destroyCallback;

    /**
     * \brief true if the endpoint can receive packets.
     */
    bool m_rxEnabled;
};

} // namespace ns3

#endif /* SCION_END_POINT_H */
