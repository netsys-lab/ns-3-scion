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

#ifndef SCION_INTERFACE_ADDRESS_H
#define SCION_INTERFACE_ADDRESS_H

#include "ns3/scion-address.h"

#include <ostream>
#include <stdint.h>

namespace ns3
{

/**
 * \ingroup address
 * \ingroup SCION
 *
 * \brief a class to store SCION address information on an interface
 *
 * A list of these addresses is stored in SCIONInterface. 
 * Here address aliasing can be done.
 * For now this class isn't technically needed (actually identical to SCIONAddress).
 * However in the future it might be required i.e. for multihoming.
 * 
 */
class SCIONInterfaceAddress
{
  public:
    /**
     * \enum InterfaceAddressScope_e
     * \brief Address scope.
     */
    enum InterfaceAddressScope_e
    {
        HOST,
        LINK,
        GLOBAL
    };

    SCIONInterfaceAddress();

    /**
     * \brief Configure local address, mask and broadcast address
     * \param local the local address
     * \param mask the network mask
     */
    SCIONInterfaceAddress(SCIONAddress local);
    /**
     * Copy constructor
     * \param o the object to copy
     */
    SCIONInterfaceAddress(const SCIONInterfaceAddress& o);

    /**
     * \brief Set local address
     * \param address the address
     *
     * \note Functially identical to `SCIONInterfaceAddress::SetLocal`.
     *       This function is consistent with `Ipv6InterfaceAddress::SetAddress`.
     */
    void SetAddress(SCIONAddress address);



    /**
     * \brief Get the local address
     * \returns the local address
     *
     * \note Functially identical to `SCIONInterfaceAddress::GetLocal`.
     *       This function is consistent with `Ipv6InterfaceAddress::GetAddress`.
     */
    SCIONAddress GetAddress() const;


    /**
     * \brief Set the scope.
     * \param scope the scope of address
     */
    void SetScope(SCIONInterfaceAddress::InterfaceAddressScope_e scope);

    /**
     * \brief Get address scope.
     * \return scope
     */
    SCIONInterfaceAddress::InterfaceAddressScope_e GetScope() const;

    /**
     * \brief Checks if the address is in the same subnet.
     * \param b the address to check
     * \return true if the address is in the same subnet.
     */
    bool IsInSameSubnet(const SCIONAddress b) const;

  private:
    SCIONAddress m_local; //!< Interface address

    InterfaceAddressScope_e m_scope; //!< Address scope
    /**
     * \brief Equal to operator.
     *
     * \param a the first operand
     * \param b the first operand
     * \returns true if the operands are equal
     */
    friend bool operator==(const SCIONInterfaceAddress& a, const SCIONInterfaceAddress& b);

    /**
     * \brief Not equal to operator.
     *
     * \param a the first operand
     * \param b the first operand
     * \returns true if the operands are not equal
     */
    friend bool operator!=(const SCIONInterfaceAddress& a, const SCIONInterfaceAddress& b);
};

/**
 * \brief Stream insertion operator.
 *
 * \param os the reference to the output stream
 * \param addr the SCIONInterfaceAddress
 * \returns the reference to the output stream
 */
std::ostream& operator<<(std::ostream& os, const SCIONInterfaceAddress& addr);

inline bool
operator==(const SCIONInterfaceAddress& a, const SCIONInterfaceAddress& b)
{
    return (a.m_local == b.m_local &&
            a.m_scope == b.m_scope );
}

inline bool
operator!=(const SCIONInterfaceAddress& a, const SCIONInterfaceAddress& b)
{
    return (a.m_local != b.m_local ||
            a.m_scope != b.m_scope );
}

} // namespace ns3

#endif /* IPV4_ADDRESS_H */
