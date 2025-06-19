#include "scion-stack-helper.h"

#include "ns3/ipv4-global-routing-helper.h" // For populating routing tables globally
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/point-to-point-module.h"
#include "ns3/string.h" // For StringValue
#include "ns3/topology-helper.h"

namespace ns3
{

// NS_LOG_COMPONENT_DEFINE("ScionStackHelper");

ScionStackHelper::ScionStackHelper()
{
}

void
ScionStackHelper::SetUnderlayType(ScionUnderlay type)
{
    // NS_LOG_FUNCTION(this << type);
    m_underlayType = type;
}

void
ScionStackHelper::InstallAll(std::vector<Ptr<ScionAsImpl>> as)
{
    // NS_LOG_FUNCTION(this << as.size());
    for (auto& asImpl : as)
    {
        Install(asImpl);
    }
}

void
ScionStackHelper::Install(Ptr<ScionAsImpl> as)
{
    // NS_LOG_FUNCTION(this << as);
    NS_ASSERT(as != nullptr);

    // 1. Create a SCION AS Context for this AS
    Ptr<ScionAsContext> asContext = CreateScionAsContext(as);
    NS_ASSERT(asContext != nullptr);

    // 2. Install the SCION stack on all nodes in this AS
    NodeContainer asNodes = as->GetAllNodes();
    NodeContainer::Iterator i;
    for (i = asNodes.Begin(); i != asNodes.End(); ++i)
    {
        // Install the SCION stack on each node
        (*i)->AggregateObject(asContext);
    }
}

Ptr<ScionAsContext>
ScionStackHelper::CreateScionAsContext(Ptr<ScionAsImpl> asImpl)
{
    // 1. Create a topology like structure that can be fetched
    Ptr<ScionAsContext> asContext = CreateObject<ScionAsContext>();
    ScionAsTopology topology;
    Ptr<ScionAs> as = asImpl->GetScionAttributes();
    topology.isdAs = as->GetIa();
    topology.isCore = as->IsCore();
    topology.mtu = as->GetMtu();

    NodeContainer asNodes = asImpl->GetAllNodes();
    NodeContainer::Iterator i;

    // Create topology map for routers
    topology.borderRouters.clear();

    for (i = asNodes.Begin(); i != asNodes.End(); ++i)
    {
        uint32_t id = (*i)->GetId();
        if (asImpl->GetNodeRole(id) == ScionNodeType::BORDER_ROUTER)
        {
            BorderRouterConfig brConfig;
            // store br- + node name as ID
            brConfig.id = "br-" + id;
            brConfig.interfaces.clear();
            if (m_underlayType == ScionUnderlay::L2_ETHERNET)
            {
                // First device is always the BR internal device
                Ptr<NetDevice> device = (*i)->GetDevice(0);
                ScionServiceAddress addr;
                addr.address = device->GetAddress();
                brConfig.internalAddress = addr; // No port here
            }
            else
            {
                // TODO: Handle L3 underlay types
                NS_LOG_WARN("ScionStackHelper: Underlay type L3"
                            << " not supported for BR internal address. Skipping router");
                continue; // Skip unsupported underlay types for BRs
            }

            std::vector<ScionLink> links = asImpl->GetScionLinks(id);
            if (links.empty())
            {
                NS_LOG_WARN("ScionStackHelper: Border Router " << id
                                                               << " has no SCION links defined.");
                continue; // Skip BRs with no links
            }

            // Border Routers have the first device always being the
            // Iterate over all Links and create interfaces
            for (const auto& link : links)
            {
                BorderRouterInterface brInterface;
                brInterface.mtu = link.mtu;
                brInterface.remoteIsdAs = link.remoteIa;
                brInterface.linkToType = link.linkType;

                Ptr<NetDevice> localBrNetDevice = (*i)->GetDevice(link.localInterfaceId);
                Ptr<NetDevice> remoteBrNetDevice =
                    link.remoteBorderRouter->GetDevice(link.remoteInterfaceId);

                brConfig.id = "br-" + std::to_string(id);
                brConfig.interfaces[link.localInterfaceId] = brInterface;

                // Set the underlay type and addresses
                if (m_underlayType == ScionUnderlay::L2_ETHERNET)
                {
                    // For L2 Ethernet, we use the MAC addresses directly
                    brInterface.underlay.publicAddress.address = localBrNetDevice->GetAddress();
                    brInterface.underlay.remoteAddress.address = remoteBrNetDevice->GetAddress();
                }
                else
                {
                    // TODO: Handle L3 underlay types
                    NS_LOG_WARN("ScionStackHelper: Underlay type L3"
                                << " not supported for BR link address. Skipping link");
                    continue;

                    // Skip unsupported underlay types for BRs
                    // For L3, we need to set the IP addresses Maybe something like this?!
                    // brInterface.underlay.publicAddress.address =
                    //    localBrNetDevice->GetObject<Ipv4>()->GetAddress(1,
                    //    0).GetLocal();
                    // brInterface.underlay.remoteAddress.address =
                    //    remoteBrNetDevice->GetObject<Ipv4>()->GetAddress(1,
                    //    0).GetLocal();
                }
            }
        }
    }

    asContext->SetTopology(topology);
    return asContext;
}

} // namespace ns3

