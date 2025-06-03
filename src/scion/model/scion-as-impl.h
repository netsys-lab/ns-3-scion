#ifndef SCION_MODEL_SCION_AS_IMPL_H
#define SCION_MODEL_SCION_AS_IMPL_H

#include "ns3/address.h" // For Address type
#include "ns3/application-container.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/scion-as.h" // For ScionAs (attribute store)
#include "ns3/scion-ia.h" // For Ia, Isd, As

#include <map>
#include <set>
#include <string> // For std::string in ScionLink
#include <vector>

namespace ns3
{

// Existing enums and structs
enum class ScionLinkType
{
    CORE,
    PARENT,
    CHILD,
    PEER
};

struct ScionLink
{
    // Consider Ptr<NetDevice> localDevice; if you want to tie to a specific device
    Address localAddress;  // Local L3 address on the BR for this link
    Address remoteAddress; // Remote L3 address on the neighbor BR
    uint16_t mtu;
    ScionLinkType linkType;
    uint16_t localInterfaceId;  // SCION Interface ID on our side
    uint16_t remoteInterfaceId; // SCION Interface ID on remote side
    Ia remoteIa;                // IA of the remote AS
    // Ptr<Node> localBorderRouter; // Will be implicitly known via m_borderRouterLinks
};

// New enum for node roles
enum class ScionNodeType
{
    UNSPECIFIED, // Default or intermediate (e.g. pure switch)
    BORDER_ROUTER,
    CONTROL_SERVICE,
    INTERNAL_ROUTER, // Could also represent a switch
    HOST
};

/**
 * \ingroup scion
 * \brief Represents the actual implementation and topology of a SCION AS.
 *
 * This class manages the nodes within an AS, their roles, internal connectivity (topology graph),
 * and links to neighboring ASes. It aims to be Layer 3 protocol agnostic in its
 * representation of the AS-level graph.
 */
class ScionAsImpl : public Object
{
  public:
    static TypeId GetTypeId(void);
    ScionAsImpl();
    ~ScionAsImpl() override;

    void DoDispose(void) override;

    // --- Configuration Methods ---
    void SetScionAttributes(Ptr<ScionAs> attributes);
    Ptr<ScionAs> GetScionAttributes() const;
    Ia GetIa() const; // Convenience

    // --- Node Management & Role Assignment ---

    /**
     * \brief Adds a node to this AS and assigns it a specific role.
     * \param node The node to add.
     * \param role The SCION-specific role of this node.
     * \note The AS takes ownership of the node by adding it to m_allNodes.
     */
    void AddNodeWithRole(Ptr<Node> node, ScionNodeType role);

    /**
     * \brief Get the role of a specific node.
     * \param node The node to query.
     * \return The ScionNodeType of the node. Returns UNSPECIFIED if node not found or no role set.
     */
    ScionNodeType GetNodeRole(Ptr<Node> node) const;
    ScionNodeType GetNodeRole(uint32_t nodeId) const;

    // --- Topology Management (Internal Connections) ---

    /**
     * \brief Adds an internal link (connection) between two nodes within this AS.
     * \param nodeA One node in the connection.
     * \param nodeB The other node in the connection.
     * \param deviceA The NetDevice on nodeA used for this link (optional, for reference).
     * \param deviceB The NetDevice on nodeB used for this link (optional, for reference).
     * \note This represents a Layer 2/3 adjacency. Assumes link is bidirectional.
     *       Stores devices primarily for lookup/debug, not for L3 protocol specifics.
     */
    void AddInternalLink(Ptr<Node> nodeA,
                         Ptr<Node> nodeB,
                         Ptr<NetDevice> deviceA = nullptr,
                         Ptr<NetDevice> deviceB = nullptr);

    /**
     * \brief Get all nodes directly connected to a given internal node.
     * \param node The node whose neighbors are requested.
     * \return A NodeContainer with the connected nodes.
     */
    NodeContainer GetInternalNeighbors(Ptr<Node> node) const;
    NodeContainer GetInternalNeighbors(uint32_t nodeId) const;

    // --- Inter-AS Link Management (SCION Links) ---

    /**
     * \brief Add information about an inter-AS SCION link.
     * \param localBorderRouter The local Border Router node that terminates this link.
     * \param linkInfo The ScionLink structure describing the link.
     * \note Assumes localBorderRouter has already been added with BORDER_ROUTER role.
     */
    void AddScionLink(Ptr<Node> localBorderRouter, const ScionLink& linkInfo);

    /**
     * \brief Get all inter-AS links associated with a specific local Border Router.
     * \param localBorderRouter The Border Router node.
     * \return A vector of ScionLink objects for that BR. Empty if BR not found or has no links.
     */
    std::vector<ScionLink> GetScionLinks(Ptr<Node> localBorderRouter) const;
    std::vector<ScionLink> GetScionLinks(uint32_t borderRouterId) const;

    /**
     * \brief Get all inter-AS links for this entire AS.
     * \return A vector containing all ScionLink objects across all BRs.
     */
    std::vector<ScionLink> GetAllScionLinks() const;

    // --- Getters for Nodes by Role ---
    NodeContainer GetNodesWithRole(ScionNodeType role) const;
    NodeContainer GetBorderRouters() const;   // Convenience for GetNodesWithRole(BORDER_ROUTER)
    NodeContainer GetControlServices() const; // Convenience for GetNodesWithRole(CONTROL_SERVICE)
    NodeContainer GetInternalRouters() const; // Convenience for GetNodesWithRole(INTERNAL_ROUTER)
    NodeContainer GetHosts() const;           // Convenience for GetNodesWithRole(HOST)
    NodeContainer GetAllNodes() const;        // All nodes owned/managed by this AS

  private:
    // Helper to ensure a node is part of this AS
    bool IsManagedNode(uint32_t nodeId) const;

    Ptr<ScionAs> m_attributes; //!< Core SCION attributes (IA, policy, etc.)

    NodeContainer m_allNodes; //!< Primary container owning all nodes in this AS.

    // Graph-like representation
    std::map<uint32_t, ScionNodeType> m_nodeRoles; //!< Node ID to its SCION role.

    // Stores internal adjacencies. Key: Node ID, Value: List of connected Node IDs.
    std::map<uint32_t, std::vector<uint32_t>> m_internalAdjacencyList;

    // Optional: Store devices for internal links if needed for richer topology info
    // std::map<std::pair<uint32_t, uint32_t>, std::pair<Ptr<NetDevice>, Ptr<NetDevice>>>
    // m_internalLinkDevices;

    // Maps Border Router Node ID to a list of its inter-AS SCION links.
    std::map<uint32_t, std::vector<ScionLink>> m_borderRouterScionLinks;
};

} // namespace ns3

#endif // SCION_MODEL_SCION_AS_IMPL_H