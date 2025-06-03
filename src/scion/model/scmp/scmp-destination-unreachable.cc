#include "ns3/scmp-destination-unreachable.h"

namespace ns3
{

    uint32_t SCMPDestinationUnreachable::Deserialize(Buffer::Iterator start) 
    {       
	    NS_ASSERT_MSG( start.GetRemainingSize()>=4, "buffer too short");     
      start.ReadU32();
    
      return 4;
    }

    void SCMPDestinationUnreachable::Serialize( Buffer::Iterator start ) const 
    {   
        start.WriteU32(0);
    }

    uint32_t SCMPDestinationUnreachable::GetSerializedSize() const 
    {
        return 4;
    }

      TypeId SCMPDestinationUnreachable::GetTypeId()
     {
        static TypeId tid = TypeId("ns3::SCMPDestinationUnreachable")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCMPDestinationUnreachable>();
    return tid;
     }

    TypeId SCMPDestinationUnreachable::GetInstanceTypeId() const
    {
        return GetTypeId();
    }

    void SCMPDestinationUnreachable::Print(std::ostream& os) const 
    {
        os << "<SCMPDestinationUnreachable/>";
    }

}