#include "scion-as-impl.h"

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/node-list.h" // For Ptr<Node> from ID

namespace ns3
{

// NS_LOG_COMPONENT_DEFINE("ScionAsImpl");
NS_OBJECT_ENSURE_REGISTERED(ScionAsImpl);

TypeId
ScionAsImpl::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::ScionAsImpl")
                            .SetParent<Object>()
                            .SetGroupName("Scion")
                            .AddConstructor<ScionAsImpl>();
    // Attributes can be added if needed, e.g., for default IA if not set via SetScionAttributes
    return tid;
}

ScionAsImpl::ScionAsImpl()
{
    // NS_LOG_FUNCTION(this);
}

ScionAsImpl::~ScionAsImpl()
{
    // NS_LOG_FUNCTION(this);
}

void
ScionAsImpl::DoDispose(void)
{
    // NS_LOG_FUNCTION(this);
    m_attributes = nullptr;
    // m_allNodes.Dispose(); // TODO: release Ptr<Node> references
    m_nodeRoles.clear();
    m_internalAdjacencyList.clear();
    m_borderRouterScionLinks.clear();
    Object::DoDispose();
}

void
ScionAsImpl::SetScionAttributes(Ptr<ScionAs> attributes)
{
    // NS_LOG_FUNCTION(this << attributes);
    m_attributes = attributes;
}

Ptr<ScionAs>
ScionAsImpl::GetScionAttributes() const
{
    return m_attributes;
}

Ia
ScionAsImpl::GetIa() const
{
    if (m_attributes)
    {
        return m_attributes->GetIa();
    }
    NS_LOG_WARN("ScionAttributes not set, returning zero IA.");
    return Ia(); // Return default (0-0) IA
}

bool
ScionAsImpl::IsManagedNode(uint32_t nodeId) const
{
    for (uint32_t i = 0; i < m_allNodes.GetN(); ++i)
    {
        if (m_allNodes.Get(i)->GetId() == nodeId)
        {
            return true;
        }
    }
    return false;
}

// --- Node Management & Role Assignment ---
void
ScionAsImpl::AddNodeWithRole(Ptr<Node> node, ScionNodeType role)
{
    NS_ASSERT_MSG(node, "Cannot add a null node.");
    // NS_LOG_FUNCTION(this << node->GetId() << role);

    // Check if node is already in m_allNodes
    bool found = false;
    for (uint32_t i = 0; i < m_allNodes.GetN(); ++i)
    {
        if (m_allNodes.Get(i) == node)
        {
            found = true;
            break;
        }
    }
    if (!found)
    {
        m_allNodes.Add(node);
    }

    m_nodeRoles[node->GetId()] = role;
}

ScionNodeType
ScionAsImpl::GetNodeRole(Ptr<Node> node) const
{
    NS_ASSERT_MSG(node, "Cannot get role of a null node.");
    return GetNodeRole(node->GetId());
}

ScionNodeType
ScionAsImpl::GetNodeRole(uint32_t nodeId) const
{
    auto it = m_nodeRoles.find(nodeId);
    if (it != m_nodeRoles.end())
    {
        return it->second;
    }
    return ScionNodeType::UNSPECIFIED;
}

// --- Topology Management (Internal Connections) ---
void
ScionAsImpl::AddInternalLink(Ptr<Node> nodeA, Ptr<Node> nodeB)
{
    NS_ASSERT_MSG(nodeA && nodeB, "Cannot add internal link with null nodes.");
    NS_ASSERT_MSG(nodeA != nodeB, "Cannot add internal link from a node to itself.");
    // NS_LOG_FUNCTION(this << "NodeA:" << nodeA->GetId() << "NodeB:" << nodeB->GetId());

    uint32_t idA = nodeA->GetId();
    uint32_t idB = nodeB->GetId();

    // Ensure nodes are managed by this AS (or add them if a policy allows it)
    if (!IsManagedNode(idA))
    {
        NS_LOG_WARN(
            "Node A (" << idA
                       << ") not explicitly managed by this AS. Adding with UNSPECIFIED role.");
        AddNodeWithRole(nodeA, ScionNodeType::UNSPECIFIED);
    }
    if (!IsManagedNode(idB))
    {
        NS_LOG_WARN(
            "Node B (" << idB
                       << ") not explicitly managed by this AS. Adding with UNSPECIFIED role.");
        AddNodeWithRole(nodeB, ScionNodeType::UNSPECIFIED);
    }

    m_internalAdjacencyList[idA].push_back(idB);
    m_internalAdjacencyList[idB].push_back(idA); // Assuming bidirectional

    // Optional: Store device info
    // if (deviceA && deviceB) {
    //     m_internalLinkDevices[std::make_pair(std::min(idA, idB), std::max(idA, idB))] =
    //     std::make_pair(deviceA, deviceB);
    // }
}

