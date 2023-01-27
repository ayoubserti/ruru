#ifndef __H_RURU_HH__
#define __H_RURU_HH__

namespace ruru
{
    // define the record ID type
    typedef uint64_t RecordId;

    // defines
    typedef int64_t RecordLength_t;
    typedef int64_t RecordPosition_t;

    // List of DataTypes
    enum class DataTypes : uint8_t
    {
        eUnknown = 0,
        eInteger = 1,
        eDouble,
        eVarChar,
        eBinary
    };

    union UnionValues
    {
        uint64_t nbr;
        double floating;
        char *txt;
    };

    static const char* db_extension = ".ru";
}

#endif //__H_RURU_HH__