#pragma once

#include "ns3/scion-types.h"
#include "ns3/header.h"
#include "ns3/scion-ia.h"

namespace ns3
{

/*!
    \brief SCMPTraceroute represents the structure of a traceroute.

	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Identifier          |        Sequence Number        |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|              ISD              |                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+         AS                    +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                        Interface ID                           +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

    class SCMPTraceroute : public Header
    {
        public:
             

    uint32_t Deserialize(Buffer::Iterator start) override;
   

    void Serialize( Buffer::Iterator start ) const override;
   
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;


  
private:

        uint16_t m_identifier;
        uint16_t m_sequence;
        Ia m_ia;
        uint16_t m_interface;
    };

    

}