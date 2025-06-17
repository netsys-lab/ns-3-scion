#include "border-router.h"

#include "ns3/log.h"
#include "ns3/node.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("ScionBorderRouterApp");
NS_OBJECT_ENSURE_REGISTERED(ScionBorderRouterApp);

TypeId
ScionBorderRouterApp::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ScionBorderRouterApp")
                            .SetParent<Application>()
                            .SetGroupName("Scion")
                            .AddConstructor<ScionBorderRouterApp>();
    return tid;
}

void
ScionBorderRouterApp::SetInternalAddress(Address internal)
{
    m_internalAddr = internal;
}

Address
ScionBorderRouterApp::GetInternalAddress() const
{
    return m_internalAddr;
}

void
ScionBorderRouterApp::AddLink(const ScionLink& link)
{
    m_links.push_back(link);
}

std::vector<ScionLink>
ScionBorderRouterApp::GetLinks() const
{
    return m_links;
}

void
ScionBorderRouterApp::StartApplication()
{
    NS_LOG_INFO("ScionBorderRouterApp started on node " << GetNode()->GetId());
    // Additional startup logic here (e.g., initialize routing tables)
}

void
ScionBorderRouterApp::StopApplication()
{
    NS_LOG_INFO("ScionBorderRouterApp stopped on node " << GetNode()->GetId());
    // Cleanup logic here
}

void
ScionBorderRouterApp::SetId(std::string id)
{
    m_id = id;
}

std::string
ScionBorderRouterApp::GetId() const
{
    return m_id;
}

} // namespace ns3
