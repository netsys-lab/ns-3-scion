#pragma once
#include "ns3/scion-types.h"
#include "ns3/header.h"
#include "ns3/scion-ia.h"
namespace ns3
{

/*!
  \brief SCMPExternalInterfaceDown message contains the data for that error.

	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
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

class SCMPExternalInterfaceDown :  public Header
{
    public:
    SCMPExternalInterfaceDown(){}
    SCMPExternalInterfaceDown( Ia ia, uint16_t id):m_ia(ia), m_ifid(id) {}


    uint32_t Deserialize(Buffer::Iterator start) override;
    void Serialize( Buffer::Iterator ) const override;    
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;


  
private:


Ia m_ia;
uint16_t m_ifid=0;
};


}