#ifndef SCION_MODEL_SCION_STACK_HELPER_H
#define SCION_MODEL_SCION_STACK_HELPER_H

#include "ns3/internet-stack-helper.h" // For IP stack
#include "ns3/ipv4-address-helper.h"   // For IP addressing
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/ipv4-list-routing.h"
#include "ns3/ipv4-routing-helper.h" // For routing
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-static-routing.h"
#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/point-to-point-helper.h" // For L2 connectivity

// SCION specific includes
#include "ns3/scion-as-impl.h" // The class we are helping to configure

namespace ns3
{

/**
 * \ingroup scion
 * \brief Helper to install and configure the SCION stack on ASes and their nodes.
 *
 * This helper automates several setup tasks:
 * - Provisioning Border Router (BR) nodes based on inter-AS link data.
 * - Creating a minimal internal L2 topology (e.g., mesh/star) for BRs and Control Services (CSs)
 *   if one is not already defined.
 * - Installing the IP stack on SCION nodes.
 * - Assigning IP addresses for intra-AS communication.
 * - Setting up basic intra-AS IP routing.
 *
 * This class is intended for use during the simulation setup phase (build time).
 */
class ScionStackHelper
{
  public:
    ScionStackHelper();

    // --- Configuration for the Helper ---

    /**
     * \brief Set the PointToPointHelper to use for creating internal L2 links.
     * \param p2phelper The PointToPointHelper instance.
     */
    void SetP2PHelper(PointToPointHelper p2phelper);

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

    // --- Main Installation Methods ---

    /**
     * \brief Install the SCION stack and configure a single SCION AS.
     * \param as The SCION AS implementation to configure.
     *
     * This method will:
     * 1. Ensure necessary BR nodes exist (or create them if policy allows).
     * 2. Setup minimal internal L2 connectivity if not already present.
     * 3. Install IP stack on all AS nodes.
     * 4. Assign IP addresses.
     * 5. Setup intra-AS IP routing.
     */
    void Install(Ptr<ScionAsImpl> as);

    /**
     * \brief Install the IP stack and configure IP addressing/routing on a container of nodes
     *        belonging to a single AS.
     * \param as The SCION AS these nodes belong to (for context, e.g., IA).
     * \param nodes The nodes to install the IP stack on.
     *
     * This is a lower-level method, typically called by Install(Ptr<ScionAsImpl>).
     */
    void InstallIpStack(Ptr<ScionAsImpl> as, NodeContainer nodes);

  private:
    /**
     * \brief Ensure Border Router nodes exist for defined SCION links.
     *
     * If a ScionLink exists but the designated local BR node is not yet
     * part of the AS or not marked as a BR, this method can (based on policy,
     * TBD) create a new node or designate an existing one.
     * For now, it mainly verifies.
     * \param as The SCION AS to process.
     */
    void ProvisionBorderRouters(Ptr<ScionAsImpl> as);

    /**
     * \brief Create a minimal internal L2 topology if none is explicitly defined.
     * Connects BRs and CSs in a specified topology (e.g., full mesh).
     * \param as The SCION AS to process.
     * \param coreNodes Nodes like BRs and CSs to interconnect.
     */
    void SetupMinimalInternalTopology(Ptr<ScionAsImpl> as, NodeContainer coreNodes);

    /**
     * \brief Assign IPv4 addresses to devices within the AS.
     * \param as The SCION AS.
     * \param devices The NetDeviceContainer of internal links to address.
     * \return The Ipv4InterfaceContainer for the addressed devices.
     */
    Ipv4InterfaceContainer AssignIpAddresses(Ptr<ScionAsImpl> as, NetDeviceContainer devices);

    PointToPointHelper m_p2pHelper;                //!< Helper for creating P2P links.
    InternetStackHelper m_stackInstaller;          //!< Helper to install TCP/IP stack.
    Ipv4AddressHelper m_ipv4AddressHelper;         //!< Helper for assigning IPv4 addresses.
    Ipv4StaticRoutingHelper m_staticRoutingHelper; //!< For basic static routes
    Ipv4ListRoutingHelper m_listRoutingHelper;     //!< For more complex routing, can add
    // static/global

    // Configuration for internal topology and addressing
    std::string m_baseIpNetwork;
    std::string m_baseIpMask;
    bool m_allocateIpPerLink; // True: /30 per link, False: larger AS subnet
    enum class InternalTopologyType
    {
        NONE, // User provides all internal links
        FULL_MESH,
        STAR // Needs a central node designation
    };
    InternalTopologyType m_internalTopologyType;
    uint32_t m_nextAsSubnet; // For allocating subnets if not m_allocateIpPerLink
};

} // namespace ns3

#endif // SCION_MODEL_SCION_STACK_HELPER_H