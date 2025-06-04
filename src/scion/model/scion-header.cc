#include "ns3/scion-header.h"
#include "ns3/ipv6-address.h"
#include "ns3/ipv4-address.h"
#include "ns3/scion-address.h"



// #include "ns3/one-hop-path.h"

namespace ns3
{

void
SCIONHeader::SetDscp(DscpType dscp)
{
    //NS_LOG_FUNCTION(this << dscp);
    TrafficClass &= 0x3; // Clear out the DSCP part, retain 2 bits of ECN
    TrafficClass |= (dscp << 2);
}

    SCIONHeader::DscpType
SCIONHeader::GetDscp() const
{
    //NS_LOG_FUNCTION(this);
    // Extract only first 6 bits of TOS byte, i.e 0xFC
    return DscpType((TrafficClass & 0xFC) >> 2);
}

    void SCIONHeader::SetEcn(EcnType ecn)
    {
        //NS_LOG_FUNCTION(this << ecn);
        // Clear out the ECN part(least significant two bits), retain 6 bits of DSCP
        TrafficClass &= 0xFC;
        TrafficClass |= ecn;
    }

    /*
    decode the path header
    Only call after Common Headers has been decoded (and Path Length is known)
    */
    void SCIONHeader::DecodePathHdr(Buffer::Iterator start )
    {


	const uint32_t addrHdrB = AddrHdrLen();
	const uint32_t hdrBytes = HdrLen * LineLen;
    NS_ASSERT_MSG(CmnHdrLen + addrHdrB <= hdrBytes, "logic error");
	const uint32_t pathLen = hdrBytes - CmnHdrLen - addrHdrB;

	NS_ASSERT_MSG( pathLen >= 0,"invalid header, negative pathLen. HdrBytes: " <<
         hdrBytes << " AddrHdrBytes: " << addrHdrB << " CommonHdrBytes: " << CmnHdrLen );
    
    
	//if(  uint32_t minLen = addrHdrB + CmnHdrLen + pathLen; start.GetRemainingSize() < minLen ) // hier muss es size() sein, anstelle von GetRemainingSize()
    NS_ASSERT_MSG( start.GetRemainingSize() >= pathLen, "provided buffer is too small - have: "
     <<  start.GetRemainingSize() << " expected: " << pathLen);
    
    // Decode path header.
    _path = _path ? _path : std::make_shared<SCIONPath>(); // FIXME use RawPath
    /*
    switch(PathType)
    {
 
    case path_type_t::DecodedPath:
    _path = SCIONPath();
    case path_type_t::OneHopPath:

    case path_type_t::EmptyPath:

    case path_type_t::RawPath:   // <= probably this should be used only
    default:
    _path = RawPath();
    break;
    };*/


    if(pathLen>0)
        auto tmp = _path->Deserialize(start);
    }

    // returns the length of the path header in serialized wire form
    uint16_t SCIONHeader::PathHdrLen()const 
    {
        return _path ? _path->Len() : 0;
    }

// AddrHdrLen returns the length of the address header (destination and source ISD-AS-Host triples)
// in bytes.
uint8_t SCIONHeader::AddrHdrLen() const{
	 return 2*sizeof(Ia_t) + AddrTypeLength( dstAddrType ) + AddrTypeLength( srcAddrType );
	// return 2*IABYTES + RawDstAddr.GetLength() + RawSrcAddr.GetLength();
}


TypeId
SCIONHeader::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SCIONHeader")
                            .SetParent<Header>()
                            .SetGroupName("Internet")
                            .AddConstructor<SCIONHeader>();
    return tid;
}

TypeId
SCIONHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

void
SCIONHeader::Print([[maybe_unused]] std::ostream& os) const
{
}

uint32_t
SCIONHeader::GetSerializedSize() const
{
    auto scnLen = CmnHdrLen + AddrHdrLen() + PathHdrLen();
    NS_ASSERT_MSG(scnLen < MaxHdrLen, "header length exceeds maximum");
    
    NS_ASSERT_MSG((scnLen % LineLen) == 0, "header length is not an integer multiple of line length");
    

    HdrLen = uint8_t(scnLen / LineLen);
    return HdrLen * LineLen;
}


