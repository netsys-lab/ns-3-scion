
#include "ns3/scion-socket-address.h"
#include "ns3/test.h"

namespace ns3{
/**
 * \ingroup scion-test
 *
 * \brief scion socket address test
 */
class SCIONSocketAddressTestCase : public TestCase
{

    void Test00();
    void Test01();

public:
    SCIONSocketAddressTestCase();
    ~SCIONSocketAddressTestCase() override=default;
    void DoRun() override;
};
}