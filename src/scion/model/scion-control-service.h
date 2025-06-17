
// control-service.h
#ifndef SCION_CONTROL_SERVICE_H
#define SCION_CONTROL_SERVICE_H

#include "ns3/application.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/node-container.h"
#include "ns3/object.h"

#include <string>
#include <vector>

namespace ns3
{
class ScionControlServiceApp : public Application
{
  public:
    static TypeId GetTypeId(void);
    std::string GetId() const;
    void SetId(std::string id);

    void SetAddress(Address internal);

    Address GetAddress() const;

  protected:
    void StartApplication() override;
    void StopApplication() override;

  private:
    Address m_addr;
    std::string m_id;
};
} // namespace ns3

#endif // CONTROL_SERVICE_H