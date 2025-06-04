#include "ns3/scion-node.h"
#include "ns3/pointer.h"
#include "ns3/attribute-accessor-helper.h"

namespace ns3
{

    SCIONNode::SCIONNode()
    : Node()
    {}


    SCIONNode::SCIONNode(uint32_t systemId)
    : Node(systemId)
    {

    }


void
SCIONNode::SetIaAttribute(Ia ia)
{
    m_ia = ia;
}

Ia
SCIONNode::GetIaAttribute() const
{
 
    return m_ia;
}

    TypeId SCIONNode::GetInstanceTypeId() const
    {
        return GetTypeId();
    }

TypeId
SCIONNode::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::SCIONNode")
            .SetParent<Node>()
            .SetGroupName("Network")
            .AddConstructor<SCIONNode>()            
            .AddAttribute("Ia",
                          "The ISD-AS identifier (e.g., '19-ffaa:1:fe4').",            
                          TypeId::ATTR_GET | TypeId::ATTR_SET | TypeId::ATTR_CONSTRUCT,
                          IaValue(Ia()), // Or StringValue("0-0:0:0") if using StringAccessor            
                         //MakeAccessorHelper<Ia>(
                         MakeIaAccessor(&SCIONNode::SetIaAttribute,
                            &SCIONNode::GetIaAttribute),
                         // MakeIaAccessor(&SCIONNode::m_ia),
                        /// MakeIaValue(&SCIONNode::m_ia),
                          MakeIaChecker());
            
    return tid;
}

}
