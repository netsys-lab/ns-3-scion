#pragma once
#include "ns3/header.h"
#include "ns3/scion-types.h"
#include "ns3/scion-ia.h"

namespace ns3
{

/*!
  \brief SCMPInternalConnectivityDown indicates the AS internal connection 
  between 2 routers is down. The format is as follows:

	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|              ISD              |                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+         AS                    +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                   Ingress Interface ID                        +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                   Egress Interface ID                         +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
class SCMPInternalConnectivityDown  : public Header
{
public:

SCMPInternalConnectivityDown(){}
SCMPInternalConnectivityDown( Ia ia, uint64_t ingress, uint64_t egress);


    uint32_t Deserialize(Buffer::Iterator start) override;
    void Serialize( Buffer::Iterator start ) const override;
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;


  
private:

Ia m_ia;

uint64_t m_ingress;
uint64_t m_egress;
};



}