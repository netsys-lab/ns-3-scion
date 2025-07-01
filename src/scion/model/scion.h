/*
 * Copyright (c) 
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
#ifndef SCION_H
#define SCION_H

#include "scion-interface-address.h"

#include "ns3/callback.h"
#include "ns3/scion-address.h"
#include "ns3/object.h"
#include "ns3/socket.h"

#include <stdint.h>

namespace ns3
{
class Ipv4Interface;
class Ipv6Interface;
class Node;
class NetDevice;
class Packet;
class SCIONRoute;
class SCIONRoutingProtocol;
class SCIONL4Protocol;
class SCIONHeader;

/**
 * \ingroup internet
 * \defgroup SCION SCION classes and sub-modules
 */
/**
 * \ingroup SCION
 * \brief Access to the SCION interfaces, and configuration
 *
 * This class defines the API to manipulate the following aspects of
 * the SCION implementation: *
 * -# register a NetDevice for use by the SCION layer 
 * -# manipulate the status of the NetDevice from the SCION perspective,
 * such as marking it as Up or Down,
 * -# adding, deleting, and getting addresses associated to the SCION
 * interfaces.
 * -# exporting SCION configuration attributes
 *
 * Each NetDevice has conceptually a single SCION interface associated
 * Each interface may have one or more SCION
 * addresses associated with it.
 *
 * 
 * \see SCIONInterfaceAddress
 */
