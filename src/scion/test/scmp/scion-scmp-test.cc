#include "scion-scmp-test.h"
#include "ns3/scmp-header.h"
#include "ns3/scmp-destination-unreachable.h"
#include "ns3/udp-header.h"
#include "ns3/packet.h"
#include "ns3/scion-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/test.h"

using namespace ns3;


SCIONSCMPTestCase::SCIONSCMPTestCase()
    : TestCase("Verify SCMP")
{
}

SCIONSCMPTestCase::~SCIONSCMPTestCase()
{
}

void SCIONSCMPTestCase::Test00()
{
    
    {
        Buffer buff;
        SCMPTypeCode scmp(SCMP::TypeDestinationUnreachable,
                                 SCMP::CodeRejectRouteToDest);
        buff.AddAtStart(scmp.GetSerializedSize());
        scmp.Serialize(buff.Begin());
        SCMPTypeCode scmp2;
        scmp2.Deserialize(buff.Begin());
        NS_TEST_ASSERT_MSG_EQ(scmp, scmp2, "");
    }

    using namespace std::literals;
    auto payload = "<payload>I am a small string payload!</payload>"sv;

    auto pkt = Create<Packet>((uint8_t*)payload.data(), payload.size());

        
    UdpHeader udp;
    udp.SetSourcePort( 3041);
    udp.SetDestinationPort(53);

    pkt->AddHeader(udp);

    SCMPDestinationUnreachable dstnreach;

    pkt->AddHeader(dstnreach);

    SCMPHeader scmp{SCMPTypeCode(SCMP::TypeDestinationUnreachable,
                                 SCMP::CodeRejectRouteToDest)};
    pkt->AddHeader(scmp);
    
    SCIONAddress a0{"19-ffaa:0:1067,127.0.0.1"};    
    SCIONAddress a1{"17-ffaa:1:fe4,[2001:db8:85a3:8d3:1319:8a2e:370:7348]"};  
    
    SCIONHeader scion;
    //scion.SetPath( rawTestPath);
    scion.SetDstAddress(a1);
    scion.SetSrcAddress(a0);

    scion.SetNextHeader(L4ProtocolType_t::L4SCMP );
    scion.SetPayloadLength( payload.size()+udp.GetSerializedSize() 
                            + scmp.GetSerializedSize() 
                            + dstnreach.GetSerializedSize() );


    Ipv4Header ipv4;
    ipv4.SetSource(Ipv4Address("127.0.0.1"));
    ipv4.SetDestination(Ipv4Address("192.168.2.1")); // underlay next hop

    pkt->AddHeader(scion);
    pkt->AddHeader(ipv4);

    Ipv4Header ip2;

    NS_TEST_ASSERT_MSG_EQ(pkt->RemoveHeader(ip2), ipv4.GetSerializedSize(), "");
    NS_TEST_ASSERT_MSG_EQ(ip2.GetSource(), ipv4.GetSource(), "");
    NS_TEST_ASSERT_MSG_EQ(ip2.GetDestination(), ipv4.GetDestination(), "");

    SCIONHeader scion2;
    NS_TEST_ASSERT_MSG_EQ( pkt->RemoveHeader(scion2), scion.GetSerializedSize(),"Decoding SCION Header");
    //NS_TEST_ASSERT_MSG_EQ(scion2.)

    SCMPHeader scmp2;
    NS_TEST_ASSERT_MSG_EQ(scion2.NextHeader(), L4SCMP, "");
    NS_TEST_ASSERT_MSG_EQ( pkt->RemoveHeader(scmp2), scmp.GetSerializedSize(),"");

    auto c2{scmp2.TypeCode()};
    auto c{ scmp.TypeCode()};
    NS_TEST_ASSERT_MSG_EQ(c2,c,"SCMP serialization error");
 
   SCMPDestinationUnreachable dst2;
   NS_TEST_ASSERT_MSG_EQ(pkt->RemoveHeader(dst2),dstnreach.GetSerializedSize(),
    "Decoding SCMP destination unreachable");

   UdpHeader udp2;
   NS_TEST_ASSERT_MSG_EQ( pkt->RemoveHeader(udp2), udp.GetSerializedSize(), "decoding UDP");

}

void SCIONSCMPTestCase::DoRun()
{
Test00();
}