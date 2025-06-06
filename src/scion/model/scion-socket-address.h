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

#ifndef SCION_SOCKET_ADDRESS_H
#define SCION_SOCKET_ADDRESS_H

#include "scion-address.h"
#include "ns3/address.h"

#include <stdint.h>

namespace ns3
{

/**
 * \ingroup address
 *
 * \brief an Inet address class
 * this class holds an SCIONAddress and a port number
 * to form a SCION transport endpoint.
 */
class SCIONSocketAddress
{
  public:
    /**
     * \param addr the scion address
     * \param port the port number
     */
    SCIONSocketAddress(SCIONAddress addr, uint16_t port);
    /**
     * \param addr the SCION address
     *
     * The port number is set to zero by default.
     */
    SCIONSocketAddress(SCIONAddress addr);
    /**
     * \param port the port number
     *
     * The host address is set to the "Any" address by default.
     */
    SCIONSocketAddress(uint16_t port);
    /**
     * \param addr string which represents a SCION address
     * \param port the port number
     */
    SCIONSocketAddress(const char* addr, uint16_t port);
    /**
     * \param addr string which represents a SCION address
     *
     * The port number is set to zero.
     */
    SCIONSocketAddress(const char* addr);
    /**
     * \returns the port number
     */
    uint16_t GetPort() const;
    /**
     * \returns the L3 SCION address
     */
    SCIONAddress GetSCION() const;
    /**
     * \returns the ToS
     */
    uint8_t GetTos() const;

    /**
     * \param port the new port number.
     */
    void SetPort(uint16_t port);
    /**
     * \param address the new L3 SCION address
     */
    void SetSCION(SCIONAddress address);
    /**
     * \param tos the new ToS.
     */
    void SetTos(uint8_t tos);

    /**
     * \param address address to test
     * \returns true if the address matches, false otherwise.
     */
    static bool IsMatchingType(const Address& address);

    /**
     * \returns an Address instance which represents this
     * SCIONSocketAddress instance.
     */
    operator Address() const;

    /**
     * \brief Returns an SCIONSocketAddress which corresponds to the input
     * Address.
     *
     * \param address the Address instance to convert from.
     * \returns an SCIONSocketAddress
     */
    static SCIONSocketAddress ConvertFrom(const Address& address);

    /**
     * \brief Convert to an Address type
     * \return the Address corresponding to this object.
     */
    Address ConvertTo() const;

  private:
    /**
     * \brief Get the underlying address type (automatically assigned).
     *
     * \returns the address type
     */
    static uint8_t GetType();
    SCIONAddress m_ipv4; //!< the L3 address
    uint16_t m_port;    //!< the port
    uint8_t m_tos;      //!< the ToS
};

} // namespace ns3

#endif /* SCION_SOCKET_ADDRESS_H */
