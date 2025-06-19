// In your simulation script:

// 1. Create ScionAsImpl objects (e.g., from a topology reader or manually)
// Ptr<ScionAsImpl> as1 = CreateObject<ScionAsImpl>();
// ... configure as1 with ScionAs attributes, nodes, roles, and SCION links ...
// e.g., as1->SetScionAttributes(...);
//       as1->AddNodeWithRole(br1_node, ScionNodeType::BORDER_ROUTER);
//       as1->AddScionLink(br1_node, linkToNeighbor);

// Ptr<ScionAsImpl> as2 = CreateObject<ScionAsImpl>();
// ... configure as2 ...

// 2. Create and configure the ScionStackHelper
// ScionStackHelper scionHelper;
// Optional: Customize helper, e.g., IP address range for AS1 internal links
// scionHelper.SetAttribute("BaseIpNetwork", StringValue("172.16.0.0"));
// scionHelper.SetAttribute("BaseIpMask", StringValue("255.255.0.0"));
// scionHelper.SetAttribute("AllocateIpPerLink", BooleanValue(false));
// scionHelper.SetAttribute("InternalTopologyType", StringValue("STAR")); // or "NONE"

// 3. Install the stack
// scionHelper.Install(as1);
// scionHelper.Install(as2);

// Or for a container:
// ObjectContainer<ScionAsImpl> allMyASes;
// allMyASes.Add(as1);
// allMyASes.Add(as2);
// scionHelper.Install(allMyASes);

#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/scion-as-impl.h"
#include "ns3/scion-stack-helper.h"
#include "ns3/scion-topology-helper.h"

using namespace ns3;

//
// This is a basic example of how to use SCION to connect two ASes
// AS1 <-> AS2 where AS1 is a core AS and AS2 is a non-core AS.
//

int
main(int argc, char* argv[])
{
    ScionTopologyHelper scionTopoHelper;
    // We only use L2 Ethernet for now
    scionTopoHelper.SetAsInternalUnderlayType(ScionUnderlay::L2_ETHERNET);
    scionTopoHelper.SetInterconnectUnderlayType(ScionUnderlay::L2_ETHERNET);

    // Maybe simplify further:
    Ptr<ScionAsImpl> as1 = scionTopoHelper.AddAs(Ia(Isd(1), Asn(1)), true);
    Ptr<Node> as1Br1 = scionTopoHelper.AddBorderRouter(as1);
    scionTopoHelper.AddControlService(as1);

    Ptr<ScionAsImpl> as2 = scionTopoHelper.AddAs(Ia(Isd(1), Asn(2)), false);
    Ptr<Node> as1Br2 = scionTopoHelper.AddBorderRouter(as2);
    scionTopoHelper.AddControlService(as2);

    // Full Mesh Internal Topology
    as1->ConnectAllNodes();
    as2->ConnectAllNodes();

    // TODO: Addresses are filled later
    ScionInterconnect scionLink;
    // scionLink.underlay = ScionInterconnectUnderlay::L2_ETHERNET; Obtained from the helper
    scionLink.linkType = ScionInterconnectType::PARENT_CHILD; // Example link type
    scionLink.mtu = 1500;                                     // Set the MTU for this link
    scionLink.br1 = as1Br1;                                   // Local Border Router
    scionLink.br2 = as1Br2;                                   // Remote Border Router
    scionLink.dateRate = 1000000;                             // Example data rate in Bps (1 Mbps)
    scionLink.delay = 10;                                     // Example delay in ms

    // 2. Adding the interconnect between AS1 and AS2
    scionTopoHelper.AddScionInterconnect(as1, as2, scionLink);

    // Installation
    std::vector<Ptr<ScionAsImpl>> allAs = scionTopoHelper.GetAllAs();

    // 1. Installing the topology
    scionTopoHelper.InstallAllInternalTopology(allAs);

    // 2. Installing interconnects
    scionTopoHelper.InstallAllInterconnects(allAs);

    // 3. Installing SCION stack on all nodes
    ScionStackHelper scionStackHelper;
    // scionStackHelper.SetUnderlayType(ScionUnderlay::L2_ETHERNET); // TODO: Maybe this could be
    // inferred? Maybe we also don't need this at all, since the underlay is already set in
    scionStackHelper.InstallAll(allAs);

    return 0;
}

/*
Verbose code samples:
Ptr<ScionAsImpl> as1 = CreateObject<ScionAsImpl>();
    Ptr<ScionAsImpl> as2 = CreateObject<ScionAsImpl>();

    // Configure AS1
    Ptr<ScionAs> as1Config = CreateObject<ScionAs>();
    as1Config->SetIa(Ia(Isd(1), Asn(1)));
    as1Config->SetIsCore(true);
    as1Config->SetBeaconPolicy(0); // Example policy

    // Configure AS2
    Ptr<ScionAs> as2Config = CreateObject<ScionAs>();
    as1Config->SetIa(Ia(Isd(1), Asn(2)));

    // Apply configurations to ASes
    as1->SetScionAttributes(as1Config);
    as2->SetScionAttributes(as2Config);

    // Create Border Router nodes for AS1 and AS2
    Ptr<Node> br1 = CreateObject<Node>();
    Ptr<Node> br2 = CreateObject<Node>();

    // Add Border Routers to ASes with specific roles
    as1->AddNodeWithRole(br1, ScionNodeType::BORDER_ROUTER);
    as2->AddNodeWithRole(br2, ScionNodeType::BORDER_ROUTER);

    // Create Control Service nodes
    Ptr<Node> cs1 = CreateObject<Node>();
    Ptr<Node> cs2 = CreateObject<Node>();

    // Add Control Services to ASes
    as1->AddNodeWithRole(cs1, ScionNodeType::CONTROL_SERVICE);
    as2->AddNodeWithRole(cs2, ScionNodeType::CONTROL_SERVICE);

    // Create internal links between Border Routers and Control Services if there is any specific
    // mapping If not, we can assume a fully connected internal topology
    as1->AddInternalLink(br1, cs1);
    as2->AddInternalLink(br2, cs2);

    // We could also do:
    as1->ConnectAllNodes();
    as2->ConnectAllNodes();

*/