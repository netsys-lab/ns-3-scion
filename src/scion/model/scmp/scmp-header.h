#pragma once
#include "ns3/header.h"
#include "ns3/scmp-typecodes.h"


namespace ns3
{


/* MaxSCMPPacketLen the maximum length a SCION packet including SCMP quote can
   have. This length includes the SCION, and SCMP header of the packet.

	+-------------------------+
	|        Underlay         |
	+-------------------------+
	|          SCION          |  \
	|          SCMP           |   \
	+-------------------------+    \_ MaxSCMPPacketLen
	|          Quote:         |    /
	|        SCION Orig       |   /
	|         L4 Orig         |  /
	+-------------------------+
*/
constexpr uint16_t MaxSCMPPacketLen = 1232;
 // this should maybe better become an attribute on SCMPHeader's TypeId

/*!
  \brief SCMP is the SCMP header on top of SCION header.

	 0                   1                   2                   3
	 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |     Code      |           Checksum            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                            InfoBlock                          |
	+                                                               +
	|                         (variable length)                     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                            DataBlock                          |
	+                                                               +
	|                         (variable length)                     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
 class SCMPHeader  :  public Header
 {
public:
	SCMPHeader() = default;
    SCMPHeader( SCMPTypeCode tc  ) : m_typecode(tc){}
    const SCMPTypeCode& TypeCode()const {return m_typecode; }
 
    uint32_t Deserialize(Buffer::Iterator start) override;    
    void Serialize( Buffer::Iterator start ) const override;
    
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;


  
private:

  
    SCMPTypeCode m_typecode;
	uint16_t m_checksum;

	// scn *SCION   SCIONHeader* m_scn; // needed in Go for computation of Checksum
};

}