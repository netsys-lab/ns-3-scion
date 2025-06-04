
#include "ns3/scion-node.h"
#include "ns3/test.h"

namespace ns3{
/**
 * \ingroup scion-test
 *
 * \brief scion dataplane path Test
 */
class SCIONNodeTestCase : public TestCase
{

    void Test00();
    void Test01();

public:
    SCIONNodeTestCase();
    ~SCIONNodeTestCase() override=default;
    void DoRun() override;
};
}