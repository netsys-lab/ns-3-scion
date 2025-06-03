#pragma once
#include "ns3/header.h"

namespace ns3
{

/*!
  \brief SCMPDestinationUnreachable represents the structure of a destination
  unreachable message.

	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                             Unused                            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

class SCMPDestinationUnreachable :  public Header
{
public:
    uint32_t Deserialize(Buffer::Iterator );
    void Serialize( Buffer::Iterator  start ) const override;   
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;

private:

};



}