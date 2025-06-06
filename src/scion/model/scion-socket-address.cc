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

#include "scion-socket-address.h"

#include "ns3/assert.h"
#include "ns3/log.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("SCIONSocketAddress");

SCIONSocketAddress::SCIONSocketAddress(SCIONAddress ipv4, uint16_t port)
    : m_ipv4(ipv4),
      m_port(port),
      m_tos(0)
{
    NS_LOG_FUNCTION(this << ipv4 << port);
}

SCIONSocketAddress::SCIONSocketAddress(SCIONAddress ipv4)
    : m_ipv4(ipv4),
      m_port(0),
      m_tos(0)
{
    NS_LOG_FUNCTION(this << ipv4);
}

SCIONSocketAddress::SCIONSocketAddress(const char* addr, uint16_t port)
    : m_ipv4(SCIONAddress(addr)),
      m_port(port),
      m_tos(0)
{
    NS_LOG_FUNCTION(this << addr << port);
}

SCIONSocketAddress::SCIONSocketAddress(const char* addr)
    : m_ipv4(SCIONAddress(addr)),
      m_port(0),
      m_tos(0)
{
    NS_LOG_FUNCTION(this << addr);
}

SCIONSocketAddress::SCIONSocketAddress(uint16_t port)
    : m_ipv4(), // SCIONAddress::GetAny()
      m_port(port),
      m_tos(0)
{
    NS_LOG_FUNCTION(this << port);
}

uint16_t
SCIONSocketAddress::GetPort() const
{
    NS_LOG_FUNCTION(this);
    return m_port;
}

SCIONAddress
SCIONSocketAddress::GetSCION() const
{
    NS_LOG_FUNCTION(this);
    return m_ipv4;
}

uint8_t
SCIONSocketAddress::GetTos() const
{
    NS_LOG_FUNCTION(this);
    return m_tos;
}

void
SCIONSocketAddress::SetPort(uint16_t port)
{
    NS_LOG_FUNCTION(this << port);
    m_port = port;
}

void
SCIONSocketAddress::SetSCION(SCIONAddress address)
{
    NS_LOG_FUNCTION(this << address);
    m_ipv4 = address;
}

void
SCIONSocketAddress::SetTos(uint8_t tos)
{
    NS_LOG_FUNCTION(this << tos);
    m_tos = tos;
}

bool
SCIONSocketAddress::IsMatchingType(const Address& address)
{
    NS_LOG_FUNCTION(&address);
    return address.CheckCompatible(GetType(), address.GetLength());
}

SCIONSocketAddress::operator Address() const
{
    return ConvertTo();
}

Address
SCIONSocketAddress::ConvertTo() const
{
    NS_LOG_FUNCTION(this);
    auto sz{m_ipv4.GetSerializedSize()};
    uint8_t* buf = (uint8_t*)malloc(sz+3);
    m_ipv4.Serialize(buf);
    buf[sz] = m_port & 0xff;
    buf[sz+1] = (m_port >> 8) & 0xff;
    buf[sz+2] = m_tos;
    auto tmp = Address(GetType(), buf, sz+3);
    free(buf);
    return tmp;
}

SCIONSocketAddress
SCIONSocketAddress::ConvertFrom(const Address& address)
{
    NS_LOG_FUNCTION(&address);
   
    uint8_t* buf = new uint8_t[address.GetLength()] ;
    address.CopyTo(buf);

    SCIONAddress scaddr;
    scaddr.Deserialize(buf);
    auto sz = scaddr.GetSerializedSize();
    uint16_t port = buf[sz] | (buf[sz+1] << 8);
    uint8_t tos = buf[sz+2];
    SCIONSocketAddress inet(scaddr, port);
    inet.SetTos(tos);
    return inet;
}

uint8_t
SCIONSocketAddress::GetType()
{
    NS_LOG_FUNCTION_NOARGS();
    static uint8_t type = Address::Register();
    return type;
}

} // namespace ns3
