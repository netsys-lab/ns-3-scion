#include "ns3/topology-helper.h"

#include "ns3/core-module.h"
#include "ns3/ipv4.h" // For node->GetObject<Ipv4>()
#include "ns3/log.h"
#include "ns3/names.h" // For naming objects

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("TopologyHelper");

TopologyHelper::TopologyHelper()
    : m_ipv4AddressHelperSet(false)
{
    // Set default attributes for P2P links if user doesn't provide their own helper
    m_p2pHelper.SetDeviceAttribute("DataRate",
                                   StringValue("10Gbps")); // Default for inter-AS/core links
    m_p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));

    // Set default attributes for CSMA links
    m_csmaHelper.SetChannelAttribute("DataRate", StringValue("100Mbps"));    // LAN speed
    m_csmaHelper.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560))); // Default CSMA delay
}

void
TopologyHelper::SetP2PHelper(PointToPointHelper p2pHelper)
{
    m_p2pHelper = p2pHelper;
}

void
TopologyHelper::SetCsmaHelper(CsmaHelper csmaHelper)
{
    m_csmaHelper = csmaHelper;
}

void
TopologyHelper::SetIpv4AddressHelper(Ipv4AddressHelper ipv4AddressHelper)
{
    m_ipv4AddressHelper = ipv4AddressHelper;
    m_ipv4AddressHelperSet = true;
}

NetDeviceContainer
TopologyHelper::CreateP2PLink(Ptr<Node> nodeA, Ptr<Node> nodeB)
{
    NS_LOG_FUNCTION(this << "NodeA:" << nodeA->GetId() << "NodeB:" << nodeB->GetId());
    NS_ASSERT_MSG(nodeA && nodeB, "Nodes for P2P link cannot be null.");

    NodeContainer nodes;
    nodes.Add(nodeA);
    nodes.Add(nodeB);
    NetDeviceContainer devices = m_p2pHelper.Install(nodes);
    NS_LOG_INFO("Created P2P link between Node "
                << nodeA->GetId() << " (dev " << devices.Get(0)->GetIfIndex() << ") and Node "
                << nodeB->GetId() << " (dev " << devices.Get(1)->GetIfIndex() << ")");
    return devices;
}

Ipv4InterfaceContainer
TopologyHelper::CreateP2PLinkWithIp(Ptr<Node> nodeA,
                                    Ptr<Node> nodeB,
                                    Ipv4Address ipA,
                                    Ipv4Address ipB,
                                    Ipv4Mask mask)
{
    NS_LOG_FUNCTION(this << "NodeA:" << nodeA->GetId() << "(" << ipA << ")"
                         << "NodeB:" << nodeB->GetId() << "(" << ipB << ")" << " Mask:" << mask);
    NetDeviceContainer devices = CreateP2PLink(nodeA, nodeB);

    Ipv4InterfaceContainer interfaces;
    Ptr<Ipv4> ipv4A = nodeA->GetObject<Ipv4>();
    Ptr<Ipv4> ipv4B = nodeB->GetObject<Ipv4>();

    NS_ASSERT_MSG(ipv4A && ipv4B,
                  "IP stack must be installed on nodes before assigning IP addresses.");

    int32_t ifIndexA = ipv4A->AddInterface(devices.Get(0));
    ipv4A->AddAddress(ifIndexA, Ipv4InterfaceAddress(ipA, mask));
    ipv4A->SetMetric(ifIndexA, 1); // Default metric
    ipv4A->SetUp(ifIndexA);
    interfaces.Add(ipv4A, ifIndexA);
    NS_LOG_INFO("Assigned " << ipA << "/" << mask << " to Node " << nodeA->GetId() << " dev "
                            << devices.Get(0)->GetIfIndex());

    int32_t ifIndexB = ipv4B->AddInterface(devices.Get(1));
    ipv4B->AddAddress(ifIndexB, Ipv4InterfaceAddress(ipB, mask));
    ipv4B->SetMetric(ifIndexB, 1);
    ipv4B->SetUp(ifIndexB);
    interfaces.Add(ipv4B, ifIndexB);
    NS_LOG_INFO("Assigned " << ipB << "/" << mask << " to Node " << nodeB->GetId() << " dev "
                            << devices.Get(1)->GetIfIndex());

    return interfaces;
}

Ipv4InterfaceContainer
TopologyHelper::CreateP2PLinkWithIp(Ptr<Node> nodeA, Ptr<Node> nodeB)
{
    NS_LOG_FUNCTION(this << "NodeA:" << nodeA->GetId() << "NodeB:" << nodeB->GetId());
    if (!m_ipv4AddressHelperSet)
    {
        NS_LOG_ERROR("Ipv4AddressHelper not set. Cannot assign IPs automatically. Call "
                     "SetIpv4AddressHelper first.");
        return Ipv4InterfaceContainer(); // Return empty
    }
    NetDeviceContainer devices = CreateP2PLink(nodeA, nodeB);
    return AssignIp(devices);
}

NetDeviceContainer
TopologyHelper::CreateCsmaLink(NodeContainer nodes)
{
    NetDeviceContainer devices = m_csmaHelper.Install(nodes);
    NS_LOG_INFO("Created CSMA link connecting " << nodes.GetN() << " nodes.");
    for (uint32_t i = 0; i < devices.GetN(); ++i)
    {
        NS_LOG_DEBUG("  Node " << nodes.Get(i)->GetId() << " on dev "
                               << devices.Get(i)->GetIfIndex());
    }
    return devices;
}

Ipv4InterfaceContainer
TopologyHelper::CreateCsmaLinkWithIp(NodeContainer nodes)
{
    NS_LOG_FUNCTION(this << "Nodes:" << nodes.GetN());
    if (!m_ipv4AddressHelperSet)
    {
        NS_LOG_ERROR("Ipv4AddressHelper not set. Cannot assign IPs automatically. Call "
                     "SetIpv4AddressHelper first.");
        return Ipv4InterfaceContainer(); // Return empty
    }
    NetDeviceContainer devices = CreateCsmaLink(nodes);
    return AssignIp(devices);
}

Ipv4InterfaceContainer
TopologyHelper::AssignIp(NetDeviceContainer devices)
{
    NS_LOG_FUNCTION(this << "Devices:" << devices.GetN());
    if (!m_ipv4AddressHelperSet)
    {
        NS_LOG_WARN(
            "Attempting to assign IPs, but Ipv4AddressHelper was not explicitly set by the user. "
            "Default Ipv4AddressHelper will be used (may not have desired base network).");
        // The default m_ipv4AddressHelper has no base set, so Assign will likely fail or use an
        // unexpected default.
    }

    // Ensure IP stack is installed on nodes associated with these devices
    for (uint32_t i = 0; i < devices.GetN(); ++i)
    {
        Ptr<Node> node = devices.Get(i)->GetNode();
        if (!node->GetObject<Ipv4>())
        {
            NS_LOG_ERROR(
                "Node "
                << node->GetId()
                << " does not have an IPv4 stack installed. Cannot assign IP to its device.");
            return Ipv4InterfaceContainer(); // Return empty on error
        }
    }

    Ipv4InterfaceContainer interfaces = m_ipv4AddressHelper.Assign(devices);
    for (uint32_t i = 0; i < interfaces.GetN(); ++i)
    {
        // NS_LOG_INFO("Assigned " << interfaces.GetAddress(i, 0) << "/" << interfaces.GetMask(i, 0)
        //                        << " to Node " << devices.Get(i)->GetNode()->GetId() << " dev "
        //                        << devices.Get(i)->GetIfIndex());
    }
    return interfaces;
}

} // namespace ns3