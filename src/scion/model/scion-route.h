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
 */
#ifndef SCION_ROUTE_H
#define SCION_ROUTE_H

#include "ns3/scion-address.h"
#include "ns3/simple-ref-count.h"

#include <list>
#include <map>
#include <ostream>

namespace ns3
{

class NetDevice;

/**
 * \ingroup scion
 * \brief routing context for SCION packets
 *\details similar to IPv4Route, SCION route is some kind of context,
         that is provided along with a packet, in order to steer it through the network
         one hop at a time.
         Although SCION is source- and not (like IP) destination-routed,
         this context is still required just the same.
         In case of SCION, the 'route' contains the dataplane path to use,
         which will be serialized into the packet's header,
         as well as the underlay-next hop (or gateway).
         The latter being the (current AS internal) IP address,
         of the next-hop border-router(AS Interface).

        The SCIONRoute is required, because not all forwarding information is contained in the 
        dataplane path/PCFS i.e. the underlay-nexthop addresses that map to an AS IF

notes on IPv4Route:
if route.Gateway IsAny the packet is sent to the IPHeader's destination address.
Otherwise it's sent to the gateway address.         

 *
 * This is a reference counted object.
 */
class SCIONRoute : public SimpleRefCount<SCIONRoute>
{
  public:
    SCIONRoute();

    /**
     * \param dest Destination SCIONAddress
     */
    void SetDestination(SCIONAddress dest);
    /**
     * \return Destination SCIONAddress of the route
     */
    SCIONAddress GetDestination() const;

    /**
     * \param src Source SCIONAddress
     */
    void SetSource(SCIONAddress src);
    /**
     * \return Source SCIONAddress of the route
     */
    SCIONAddress GetSource() const;

    /**
     * \param gw underlay (next hop) SCIONAddress
     */
    void SetNextHop(SCIONAddress gw);
    /**
     * \return SCIONAddress of the gateway (next hop)
     */
    SCIONAddress GetNextHop() const;

    /**
     * Equivalent in Linux to dst_entry.dev
     *
     * \param outputDevice pointer to NetDevice for outgoing packets
     */
    void SetOutputDevice(Ptr<NetDevice> outputDevice);
    /**
     * \return pointer to NetDevice for outgoing packets
     */
    Ptr<NetDevice> GetOutputDevice() const;

    const BasePath& GetPath() const;

    // this would propably allow for the most efficient implementations.
    bool SetPath(SCIONHeader& hdr)const;

  private:

    std::function<bool , SCIONHeader&> m_set_path;
    BasePath m_path; //!< Dataplane forwarding path

    // is destination really necessary ?! its already in the SCIONHeader
    SCIONAddress m_dest;            //!< Destination address.
    SCIONAddress m_source;          //!< Source address.
    
    // could be changed to generic 'Address' as well.
    // IA number is actually not required for intra-AS forwarding
    SCIONAddress m_next_hop;         //!< Underlay next hop address. (of border router)

    // this doesn't have to be a netdevice. The L3Protocol needs the outInterface to call Send() on it.
    // The device is translated to an interface with GetInterfaceForDevice(dev) in SendRealRout()
    
    Ptr<NetDevice> m_outputDevice; //!< Output device.
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the reference to the output stream
 * \param route the Ipv4 route
 * \returns the reference to the output stream
 */
std::ostream& operator<<(std::ostream& os, const SCIONRoute& route);

} // namespace ns3

#endif /* SCION_ROUTE_H */
