#include "ns3/scmp-traceroute.h"

namespace ns3
{


void
SCMPTraceroute::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU16(m_identifier);
    start.WriteHtonU16(m_sequence);
    start.WriteHtonU64(m_ia.GetValue());
    start.WriteHtonU64(m_interface);

}

uint32_t
SCMPTraceroute::GetSerializedSize() const
{
    return 20;
}

uint32_t
SCMPTraceroute::Deserialize(Buffer::Iterator start)
{

	NS_ASSERT_MSG( start.GetRemainingSize() >= GetSerializedSize(), "buffer too short" );
    
	
	m_identifier = start.ReadNtohU16();	
    m_sequence = start.ReadNtohU16();	
	m_ia = Ia(start.ReadNtohU64());	
    m_interface = start.ReadNtohU64();	

    return GetSerializedSize() ;
}

TypeId
SCMPTraceroute::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCMPTraceroute")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCMPTraceroute>();
    return tid;
}

TypeId
SCMPTraceroute::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCMPTraceroute::Print(std::ostream& os) const
{
    os << "<SCMPTraceroute/>";
}

}