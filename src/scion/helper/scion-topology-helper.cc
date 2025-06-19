#include "ns3/scion-topology-helper.h"

#include "ns3/core-module.h"
#include "ns3/ipv4.h" // For node->GetObject<Ipv4>()
#include "ns3/log.h"
#include "ns3/names.h" // For naming objects

namespace ns3
{

// NS_LOG_COMPONENT_DEFINE("SCIONTopologyHelper");

ScionTopologyHelper::ScionTopologyHelper()
    : m_topoHelper() // Initialize member variable(s) here
{
}

void
ScionTopologyHelper::SetTopologyHelper(TopologyHelper topoHelper)
{
    m_topoHelper = topoHelper;
}

void
ScionTopologyHelper::InstallInternalTopology(Ptr<ScionAsImpl> asImpl)
{
    NodeContainer asNodes = asImpl->GetAllNodes();
    CsmaHelper csma;
    // The simple case: we can use a full mesh for internal links
    if (asImpl->IsFullMesh())
    {
        csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(asImpl->GetDefaultDataRate())));
        csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(asImpl->GetDefaultDelay())));
        csma.SetDeviceAttribute("Mtu", UintegerValue(asImpl->GetFullMeshMtu()));

        // Create a full mesh of internal links
        if (asNodes.GetN() > 0)
        {
            csma.Install(asNodes);
        }
        else
        {
            NS_LOG_WARN("ScionTopologyHelper: No internal nodes found for full mesh topology.");
        }
    }
    else
    {
        // TODO: We need to go over the graph and create the internal links
        NS_LOG_WARN("ScionTopologyHelper: Non-full mesh topology not implemented yet.");
    }
}

void
ScionTopologyHelper::InstallInterConnects(Ptr<ScionAsImpl> asImpl)
{
}

void
ScionTopologyHelper::InstallAllInternalTopology(std::vector<Ptr<ScionAsImpl>> asImpls)
{
    for (auto& asImpl : asImpls)
    {
        // Install internal topology for each AS
        InstallInternalTopology(asImpl);
    }
}

void
ScionTopologyHelper::InstallAllInterconnects(std::vector<Ptr<ScionAsImpl>> asImpls)
{
    for (auto& asImpl : asImpls)
    {
        // Install SCION Link Underlays for each AS
        InstallInterConnects(asImpl);
    }
}

void
ScionTopologyHelper::SetAsInternalUnderlayType(ScionUnderlay type)
{
    // NS_LOG_FUNCTION(this << type);
    m_internalUnderlayType = type;
}

void
ScionTopologyHelper::SetInterconnectUnderlayType(ScionUnderlay type)
{
    // NS_LOG_FUNCTION(this << type);
    m_interconnectUnderlayType = type;
}

Ptr<ScionAsImpl>
ScionTopologyHelper::AddAs(Ia ia, bool isCore = false)
{
    Ptr<ScionAsImpl> as = CreateObject<ScionAsImpl>();
    Ptr<ScionAs> asConfig = CreateObject<ScionAs>();
    asConfig->SetIa(ia);
    asConfig->SetIsCore(isCore);

    as->SetScionAttributes(asConfig);
    m_allAs.push_back(as);

    return as;
}

Ptr<Node>
ScionTopologyHelper::AddBorderRouter(Ptr<ScionAsImpl> as)
{
    // Create Border Router nodes for AS1 and AS2
    Ptr<Node> br = CreateObject<Node>();

    // Add Border Routers to ASes with specific roles
    as->AddNodeWithRole(br, ScionNodeType::BORDER_ROUTER);
    return br;
}

Ptr<Node>
ScionTopologyHelper::AddControlService(Ptr<ScionAsImpl> as)
{
    Ptr<Node> cs = CreateObject<Node>();

    // Add Control Services to ASes
    as->AddNodeWithRole(cs, ScionNodeType::CONTROL_SERVICE);
    return cs;
}

Ptr<Node>
ScionTopologyHelper::AddEndhost(Ptr<ScionAsImpl> as)
{
    Ptr<Node> host = CreateObject<Node>();
    // Add Host to ASes
    as->AddNodeWithRole(host, ScionNodeType::HOST);
    return host;
}

