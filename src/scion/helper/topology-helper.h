#ifndef TOPOLOGY_HELPER_H
#define TOPOLOGY_HELPER_H

#include "ns3/application-container.h" // Potentially for on/off apps for testing
#include "ns3/csma-helper.h"           // For CSMA link example
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/point-to-point-helper.h"

namespace ns3
{

/**
 * \ingroup scion
 * \brief Helper to create L2 and optionally L3 links between nodes.
 *
 * This helper provides methods to establish point-to-point or CSMA links
 * between specified nodes, and can optionally assign IP addresses to the
 * created interfaces. It allows for more granular control over link setup
 * than higher-level stack helpers might offer for default topologies.
 */
class TopologyHelper
{
  public:
    TopologyHelper();

    // --- L2 Link Configuration ---

    /**
     * \brief Set the base IP network for intra-AS addressing.
     * \param network The base IPv4 network (e.g., "10.0.0.0").
     * \param mask The network mask (e.g., "255.0.0.0").
     */
    void SetBaseIp(std::string network, std::string mask);

    /**
     * \brief Set the IP allocation policy.
     * \param allocatePerLink If true, allocates a /30 per link. Otherwise, uses a larger AS-wide
     * subnet.
     */
    void SetIpAllocationPolicy(bool allocatePerLink);

    /**
     * \brief Set the type of minimal internal topology to create if one is not defined by the user.
     * \param type A string representing the topology ("NONE", "FULL_MESH", "STAR").
     */
    void SetInternalTopologyType(std::string type);

    /**
     * \brief Set the PointToPointHelper to use for creating point-to-point links.
     * Users can pre-configure this helper (e.g., DataRate, Delay, MTU).
     * \param p2pHelper The PointToPointHelper instance.
     */
    void SetP2PHelper(PointToPointHelper p2pHelper);

    /**
     * \brief Set the CsmaHelper to use for creating CSMA (multi-access) links.
     * Users can pre-configure this helper.
     * \param csmaHelper The CsmaHelper instance.
     */
    void SetCsmaHelper(CsmaHelper csmaHelper);

    // --- L3 Configuration ---

    /**
     * \brief Set the Ipv4AddressHelper to use for assigning IP addresses.
     * The user should configure the base network and mask on this helper
     * before calling methods that assign IP addresses.
     * \param ipv4AddressHelper The Ipv4AddressHelper instance.
     */
    void SetIpv4AddressHelper(Ipv4AddressHelper ipv4AddressHelper);

    // --- Link Creation Methods ---

    /**
     * \brief Create a point-to-point link between two nodes.
     * \param nodeA The first node.
     * \param nodeB The second node.
     * \return A NetDeviceContainer containing the two created NetDevices (one on each node).
     */
    NetDeviceContainer CreateP2PLink(Ptr<Node> nodeA, Ptr<Node> nodeB);

    /**
     * \brief Create a point-to-point link between two nodes and assign IP addresses.
     * \param nodeA The first node.
     * \param nodeB The second node.
     * \param ipA The IPv4 address for nodeA's interface on this link.
     * \param ipB The IPv4 address for nodeB's interface on this link.
     * \param mask The IPv4 network mask for this link (e.g., "255.255.255.252").
     * \return An Ipv4InterfaceContainer for the configured interfaces.
     * \note This method assumes the IP stack is already installed on the nodes.
     */
    Ipv4InterfaceContainer CreateP2PLinkWithIp(Ptr<Node> nodeA,
                                               Ptr<Node> nodeB,
                                               Ipv4Address ipA,
                                               Ipv4Address ipB,
                                               Ipv4Mask mask);

    /**
     * \brief Create a point-to-point link between two nodes and assign IP addresses
     *        using the configured Ipv4AddressHelper (which should have a base network set).
     * \param nodeA The first node.
     * \param nodeB The second node.
     * \return An Ipv4InterfaceContainer for the configured interfaces.
     * \note This method assumes the IP stack is already installed on the nodes.
     *       The Ipv4AddressHelper will assign the next available IPs from its base.
     */
    Ipv4InterfaceContainer CreateP2PLinkWithIp(Ptr<Node> nodeA, Ptr<Node> nodeB);

    /**
     * \brief Create a CSMA (multi-access) link connecting a set of nodes.
     * \param nodes The NodeContainer with nodes to connect to the CSMA channel.
     * \return A NetDeviceContainer containing the NetDevices created on each node.
     */
    NetDeviceContainer CreateCsmaLink(NodeContainer nodes);

    /**
     * \brief Create a CSMA link and assign IP addresses using the configured Ipv4AddressHelper.
     * \param nodes The NodeContainer with nodes to connect.
     * \return An Ipv4InterfaceContainer for the configured interfaces.
     * \note This method assumes the IP stack is already installed on the nodes.
     *       The Ipv4AddressHelper should have its base network and mask set appropriately
     *       for the number of nodes on the CSMA segment.
     */
    Ipv4InterfaceContainer CreateCsmaLinkWithIp(NodeContainer nodes);

    /**
     * \brief Assign IP addresses to a pre-existing set of NetDevices.
     * Uses the configured Ipv4AddressHelper.
     * \param devices The NetDeviceContainer whose interfaces need IP addresses.
     * \return An Ipv4InterfaceContainer for the configured interfaces.
     */
    Ipv4InterfaceContainer AssignIp(NetDeviceContainer devices);

  private:
    PointToPointHelper m_p2pHelper;        //!< Helper for PointToPoint links.
    CsmaHelper m_csmaHelper;               //!< Helper for CSMA links.
    Ipv4AddressHelper m_ipv4AddressHelper; //!< Helper for IPv4 addressing.
    bool m_ipv4AddressHelperSet;           //!< Flag to check if user provided Ipv4AddressHelper
};

} // namespace ns3

#endif // SCION_MODEL_SCION_TOPOLOGY_HELPER_H
