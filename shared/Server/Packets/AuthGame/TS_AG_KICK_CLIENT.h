#ifndef PACKETS_TS_AG_KICK_CLIENT_H
#define PACKETS_TS_AG_KICK_CLIENT_H

#define TS_AG_KICK_CLIENT_DEF(_) _(string)(account, 61)
CREATE_PACKET(TS_AG_KICK_CLIENT, 20013);

/*
    enum KickType : char
    {
        KICK_TYPE_ANOTHER_LOGIN = 0x0,  //Gameguard kick ?
        KICK_TYPE_DUPLICATED_LOGIN = 0x1,
        KICK_TYPE_BILLING_EXPIRED = 0x2,
        KICK_TYPE_GAME_ADDICTION = 0x3
    } kick_type; //not in 5.2 gs
*/

#endif // PACKETS_TS_AG_KICK_CLIENT_H