class SCION : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    SCION();
    ~SCION() override;


    /**
     * \param device device to add to the list of SCION interfaces
     *        which can be used as output interfaces during packet forwarding.
     * \returns the index of the SCION interface added.
     *
     * Once a device has been added, it can never be removed: if you want
     * to disable it, you can invoke SCION::SetDown which will
     * make sure that it is never used during packet forwarding.
     */
    virtual uint32_t AddInterface(Ptr<NetDevice> device) = 0; // scion over L2 (XC links between BRs)

    virtual uint32_t AddInterface(Ptr<Ipv4Interface> ) = 0; // scion over IP
    virtual uint32_t AddInterface(Ptr<Ipv6Interface> ) = 0;

    /**
     * \returns the number of interfaces added by the user.
     */
    virtual uint32_t GetNInterfaces() const = 0;

    /**
     * \brief Return the interface number of the interface that has been
     *        assigned the specified IP address.
     *
     * \param address The IP address being searched for
     * \returns The interface number of the SCION interface with the given
     *          address or -1 if not found.
     *
     * Each IP interface has one or more IP addresses associated with it.
     * This method searches the list of interfaces for one that holds a
     * particular address.  This call takes an IP address as a parameter and
     * returns the interface number of the first interface that has been assigned
     * that address, or -1 if not found.  There must be an exact match; this
     * method will not match broadcast or multicast addresses.
     */
    virtual int32_t GetInterfaceForAddress(SCIONAddress address) const = 0;

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
    virtual void Send(Ptr<Packet> packet,
                      SCIONAddress source,
                      SCIONAddress destination,
                      uint8_t protocol,
                      Ptr<SCIONRoute> route) = 0;

    /**
     * \param packet packet to send
     * \param header SCION Header
     * \param route route entry
     *
     * Higher-level layers call this method to send a packet with SCION Header
     * (Intend to be used with SCIONHeaderInclude attribute.)
     */
    virtual void SendWithHeader(Ptr<Packet> packet, SCIONHeader header, Ptr<SCIONRoute> route) = 0;

    /**
     * \param protocol a template for the protocol to add to this L4 Demux.
     *
     * Invoke Copy on the input template to get a copy of the input
     * protocol which can be used on the Node on which this L4 Demux
     * is running. The new L4Protocol is registered internally as
     * a working L4 Protocol and returned from this method.
     * The caller does not get ownership of the returned pointer.
     */
    virtual void Insert(Ptr<SCIONL4Protocol> protocol) = 0;

    /**
     * \brief Add a L4 protocol to a specific interface.
     *
     * This may be called multiple times for multiple interfaces for the same
     * protocol.  To insert for all interfaces, use the separate
     * Insert (Ptr<SCIONL4Protocol> protocol) method.
     *
     * Setting a protocol on a specific interface will overwrite the
     * previously bound protocol.
     *
     * \param protocol L4 protocol.
     * \param interfaceIndex interface index.
     */
    virtual void Insert(Ptr<SCIONL4Protocol> protocol, uint32_t interfaceIndex) = 0;

    /**
     * \param protocol protocol to remove from this demux.
     *
     * The input value to this method should be the value
     * returned from the SCIONL4Protocol::Insert method.
     */
    virtual void Remove(Ptr<SCIONL4Protocol> protocol) = 0;

    /**
     * \brief Remove a L4 protocol from a specific interface.
     * \param protocol L4 protocol to remove.
     * \param interfaceIndex interface index.
     */
    virtual void Remove(Ptr<SCIONL4Protocol> protocol, uint32_t interfaceIndex) = 0;

    /**
     * \brief Determine whether address and interface corresponding to
     *        received packet can be accepted for local delivery
     *
     * \param address The SCION address being considered
     * \param iif The incoming SCION interface index
     * \returns true if the address is associated with the interface index
     *
     * This method can be used to determine whether a received packet has
     * an acceptable address for local delivery on the host.  The address
     * may be a unicast, multicast, or broadcast address.  This method will
     * return true if address is an exact match of a unicast address on
     * one of the host's interfaces (see below), if address corresponds to
     * a multicast group that the host has joined (and the incoming device
     * is acceptable), or if address corresponds to a broadcast address.
     *
     * If the SCION attribute WeakEsModel is true, the unicast address may
     * match any of the SCION addresses on any interface.  If the attribute is
     * false, the address must match one assigned to the incoming device.
     */
    virtual bool IsDestinationAddress(SCIONAddress address, uint32_t iif) const = 0;


    /**
     * \param interface The interface number of an SCION interface.
     * \returns The NetDevice associated with the SCION interface number.
     */
    virtual Ptr<NetDevice> GetNetDevice(uint32_t interface) = 0;

    /**
     * \param device The NetDevice for an SCIONInterface
     * \returns The interface number of an SCION interface or -1 if not found.
     */
    virtual int32_t GetInterfaceForDevice(Ptr<const NetDevice> device) const = 0;

    /**
     * \param interface Interface number of an SCION interface
     * \param address SCIONInterfaceAddress address to associate with the underlying SCION interface
     * \returns true if the operation succeeded
     */
    virtual bool AddAddress(uint32_t interface, SCIONInterfaceAddress address) = 0;

    /**
     * \param interface Interface number of an SCION interface
     * \returns the number of SCIONInterfaceAddress entries for the interface.
     */
    virtual uint32_t GetNAddresses(uint32_t interface) const = 0;

    /**
     * Because addresses can be removed, the addressIndex is not guaranteed
     * to be static across calls to this method.
     *
     * \param interface Interface number of an SCION interface
     * \param addressIndex index of SCIONInterfaceAddress
     * \returns the SCIONInterfaceAddress associated to the interface and addressIndex
     */
    virtual SCIONInterfaceAddress GetAddress(uint32_t interface, uint32_t addressIndex) const = 0;

    /**
     * Remove the address at addressIndex on named interface.  The addressIndex
     * for all higher indices will decrement by one after this method is called;
     * so, for example, to remove 5 addresses from an interface i, one could
     * call RemoveAddress (i, 0); 5 times.
     *
     * \param interface Interface number of an SCION interface
     * \param addressIndex index of SCIONInterfaceAddress to remove
     * \returns true if the operation succeeded
     */
    virtual bool RemoveAddress(uint32_t interface, uint32_t addressIndex) = 0;

    /**
     * \brief Remove the given address on named SCION interface
     *
     * \param interface Interface number of an SCION interface
     * \param address The address to remove
     * \returns true if the operation succeeded
     */
    virtual bool RemoveAddress(uint32_t interface, SCIONAddress address) = 0;

  
    /**
     * \param interface Interface number of SCION interface
     * \returns the Maximum Transmission Unit (in bytes) associated
     *          to the underlying SCION interface
     */
    virtual uint16_t GetMtu(uint32_t interface) const = 0;

    /**
     * \param interface Interface number of SCION interface
     * \returns true if the underlying interface is in the "up" state,
     *          false otherwise.
     */
    virtual bool IsUp(uint32_t interface) const = 0;

    /**
     * \param interface Interface number of SCION interface
     *
     * Set the interface into the "up" state. In this state, it is
     * considered valid during SCION forwarding.
     */
    virtual void SetUp(uint32_t interface) = 0;

    /**
     * \param interface Interface number of SCION interface
     *
     * Set the interface into the "down" state. In this state, it is
     * ignored during SCION forwarding.
     */
    virtual void SetDown(uint32_t interface) = 0;

    /**
     * \param interface Interface number of SCION interface
     * \returns true if IP forwarding enabled for input datagrams on this device
     */
    virtual bool IsForwarding(uint32_t interface) const = 0;

    /**
     * \param interface Interface number of SCION interface
     * \param val Value to set the forwarding flag
     *
     * If set to true, forwarding is enabled for input datagrams on this device
     */
    virtual void SetForwarding(uint32_t interface, bool val) = 0;

    /**
     * \brief Choose the source address to use with destination address.
     * \param interface interface index
     * \param dest SCION destination address
     * \return SCION source address to use
     *
    virtual SCIONAddress SourceAddressSelection(uint32_t interface, SCIONAddress dest) = 0;

    this was only required by IP RoutingProtocols i.e. Ptr<Ipv4Route> Ipv4StaticRouting::Lookup(Ipv4Address dest, Ptr<NetDevice> oif)
    */

    /**
     * \param protocolNumber number of protocol to lookup
     *        in this L4 Demux
     * \returns a matching L4 Protocol
     *
     * This method is typically called by lower layers
     * to forward packets up the stack to the right protocol.
     */
    virtual Ptr<SCIONL4Protocol> GetProtocol(int protocolNumber) const = 0;

    /**
     * \brief Get L4 protocol by protocol number for the specified interface.
     * \param protocolNumber protocol number
     * \param interfaceIndex interface index, -1 means "any" interface.
     * \return corresponding SCIONL4Protocol or 0 if not found
     */
    virtual Ptr<SCIONL4Protocol> GetProtocol(int protocolNumber, int32_t interfaceIndex) const = 0;

    /**
     * \brief Creates a raw socket
     *
     * \returns a smart pointer to the instantiated raw socket
     */
    virtual Ptr<Socket> CreateRawSocket() = 0;

    /**
     * \brief Deletes a particular raw socket
     *
     * \param socket Smart pointer to the raw socket to be deleted
     */
    virtual void DeleteRawSocket(Ptr<Socket> socket) = 0;

    static const uint32_t IF_ANY = 0xffffffff; //!< interface wildcard, meaning any interface

  private:
    // Indirect the SCION attributes through private pure virtual methods


    /**
     * \brief Set or unset the Weak Es Model
     *
     * RFC1122 term for whether host accepts datagram with a dest. address on another interface
     * \param model true for Weak Es Model
     */
    virtual void SetWeakEsModel(bool model) = 0;
    /**
     * \brief Get the Weak Es Model status
     *
     * RFC1122 term for whether host accepts datagram with a dest. address on another interface
     * \returns true for Weak Es Model activated
     */
    virtual bool GetWeakEsModel() const = 0;
};

} // namespace ns3

#endif /* SCION_H */
