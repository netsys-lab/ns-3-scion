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

#ifndef SCION_L3_PROTOCOL_H
#define SCION_L3_PROTOCOL_H

#include "ns3/scion-address.h"
#include "ns3/scion-header.h"

#include "ns3/scion.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simulator.h"
#include "ns3/traced-callback.h"

#include <list>
#include <map>
#include <stdint.h>
#include <vector>

class SCIONL3ProtocolTestCase;

namespace ns3
{

class Packet;
class NetDevice;
class SCIONInterface;
class SCIONAddress;
class SCIONHeader;

class SCIONRoute;
class Node;
class Socket;
class SCIONRawSocketImpl;
class SCIONL4Protocol;
class ScmpL4Protocol;

/**
 * \ingroup scion
 *
 * \brief Implement the SCION layer.
 *
 * This is the actual implementation of SCION.  It contains APIs to send and
 * receive packets at the SCION layer but NO routing (as with IP)
 *
 * This class contains two distinct groups of trace sources.  The
 * trace sources 'Rx' and 'Tx' are called, respectively, immediately
 * after receiving from the NetDevice and immediately before sending
 * to a NetDevice for transmitting a packet.  These are low level
 * trace sources that include the SCIONHeader already serialized into
 * the packet.  In contrast, the Drop, SendOutgoing, UnicastForward,
 * and LocalDeliver trace sources are slightly higher-level and pass
 * around the SCIONHeader as an explicit parameter and not as part of
 * the packet.
 *
 * SCION does not fragment packets
 */
class SCIONL3Protocol : public SCION
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    static const uint16_t PROT_NUMBER; //!< Protocol number 

    SCIONL3Protocol();
    ~SCIONL3Protocol() override;

    // Delete copy constructor and assignment operator to avoid misuse
    SCIONL3Protocol(const SCIONL3Protocol&) = delete;
    SCIONL3Protocol& operator=(const SCIONL3Protocol&) = delete;

    /**
     * \enum DropReason
     * \brief Reason why a packet has been dropped.
     */
    enum DropReason
    {

        DROP_NO_ROUTE,         /**< No route to host */     
        DROP_INTERFACE_DOWN,   /**< Interface is down so can not send packet */
        DROP_ROUTE_ERROR      /**< Route error */             
    };

    /**
     * \brief Set node associated with this stack.
     * \param node node to set
     */
    void SetNode(Ptr<Node> node);

    // functions defined in base class SCION

    Ptr<Socket> CreateRawSocket() override;
    void DeleteRawSocket(Ptr<Socket> socket) override;

    void Insert(Ptr<SCIONL4Protocol> protocol) override;
    void Insert(Ptr<SCIONL4Protocol> protocol, uint32_t interfaceIndex) override;

    void Remove(Ptr<SCIONL4Protocol> protocol) override;
    void Remove(Ptr<SCIONL4Protocol> protocol, uint32_t interfaceIndex) override;

    Ptr<SCIONL4Protocol> GetProtocol(int protocolNumber) const override;
    Ptr<SCIONL4Protocol> GetProtocol(int protocolNumber, int32_t interfaceIndex) const override;

    /**
     * Lower layer calls this method after calling L3Demux::Lookup
     * The ARP subclass needs to know from which NetDevice this
     * packet is coming to:
     *    - implement a per-NetDevice ARP cache
     *    - send back arp replies on the right device
     * \param device network device
     * \param p the packet
     * \param protocol protocol value
     * \param from address of the correspondent
     * \param to address of the destination
     * \param packetType type of the packet
     */
    void Receive(Ptr<NetDevice> device,
                 Ptr<const Packet> p,
                 uint16_t protocol,
                 const Address& from,
                 const Address& to,
                 NetDevice::PacketType packetType);

    /**
     * \param packet packet to send
     * \param source source address of packet
     * \param destination address of packet
     * \param protocol number of packet
     * \param route route entry
     *
     * Higher-level layers call this method to send a packet
     * down the stack to the MAC and PHY layers.
     */
    void Send(Ptr<Packet> packet,
              SCIONAddress source,
              SCIONAddress destination,
              uint8_t protocol,
              Ptr<SCIONRoute> route) override;
    /**
     * \param packet packet to send
     * \param ipHeader SCION Header
     * \param route route entry
     *
     * Higher-level layers call this method to send a packet with SCION Header
     * (Intend to be used with IpHeaderInclude attribute.)
     */
    void SendWithHeader(Ptr<Packet> packet, SCIONHeader ipHeader, Ptr<SCIONRoute> route) override;

