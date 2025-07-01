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

#ifndef SCMP_L4_PROTOCOL_H
#define SCMP_L4_PROTOCOL_H

//#include "scmp-header.h"
#include "scion-l4-protocol.h"

#include "ns3/scion-address.h"

namespace ns3
{

class Node;
class SCIONInterface;
class SCIONRoute;

/**
 * \ingroup scion
 * \defgroup scmp SCMP protocol and associated headers.
 */

/**
 * \ingroup icmp
 *
 * \brief This is the implementation of the SCMP protocol
 */
class ScmpL4Protocol : public SCIONL4Protocol
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    static const uint8_t PROT_NUMBER; //!< SCMP protocol number (0x1)

    ScmpL4Protocol();
    ~ScmpL4Protocol() override;

    /**
     * \brief Set the node the protocol is associated with.
     * \param node the node
     */
    void SetNode(Ptr<Node> node);

    /**
     * Get the protocol number
     * \returns the protocol number
     */
    static uint16_t GetStaticProtocolNumber();

    /**
     * Get the protocol number
     * \returns the protocol number
     */
    int GetProtocolNumber() const override;

    /**
     * \brief Receive method.
     * \param p the packet
     * \param header the IPv4 header
     * \param incomingInterface the interface from which the packet is coming
     * \returns the receive status
     */
    enum IpL4Protocol::RxStatus Receive(Ptr<Packet> p,
                                        const SCIONHeader& header,
                                        Ptr<SCIONInterface> incomingInterface) override;

    
    /**
     * \brief Send a Destination Unreachable - Fragmentation needed SCMP error
     * \param header the original IP header
     * \param orgData the original packet
     * \param nextHopMtu the next hop MTU
     */
    void SendDestUnreachFragNeeded(SCIONHeader header, Ptr<const Packet> orgData, uint16_t nextHopMtu);

    /**
     * \brief Send a Time Exceeded SCMP error
     * \param header the original IP header
     * \param orgData the original packet
     * \param isFragment true if the opcode must be FRAGMENT_REASSEMBLY
     */
    // void SendTimeExceededTtl(Ipv4Header header, Ptr<const Packet> orgData, bool isFragment);

    /**
     * \brief Send a Time Exceeded SCMP error
     * \param header the original SCION header
     * \param orgData the original packet
     */
    void SendDestUnreachPort(SCIONHeader header, Ptr<const Packet> orgData);

    
    void SetDownTargetSCION(SCIONL4Protocol::DownTargetCallback cb) override;
    SCIONL4Protocol::DownTargetCallback GetDownTargetSCION() const override;

  protected:
    /*
     * This function will notify other components connected to the node that a new stack member is
     * now connected This will be used to notify Layer 3 protocol of layer 4 protocol stack to
     * connect them together.
     */
    void NotifyNewAggregate() override;

  private:
    /**
     * \brief Handles an incoming SCMP Echo packet
     * \param p the packet
     * \param header the IP header
     * \param source the source address
     * \param destination the destination address
     */
    void HandleEcho(Ptr<Packet> p,
                    ScmpHeader header,
                    SCIONAddress source,
                    SCIONAddress destination);
    /**
     * \brief Handles an incoming SCMP Destination Unreachable packet
     * \param p the packet
     * \param header the IP header
     * \param source the source address
     * \param destination the destination address
     */
    void HandleDestUnreach(Ptr<Packet> p,
                           ScmpHeader header,
                           SCIONAddress source,
                           SCIONAddress destination);
    /**
     * \brief Handles an incoming SCMP Time Exceeded packet
     * \param p the packet
     * \param icmp the SCMP header
     * \param source the source address
     * \param destination the destination address
     *
    void HandleTimeExceeded(Ptr<Packet> p,
                            ScmpHeader icmp,
                            SCIONAddress source,
                            SCIONAddress destination);
     */
    /**
     * \brief Send an SCMP Destination Unreachable packet
     *
     * \param header the original IP header
     * \param orgData the original packet
     * \param code the SCMP code
     * \param nextHopMtu the next hop MTU
     */
    void SendDestUnreach(SCIONHeader header,
                         Ptr<const Packet> orgData,
                         uint8_t code,
                         uint16_t nextHopMtu);
    /**
     * \brief Send a generic SCMP packet
     *
     * \param packet the packet
     * \param dest the destination
     * \param type the SCMP type
     * \param code the SCMP code
     */
    void SendMessage(Ptr<Packet> packet, SCIONAddress dest, uint8_t type, uint8_t code);
    /**
     * \brief Send a generic SCMP packet
     *
     * \param packet the packet
     * \param source the source
     * \param dest the destination
     * \param type the SCMP type
     * \param code the SCMP code
     * \param route the route to be used
     */
    void SendMessage(Ptr<Packet> packet,
                     SCIONAddress source,
                     SCIONAddress dest,
                     uint8_t type,
                     uint8_t code,
                     Ptr<SCIONRoute> route);
    /**
     * \brief Forward the message to an L4 protocol
     *
     * \param source the source
     * \param icmp the SCMP header
     * \param info info data (e.g., the target MTU)
     * \param ipHeader the IP header carried by SCMP
     * \param payload payload chunk carried by SCMP
     */
    void Forward(SCIONAddress source,
                 ScmpHeader icmp,
                 uint32_t info,
                 SCIONHeader ipHeader,
                 const uint8_t payload[8]);

    void DoDispose() override;

    Ptr<Node> m_node;                              //!< the node this protocol is associated with
    SCIONL4Protocol::DownTargetCallback m_downTarget; //!< callback to SCION::Send
};

} // namespace ns3

#endif /* SCMPV4_L4_PROTOCOL_H */
