#ifndef TINYINT_H
#define TINYINT_H

#include "includes.h"

struct TinyInt
{
private:
    uint8_t data[3];

public:

    TinyInt(uint32_t val)
    {
        data[0] = (val >> 16) & 0xFF;
        data[1] = (val >> 8) & 0xFF;
        data[2] = val & 0xFF;
    }

    bool operator==(TinyInt other) const
    {
        return (data[0] == other.data[0]) && (data[1] == other.data[1]) && (data[2] == other.data[2]);
    }

    bool operator!=(TinyInt other) const
    {
        return (*this == other);
    }

    uint ToUint() const
    {
        return (data[0] << 16) | (data[1] << 8) | data[2];
    }
};

inline uint qHash(const TinyInt & val)
{
    return qHash(val.ToUint());
}

#endif // TINYINT_H
