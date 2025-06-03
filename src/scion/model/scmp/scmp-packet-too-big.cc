#include "ns3/scmp-packet-too-big.h"

namespace ns3
{

 
    uint32_t SCMPPacketTooBig::Deserialize(Buffer::Iterator start) 
    {


	NS_ASSERT_MSG( start.GetRemainingSize() >= GetSerializedSize(), "buffer too short" );
    
    start.Next(2);
    m_mtu = start.ReadNtohU16();

    return 4;

    }

    void SCMPPacketTooBig::Serialize( Buffer::Iterator start ) const
    {
        start.WriteHtonU16(0);
        start.WriteHtonU16(m_mtu);
    }

   

    uint32_t SCMPPacketTooBig::GetSerializedSize() const 
    {
        return 4;
    }

     TypeId SCMPPacketTooBig::GetTypeId()
     {
  static TypeId tid = TypeId("ns3::SCMPPacketTooBig")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                             .AddConstructor<SCMPPacketTooBig>();
    return tid;
     }

    TypeId SCMPPacketTooBig::GetInstanceTypeId() const
    {
        return GetTypeId();
    }

    void SCMPPacketTooBig::Print(std::ostream& os) const
    {
        os << "<SCMPPacketTooBig/>";
    }

    
}