    uint32_t AddInterface(Ptr<NetDevice> device) override;

    uint32_t AddInterface(Ptr<Ipv4Interface> ) override;
    uint32_t AddInterface(Ptr<Ipv6Interface> ) override;
    /**
     * \brief Get an interface.
     * \param i interface index
     * \return SCION interface pointer
     */
    Ptr<SCIONInterface> GetInterface(uint32_t i) const;
    uint32_t GetNInterfaces() const override;

    int32_t GetInterfaceForAddress(SCIONAddress addr) const override;
    int32_t GetInterfaceForDevice(Ptr<const NetDevice> device) const override;
    bool IsDestinationAddress(SCIONAddress address, uint32_t iif) const override;

    bool AddAddress(uint32_t i, SCIONInterfaceAddress address) override;
    SCIONInterfaceAddress GetAddress(uint32_t interfaceIndex, uint32_t addressIndex) const override;
    uint32_t GetNAddresses(uint32_t interface) const override;
    bool RemoveAddress(uint32_t interfaceIndex, uint32_t addressIndex) override;
    bool RemoveAddress(uint32_t interface, SCIONAddress address) override;

    uint16_t GetMtu(uint32_t i) const override;
    bool IsUp(uint32_t i) const override;
    void SetUp(uint32_t i) override;
    void SetDown(uint32_t i) override;
    bool IsForwarding(uint32_t i) const override;
    void SetForwarding(uint32_t i, bool val) override;

    Ptr<NetDevice> GetNetDevice(uint32_t i) override;

    /**
     * TracedCallback signature for packet send, forward, or local deliver events.
     *
     * \param [in] header the SCIONHeader
     * \param [in] packet the packet
     * \param [in] interface L3-level interface number
     */
    typedef void (*SentTracedCallback)(const SCIONHeader& header,
                                       Ptr<const Packet> packet,
                                       uint32_t interface);

    /**
     * TracedCallback signature for packet transmission or reception events.
     *
     * \param [in] packet the packet.
     * \param [in] SCION the SCION protocol
     * \param [in] interface L3-level interface number
     * \deprecated The non-const \c Ptr<SCION> argument is deprecated
     * and will be changed to \c Ptr<const SCION> in a future release.
     */
    typedef void (*TxRxTracedCallback)(Ptr<const Packet> packet,
                                       Ptr<SCION> SCION,
                                       uint32_t interface);

    /**
     * TracedCallback signature for packet drop events.
     *
     * \param [in] header the SCIONHeader.
     * \param [in] packet the packet.
     * \param [in] reason the reason the packet was dropped.
     * \param [in] SCION the SCION protocol
     * \param [in] interface IP-level interface number
     * \deprecated The non-const \c Ptr<SCION> argument is deprecated
     * and will be changed to \c Ptr<const SCION> in a future release.
     */
    typedef void (*DropTracedCallback)(const SCIONHeader& header,
                                       Ptr<const Packet> packet,
                                       DropReason reason,
                                       Ptr<SCION> SCION,
                                       uint32_t interface);

  protected:
    void DoDispose() override;
    /**
     * This function will notify other components connected to the node that a new stack member is
     * now connected This will be used to notify Layer 3 protocol of layer 4 protocol stack to
     * connect them together.
     */
    void NotifyNewAggregate() override;

  private:
    /**
     * \brief SCIONL3ProtocolTestCase test case.
     * \relates SCIONL3ProtocolTestCase
     */
    friend class ::SCIONL3ProtocolTestCase;

    // class SCION attributes
    void SetSCIONForward(bool forward);
    bool GetSCIONForward() const ;

    void SetWeakEsModel(bool model) override;
    bool GetWeakEsModel() const override;

    /**
     * \brief Construct an SCION header.
     * \param source source SCION address
     * \param destination destination SCION address
     * \param protocol L4 protocol
     * \param payloadSize payload size     
     * \param tos Type of Service
     * \param mayFragment true if the packet can be fragmented
     * \return newly created SCION header
     */
    SCIONHeader BuildHeader(SCIONAddress source,
                           SCIONAddress destination,
                           uint8_t l4protocol,
                           uint16_t payloadSize,                           
                           uint8_t tos);

