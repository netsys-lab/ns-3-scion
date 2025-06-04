#include "scion-node-test.h"

#include "ns3/scion-path.h"
#include "ns3/buffer.h"
#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/scion-ia.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("SCIONNodeTest");


SCIONNodeTestCase::SCIONNodeTestCase()
    : TestCase("Verify SCION Node")   
{}

void SCIONNodeTestCase::Test00()
{
    auto n = Create<SCIONNode>();
    IaValue test_ia(Ia(5629130167029863));
    n->SetAttribute("Ia", test_ia);
    IaValue ia;
    n->GetAttribute("Ia", ia);
    // 5629130167029863  <==> 19-ffaa:0:1067
    NS_TEST_ASSERT_MSG_EQ(ia.Get(), test_ia.Get(), "");
}

void SCIONNodeTestCase::DoRun()
{
    Test00();
}

}