SCIONAddress
SCIONHeader::SCIONSrcAddress() const
{

	Address hostAddr;
	if( srcAddrType  == AddrType_t::T4Ip)
	{
		hostAddr = Ipv4Address::ConvertFrom( RawSrcAddr);
	}
	else if ( srcAddrType == AddrType_t::T16Ip )
	{
		hostAddr = Ipv6Address::ConvertFrom( RawSrcAddr );
	}

    SCIONAddress src(SrcIA, hostAddr);
	
    return src;
}

/*!
 \returns raw L3 Ipv4/6 Address without port
*/
Address
SCIONHeader::SrcAddress() const
{
    /*if( srcAddrType == AddrType_t::T4Ip )
    {
       InetSocketAddress src{ Ipv4Address::ConvertFrom(RawSrcAddr) };
       src.SetPort( )
       return
    }*/

    return RawSrcAddr;
}

/*!
 \returns raw L3 Ipv4/6 Address without port
*/
const Address&
SCIONHeader::DstAddress() const
{
    return RawDstAddr;
}

SCIONAddress
SCIONHeader::SCIONDstAddress() const
{
Address hostAddr;
	if(dstAddrType == AddrType_t::T4Ip)
	{
		hostAddr = Ipv4Address::ConvertFrom( RawDstAddr);
	}
	else if ( dstAddrType == AddrType_t::T16Ip )
	{
		hostAddr = Ipv6Address::ConvertFrom( RawDstAddr );
	}
    // TODO: Service Address

    SCIONAddress dst(DstIA, hostAddr);
    return dst;
}

void
SCIONHeader::Serialize(Buffer::Iterator start) const
{
  
	auto iter_to_begin = start; // only for debug assertions
    auto iter = start;

    uint32_t scnLen = CmnHdrLen + AddrHdrLen() + PathHdrLen();
    NS_ASSERT_MSG(scnLen < MaxHdrLen, "The ScionHeader must not exceede a max of " << MaxHdrLen << " bytes");
    
    NS_ASSERT_MSG((scnLen % LineLen) == 0, 
    "The ScionHeader length must be a multiple of 4 bytes but is: "<< scnLen);    

    NS_ASSERT_MSG( iter.GetRemainingSize() >= scnLen,
     "Not enough buffer space to serialize SCIONHeader");

    
    HdrLen = uint8_t(scnLen / LineLen);
    // PayloadLen = uint16_t(len(b.Bytes()) - scnLen);

    // Serialize common header.
    uint32_t firstLine =
        uint32_t(_version & 0xF) << 28 | (uint32_t(TrafficClass) << 20) | (FlowID & 0xFFFFF);
    iter.WriteHtonU32(firstLine);
    iter.WriteU8(_nextHdr);
    iter.WriteU8(HdrLen);
    iter.WriteHtonU16(PayloadLen);
    iter.WriteU8(static_cast<uint8_t>(PathType)); // 8
    iter.WriteU8((uint8_t(dstAddrType & 0xF) << 4) | uint8_t(srcAddrType & 0xF));
    iter.WriteU16(0);   

	NS_ASSERT_MSG( iter.GetDistanceFrom(iter_to_begin) == CmnHdrLen ,
	 "Buffer Iter Position should be CmHdrLen after serialization of CommonHeader");

    // Serialize address header.
    SerializeAddrHdr(iter);
    iter.Next( AddrHdrLen());

	auto actualCmnAddr = iter.GetDistanceFrom(iter_to_begin);
	auto expectedCmnAddr =  uint32_t( CmnHdrLen + AddrHdrLen() );
	NS_ASSERT_MSG( (actualCmnAddr == expectedCmnAddr),
	 "The ScionHeader size does not match expectations after serializing Common and Address Headers: was " << actualCmnAddr << " expected: "<< expectedCmnAddr );
    

    // Serialize path header.
    if(_path)
        _path->Serialize(iter);
        iter.Next( PathHdrLen());

	NS_ASSERT_MSG( iter.GetDistanceFrom(iter_to_begin) == scnLen,
	 "Actual ScionHeader size diverges from computed size" );

    
}

