#include "dccl/codec.h"

namespace moossafir
{
    class IDCodec : public dccl::TypedFixedFieldCodec<dccl::uint32>
    {
    private:
        dccl::Bitset encode(const dccl::uint32& wire_value)
            { return dccl::Bitset(MINI_ID_SIZE, wire_value - MINI_ID_OFFSET); }
    
        dccl::Bitset encode()
            { return encode(MINI_ID_OFFSET); }
    
        dccl::uint32 decode(dccl::Bitset* bits)
            { return bits->to_ulong() + MINI_ID_OFFSET; }
    
        unsigned size()
            { return MINI_ID_SIZE; }
    
        void validate()
            { }
    

        // Add this value when decoding to put us safely in our own namespace
        // from the normal default DCCL Codec
        enum { MINI_ID_OFFSET = 2000000 };    
        enum { MINI_ID_SIZE = 3 };
    };

    class CodecLoader
    {
    public:
        CodecLoader()
            { dccl::FieldCodecManager::add<moossafir::IDCodec>("moossafir.id"); }
        ~CodecLoader()
            { dccl::FieldCodecManager::remove<moossafir::IDCodec>("moossafir.id"); }
    };
}

moossafir::CodecLoader loader;
