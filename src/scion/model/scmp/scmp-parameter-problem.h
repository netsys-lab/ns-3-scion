#pragma once
#include "ns3/header.h"

namespace ns3
{
/*!
 \brief SCMPParameterProblem represents the structure of a parameter problem message.

	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|            reserved           |           Pointer             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

    class SCMPParameterProblem :  public Header
    {
    public:
    SCMPParameterProblem(){}
    SCMPParameterProblem( uint16_t ptr )
    : m_pointer(ptr){}


    uint32_t Deserialize(Buffer::Iterator start) override;
    void Serialize( Buffer::Iterator start ) const override;
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;

private:
    
        uint16_t m_pointer;
    };



}