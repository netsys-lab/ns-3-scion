#ifndef SCION_TOPOLOGY_HELPER_H
#define SCION_TOPOLOGY_HELPER_H

#include "ns3/application-container.h" // Potentially for on/off apps for testing
#include "ns3/csma-helper.h"           // For CSMA link example
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-module.h"
#include "ns3/scion-as-impl.h" // For ScionAsImpl class
#include "ns3/topology-helper.h"

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
class ScionTopologyHelper
{
  public:
    ScionTopologyHelper();

    /**
     * \brief Set the TopologyHelper to use for creating underlay
     * \param p2pHelper The TopologyHelper instance.
     */
    void SetTopologyHelper(TopologyHelper topoHelper);

    void InstallInternalTopology(Ptr<ScionAsImpl> asImpl);
    void InstallInterConnects(Ptr<ScionAsImpl> asImpl);
    void InstallInterConnectDevices(Ptr<ScionAsImpl> asImpl);

    void InstallAllInterconnects(std::vector<Ptr<ScionAsImpl>> asImpls);
    void InstallAllInternalTopology(std::vector<Ptr<ScionAsImpl>> asImpls);

    Ptr<ScionAsImpl> AddAs(Ia ia, bool isCore = false);
    Ptr<Node> AddBorderRouter(Ptr<ScionAsImpl> as);
    Ptr<Node> AddControlService(Ptr<ScionAsImpl> as);
    Ptr<Node> AddEndhost(Ptr<ScionAsImpl> as);

    void SetAsInternalUnderlayType(ScionUnderlay type);
    void SetInterconnectUnderlayType(ScionUnderlay type);

    std::vector<Ptr<ScionAsImpl>> GetAllAs() const
    {
        return m_allAs;
    }

    /**
     *
     */
    void AddScionInterconnect(Ptr<ScionAsImpl> as1, Ptr<ScionAsImpl> as2, ScionInterconnect link);

  private:
    TopologyHelper m_topoHelper;              //!< Helper for Underlay links.
    ScionUnderlay m_internalUnderlayType;     //!< Underlay type (L2 or L3)
    ScionUnderlay m_interconnectUnderlayType; //!< Underlay type (L2 or L3)

    std::vector<Ptr<ScionAsImpl>> m_allAs; //!< All ASes created by this helper
    uint64_t m_nextLinkId = 1;             //!< Unique ID for interconnect links
};

} // namespace ns3

#endif // SCION_MODEL_SCION_TOPOLOGY_HELPER_H
