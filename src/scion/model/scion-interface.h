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
 * Authors:
 */
#ifndef SCION_INTERFACE_H
#define SCION_INTERFACE_H

#include "ns3/object.h"
#include "ns3/ptr.h"

#include <list>

namespace ns3
{

class NetDevice;
class Packet;
class Node;
class SCIONInterfaceAddress;
class SCIONAddress;
class SCIONHeader;
class TrafficControlLayer;

/**
 * \ingroup scion
 *
 * \brief The SCION representation of a network interface
 *
 *
 * By default, Ipv4 interface are created in the "down" state
 * no IP addresses.  Before becoming usable, the user must
 * add an address of some type and invoke Setup on them.
 */
class SCIONInterface : public Object
{
  public:
    /**
     * \brief Get the type ID
     * \return type ID
     */
    static TypeId GetTypeId();

    SCIONInterface();
    ~SCIONInterface() override;

    // Delete copy constructor and assignment operator to avoid misuse
    SCIONInterface(const SCIONInterface&) = delete;
    SCIONInterface& operator=(const SCIONInterface&) = delete;

    /**
     * \brief Set node associated with interface.
     * \param node node
     */
    void SetNode(Ptr<Node> node);
    /**
     * \brief Set the NetDevice.
     * \param device NetDevice
     */
    void SetDevice(Ptr<NetDevice> device);
    /**
     * \brief Set the TrafficControlLayer.
     * \param tc TrafficControlLayer object
     */
    void SetTrafficControl(Ptr<TrafficControlLayer> tc);

    /**
     * \returns the underlying NetDevice. This method cannot return zero.
     */
    Ptr<NetDevice> GetDevice() const;

  
    /**
     * These are IP interface states and may be distinct from
     * NetDevice states, such as found in real implementations
     * (where the device may be down but IP interface state is still up).
     */
    /**
     * \returns true if this interface is enabled, false otherwise.
     */
    bool IsUp() const;

    /**
     * \returns true if this interface is disabled, false otherwise.
     */
    bool IsDown() const;

    /**
     * Enable this interface
     */
    void SetUp();

    /**
     * Disable this interface
     */
    void SetDown();

    /**
     * \returns true if this interface is enabled for IP forwarding of input datagrams
     */
    bool IsForwarding() const;

    /**
     * \param val Whether to enable or disable IP forwarding for input datagrams
     */
    void SetForwarding(bool val);

    /**
     * \param p packet to send
     * \param hdr IPv4 header
     * \param dest next hop address of packet.
     *
     * This method will eventually call the private
     * SendTo method which must be implemented by subclasses.
     */
    void Send(Ptr<Packet> p, const SCIONHeader& hdr, SCIONAddress dest);

    /**
     * \param address The SCIONInterfaceAddress to add to the interface
     * \returns true if succeeded
     */
    bool AddAddress(SCIONInterfaceAddress address);

    /**
     * \param index Index of SCIONInterfaceAddress to return
     * \returns The SCIONInterfaceAddress address whose index is i
     */
    SCIONInterfaceAddress GetAddress(uint32_t index) const;

    /**
     * \returns the number of SCIONInterfaceAddress stored on this interface
     */
    uint32_t GetNAddresses() const;

    /**
     * \param index Index of SCIONInterfaceAddress to remove
     * \returns The SCIONInterfaceAddress address whose index is index
     */
    SCIONInterfaceAddress RemoveAddress(uint32_t index);

    /**
     * \brief Remove the given Ipv4 address from the interface.
     * \param address The Ipv4 address to remove
     * \returns The removed Ipv4 interface address
     * \returns The null interface address if the interface did not contain the
     * address or if loopback address was passed as argument
     */
    SCIONInterfaceAddress RemoveAddress(SCIONAddress address);

    /**
     * This callback is set when an address is removed from an interface with
     * auto-generated Arp cache and it allow the neighbor cache helper to update
     * neighbor's Arp cache
     *
     * \param removeAddressCallback Callback when remove an address.
     */
    void RemoveAddressCallback(
        Callback<void, Ptr<SCIONInterface>, SCIONInterfaceAddress> removeAddressCallback);

    /**
     * This callback is set when an address is added from an interface with
     * auto-generated Arp cache and it allow the neighbor cache helper to update
     * neighbor's Arp cache
     *
     * \param addAddressCallback Callback when remove an address.
     */
    void AddAddressCallback(
        Callback<void, Ptr<SCIONInterface>, SCIONInterfaceAddress> addAddressCallback);

  protected:
    void DoDispose() override;

  private:
    /**
     * \brief Initialize interface.
     */
    void DoSetup();

    /**
     * \brief Container for the SCIONInterfaceAddresses.
     */
    typedef std::list<SCIONInterfaceAddress> SCIONInterfaceAddressList;

    /**
     * \brief Container Iterator for the SCIONInterfaceAddresses.
     */
    typedef std::list<SCIONInterfaceAddress>::const_iterator SCIONInterfaceAddressListCI;

    /**
     * \brief Const Container Iterator for the SCIONInterfaceAddresses.
     */
    typedef std::list<SCIONInterfaceAddress>::iterator SCIONInterfaceAddressListI;

    bool m_ifup;                        //!< The state of this interface
    bool m_forwarding;                  //!< Forwarding state.
    uint16_t m_metric;                  //!< Interface metric
    SCIONInterfaceAddressList m_ifaddrs; //!< Address list
    Ptr<Node> m_node;                   //!< The associated node
    Ptr<NetDevice> m_device;            //!< The associated NetDevice
    Ptr<TrafficControlLayer> m_tc;      //!< The associated TrafficControlLayer
    
    Callback<void, Ptr<SCIONInterface>, SCIONInterfaceAddress>
        m_removeAddressCallback; //!< remove address callback
    Callback<void, Ptr<SCIONInterface>, SCIONInterfaceAddress>
        m_addAddressCallback; //!< add address callback
};

} // namespace ns3

#endif
