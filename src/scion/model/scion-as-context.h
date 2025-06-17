/*
 * The ScionAsContext is value-type/dataclass which inherits Object and is aggregated onto every
 * Node in a ScionAS (along with the ‘ScionL3Protocol’) making it a ‘ScionHost’ in effect (just as
 * IPv4/6 does with ‘IpEndHosts’).
 *
 * Once the ScionStackHelper has finished the configuration of an AS, a subset of the AS’s
 * configuration state is carried-over from build time to runtime, by storing a copy of it on every
 * of the AS’s Nodes, where it is available/accessible for Applications (and kernel protos) living
 * on these Nodes during the simulation.
 *
 * The ScionAsContext has an interface similar to the SCION Daemon, which can be invoked by Apps at
 * runtime to get AS internal information, paths to particular endpoints and service addresses in
 * the local AS (i.e. getLocal_IA() , getPaths(src_ia, dst_ia ), getSVCAddress(svc_type ) etc ).
 *
 * Notably, the ScionAsContext is not an application itself. While it caches path requests leading
 * to faster response times, it won’t automatically update paths periodically, only because an App
 * requested them some time ago. Similar to the current SCION Daemon, it’ll serve as a shared
 * path-cache for all a Node’s Applications.
 */

#ifndef SCION_AS_CONTEXT_H
#define SCION_AS_CONTEXT_H

#include "ns3/address.h"
#include "ns3/as.h"
#include "ns3/attribute-accessor-helper.h" // For accessor helpers
#include "ns3/attribute.h"                 // Needed for IaValue etc.
#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/scion-ia.h"
#include "ns3/string.h" // For StringValue

namespace ns3
{

// Forward declaration for Ia if not fully defined/visible via scion-addr.h
// class Ia;

/**
 * \brief Represents an address with IP and Port.
 */
struct ScionServiceAddress
{
    Address address;
    uint16_t port; // TODO: Maybe we dont want to have the port here when using plain L2 as underlay
};

/**
 * \brief Configuration for an underlay connection of a BR interface.
 */
struct BorderRouterInterfaceUnderlay
{
    ScionServiceAddress publicAddress; // Local public IP:Port for the underlay
    ScionServiceAddress remoteAddress; // Remote IP:Port of the neighbor's underlay
};

/**
 * \brief Configuration for a specific interface on a Border Router.
 */
struct BorderRouterInterface
{
    BorderRouterInterfaceUnderlay underlay;
    Ia remoteIsdAs;           // ISD-AS of the peer AS connected via this interface
    ScionLinkType linkToType; // e.g., "CORE", "PARENT", "CHILD", "PEER"
    uint16_t mtu;             // MTU for this specific link, if different from AS default
    // uint16_t interfaceId; // The key "1" in JSON becomes the map key
};

/**
 * \brief Configuration for a Border Router.
 */
struct BorderRouterConfig
{
    std::string id;                                       // e.g., "br-1" (the key from JSON)
    ScionServiceAddress internalAddress;                  // Internal IP:Port of the BR
    std::map<uint16_t, BorderRouterInterface> interfaces; // Key is the interface ID (e.g., 1, 2)
};

/**
 * \brief Configuration for a Control Service or Discovery Service instance.
 */
struct ServiceInstanceConfig
{
    std::string id; // e.g., "cs-1" (the key from JSON)
    ScionServiceAddress address;
};

/**
 * \brief Top-level structure representing the configuration of a SCION AS.
 * Similar to topology.json config file
 */
struct ScionAsTopology
{
    Ia isdAs;     // ISD-AS of this AS
    uint16_t mtu; // Default MTU for the AS

    // AS-level attributes: is core AS. Things like "issuing" or "authoritative" are skipped for now
    // since we assume That the control plane works, we don't need to simulate trust or crypto
    bool isCore;

    // Map of service ID (e.g., "cs-1") to its configuration
    std::map<std::string, ServiceInstanceConfig> controlServices;
    std::map<std::string, ServiceInstanceConfig> discoveryServices;
    // Add other service types here (e.g., SIGs) if needed

    // Map of Border Router ID (e.g., "br-1") to its configuration
    std::map<std::string, BorderRouterConfig> borderRouters;

    ScionAsTopology()
        : mtu(DefaultScionMtu)
    {
    } // Default constructor
};

class ScionAsContext : public Object

{
  public:
    static TypeId GetTypeId(void);
    virtual TypeId GetInstanceTypeId(void) const;
    virtual void DoDispose(void);

    // Constructor
    ScionAsContext();

    // Destructor
    virtual ~ScionAsContext();

    // Setters
    void SetIsCore(bool isCore);
    void SetIa(const Ia& ia);
    void SetBeaconPolicy(uint32_t policy);

    // Getters
    const Ia& GetIa() const;
    Isd GetIsd() const;
    Asn GetAs() const;

    uint32_t GetBeaconPolicy() const;
    ScionAsTopology& GetTopology();
    void SetTopology(ScionAsTopology topology);

  private:
    uint32_t m_beaconPolicy; // Beacon policy
    ScionAsTopology m_topology;
};

} // namespace ns3

#endif // SCION_MODEL_SCION_AS_H