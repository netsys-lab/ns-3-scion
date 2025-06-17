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

    // TODO: We need to obtain local and remote addresses dynamically or configure them
    // TODO: We need to assign interface IDs automatically
    // TODO: The SCION Stack Helper should install and configure BRs and CSes itself
    // link1.remoteIa = as2->GetIa();
    // link1.linkType = ScionLinkType::CHILD;

    // This method will add the inter-AS link information depending on the AS configraution
    // TODO: To keep it understandable, do we want to do this on both ASes ot only only once?

    // TODO: Addresses are filled later
    ScionInterconnect scionLink;
    scionLink.linkId = 1; // Unique ID for this link, can be auto-generated or set manually
    scionLink.linkType = ScionInterconnectType::PARENT_CHILD; // Example link type
    scionLink.mtu = 1500;                                     // Set the MTU for this link
    scionLink.br1 = br1;                                      // Local Border Router
    scionLink.br2 = br2;                                      // Remote Border Router
    scionLink.dateRate = 1000000;                             // Example data rate in Bps (1 Mbps)
    scionLink.delay = 10;                                     // Example delay in ms

    ScionTopologyHelper scionTopoHelper;
    scionTopoHelper.AddScionInterconnect(as1, as2, scionLink);

    // as1->AddScionInterconnet(scionLink);
    // as2->AddScionInterconnet(scionLink);

    // as1->AddScionLink(br1, br2, as2, ScionLinkType::CHILD);
    // as2->AddScionLink(br2, br1, as1, ScionLinkType::PARENT);

    // This should be done by scionStackHelper
    // Connect Border Routers physically via point-to-point link

    // This is not done here, but maybe internally. Or we remove this.
    // Absolutely "dont-care" mode
    // ScionTopologyHelper scionTopoHelper;
    // Setup connection between AS1 and AS2 Border Routers
    // scionTopoHelper.CrossConnect(as1, br1, as2, br2);

    return 0;
}
