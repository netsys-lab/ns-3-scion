#pragma once
#include "ns3/header.h"

namespace ns3
{

    /*!
     \brief SCMPPacketTooBig represents the structure of a packet too big message.
    
    	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    	|            reserved           |             MTU               |
    	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    */

    class SCMPPacketTooBig :  public Header
    {
    public:
    
      SCMPPacketTooBig( uint16_t mtu = std::numeric_limits<uint16_t>::max() ) : m_mtu(mtu){}
      auto GetMtu()const{return m_mtu; }

    uint32_t Deserialize( Buffer::Iterator start) override;
    void Serialize( Buffer::Iterator ) const override;
    
    uint32_t GetSerializedSize() const ;
     static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;


  
private:

    
        uint16_t m_mtu;
    };

    

}