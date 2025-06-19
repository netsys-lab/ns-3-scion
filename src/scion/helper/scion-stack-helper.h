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
#include "ns3/scion-as-context.h"
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

    // --- Main Installation Methods ---

    /**
     * \brief Set the underlay intra-AS addressing.
     * \param type Either L2_ETHERNET or L3_IP for now.
     **/
    void SetUnderlayType(ScionUnderlay type);

    /**
     * \brief Install the SCION stack and configure a single SCION AS.
     * \param as The SCION AS implementation to configure.
     *
     * This method will:
     * 1. Generate a SCION AS Context for the AS and installs it on all the nodes.
     */
    void Install(Ptr<ScionAsImpl> as);

    /**
     * \brief Install the SCION stack and configure all SCION ASes.
     * \param ases A vector with all the SCION ASes.
     *
     */
    void InstallAll(std::vector<Ptr<ScionAsImpl>> as);

    /**
     * \brief Generate a SCION AS Context that will be passed to runtime out of the given
     * ScionAsImpl class
     * \param as The SCION AS implementation.
     */
    Ptr<ScionAsContext> CreateScionAsContext(Ptr<ScionAsImpl> as);

  private:
        ScionUnderlay m_underlayType; //!< Underlay type (L2 or L3)

    // TODO: Keep a map of which remote BRs have already been provisioned with inter-domain links
};

} // namespace ns3

#endif // SCION_MODEL_SCION_STACK_HELPER_H