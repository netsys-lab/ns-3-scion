#include "ns3/scmp-external-interface-down.h"

namespace ns3
{


void
SCMPExternalInterfaceDown::Serialize(Buffer::Iterator start) const
{
    start.WriteHtonU64(m_ia.GetValue());
    start.WriteHtonU64(m_ifid);
}

uint32_t
SCMPExternalInterfaceDown::GetSerializedSize() const
{
    return 16;
}

uint32_t
SCMPExternalInterfaceDown::Deserialize(Buffer::Iterator start)
{

    m_ia = Ia(start.ReadNtohU64());
    m_ifid = start.ReadNtohU64();
    return 16;
}

TypeId
SCMPExternalInterfaceDown::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCMPExternalInterfaceDown")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCMPExternalInterfaceDown>();
    return tid;
}

TypeId
SCMPExternalInterfaceDown::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCMPExternalInterfaceDown::Print(std::ostream& os) const
{
    os << "<SCMPExternalInterfaceDown/>";
}

} // namespace ns3