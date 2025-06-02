#pragma once


#include "ns3/header.h"
#include "ns3/scion-path.h"
#include "ns3/address.h"
#include "ns3/scion-ia.h"
#include "ns3/scion-types.h"
#include <memory>


namespace ns3 
{

class SCIONAddress;


/* L3 header of a SCION packet.

*/
class SCIONHeader : public Header
{
public:
	~SCIONHeader(){};
	SCIONHeader()=default;

  /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;

    void Serialize(Buffer::Iterator start) const override;
	uint32_t Deserialize( Buffer::Iterator start) override;
	

	SCIONAddress SCIONSrcAddress() const;
	SCIONAddress SCIONDstAddress() const;

	Address SrcAddress() const;
	const Address& DstAddress() const;

	void SetSrcAddress( const SCIONAddress&);
	void SetDstAddress( const SCIONAddress&);
	void SetSrcAddress( const Address& );
	void SetDstAddress( const Address& );

	path_type_t GetPathType()const{return PathType; }
	std::shared_ptr<BasePath>& GetPath(){ return _path;}
	void SetPath( std::shared_ptr<BasePath> p){ _path = std::move(p); }

	Ia GetSrcIA()const {return SrcIA; }
	Ia GetDstIA() const {return DstIA; }
	void SetSrcIA( Ia ia ){ SrcIA = ia; }
	void SetDstIA( Ia ia ){ DstIA = ia; }

	uint8_t Version() const {return _version; }
	L4ProtocolType_t NextHeader()const {return _nextHdr;}
	void SetNextHeader( L4ProtocolType_t type ){ _nextHdr = type; }

	void SetPayloadLength( uint16_t paylen){ PayloadLen = paylen;}
	uint16_t GetPayloadLength()const{ return PayloadLen; }

	void SetVersion( uint8_t v){_version = v;}
	uint8_t GetVersion()const{return _version;}

	void SetTrafficClass( uint8_t tc ){ TrafficClass = tc;}
	uint8_t GetTrafficClass()const{ return TrafficClass;}

	AddrType_t GetDstAddrType() const {return dstAddrType;}
	AddrType_t GetSrcAddrType() const {return srcAddrType;}
    
	
private:


	//------- Common Header fields--------------------

	// Version is version of the SCION Header. Currently, only 0 is supported.
	uint8_t _version = 0;
	// TrafficClass denotes the traffic class. Its value in a received packet or fragment might be
	// different from the value sent by the packet’s source. The current use of the Traffic Class
	// field for Differentiated Services and Explicit Congestion Notification is specified in
	// RFC2474 and RFC3168
	uint8_t TrafficClass;
	// FlowID is a 20-bit field used by a source to label sequences of packets to be treated in the
	// network as a single flow. It is mandatory to be set.
	uint32_t FlowID ;
	// NextHdr  encodes the type of the first header after the SCION header.
	// This can be either a SCION extension or a layer-4 protocol such as TCP or UDP.
	// Values of this field respect and
	// extend IANA’s assigned internet protocol numbers.
	L4ProtocolType_t _nextHdr ;
	// HdrLen is the length of the SCION header in multiples of 4 bytes.
	// The SCION header length is computed as HdrLen * 4 bytes.
	// The 8 bits of the HdrLen field limit the SCION header to a
	// maximum of 255 * 4 == 1020 bytes.
	mutable uint8_t HdrLen ; //(mutable because it is computed in serialize() which is const )
	// PayloadLen is the length of the payload in bytes. The payload includes extension headers and
	// the L4 payload. This field is 16 bits long, supporting a maximum payload size of 64KB.
	uint16_t PayloadLen =0;
	// PathType specifies the type of path in this SCION header.
	path_type_t PathType ;
	// DstAddrType (4 bit) is the type/length of the destination address.
	AddrType_t dstAddrType ;
	// SrcAddrType (4 bit) is the type/length of the source address.
	AddrType_t srcAddrType ;

	// ------------ Address header fields. -------------

	// DstIA is the destination ISD-AS.
	Ia DstIA;
	// SrcIA is the source ISD-AS.
	Ia SrcIA;
	// RawDstAddr is the destination address.
    Address	RawDstAddr;
	// RawSrcAddr is the source address.
	Address RawSrcAddr;

	// Path is the path contained in the SCION header. It depends on the PathType field.
 	std::shared_ptr<BasePath> _path;

uint16_t PathHdrLen() const;
uint8_t AddrHdrLen() const;
void SerializeAddrHdr( Buffer::Iterator start ) const;
void DecodeAddrHdr( Buffer::Iterator start );
void DecodePathHdr(Buffer::Iterator start);

// uint32_t pseudoHeaderChecksum(int length,uint8_t protocol )const;



};

}  // ns3 namespace 
/*

// computeChecksum computes the checksum with the SCION pseudo header.
func (s *SCION) computeChecksum(upperLayer []byte, protocol uint8) (uint16, error) {
	if s == nil {
		return 0, serrors.New("SCION header missing")
	}
	csum, err := s.pseudoHeaderChecksum(len(upperLayer), protocol)
	if err != nil {
		return 0, err
	}
	csum = s.upperLayerChecksum(upperLayer, csum)
	folded := s.foldChecksum(csum)
	return folded, nil
}

func (s *SCION) upperLayerChecksum(upperLayer []byte, csum uint32) uint32 {
	// Compute safe boundary to ensure we do not access out of bounds.
	// Odd lengths are handled at the end.
	safeBoundary := len(upperLayer) - 1
	for i := 0; i < safeBoundary; i += 2 {
		csum += uint32(upperLayer[i]) << 8
		csum += uint32(upperLayer[i+1])
	}
	if len(upperLayer)%2 == 1 {
		csum += uint32(upperLayer[safeBoundary]) << 8
	}
	return csum
}

func (s *SCION) foldChecksum(csum uint32) uint16 {
	for csum > 0xffff {
		csum = (csum >> 16) + (csum & 0xffff)
	}
	return ^uint16(csum)
}
*/