#include "ns3/scmp-header.h"

#include "ns3/scmp-packet-too-big.h"
#include "ns3/scmp-external-interface-down.h"
#include "ns3/scmp-destination-unreachable.h"
#include "ns3/scmp-internal-connectivity-down.h"
#include "ns3/scmp-traceroute.h"
#include "ns3/scmp-parameter-problem.h"
#include "ns3/scmp-echo.h"

namespace ns3
{


uint32_t
SCMPHeader::Deserialize(Buffer::Iterator start)
{
  
    NS_ASSERT_MSG(start.GetRemainingSize() >= 4 , "buffer too short");
        
    /*auto fst = start.ReadU8();
    auto snd = start.ReadU8();
    m_typecode = SCMPTypeCode(fst, snd);
    */
   m_typecode = SCMPTypeCode(start.ReadNtohU16());
    m_checksum = start.ReadNtohU16(); // =binary.BigEndian.Uint16(data[2:4])

    return 4;
}

void
SCMPHeader::Serialize(Buffer::Iterator start) const
{
   	
	start.WriteHtonU16(m_typecode.TypeCode() );

/*	if opts.ComputeChecksums
    {
		if s.scn == nil {
			return serrors.New("can not calculate checksum without SCION header")
		}
		// zero out checksum bytes
		//bytes[2] = 0
		//bytes[3] = 0
        start.WriteHtonU16(0);
		m_checksum, err = s.scn.computeChecksum(b.Bytes(), uint8(L4SCMP))



		if err != nil {
			return err
		}

	}
*/    
	start.WriteHtonU16( m_checksum);
	
}

uint32_t
SCMPHeader::GetSerializedSize() const
{
    return 4;
}

TypeId
SCMPHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCMPHeader")
                            .SetParent<Header>()
                            .SetGroupName("Internet");
                            // .AddConstructor<SCMPHeader>();
    return tid;
}

TypeId
SCMPHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCMPHeader::Print(std::ostream& os) const
{
    os << "SCMPHeader";
}

} // namespace ns3