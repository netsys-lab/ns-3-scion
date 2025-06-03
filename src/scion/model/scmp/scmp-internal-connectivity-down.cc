#include "ns3/scmp-internal-connectivity-down.h"
#include "ns3/assert.h"

namespace ns3
{

SCMPInternalConnectivityDown::SCMPInternalConnectivityDown( Ia ia, uint64_t ingress, uint64_t egress)
: m_ia(ia),
m_ingress(ingress),
m_egress(egress)
{

}


void
SCMPInternalConnectivityDown::Serialize(Buffer::Iterator start) const
{
   
   start.WriteHtonU64(m_ia.GetValue());
    start.WriteHtonU64(m_ingress);
    start.WriteHtonU64(m_egress);
}

uint32_t
SCMPInternalConnectivityDown::GetSerializedSize() const
{
    return 24;
}

uint32_t
SCMPInternalConnectivityDown::Deserialize(Buffer::Iterator start)
{

	NS_ASSERT_MSG( start.GetRemainingSize() >= GetSerializedSize(), "buffer too short" );   
    	
    m_ia = Ia(start.ReadNtohU64());	
    m_ingress = start.ReadNtohU64();
    m_egress = start.ReadNtohU64();

    return 24;
}

TypeId
SCMPInternalConnectivityDown::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCMPInternalConnectivityDown")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCMPInternalConnectivityDown>();
    return tid;
}

TypeId
SCMPInternalConnectivityDown::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCMPInternalConnectivityDown::Print(std::ostream& os) const
{
    os << "<SCMPInternalConnectivityDown/>";
}

}