NodeContainer
ScionAsImpl::GetInternalNeighbors(Ptr<Node> node) const
{
    NS_ASSERT_MSG(node, "Cannot get neighbors of a null node.");
    return GetInternalNeighbors(node->GetId());
}

NodeContainer
ScionAsImpl::GetInternalNeighbors(uint32_t nodeId) const
{
    NodeContainer neighbors;
    auto it = m_internalAdjacencyList.find(nodeId);
    if (it != m_internalAdjacencyList.end())
    {
        for (uint32_t neighborId : it->second)
        {
            Ptr<Node> neighborNode = NodeList::GetNode(neighborId);
            if (neighborNode)
            { // Should always be true if IDs are valid
                neighbors.Add(neighborNode);
            }
            else
            {
                NS_LOG_WARN("Could not find node with ID: " << neighborId << " in NodeList.");
            }
        }
    }
    return neighbors;
}

// --- Inter-AS Link Management ---

void
ScionAsImpl::AddScionLink(ScionLink& link)
{
    uint32_t brId = link.localBorderRouter->GetId();
    if (!IsManagedNode(brId) || GetNodeRole(brId) != ScionNodeType::BORDER_ROUTER)
    {
        NS_LOG_ERROR(
            "Node " << brId
                    << " is not a managed Border Router in this AS. Cannot add SCION link.");
        // Or, optionally, add/designate it as a BR here if that's desired behavior:
        // AddNodeWithRole(localBorderRouter, ScionNodeType::BORDER_ROUTER);
        return;
    }
    m_borderRouterScionLinks[brId].push_back(link);
}

void
ScionAsImpl::AddScionLink(Ptr<Node> localBorderRouter,
                          Ptr<Node> remoteBorderRouter,
                          Ptr<ScionAsImpl> remoteAs,
                          ScionLinkType linkType,
                          uint32_t mtu)
{
    NS_ASSERT_MSG(localBorderRouter, "Local Border Router cannot be null.");
    // NS_LOG_FUNCTION(this << "BR:" << localBorderRouter->GetId()
    //                     << " RemoteIA:" << linkInfo.remoteIa);

    uint32_t brId = localBorderRouter->GetId();
    if (!IsManagedNode(brId) || GetNodeRole(brId) != ScionNodeType::BORDER_ROUTER)
    {
        NS_LOG_ERROR(
            "Node " << brId
                    << " is not a managed Border Router in this AS. Cannot add SCION link.");
        // Or, optionally, add/designate it as a BR here if that's desired behavior:
        // AddNodeWithRole(localBorderRouter, ScionNodeType::BORDER_ROUTER);
        return;
    }

    // TODO: Addresses are filled later
    ScionLink linkInfo;
    linkInfo.remoteIa = remoteAs->GetIa(); // Assuming remoteAs is valid and has IA set
    linkInfo.linkType = linkType;
    linkInfo.mtu = mtu; // Set the MTU for this link
    linkInfo.localBorderRouter = localBorderRouter;
    linkInfo.remoteBorderRouter = remoteBorderRouter;
    m_borderRouterScionLinks[brId].push_back(linkInfo);
}

void
ScionAsImpl::AddScionLink(Ptr<Node> localBorderRouter,
                          Ptr<Node> remoteBorderRouter,
                          Ptr<ScionAsImpl> remoteAs,
                          ScionLinkType linkType)
{
    AddScionLink(localBorderRouter, remoteBorderRouter, remoteAs, linkType, DefaultScionMtu);
}

std::vector<ScionLink>
ScionAsImpl::GetScionLinks(Ptr<Node> localBorderRouter) const
{
    NS_ASSERT_MSG(localBorderRouter, "Local Border Router cannot be null.");
    return GetScionLinks(localBorderRouter->GetId());
}

std::vector<ScionLink>
ScionAsImpl::GetScionLinks(uint32_t borderRouterId) const
{
    auto it = m_borderRouterScionLinks.find(borderRouterId);
    if (it != m_borderRouterScionLinks.end())
    {
        return it->second;
    }
    return {}; // Return empty vector
}

