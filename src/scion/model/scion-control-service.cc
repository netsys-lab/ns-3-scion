#include "control-service.h"

#include "ns3/log.h"
#include "ns3/node.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScionControlServiceApp");
NS_OBJECT_ENSURE_REGISTERED(ScionControlServiceApp);

TypeId
ScionControlServiceApp::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ScionControlServiceApp")
                            .SetParent<Application>()
                            .SetGroupName("Scion")
                            .AddConstructor<ScionControlServiceApp>();
    return tid;
}

void
ScionControlServiceApp::SetAddress(Address addr)
{
    m_addr = addr;
}

Address
ScionControlServiceApp::GetAddress() const
{
    return m_addr;
}

void
ScionControlServiceApp::StartApplication()
{
    NS_LOG_INFO("ScionControlServiceApp started on node " << GetNode()->GetId());
    // Additional startup logic here (e.g., initialize routing tables)
}

void
ScionControlServiceApp::StopApplication()
{
    NS_LOG_INFO("ScionControlServiceApp stopped on node " << GetNode()->GetId());
    // Cleanup logic here
}

void
ScionControlServiceApp::SetId(std::string id)
{
    m_id = id;
}

std::string
ScionControlServiceApp::GetId() const
{
    return m_id;
}

} // namespace ns3
