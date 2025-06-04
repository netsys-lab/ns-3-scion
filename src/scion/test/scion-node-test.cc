#include "scion-node-test.h"

#include "ns3/scion-path.h"
#include "ns3/buffer.h"
#include "ns3/test.h"
#include "ns3/log.h"
#include "ns3/scion-ia.h"
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/string.h"
#include "ns3/scion.h"

namespace ns3{

NS_LOG_COMPONENT_DEFINE("SCIONNodeTest");


SCIONNodeTestCase::SCIONNodeTestCase()
    : TestCase("Verify SCION Node")   
{}


void CreateAndAggregateSCION(Ptr<Node> node)
{   ObjectFactory factory;
    factory.SetTypeId("ns3::SCIONL3Protocol");
    Ptr<Object> protocol = factory.Create<Object>();
    node->AggregateObject(protocol);
}

void SCIONNodeTestCase::Test01()
{

    /*
    N0, N3 are EndHosts
    N1, N2 BorderRouters (each with one external and internal interface)
    No IP routing required

    +--------------------+     +------------------+                                        
    |    AS I            |     | AS II            |                                        
    |                    |     |                  |                                        
    |                    |     |                  |                                        
    |    +-+  S0   +-+   |  S1 | +-+   S2    +-+  |                                        
    |    +-+-------+-+===========+-+---------+-+  |                                        
    |  N0          N1    |     |N2           N3   |                                        
    |                    |     |                  |                                        
    |                    |     |                  |                                        
    +--------------------+     +------------------+       
    */
    IaValue ia1(Ia(Isd(1), Asn(1)));
    IaValue ia2(Ia(Isd(1), Asn(2)));
    NodeContainer nodes_s0;
    NodeContainer nodes_s2;

    auto n0 = Create<SCIONNode>();    
    n0->SetAttribute("Ia", ia1);
    nodes_s0.Add(n0);
    auto n1 = Create<SCIONNode>();
    n1->SetAttribute("Ia", ia1);
    nodes_s0.Add(n1);

    
    auto n2 = Create<SCIONNode>();
    n2->SetAttribute("Ia", ia2);
    nodes_s2.Add(n2);
    auto n3 = Create<SCIONNode>();
    n3->SetAttribute("Ia", ia2);
    nodes_s2.Add(n3);

    // ------ setup L2 topology -----
    //  this is L3 agnostic

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer devices_s0;
    devices_s0 = pointToPoint.Install(nodes_s0);

    NetDeviceContainer devices_s2;
    devices_s2 = pointToPoint.Install(nodes_s2);

    pointToPoint.SetDeviceAttribute("DataRate", StringValue("20Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("1ms"));

    NetDeviceContainer devices_s1;
    devices_s1 = pointToPoint.Install(n1,n2);

    // ------- setup L3 stuff (Interfacess)  ------------


    InternetStackHelper stack;
    stack.InstallAll();

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces_s0 = address.Assign(devices_s0);

    address.SetBase("10.2.2.0", "255.255.255.252");
    Ipv4InterfaceContainer interfaces_s2 = address.Assign(devices_s2);

    // dont create IpInterfaces for the s_1 segment
    // But make them SCION (over L2) Interfaces
    auto nd1=devices_s1.Get(0);
    auto nd2=devices_s1.Get(1);

    CreateAndAggregateSCION(n0);
    CreateAndAggregateSCION(n1);
    CreateAndAggregateSCION(n2);
    CreateAndAggregateSCION(n3);

    auto sif1 = n1->GetObject<SCION>()->AddInterface(nd1);

    auto sif2 = n2->GetObject<SCION>()->AddInterface(nd2);

}


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
