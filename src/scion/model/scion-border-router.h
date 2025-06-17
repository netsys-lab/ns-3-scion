
// border-router.h
#ifndef SCION_BORDER_ROUTER_H
#define SCION_BORDER_ROUTER_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/scion-types.h" // For ScionLink structure

#include <string>
#include <vector>

namespace ns3
{
class ScionBorderRouterApp : public Application
{
  public:
    static TypeId GetTypeId(void);
    std::string GetId() const;
    void SetId(std::string id);

    void SetInternalAddress(Address internal);
    void AddLink(const ScionLink& link);

    Address GetInternalAddress() const;
    std::vector<ScionLink> GetLinks() const;

  protected:
    void StartApplication() override;
    void StopApplication() override;

  private:
    Address m_internalAddr;
    std::string m_id;
    std::vector<ScionLink> m_links;
};
} // namespace ns3

#endif // BORDER_ROUTER_H