/**
 *
 */
void
ScionTopologyHelper::AddScionInterconnect(Ptr<ScionAsImpl> as1,
                                          Ptr<ScionAsImpl> as2,
                                          ScionInterconnect link)
{
    uint64_t linkId = m_nextLinkId++;
    ScionLink linkInfo1;
    linkInfo1.linkId = linkId;

    ScionLink linkInfo2;
    linkInfo2.linkId = linkId;

    Ptr<ScionAs> asInfo1 = as1->GetScionAttributes();
    Ptr<ScionAs> asInfo2 = as2->GetScionAttributes();

    switch (link.linkType)
    {
    case ScionInterconnectType::CORE:
        if (asInfo1->IsCore() && asInfo2->IsCore())
        {
            linkInfo1.linkType = ScionLinkType::CORE;
            linkInfo2.linkType = ScionLinkType::CORE;
        }
        else
        {
            NS_LOG_ERROR("Both ASes must be core ASes for CORE interconnect type.");
            return; // Invalid type, do not add
        }
        break;
    case ScionInterconnectType::PARENT_CHILD:
        if (asInfo1->IsCore() && !asInfo2->IsCore())
        {
            linkInfo1.linkType = ScionLinkType::PARENT;
            linkInfo2.linkType = ScionLinkType::CHILD;
        }
        else if (!asInfo1->IsCore() && asInfo2->IsCore())
        {
            linkInfo1.linkType = ScionLinkType::CHILD;
            linkInfo2.linkType = ScionLinkType::PARENT;
        }
        else
        {
            NS_LOG_ERROR(
                "One of ASes must be core ASes and the other must not for Parent_Child link.");
            return;
        }

        break;
    case ScionInterconnectType::PEER:
        if (asInfo1->IsCore() || asInfo2->IsCore())
        {
            NS_LOG_ERROR("No of ASes must be core ASes for Peering link.");
            return;
        }
        linkInfo1.linkType = ScionLinkType::PEER;
        linkInfo2.linkType = ScionLinkType::PEER;
        break;
    default:
        NS_LOG_ERROR("Unknown SCION interconnect type.");
        return; // Invalid type, do not add
    }

    std::vector<ScionLink> existingLinks1 = as1->GetScionLinks(link.br1);
    std::vector<ScionLink> existingLinks2 = as2->GetScionLinks(link.br2);

    linkInfo1.underlay = link.underlay; // Set the underlay type
    linkInfo1.mtu = link.mtu;
    linkInfo1.dateRate = link.dateRate; // Data Rate in Bytes / second (Bps)
    linkInfo1.delay = link.delay;       // Delay in MS
    linkInfo1.localBorderRouter = link.br1;
    linkInfo1.remoteBorderRouter = link.br2;
    linkInfo1.localInterfaceId = existingLinks1.size();  // TODO: Get Interface ID counter
    linkInfo1.remoteInterfaceId = existingLinks2.size(); // TODO: Get Interface ID counter
    linkInfo1.remoteIa = as2->GetIa(); // Assuming remoteAs is valid and has IA set
    as1->AddScionLink(linkInfo1);

    linkInfo2.underlay = link.underlay; // Set the underlay type
    linkInfo2.mtu = link.mtu;
    linkInfo2.dateRate = link.dateRate; // Data Rate in Bytes / second (Bps)
    linkInfo2.delay = link.delay;       // Delay in MS
    linkInfo2.localBorderRouter = link.br2;
    linkInfo2.remoteBorderRouter = link.br1;
    linkInfo1.localInterfaceId = existingLinks2.size();  // TODO: Get Interface ID counter
    linkInfo1.remoteInterfaceId = existingLinks1.size(); // TODO: Get Interface ID counter
    linkInfo2.localInterfaceId = 0;                      // TODO: Get Interface ID counter
    linkInfo2.remoteIa = as1->GetIa(); // Assuming remoteAs is valid and has IA set
    as2->AddScionLink(linkInfo2);
}

} // namespace ns3