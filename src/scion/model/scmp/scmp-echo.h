#pragma once
#include "ns3/header.h"

namespace ns3
{

/*!
  \brief SCMPEcho represents the structure of a ping.

	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Identifier          |        Sequence Number        |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
class SCMPEcho : public Header
{
public:
SCMPEcho(){}
SCMPEcho( uint16_t id, uint16_t seq )
: m_identifier(id),
 m_sequence_number(seq)
 {}

    uint32_t Deserialize(Buffer::Iterator start) override;
    void Serialize( Buffer::Iterator start ) const override;
   
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;


  
private:


uint16_t m_identifier=0;
uint16_t m_sequence_number=0;
};



}