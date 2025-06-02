#pragma once
#include <stdint.h>
#include "ns3/buffer.h"
#include <string>
#include "ns3/fields-forward.h"


namespace ns3
{

// InfoField has the following format:
//
//	 0                   1                   2                   3
//	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	|r r r r r r P C|      RSV      |             SegID             |
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//	|                           Timestamp                           |
//	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
struct InfoField 
{
	// Peer is the peering flag. If set to true, then the forwarding path is built as a peering
	// path, which requires special processing on the dataplane.
	bool Peer;
	// ConsDir is the construction direction flag. If set to true then the hop fields are arranged
	// in the direction they have been constructed during beaconing.
	bool ConsDir;
    
    #ifndef SCION_SIMPLIFIED_SIMULATION
	// SegID is a updatable field that is required for the MAC-chaining mechanism.
	uint16_t SegID;
	// Timestamp created by the initiator of the corresponding beacon.
	// The timestamp is expressed in Unix time,
	// and is encoded as an unsigned integer within 4 bytes with 1-second time granularity.
	//  This timestamp enables validation of the hop field by verification of the expiration time and MAC.
	uint32_t Timestamp;
    void UpdateSegID(  uint8_t hfMac[MACLEN] );
    #endif

    void Serialize( Buffer::Iterator start) const;
    uint32_t Deserialize( Buffer::Iterator start);

    
	constexpr static uint8_t Len();
    operator std::string()const;
    
    #if __cplusplus >= 202002L
	auto operator<=>(const InfoField& other)const = default;
    #else
    #endif

	friend std::ostream& operator<< (std::ostream& out, const InfoField& info);
};

inline std::ostream& operator<< (std::ostream& out, const InfoField& info)
{
 return	out << static_cast<std::string>(info) << std::endl;
}




} // ns3 namespace 