    /**
     * \brief Send packet with route.
     * \param route route
     * \param packet packet to send
     * \param header SCION header to add to the packet
     * 
     * Other than IPv4 SCION doesn't fragment here.
     * Just determine the right OutInterface from the given route,
     * and call Send() on it, passing the packet and header
     */
    void SendRealOut(Ptr<SCIONRoute> route, Ptr<Packet> packet, const SCIONHeader& header);

    
    /**
     * \brief Deliver a packet.
     * \param p packet delivered
     * \param ip SCION header
     * \param iif input interface packet was received
     */
    void LocalDeliver(Ptr<const Packet> p, const SCIONHeader& ip, uint32_t iif);
    

    /**
     * \brief Add an SCION interface to the stack.
     * \param interface interface to add
     * \return index of newly added interface
     */
    uint32_t AddSCIONInterface(Ptr<SCIONInterface> interface);

    /**
     * \brief Setup loopback interface.
     */
    void SetupLoopback();

    /**
     * \brief Get ICMPv4 protocol.
     * \return Icmpv4L4Protocol pointer
     */
    Ptr<ScmpL4Protocol> GetScmp() const;

    /**
     * \brief Make a copy of the packet, add the header and invoke the TX trace callback
     * \param header the IP header that will be added to the packet
     * \param packet the packet
     * \param SCION the SCION protocol
     * \param interface the L3-level interface index
     *
     * Note: If the TracedCallback API ever is extended, we could consider
     * to check for connected functions before adding the header
     */
    void CallTxTrace(const SCIONHeader& header,
                     Ptr<Packet> packet,
                     Ptr<SCION> SCION,
                     uint32_t interface);

    /**
     * \brief Container of the SCION Interfaces.
     */
    typedef std::vector<Ptr<SCIONInterface>> SCIONInterfaceList;
    /**
     * \brief Container of NetDevices registered to SCION and their interface indexes.
     */
    typedef std::map<Ptr<const NetDevice>, uint32_t> SCIONInterfaceReverseContainer;
    /**
     * \brief Container of the SCION Raw Sockets.
     */
    typedef std::list<Ptr<SCIONRawSocketImpl>> SocketList;

    /**
     * \brief Container of the SCION L4 keys: protocol number, interface index
     */
    typedef std::pair<int, int32_t> L4ListKey_t;

    /**
     * \brief Container of the SCION L4 instances.
     */
    typedef std::map<L4ListKey_t, Ptr<SCIONL4Protocol>> L4List_t;

    bool m_scionForward;               //!< Forwarding packets (i.e. router mode) state.
                                      // Note: no forwarding is done by the L3 layer. This only sets the attributes of all interfaces
    bool m_weakEsModel;             //!< Weak ES model state
    L4List_t m_protocols;           //!< List of transport protocol.
    SCIONInterfaceList m_interfaces; //!< List of SCION interfaces.
    SCIONInterfaceReverseContainer
        m_reverseInterfacesContainer; //!< Container of NetDevice / Interface index associations.

    std::map<std::pair<uint64_t, uint8_t>, uint16_t>
        m_identification; //!< Identification (for each {src, dst, proto} tuple)
    Ptr<Node> m_node;     //!< Node attached to stack.

    /// Trace of sent packets
    TracedCallback<const SCIONHeader&, Ptr<const Packet>, uint32_t> m_sendOutgoingTrace;

    /// Trace of locally delivered packets
    TracedCallback<const SCIONHeader&, Ptr<const Packet>, uint32_t> m_localDeliverTrace;

    // The following two traces pass a packet with an IP header
    /// Trace of transmitted packets
    /// \deprecated The non-const \c Ptr<SCION> argument is deprecated
    /// and will be changed to \c Ptr<const SCION> in a future release.
    TracedCallback<Ptr<const Packet>, Ptr<SCION>, uint32_t> m_txTrace;
    /// Trace of received packets
    /// \deprecated The non-const \c Ptr<SCION> argument is deprecated
    /// and will be changed to \c Ptr<const SCION> in a future release.
    TracedCallback<Ptr<const Packet>, Ptr<SCION>, uint32_t> m_rxTrace;
    // <ip-header, payload, reason, ifindex> (ifindex not valid if reason is DROP_NO_ROUTE)
    /// Trace of dropped packets
    /// \deprecated The non-const \c Ptr<SCION> argument is deprecated
    /// and will be changed to \c Ptr<const SCION> in a future release.
    TracedCallback<const SCIONHeader&, Ptr<const Packet>, DropReason, Ptr<SCION>, uint32_t>
        m_dropTrace;

    SocketList m_sockets; //!< List of SCION raw sockets.

};

} // Namespace ns3

#endif /* SCION_L3_PROTOCOL_H */