/*
void
ScionStackHelper::SetP2PHelper(PointToPointHelper p2phelper)
{
    m_p2pHelper = p2phelper;
}


void
ScionStackHelper::SetBaseIp(std::string network, std::string mask)
{
    m_baseIpNetwork = network;
    m_baseIpMask = mask;
}


void
ScionStackHelper::SetIpAllocationPolicy(bool allocatePerLink)
{
    m_allocateIpPerLink = allocatePerLink;
}

void
ScionStackHelper::SetInternalTopologyType(std::string type)
{
    // NS_LOG_FUNCTION(this << type);
    if (type == "NONE")
    {
        m_internalTopologyType = InternalTopologyType::NONE;
    }
    else if (type == "FULL_MESH")
    {
        m_internalTopologyType = InternalTopologyType::FULL_MESH;
    }
    else if (type == "STAR")
    {
        m_internalTopologyType = InternalTopologyType::STAR;
    }
    else
    {
        NS_LOG_WARN("ScionStackHelper: Unknown InternalTopologyType '" << type
                                                                       << "'. No change made.");
    }
}

void
ScionStackHelper::SetupMinimalInternalTopology(Ptr<ScionAsImpl> as, NodeContainer coreNodes)
{
    // NS_LOG_FUNCTION(this << as->GetIa() << " Core nodes: " << coreNodes.GetN());
    if (coreNodes.GetN() < 2)
    {
        NS_LOG_INFO("Not enough core nodes ("
                    << coreNodes.GetN() << ") to build internal topology for AS " << as->GetIa());
        return;
    }

    NetDeviceContainer internalDevs;

    if (m_internalTopologyType == InternalTopologyType::FULL_MESH)
    {
        NS_LOG_INFO("Building FULL_MESH internal topology for AS " << as->GetIa());
        for (uint32_t i = 0; i < coreNodes.GetN(); ++i)
        {
            for (uint32_t j = i + 1; j < coreNodes.GetN(); ++j)
            {
                NodeContainer pair;
                pair.Add(coreNodes.Get(i));
                pair.Add(coreNodes.Get(j));
                NetDeviceContainer devices = m_p2pHelper.Install(pair);
                internalDevs.Add(devices);
                // Update ScionAsImpl's internal graph
                as->AddInternalLink(coreNodes.Get(i),
                                    coreNodes.Get(j),
                                    devices.Get(0),
                                    devices.Get(1));
                NS_LOG_DEBUG("AS " << as->GetIa() << ": Connected " << coreNodes.Get(i)->GetId()
                                   << " with " << coreNodes.Get(j)->GetId());
            }
        }
    }
    else if (m_internalTopologyType == InternalTopologyType::STAR)
    {
        NS_LOG_INFO("Building STAR internal topology for AS " << as->GetIa());
        // Requires identifying a central node. For simplicity, pick the first one.
        // A better approach would be to allow user to specify the star center.
        Ptr<Node> centerNode = coreNodes.Get(0);
        NS_LOG_INFO("Using node " << centerNode->GetId() << " as STAR center for AS "
                                  << as->GetIa());
        for (uint32_t i = 1; i < coreNodes.GetN(); ++i) // Connect all others to center
        {
            NodeContainer pair;
            pair.Add(centerNode);
            pair.Add(coreNodes.Get(i));
            NetDeviceContainer devices = m_p2pHelper.Install(pair);
            internalDevs.Add(devices);
            as->AddInternalLink(centerNode, coreNodes.Get(i), devices.Get(0), devices.Get(1));
            NS_LOG_DEBUG("AS " << as->GetIa() << ": Connected " << centerNode->GetId() << " with "
                               << coreNodes.Get(i)->GetId());
        }
    }
    // Assign IP addresses to these newly created internal links
    if (internalDevs.GetN() > 0)
    {
        AssignIpAddresses(as, internalDevs);
    }
}

void
ScionStackHelper::InstallIpStack(Ptr<ScionAsImpl> as, NodeContainer nodes)
{
    // NS_LOG_FUNCTION(this << as->GetIa() << " Nodes: " << nodes.GetN());
    if (nodes.GetN() == 0)
        return;

    NS_LOG_INFO("Installing IP stack on " << nodes.GetN() << " nodes in AS " << as->GetIa());
    m_stackInstaller.Install(nodes);

    // IP Addressing:
    // This is a simplified addressing. A more robust scheme would be needed for large ASes.
    // Collect all NetDevices on internal links that need addressing.
    // For now, we assume devices created by SetupMinimalInternalTopology are the primary ones.
    // If user creates topology, they might also need to call a more granular IP assignment.

    // This current simplified approach might re-address devices if InstallIpStack is called
    // after some addresses are already set by SetupMinimalInternalTopology.
    // A better way: AssignIpAddresses should be called ONCE for all relevant devices.
    // For now, we rely on the fact that AssignIpAddresses can be idempotent if IPs are not
    // changing.

    // If not allocating per link, set up a larger subnet for the whole AS
    if (!m_allocateIpPerLink)
    {
        std::stringstream ss;
        // Construct a unique subnet for this AS based on a base and its IA or a counter
        // This is a very basic example of subnet allocation.
        uint16_t isdVal = as->GetIa().GetIsd().GetValue();
        //  uint64_t asVal =
        //     as->GetIa().GetAsn().GetValue() % 250; // Keep AS part small for this example
        uint32_t secondOctet = (isdVal % 250) + 1; // Avoid 0
        uint32_t thirdOctet = m_nextAsSubnet++;
        if (m_nextAsSubnet > 254)
            m_nextAsSubnet = 1; // Cycle, very basic

        ss << "10." << secondOctet << "." << thirdOctet << ".0";
        m_ipv4AddressHelper.SetBase(Ipv4Address(ss.str().c_str()),
                                    "255.255.255.0"); // Assign /24 for the AS
        NS_LOG_INFO("AS " << as->GetIa() << " using base subnet: " << ss.str() << "/24");
    }

    // Iterate through all nodes and their devices to assign IPs if they are P2P and unaddressed
    // This is a bit broad; ideally, we'd only address devices intended for intra-AS comms.
    for (uint32_t i = 0; i < nodes.GetN(); ++i)
    {
        Ptr<Node> node = nodes.Get(i);
        for (uint32_t j = 0; j < node->GetNDevices(); ++j)
        {
            Ptr<NetDevice> dev = node->GetDevice(j);
            // Check if device is PointToPoint and doesn't have an IPv4 address yet
            Ptr<PointToPointNetDevice> p2pDev = DynamicCast<PointToPointNetDevice>(dev);
            Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
            if (p2pDev && ipv4)
            {
                int32_t ifIndex = ipv4->GetInterfaceForDevice(dev);
                if (ifIndex >= 0 && ipv4->GetNAddresses(ifIndex) == 0)
                {
                    NetDeviceContainer singlePairDevs;
                    // Find the peer device to assign addresses as a pair
                    Ptr<Channel> channel = dev->GetChannel();
                    for (uint32_t k = 0; k < channel->GetNDevices(); ++k)
                    {
                        if (channel->GetDevice(k) != dev)
                        { // Found the peer
                            singlePairDevs.Add(dev);
                            singlePairDevs.Add(channel->GetDevice(k));
                            break;
                        }
                    }
                    if (singlePairDevs.GetN() == 2)
                    {
                        NS_LOG_DEBUG("Assigning IP to unaddressed P2P link for node "
                                     << node->GetId() << " dev " << dev->GetIfIndex());
                        AssignIpAddresses(as, singlePairDevs);
                    }
                }
            }
        }
    }

    // Set up routing
    // For simple topologies, global routing can work well initially.
    // For SCION, intra-AS routing primarily needs to ensure BRs can reach each other
    // and CSs, and hosts can reach their default BR/gateway.
    NS_LOG_INFO("Populating IPv4 routing tables for AS " << as->GetIa());
    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    // Alternatively, or in addition, configure static routes if needed,
    // e.g., default routes for hosts towards an internal router or BR.
}

Ipv4InterfaceContainer
ScionStackHelper::AssignIpAddresses(Ptr<ScionAsImpl> as, NetDeviceContainer devices)
{
    // NS_LOG_FUNCTION(this << as->GetIa() << " Devices: " << devices.GetN());
    if (devices.GetN() == 0)
        return Ipv4InterfaceContainer();

    if (m_allocateIpPerLink)
    {
        // This will assign a new /30 for each call if base is not changed
        // We need a strategy to advance the base for each link if using m_allocateIpPerLink
        // globally. For now, assumes m_ipv4AddressHelper is reset or managed outside for sequential
        // /30s. A simple advancement for unique /30s:
        static uint32_t linkSubnetCounter = 0;          // Very basic, reset per simulation run
        uint32_t thirdOctet = (linkSubnetCounter / 64); // ~64 /30s per /24
        uint32_t fourthOctetBase = (linkSubnetCounter % 64) * 4;
        linkSubnetCounter++;
        std::stringstream ss;
        ss << "192.168." << thirdOctet << "." << fourthOctetBase; // Example private range for links
        m_ipv4AddressHelper.SetBase(Ipv4Address(ss.str().c_str()), "255.255.255.252"); // /30
        NS_LOG_INFO("AS " << as->GetIa() << " assigning IPs from base " << ss.str()
                          << "/30 for a link pair.");
    }
    // If !m_allocateIpPerLink, m_ipv4AddressHelper was set up in InstallIpStack for the AS.

    Ipv4InterfaceContainer interfaces = m_ipv4AddressHelper.Assign(devices);
    for (uint32_t i = 0; i < interfaces.GetN(); ++i)
    {
        NS_LOG_INFO("AS " << as->GetIa() << ": Assigned " << interfaces.GetAddress(i, 0)
                          << " to device on node " << devices.Get(i)->GetNode()->GetId());
    }
    return interfaces;
}

void
ScionStackHelper::Install(Ptr<ScionAsImpl> as)
{
    // NS_LOG_FUNCTION(this << as->GetIa());
    NS_ASSERT_MSG(as, "Cannot install on a null ScionAsImpl Ptr.");

    // 1. Provision/Verify Border Routers based on as->GetAllScionLinks()
    //    This step is mostly about ensuring nodes designated in ScionLinks are
    //    actually present in the AS and marked as BORDER_ROUTER.
    //    Creation of BR nodes if missing based on links is a more advanced feature.
    ProvisionBorderRouters(as);

    // 2. Setup minimal internal L2 topology if needed
    NodeContainer coreServiceNodes = as->GetBorderRouters();
    coreServiceNodes.Add(as->GetControlServices());
    // Check if internal topology already exists for these core nodes.
    // A simple check: if a BR has no internal neighbors, assume minimal topo is needed.
    bool needsMinimalTopo = false;
    if (m_internalTopologyType != InternalTopologyType::NONE && !coreServiceNodes.GetN() == 0)
    {
        if (as->GetInternalNeighbors(coreServiceNodes.Get(0)).GetN() == 0)
        {
            // And check if a BR has any internal connections already made by the user
            bool brHasInternalLinks = false;
            NodeContainer brs = as->GetBorderRouters();
            for (uint32_t i = 0; i < brs.GetN(); ++i)
            {
                if (!as->GetInternalNeighbors(brs.Get(i)).GetN() == 0)
                {
                    brHasInternalLinks = true;
                    break;
                }
            }
            if (!brHasInternalLinks && coreServiceNodes.GetN() > 1)
            { // Only need topo if >1 core node
                needsMinimalTopo = true;
            }
        }
    }

    if (needsMinimalTopo)
    {
        NS_LOG_INFO("AS " << as->GetIa()
                          << ": Setting up minimal internal topology for core services.");
        // SetupMinimalInternalTopology(as, coreServiceNodes);
    }
    else
    {
        NS_LOG_INFO(
            "AS " << as->GetIa()
                  << ": Skipping minimal internal topology setup (already present or NONE type).");
    }

    // 3. Install IP stack on ALL nodes within the AS
    NodeContainer allAsNodes = as->GetAllNodes();
    if (!allAsNodes.GetN() == 0)
    {
        InstallIpStack(as, allAsNodes); // Installs stack, assigns IPs, sets routing
    }
    else
    {
        NS_LOG_WARN("AS " << as->GetIa() << " has no nodes. Skipping IP stack installation.");
    }

    // Further SCION-specific application installations would go here or in another helper
}

void
ScionStackHelper::ProvisionBorderRouters(Ptr<ScionAsImpl> as)
{
    // NS_LOG_FUNCTION(this << as->GetIa());
    std::vector<ScionLink> links = as->GetAllScionLinks();
    std::set<uint32_t> referencedBrIds;

    // This logic assumes ScionAsImpl::AddScionLink already ensures the BR node
    // is valid and has the BORDER_ROUTER role. Here we just log.
    // A more advanced version could create Ptr<Node> if topology only defines links.
    NodeContainer brs = as->GetBorderRouters();
    for (uint32_t i = 0; i < brs.GetN(); ++i)
    {
        referencedBrIds.insert(brs.Get(i)->GetId());
    }

    if (links.empty() && brs.GetN() == 0)
    {
        NS_LOG_INFO("AS " << as->GetIa() << " has no SCION links and no designated BRs.");
    }
    else if (links.empty() && !brs.GetN() == 0)
    {
        NS_LOG_INFO("AS " << as->GetIa() << " has designated BRs but no SCION links defined yet.");
    }
    else if (!links.empty() && brs.GetN() == 0)
    {
        NS_LOG_WARN(
            "AS " << as->GetIa()
                  << " has SCION links defined but NO Border Routers designated in ScionAsImpl.");
        // TODO: Policy to create BR nodes here if desired.
    }
    else
    {
        NS_LOG_INFO("AS " << as->GetIa() << " has " << links.size() << " SCION link(s) and "
                          << brs.GetN() << " border router(s).");
    }
}


    */