/* this is to remain compatibility with Add/Remove Header */
uint32_t
SCIONHeader::Deserialize( Buffer::Iterator start)
{
	
    auto m_content { start};

    // Decode common header.

    NS_ASSERT_MSG( !(start.GetRemainingSize() < CmnHdrLen ), "packet is shorter than the common header length");  

    auto firstLine = start.ReadNtohU32(); // binary.BigEndian.Uint32(data[:4])
    _version = uint8_t(firstLine >> 28);
    TrafficClass = uint8_t((firstLine >> 20) & 0xFF);
    FlowID = firstLine & 0xFFFFF;

    _nextHdr = L4ProtocolType_t(start.ReadU8());
    HdrLen = start.ReadU8();
    PayloadLen = start.ReadNtohU16();                   // binary.BigEndian.Uint16(data[6:8])
    PathType = static_cast<path_type_t>(start.ReadU8()); // path.Type(data[8])
    auto byte9 = start.ReadU8();
    dstAddrType = AddrType_t(byte9 >> 4 & 0xF);
    srcAddrType = AddrType_t(byte9 & 0xF);
	start.Next(2); // skip reserved Uint16 zero bits
    NS_ASSERT_MSG(start.GetDistanceFrom(m_content) == CmnHdrLen, "CmnHeader size mismatch on Deserialization");
    

    // Decode address header.
   DecodeAddrHdr(start);   
   start.Next(AddrHdrLen());   
	
    DecodePathHdr(start);    
  //  start.Next(PathHdrLen());

    
	// 'start' this iterator is advanced by HdrLen*LineLen bytes compared to m_content
	// this is where the NextHeader starts

    
    return start.GetDistanceFrom(m_content);
}

/*
uint32_t
SCIONHeader::pseudoHeaderChecksum(int length, uint8_t protocol) const
{
    NS_ASSERT_MSG(!RawSrcAddr.IsInvalid(), "Src Address missing from header");
    NS_ASSERT_MSG(!RawDstAddr.IsInvalid(), "Dst Address missing from header");
     
    uint32_t csum;
    uint8_t* srcIA;
    uint8_t* dstIA;
    srcIA = (uint8_t*)malloc(8);
    dstIA = (uint8_t*)malloc(8);
    BigEndian::PutUint64(srcIA, uint64_t(SrcIA.GetValue()));
    BigEndian::PutUint64(dstIA, uint64_t(DstIA.GetValue()));
    for (auto i = 0; i < 8; i += 2)
    {
        csum += uint32_t(srcIA[i]) << 8;
        csum += uint32_t(srcIA[i + 1]);
        csum += uint32_t(dstIA[i]) << 8;
        csum += uint32_t(dstIA[i + 1]);
    }
    free(srcIA);
    free(dstIA);

    uint8_t* src_addr_buf = (uint8_t*)malloc(RawSrcAddr.GetLength());
    uint8_t* dst_addr_buf = (uint8_t*)malloc(RawDstAddr.GetLength());

    RawSrcAddr.CopyTo(src_addr_buf);
    RawDstAddr.CopyTo(dst_addr_buf);

    // Address length is guaranteed to be a multiple of 2 by the protocol.
    for (auto i = 0; i < RawSrcAddr.GetLength(); i += 2)
    {
        csum += uint32_t(src_addr_buf[i]) << 8;
        csum += uint32_t(src_addr_buf[i + 1]);
    }
    for (auto i = 0; i < RawDstAddr.GetLength(); i += 2)
    {
        csum += uint32_t(dst_addr_buf[i]) << 8;
        csum += uint32_t(dst_addr_buf[i + 1]);
    }
    auto l = uint32_t(length);
    csum += (l >> 16) + (l & 0xffff);
    csum += uint32_t(protocol);

    free(src_addr_buf);
    free(dst_addr_buf);
    return csum;
}
    */

/* DecodeAddrHdr decodes the destination and source ISD-AS-Host address triples from the provided
 buffer. The caller must ensure that the correct address types and lengths are set in the SCION
 layer, otherwise the results of this method are undefined.*/
void
SCIONHeader::DecodeAddrHdr( Buffer::Iterator start)
{
    NS_ASSERT_MSG(!( start.GetRemainingSize() < AddrHdrLen() ), "provided buffer is too small to decode AddressHeader");  	
     
    DstIA = Ia(start.ReadNtohU64());
    SrcIA = Ia(start.ReadNtohU64());

    // the addressTypes of both Src and Dst have already been parsed at this point
    auto dstAddrBytes = AddrTypeLength(dstAddrType);
    auto srcAddrBytes = AddrTypeLength(srcAddrType);

    // RawDstAddr = Address(uint8_t type, const uint8_t* buffer, uint8_t len);
    // RawDstAddr = data[offset : offset+dstAddrBytes]
    uint8_t* tmp = (uint8_t*)malloc(dstAddrBytes);
    start.Read(tmp, dstAddrBytes);
    auto bytesRead = RawDstAddr.CopyFrom(tmp, dstAddrBytes);
    NS_ASSERT(bytesRead == uint32_t(dstAddrBytes));

    tmp = (uint8_t*)realloc((void*)tmp, srcAddrBytes);
    start.Read(tmp, srcAddrBytes);
    bytesRead = RawSrcAddr.CopyFrom(tmp, srcAddrBytes);
    NS_ASSERT(bytesRead == uint32_t(srcAddrBytes));
    // RawSrcAddr = data[offset : offset+srcAddrBytes]

    free(tmp);
    // return start;
}

