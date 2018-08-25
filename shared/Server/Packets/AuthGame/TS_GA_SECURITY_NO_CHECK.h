#ifndef PACKETS_TS_GA_SECURITY_NO_CHECK_H
#define PACKETS_TS_GA_SECURITY_NO_CHECK_H
#pragma pack(push, 1)
struct TS_GA_SECURITY_NO_CHECK : public TS_MESSAGE
{
    char    account[61];
    char    security[19];
    int32_t mode; //since e6

    enum Mode
    {
        SC_NONE             = 0x0,
        SC_OPEN_STORAGE     = 0x1,
        SC_DELETE_CHARACTER = 0x2,
    };

    static const uint16_t packetID = 40001;
};

struct TS_GA_SECURITY_NO_CHECK_EPIC5 : public TS_MESSAGE
{
    char account[61];
    char security[19];

    static const uint16_t packetID = 40001;
};
#pragma pack(pop)

#endif // PACKETS_TS_GA_SECURITY_NO_CHECK_H
