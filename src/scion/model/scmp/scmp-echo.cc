#include "ns3/scmp-echo.h"

namespace ns3
{

void
SCMPEcho::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU16( m_identifier );
    start.WriteHtonU16( m_sequence_number );
}

uint32_t
SCMPEcho::GetSerializedSize() const
{
    return 4;
}

uint32_t
SCMPEcho::Deserialize(Buffer::Iterator start)
{
   
    
	NS_ASSERT_MSG( start.GetRemainingSize() >= 4, "buffer too short");    
	
	m_identifier = start.ReadNtohU16();
	
	m_sequence_number = start.ReadNtohU16();
		
    return 4;
}

TypeId
SCMPEcho::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCMPEcho")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCMPEcho>();
    return tid;
}

TypeId
SCMPEcho::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCMPEcho::Print(std::ostream& os) const
{
    os << "<SCMPEcho/>";
}


}