void
SCIONHeader::SetDstAddress(const SCIONAddress& dst)
{
    DstIA = dst.GetIA();
    RawDstAddr = dst.GetHostAddress();
	dstAddrType = ConvAddrType(RawDstAddr);
	NS_ASSERT( (dstAddrType == AddrType_t::T4Ip) || (dstAddrType == AddrType_t::T16Ip) ); //TODO Service Address
}

void
SCIONHeader::SetSrcAddress(const Address& src)
{
    RawSrcAddr = src;
    srcAddrType = ConvAddrType(RawSrcAddr);
}

void
SCIONHeader::SetDstAddress(const Address& dst)
{
    RawDstAddr = dst;
    dstAddrType = ConvAddrType(dst);
}

void
SCIONHeader::SetSrcAddress(const SCIONAddress& src)
{
    SrcIA = src.GetIA();
    RawSrcAddr = src.GetHostAddress();
    srcAddrType = ConvAddrType(RawSrcAddr);
}

/* SerializeAddrHdr serializes destination and source ISD-AS-Host address triples into the provided
 buffer. The caller must ensure that the correct address types and lengths are set in the SCION
 layer, otherwise the results of this method are undefined. */
void
SCIONHeader::SerializeAddrHdr( Buffer::Iterator start) const
{
    const auto expected = static_cast<std::ptrdiff_t>(2*sizeof(Ia_t) + RawDstAddr.GetLength() + RawSrcAddr.GetLength() ); // TODO use AddrHeaderLen()
    NS_ASSERT_MSG( static_cast<std::ptrdiff_t>(start.GetRemainingSize() ) >= expected, "Not enough buffer space to serialize AddressHeader" );
	auto iter_begin = start;

    start.WriteHtonU64(DstIA.GetValue());
    start.WriteHtonU64(SrcIA.GetValue());

    /*
	NS_ASSERT_MSG( AddrTypeConv(dstAddrType) == RawDstAddr.GetType() ,
	 "logic error expected: " << std::to_string(dstAddrType) << " conv: "
	  << std::to_string( AddrTypeConv(dstAddrType) ) 
	  << " actual: " << std::to_string( RawDstAddr.GetType() ) );

	NS_ASSERT_MSG( AddrTypeConv(srcAddrType) == RawSrcAddr.GetType() , "logic error" );*/

	NS_ASSERT_MSG( AddrTypeLength(dstAddrType) == RawDstAddr.GetLength(),
	 "Expected: " << AddrTypeLength(dstAddrType)<< " got: " << std::to_string( RawDstAddr.GetLength() ) );

	uint8_t* buffer =(uint8_t*) malloc( AddrTypeLength(dstAddrType) );
	RawDstAddr.CopyTo(buffer);
	start.Write(buffer, RawDstAddr.GetLength( )) ;
    // WriteTo(start, RawDstAddr);

//    WriteTo(start, RawSrcAddr);

    NS_ASSERT( AddrTypeLength(srcAddrType) == RawSrcAddr.GetLength() );
	uint8_t* buffer2 =(uint8_t*) malloc( AddrTypeLength(srcAddrType) );
	RawSrcAddr.CopyTo(buffer2);
	start.Write(buffer2, RawSrcAddr.GetLength( )) ;
	free(buffer);
	free(buffer2);

	// WriteTo does NOT serialize the Addresses type, but only the internal mac array
	// (writes Address::GetLength() bytes not Address::GetSerializedSize() )
    
    //auto actual = iter_begin.distance_to( start );
    auto actual = start.GetDistanceFrom(iter_begin);
    NS_ASSERT_MSG( actual == expected
    ,"size mismatch in AddressHeader: actual:" << std::to_string(actual)
    << " expected: " << std::to_string( expected ) 
    );

    //return start.GetDistanceFrom(iter_begin);
}

} // namespace ns3