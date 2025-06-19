#pragma once
#include "ns3/address.h"

#include <stdint.h>

#define ENDHOST_PORT 30041

// LineLen is the length of a SCION header line in bytes.
#define LineLen 4
// CmnHdrLen is the length of the SCION common header in bytes.
#define CmnHdrLen 12
// MaxHdrLen is the maximum allowed length of a SCION header in bytes.
#define MaxHdrLen 1020
// Per default link MTU set to 1500 and SCION Mtu to 1472 consequently
#define DefaultScionMtu 1472

namespace ns3
{

//
enum class ScionUnderlay
{
    L2_ETHERNET, // Use Ethernet as the underlay
    L3_IPv4,     // For IPv4-based underlay
    L3_IPv6,     // For IPv4-based underlay
};

// IA represents the ISD (ISolation Domain) and AS (Autonomous System) Id of a given SCION AS.
// The highest 16 bit form the ISD number and the lower 48 bits form the AS number.
using Ia_t = uint64_t;
using Asn_t = uint64_t;
using Isd_t = uint16_t;

enum L4ProtocolType_t : uint8_t
{
    L4None = 0,
    L4TCP = 6,
    L4UDP = 17,
    L4SCMP = 202,
    L4BFD = 203,
    HopByHopClass = 200,
    End2EndClass = 201
};

// AddrType indicates the type of a host address in the SCION header.
// The AddrType consists of a sub-type and length part, both two bits wide.
// The four possible lengths are 4B (0), 8B (1), 12B (2), or 16B (3) bytes.
// There are four possible sub-types per address length.

// AddrType constants
enum AddrType_t : uint8_t
{
    T4Ip = 0b0000,  // T=0, L=0
    T4Svc = 0b0100, // T=1, L=0
    T16Ip = 0b0011  // T=0, L=3
};

// Length returns the length of this AddrType value.
constexpr inline int
AddrTypeLength(AddrType_t a)
{
    return LineLen * (1 + (a & 0x3));
}

static_assert(AddrTypeLength(T4Ip) == 4);
static_assert(AddrTypeLength(T4Svc) == 4);
static_assert(AddrTypeLength(T16Ip) == 16);

enum LinkType_t : uint8_t
{
    // LinkTypeUnset represents an unspecified link type.
    LinkTypeUnset,
    // LinkTypeDirect represents a direct physical connection.
    LinkTypeDirect,
    // LinkTypeMultihop represents a connection with local routing/switching.
    LinkTypeMultiHop,
    // LinkTypeOpennet represents a connection overlayed over publicly routed Internet
    LinkTypeOpenNet
};

// Existing enums and structs
enum class ScionLinkType
{
    CORE,
    PARENT,
    CHILD,
    PEER
};

// New enum for node roles
enum class ScionNodeType
{
    UNSPECIFIED, // Default or intermediate (e.g. pure switch)
    BORDER_ROUTER,
    CONTROL_SERVICE,
    INTERNAL_ROUTER, // Could also represent a switch
    HOST
};

} // namespace ns3