#pragma once

#include "ns3/hop-field.h"
#include "ns3/info-field.h"

namespace ns3 
{
	// MaxINFs is the maximum number of info fields in a SCION path.
#define	MaxINFs  3
	// MaxHops is the maximum number of hop fields in a SCION path.
#define	MaxHops 64

enum class path_type_t : uint8_t
{
    EmptyPath,
    RawPath,  // corresponds to scion.PathType ('1')
    OneHopPath,
    EpicPath, 
    DecodedPath

};

/**
 * a combination of Base (GoPacket Layer) and MetaHeader in scionproto
 */
class BasePath
{
public:
   virtual void Serialize( Buffer::Iterator start ) const;
    virtual uint32_t Deserialize(Buffer::Iterator);
    uint16_t Len() const;
    bool IsXover() const;
    bool IsPenultimateHop()const;
    bool IsFirstHop()const;
    bool IsLastHop()const;
    bool IsFirstHopAfterXover() const;
    bool IncPath();
    static constexpr uint32_t GetSerializedSize();
    auto GetCurrINF()const{return CurrINF;}
	auto GetCurrHF()const{return CurrHF;};
    auto GetNumINF()const{return NumINF;}
    auto GetNumHF()const{return NumHops;}
    virtual BasePath& Reverse();
    virtual path_type_t Type() const = 0;
    InfoField GetCurrentInfoField()const;
    HopField GetCurrentHopField() const;
    virtual InfoField GetInfoField( int index) const = 0;
    virtual HopField GetHopField(int index ) const = 0;
private:
    uint8_t infIndexForHF(uint8_t hf)const;
    // TODO: could both the Curr_ fields be implemented with PacketTags which are appended to a Packet's PacketTagList ?!
    // to benefit from CopyOnWrite semantics
    uint8_t CurrINF;
    uint8_t CurrHF;
    uint8_t SegLen[3];

    // theese two fields are derived from the metaHeader 
    // and not present in the binary representation
    // NumINF is the number of InfoFields in the path.
    uint8_t NumINF;
    // NumHops is the number HopFields in the path.
    uint8_t NumHops; // its type matches MetaHdr::CurrtHF
};

/*
// TODO: implement me
class RawPath : public BasePath {
    public:
    virtual void Serialize( Buffer::Iterator start ) const override;
    virtual uint32_t Deserialize(Buffer::Iterator) override;
    virtual BasePath& Reverse() override;
    virtual InfoField GetInfoField( int index) const override;
    virtual HopField GetHopField(int index ) const override;
    virtual path_type_t Type() const override {return path_type_t::RawPath;}

};
*/

/**
 * a SCION dataplane path assembled from Hop & Info Fields 
 * through path-combination process, potentially out of multiple path-segments.
 */
class SCIONPath : public BasePath
{
	
	// InfoFields contains all the InfoFields of the path.
	std::vector< InfoField > InfoFields;
	// HopFields contains all the HopFields of the path.
	std::vector<HopField> HopFields;

public:
	
    virtual InfoField GetInfoField( int index) const override;
    virtual HopField GetHopField(int index ) const override;

  //  auto operator<=>(const SCIONPath& other )const =default;

    virtual uint32_t Deserialize( Buffer::Iterator ) override; 
    virtual void Serialize( Buffer::Iterator ) const override;

   	virtual path_type_t Type() const override { return path_type_t::DecodedPath; };
   

    // Reverse reverses a SCION path in-place.
    virtual SCIONPath& Reverse() override;

	

	friend std::ostream& operator<<( std::ostream&, const SCIONPath& );
};

 std::ostream&
operator<<(std::ostream& os, const SCIONPath& dp);

}// namespace ns3