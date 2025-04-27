
#include "ns3/as.h"

#include "ns3/log.h"

namespace ns3
{

TypeId
BaseAs::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::BaseAs")
                            .SetParent<Object>()
                            .SetGroupName("Scion")
                            .AddConstructor<BaseAs>()
                            .AddAttribute("Asn",
                                          "Autonomous System Number",
                                          IntegerValue(0),
                                          MakeIntegerAccessor(&BaseAs::m_asn),
                                          MakeIntegerChecker<uint32_t>())
                            .AddAttribute("Mtu",
                                          "Maximum Transmission Unit",
                                          IntegerValue(0),
                                          MakeIntegerAccessor(&BaseAs::m_mtu),
                                          MakeIntegerChecker<uint32_t>());
    return tid;
}

TypeId
BaseAs::GetInstanceTypeId(void) const
{
    return GetTypeId();
}

BaseAs::BaseAs()
    : m_asn(0),
      m_mtu(0)
{
    NS_LOG_FUNCTION(this);
}

BaseAs::~BaseAs()
{
    NS_LOG_FUNCTION(this);
}

void
BaseAs::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Object::DoDispose();
}

void
BaseAs::SetAsn(uint32_t asn)
{
    NS_LOG_FUNCTION(this << asn);
    m_asn = asn;
}

void
BaseAs::SetMtu(uint32_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
}

uint32_t
BaseAs::GetAsn() const
{
    NS_LOG_FUNCTION(this);
    return m_asn;
}

uint32_t
BaseAs::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

} // namespace ns3