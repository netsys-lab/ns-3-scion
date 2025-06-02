
#include "ns3/scion-address.h"
#include "ns3/test.h"

namespace ns3{
/**
 * \ingroup scion-test
 *
 * \brief scion address Test
 */
class SCIONAddressTestCase : public TestCase
{
  public:
    SCIONAddressTestCase();
    ~SCIONAddressTestCase() override;
    void DoRun() override;
};
}