#pragma once
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef SCION_ADDRESS_H
#define SCION_ADDRESS_H

#include "ns3/address.h"
#include "ns3/attribute-helper.h"
#include "ns3/scion-ia.h"
#include "ns3/buffer.h"

#include <ostream>
#include <stdint.h>

namespace ns3
{


// inverse of AddrTypeConf
AddrType_t ConvAddrType( const Address&  );

/**
 * \ingroup address
 * \brief a L3 SCION address (without L4 Port)
 *      

 */
class SCIONAddress
{
  public:
    SCIONAddress() = default;
    
    explicit SCIONAddress( Ia ia, const Address& host );    
    explicit SCIONAddress( Isd isd, Asn as, const Address& host);
    SCIONAddress( const std::string & address);

    uint32_t GetSerializedSize() const;

    uint8_t GetLength() const;

    // SCIONAddress(const char* address);

    // void Set(const char* address);
    void Serialize(  uint8_t * buffer ) const;
    void Serialize(TagBuffer) const;
    void Serialize(Buffer::Iterator) const;

    SCIONAddress& Deserialize( TagBuffer);
    SCIONAddress& Deserialize( Buffer::Iterator );
    SCIONAddress& Deserialize( const uint8_t * buffer );
    
    Isd GetISD() const;
    Asn GetAS() const;
    //uint16_t GetPort()const{return _port; }
    //void SetPort( uint16_t p ) { _port = p; }
    const Address& GetHostAddress() const { return _hostAddr; }
    const Ia& GetIA()const;

    
    void Print(std::ostream& os) const;

    /**
     * \return true if address is initialized (i.e., set to something), false otherwise
     */
    //bool IsInitialized() const;
    /**
     * \return true if address is 0.0.0.0; false otherwise
     */
    //bool IsAny() const;
    /**
     * \return true if address is 127.0.0.1; false otherwise
     */
    // bool IsLocalhost() const;

   static bool IsMatchingType(const Address& address);
    
    operator Address() const;

    static SCIONAddress ConvertFrom(const Address& address);

    Address ConvertTo() const;

    #if __cplusplus >= 202002L
    std::strong_ordering operator <=>( const SCIONAddress &other ) const;
    #else

    /**
     * \brief Equal to operator.
     *
     * \param a the first operand.
     * \param b the first operand.
     * \returns true if the operands are equal.
     */
    friend bool operator==(const SCIONAddress& a, const SCIONAddress& b);

    /**
     * \brief Not equal to operator.
     *
     * \param a the first operand.
     * \param b the first operand.
     * \returns true if the operands are not equal.
     */
    friend bool operator!=(const SCIONAddress& a, const SCIONAddress& b);

    /**
     * \brief Less than to operator.
     *
     * \param a the first operand.
     * \param b the first operand.
     * \returns true if the first operand is less than the second.
     */
    friend bool operator<(const SCIONAddress& a, const SCIONAddress& b);
    #endif


  private:

    static uint8_t GetType();
    Ia _ia;
    AddrType_t addrType;
    Address _hostAddr; // either an Ipv4 or Ipv6 addr. Not InetSocketAddr
    
    // bool m_initialized; //!<  address has been explicitly initialized to a valid value.

};

ATTRIBUTE_HELPER_HEADER(SCIONAddress);

// Address SetPort( const Address& address, uint16_t port );

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param address the address
 * \returns a reference to the stream
 */
std::ostream& operator<<(std::ostream& os, const SCIONAddress& address);

/**
 * \brief Stream extraction operator.
 *
 * \param is the stream
 * \param address the address
 * \returns a reference to the stream
 */
std::istream& operator>>(std::istream& is, SCIONAddress& address);

#if __cplusplus < 202002L
inline bool
operator==(const SCIONAddress& a, const SCIONAddress& b)
{
    return (a._hostAddr == b._hostAddr);
}

inline bool
operator!=(const SCIONAddress& a, const SCIONAddress& b)
{
    return (a._hostAddr != b._hostAddr);
}

inline bool
operator<(const SCIONAddress& a, const SCIONAddress& b)
{
    return (a._hostAddr < b._hostAddr);
}
#endif



} // namespace ns3

namespace std
{
template <>
struct hash<ns3::SCIONAddress>
{
    auto operator()(const ns3::SCIONAddress& a) const -> size_t
    {
      std::stringstream ss;
      a.Print(ss);
      return std::hash<std::string>{}(ss.str() );
        //  return hash<SNETPath>{}(xyz.value);
    }
};
}

#endif // SCION_ADDRESS