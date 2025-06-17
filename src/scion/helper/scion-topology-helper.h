#ifndef SCION_TOPOLOGY_HELPER_H
#define SCION_TOPOLOGY_HELPER_H

#include "ns3/application-container.h" // Potentially for on/off apps for testing
#include "ns3/csma-helper.h"           // For CSMA link example
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "ns3/point-to-point-helper.h"
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

    /**
     *
     */
    void AddScionInterconnect(Ptr<ScionAsImpl> as1, Ptr<ScionAsImpl> as2, ScionInterconnect link);

  private:
    TopologyHelper m_topoHelper; //!< Helper for Underlay links.
};

} // namespace ns3

#endif // SCION_MODEL_SCION_TOPOLOGY_HELPER_H
