#ifndef SCION_MODEL_SCION_AS_IMPL_H
#define SCION_MODEL_SCION_AS_IMPL_H

#include "ns3/address.h" // For Address type
#include "ns3/application-container.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/scion-as.h"    // For ScionAs (attribute store)
#include "ns3/scion-ia.h"    // For Ia, Isd, As
#include "ns3/scion-types.h" // For ScionLink structure

#include <map>
#include <set>
#include <string> // For std::string in ScionLink
#include <vector>

namespace ns3
{

struct ScionLink
{
    // Consider Ptr<NetDevice> localDevice; if you want to tie to a specific device
    Address localAddress;  // Local L3 address on the BR for this link
    Address remoteAddress; // Remote L3 address on the neighbor BR
    uint16_t mtu;
    uint32_t dateRate; // Data Rate in Bytes / second (Bps)
    uint16_t delay;    // Delay in MS
    ScionLinkType linkType;

    uint16_t localInterfaceId;   // SCION Interface ID on our side
    uint16_t remoteInterfaceId;  // SCION Interface ID on remote side
    Ia remoteIa;                 // IA of the remote AS
    Ptr<Node> localBorderRouter; // Will be implicitly known via m_borderRouterLinks
    Ptr<Node> remoteBorderRouter;

    uint32_t linkId; // Unique ID for this link
};

enum class ScionInterconnectType
{
    CORE,
    PARENT_CHILD,
    PEER
};

struct ScionInterconnect
{
    uint16_t mtu;
    uint32_t dateRate; // Data Rate in Bytes / second (Bps)
    uint16_t delay;    // Delay in MS
    ScionInterconnectType linkType;

    Ptr<Node> br1; // Will be implicitly known via m_borderRouterLinks
    Ptr<Node> br2;

    uint32_t linkId; // Unique ID for this link
};

struct InternalLink
{
    Ptr<Node> nodeA; // One end of the internal link
    Ptr<Node> nodeB; // The other end of the internal link

    uint16_t mtu;
    uint32_t dateRate; // Data Rate in Bytes / second (Bps)
    uint16_t delay;    // Delay in MS
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
     * \note This represents a Layer 2/3 adjacency. Assumes link is bidirectional.
     *       Stores devices primarily for lookup/debug, not for L3 protocol specifics.
     */
    void AddInternalLink(Ptr<Node> nodeA, Ptr<Node> nodeB);

    /**
     * \brief This method fills the adjacency list for all nodes in this AS.
     * All nodes are now direct neighbors of each other.
     **/
    void ConnectAllNodes();

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
     * \param remoteBorderRouter The remote Border Router node that terminates this link.
     * \param remoteAs The remote AS this link connects to.
     * \param linkType The SCION link type that matches this link
     * \note Assumes localBorderRouter has already been added with BORDER_ROUTER role.
     */
    void AddScionLink(Ptr<Node> localBorderRouter,
                      Ptr<Node> remoteBorderRouter,
                      Ptr<ScionAsImpl> remoteAs,
                      ScionLinkType linkType);

    /**
     *
     */
    void AddScionLink(ScionLink& link);

    /**
     *
     */
    void AddScionInterconnet(ScionInterconnect& link);

    /**
     * \brief Add information about an inter-AS SCION link.
     * \param localBorderRouter The local Border Router node that terminates this link.
     * \param remoteBorderRouter The remote Border Router node that terminates this link.
     * \param remoteAs The remote AS this link connects to.
     * \param linkType The SCION link type that matches this link
     * \param mtu The MTU for this link. Will later be set by the ScionStackHelper
     * \note Assumes localBorderRouter has already been added with BORDER_ROUTER role.
     */
    void AddScionLink(Ptr<Node> localBorderRouter,
                      Ptr<Node> remoteBorderRouter,
                      Ptr<ScionAsImpl> remoteAs,
                      ScionLinkType linkType,
                      uint32_t mtu);

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

    void SetFullMesh(bool isFullMesh);
    bool IsFullMesh() const;

    void SetFullMeshMtu(uint16_t mtu);
    uint16_t GetFullMeshMtu() const;

    void SetDefaultDataRate(uint32_t dataRate);
    uint32_t GetDefaultDataRate() const;

    void SetDefaultDelay(uint16_t delay);
    uint16_t GetDefaultDelay() const;

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

    std::vector<InternalLink>
        m_internalLinks; //!< List of internal links (bidirectional connections).

    // Optional: Store devices for internal links if needed for richer topology info
    // std::map<std::pair<uint32_t, uint32_t>, std::pair<Ptr<NetDevice>, Ptr<NetDevice>>>
    // m_internalLinkDevices;

    // Maps Border Router Node ID to a list of its inter-AS SCION links.
    std::map<uint32_t, std::vector<ScionLink>> m_borderRouterScionLinks;

    bool m_isFullMesh; //!< True if this AS uses a full mesh for internal links.

    uint16_t m_fullMeshMtu;     //!< MTU for full mesh internal links, if applicable.
    uint32_t m_defaultDataRate; //!< Default data rate for internal links, if applicable.
    uint16_t m_defaultDelay;    //!< Default delay for internal links, if applicable.
};

} // namespace ns3

#endif // SCION_MODEL_SCION_AS_IMPL_H