std::vector<ScionLink>
ScionAsImpl::GetAllScionLinks() const
{
    std::vector<ScionLink> allLinks;
    for (const auto& pair : m_borderRouterScionLinks)
    {
        allLinks.insert(allLinks.end(), pair.second.begin(), pair.second.end());
    }
    return allLinks;
}

// --- Getters for Nodes by Role ---
NodeContainer
ScionAsImpl::GetNodesWithRole(ScionNodeType role) const
{
    NodeContainer nodes;
    for (const auto& pair : m_nodeRoles)
    {
        if (pair.second == role)
        {
            Ptr<Node> node = NodeList::GetNode(pair.first); // Get Ptr<Node> from ID
            if (node)
            {
                nodes.Add(node);
            }
            else
            {
                NS_LOG_WARN("Could not find node with ID: " << pair.first
                                                            << " in NodeList (for role query).");
            }
        }
    }
    return nodes;
}

NodeContainer
ScionAsImpl::GetBorderRouters() const
{
    return GetNodesWithRole(ScionNodeType::BORDER_ROUTER);
}

NodeContainer
ScionAsImpl::GetControlServices() const
{
    return GetNodesWithRole(ScionNodeType::CONTROL_SERVICE);
}

NodeContainer
ScionAsImpl::GetInternalRouters() const
{
    return GetNodesWithRole(ScionNodeType::INTERNAL_ROUTER);
}

NodeContainer
ScionAsImpl::GetHosts() const
{
    return GetNodesWithRole(ScionNodeType::HOST);
}

NodeContainer
ScionAsImpl::GetAllNodes() const
{
    return m_allNodes;
}

void
ScionAsImpl::SetFullMesh(bool isFullMesh)
{
    // NS_LOG_FUNCTION(this << isFullMesh);
    m_isFullMesh = isFullMesh;
}

bool
ScionAsImpl::IsFullMesh() const
{
    // NS_LOG_FUNCTION(this);
    return m_isFullMesh;
}

void
ScionAsImpl::SetFullMeshMtu(uint16_t mtu)
{
    // NS_LOG_FUNCTION(this << mtu);
    m_fullMeshMtu = mtu;
}

uint16_t
ScionAsImpl::GetFullMeshMtu() const
{
    // NS_LOG_FUNCTION(this);
    return m_fullMeshMtu;
}

void
ScionAsImpl::SetDefaultDataRate(uint32_t dataRate)
{
    // NS_LOG_FUNCTION(this << dataRate);
    m_defaultDataRate = dataRate;
}

uint32_t
ScionAsImpl::GetDefaultDataRate() const
{
    // NS_LOG_FUNCTION(this);
    return m_defaultDataRate;
}

void
ScionAsImpl::SetDefaultDelay(uint16_t delay)
{
    // NS_LOG_FUNCTION(this << delay);
    m_defaultDelay = delay;
}

uint16_t
ScionAsImpl::GetDefaultDelay() const
{
    // NS_LOG_FUNCTION(this);
    return m_defaultDelay;
}

} // namespace ns3

/**
 * void
ScionAsImpl::AddScionInterconnet(ScionInterconnect& link)
{
    ScionLink linkInfo;
    linkInfo.remoteIa = link.ia2; // Assuming ia2 is the

    Ptr<ScionAs> as = this->GetScionAttributes();

    switch (link.linkType)
    {
    case ScionInterconnectType::CORE:
        linkInfo.linkType = ScionLinkType::CORE;
        break;
    case ScionInterconnectType::PARENT_CHILD:
        if (as->IsCore())
        {
            linkInfo.linkType = ScionLinkType::CHILD;
        }
        else
        {
            linkInfo.linkType = ScionLinkType::PARENT;
        }

        break;
    case ScionInterconnectType::PEER:
        linkInfo.linkType = ScionLinkType::PEER;
        break;
    default:
        NS_LOG_ERROR("Unknown SCION interconnect type.");
        return; // Invalid type, do not add
    }

    linkInfo.mtu = link.mtu;
    linkInfo.dateRate = link.dateRate; // Data Rate in Bytes / second (Bps)
    linkInfo.delay = link.delay;       // Delay in MS
    linkInfo.localBorderRouter = link.br1;
    linkInfo.remoteBorderRouter = link.br2;
    linkInfo.localInterfaceId = 0; // TODO: Get Interface ID counter
    linkInfo.linkId = link.linkId;

    this->AddScionLink(linkInfo);
}
 */