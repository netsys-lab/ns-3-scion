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

/**
 *
 */
void
ScionTopologyHelper::AddScionInterconnect(Ptr<ScionAsImpl> as1,
                                          Ptr<ScionAsImpl> as2,
                                          ScionInterconnect link)
{
    ScionLink linkInfo1;
    linkInfo1.linkId = link.linkId; // Unique ID for this link

    ScionLink linkInfo2;
    linkInfo2.linkId = link.linkId; // Unique ID for this link

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

    linkInfo1.mtu = link.mtu;
    linkInfo1.dateRate = link.dateRate; // Data Rate in Bytes / second (Bps)
    linkInfo1.delay = link.delay;       // Delay in MS
    linkInfo1.localBorderRouter = link.br1;
    linkInfo1.remoteBorderRouter = link.br2;
    linkInfo1.localInterfaceId = 0;    // TODO: Get Interface ID counter
    linkInfo1.remoteIa = as2->GetIa(); // Assuming remoteAs is valid and has IA set
    as1->AddScionLink(linkInfo1);

    linkInfo2.mtu = link.mtu;
    linkInfo2.dateRate = link.dateRate; // Data Rate in Bytes / second (Bps)
    linkInfo2.delay = link.delay;       // Delay in MS
    linkInfo2.localBorderRouter = link.br2;
    linkInfo2.remoteBorderRouter = link.br1;
    linkInfo2.localInterfaceId = 0;    // TODO: Get Interface ID counter
    linkInfo2.remoteIa = as1->GetIa(); // Assuming remoteAs is valid and has IA set
    as2->AddScionLink(linkInfo2);
}

} // namespace ns3