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

#ifndef SCION_END_POINT_DEMUX_H
#define SCION_END_POINT_DEMUX_H

#include "scion-interface.h"

#include "ns3/scion-address.h"

#include <list>
#include <stdint.h>

namespace ns3
{

class SCIONEndPoint;

/**
 * \ingroup scion
 *
 * \brief Demultiplexes packets to various transport layer endpoints
 *
 * This class serves as a lookup table to match partial or full information
 * about a four-tuple to an ns3::SCIONEndPoint.  It internally contains a list
 * of endpoints, and has APIs to add and find endpoints in this demux.  This
 * code is shared in common to TCP and UDP protocols in ns3.  This demux
 * sits between ns3's layer four and the socket layer
 */

class SCIONEndPointDemux
{
  public:
    /**
     * \brief Container of the IPv4 endpoints.
     */
    typedef std::list<SCIONEndPoint*> EndPoints;

    /**
     * \brief Iterator to the container of the IPv4 endpoints.
     */
    typedef std::list<SCIONEndPoint*>::iterator EndPointsI;

    SCIONEndPointDemux();
    ~SCIONEndPointDemux();

    /**
     * \brief Get the entire list of end points registered.
     * \return list of SCIONEndPoint
     */
    EndPoints GetAllEndPoints();

    /**
     * \brief Lookup for port local.
     * \param port port to test
     * \return true if a port local is in EndPoints, false otherwise
     */
    bool LookupPortLocal(uint16_t port);

    /**
     * \brief Lookup for address and port.
     * \param boundNetDevice Bound NetDevice (if any)
     * \param addr address to test
     * \param port port to test
     * \return true if there is a match in EndPoints, false otherwise
     */
    bool LookupLocal(Ptr<NetDevice> boundNetDevice, SCIONAddress addr, uint16_t port);

    /**
     * \brief lookup for a match with all the parameters.
     *
     * The function will return a list of most-matching EndPoints, in this order:
     *   -# Full match
     *   -# All but local address
     *   -# Only local port and local address match
     *   -# Only local port match
     *
     * EndPoint with disabled Rx are skipped.
     *
     * \param daddr destination address to test
     * \param dport destination port to test
     * \param saddr source address to test
     * \param sport source port to test
     * \param incomingInterface the incoming interface
     * \return list of SCIONEndPoints (could be 0 element)
     */
    EndPoints Lookup(SCIONAddress daddr,
                     uint16_t dport,
                     SCIONAddress saddr,
                     uint16_t sport,
                     Ptr<SCIONInterface> incomingInterface);

    /**
     * \brief simple lookup for a match with all the parameters.
     * \param daddr destination address to test
     * \param dport destination port to test
     * \param saddr source address to test
     * \param sport source port to test
     * \return SCIONEndPoint (0 if not found)
     */
    SCIONEndPoint* SimpleLookup(SCIONAddress daddr,
                               uint16_t dport,
                               SCIONAddress saddr,
                               uint16_t sport);

    /**
     * \brief Allocate a SCIONEndPoint.
     * \return an empty SCIONEndPoint instance
     */
    SCIONEndPoint* Allocate();

    /**
     * \brief Allocate a SCIONEndPoint.
     * \param address IPv4 address
     * \return an SCIONEndPoint instance
     */
    SCIONEndPoint* Allocate(SCIONAddress address);

    /**
     * \brief Allocate a SCIONEndPoint.
     * \param boundNetDevice Bound NetDevice (if any)
     * \param port local port
     * \return an SCIONEndPoint instance
     */
    SCIONEndPoint* Allocate(Ptr<NetDevice> boundNetDevice, uint16_t port);

    /**
     * \brief Allocate a SCIONEndPoint.
     * \param boundNetDevice Bound NetDevice (if any)
     * \param address local address
     * \param port local port
     * \return an SCIONEndPoint instance
     */
    SCIONEndPoint* Allocate(Ptr<NetDevice> boundNetDevice, SCIONAddress address, uint16_t port);

    /**
     * \brief Allocate a SCIONEndPoint.
     * \param boundNetDevice Bound NetDevice (if any)
     * \param localAddress local address
     * \param localPort local port
     * \param peerAddress peer address
     * \param peerPort peer port
     * \return an SCIONEndPoint instance
     */
    SCIONEndPoint* Allocate(Ptr<NetDevice> boundNetDevice,
                           SCIONAddress localAddress,
                           uint16_t localPort,
                           SCIONAddress peerAddress,
                           uint16_t peerPort);

    /**
     * \brief Remove a end point.
     * \param endPoint the end point to remove
     */
    void DeAllocate(SCIONEndPoint* endPoint);

  private:
    /**
     * \brief Allocate an ephemeral port.
     * \returns the ephemeral port
     */
    uint16_t AllocateEphemeralPort();

    /**
     * \brief The ephemeral port.
     */
    uint16_t m_ephemeral;

    /**
     * \brief The last ephemeral port.
     */
    uint16_t m_portLast;

    /**
     * \brief The first ephemeral port.
     */
    uint16_t m_portFirst;

    /**
     * \brief A list of IPv4 end points.
     */
    EndPoints m_endPoints;
};

} // namespace ns3

#endif /* IPV4_END_POINTS_H */
