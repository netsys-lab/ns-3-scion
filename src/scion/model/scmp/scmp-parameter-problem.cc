#include "ns3/scmp-parameter-problem.h"


namespace ns3
{

void
SCMPParameterProblem::Serialize(Buffer::Iterator start) const
{
   
   start.WriteHtonU16(0);
   start.WriteHtonU16(m_pointer);
}


uint32_t
SCMPParameterProblem::GetSerializedSize() const
{
    return 4;
}

uint32_t
SCMPParameterProblem::Deserialize(Buffer::Iterator start)
{
    NS_ASSERT_MSG( start.GetRemainingSize() >= GetSerializedSize(), "buffer too short" );
    
    start.Next(2);
    m_pointer = start.ReadNtohU16();	
	
    return 4;
}

TypeId
SCMPParameterProblem::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCMPParameterProblem")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCMPParameterProblem>();
    return tid;
}

TypeId
SCMPParameterProblem::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCMPParameterProblem::Print(std::ostream& os) const
{
    os << "<SCMPParameterProblem/>";
}


}