#pragma force_active on
#include <string.h>

#include "global.h"

#include "dwc/gt_internal.h"

// The code below is compiled as ARM (32-bit) code.
#include <nitro/code32.h>

// Reliable UDP sockets: the protocol messages (reliable messages with their serial numbers, acks,
// pings, challenge / response), the connection table, socket and connection creation and teardown,
// raw sends, and the "host:port" address conversions.

typedef struct GTInAddr {
    u32 addr;
} GTInAddr;

typedef struct GTHostEnt {
    u8 unk_00[12];
    u32 **addresses; // 0x0C
} GTHostEnt;

typedef struct GTBindAddr {
    u16 port;
    u16 unk_02;
    int length;
    u32 host;
    GTSockAddr address;
} GTBindAddr;

extern const char sStr_gti2EndReliableMessage[]; // "gti2EndReliableMessage"
extern const u8 gti2ProtocolMagic[];   // 0xFE 0xFE, protocol magic
extern const char sStr_LenGt0_2[]; // "len > 0"
extern const char sStr_Gt2MessageC[]; // "gt2Message.c"
extern const char sStr_gt2StringToAddress[]; // "gt2StringToAddress"
extern const char sFmt_StrColonInt[]; // "%s:%d"
extern const char sFmt_Str_2[]; // "%s"
extern const char sFmt_ColonInt[]; // ":%d"
extern const char sStr_LenLtGTI2_STACK_HOSTLEN_MAX[]; // "len < GTI2_STACK_HOSTLEN_MAX"
extern const char sStr_Gt2UtilityC[]; // "gt2Utility.c"
extern int gti2AddressStringIndex;          // which of the two address string buffers is next
extern char gti2AddressStrings[];       // two address string buffers of 22 bytes
extern u16 __msl_digit[];

extern int sprintf(char *, const char *, ...);
extern long atol(const char *);
extern char *strchr(const char *, int);
extern void __msl_assertion_failed(const char *, const char *, const char *, int);
extern char *SOC_InetNtoA(GTInAddr addr);
extern GTHostEnt *SOC_GetHostByName(const char *);
extern void *DWCi_GsMalloc(u32 size);
extern void DWCi_GsFree(void *p);
extern int ArrayLength(void *);
extern void *ArrayNew(int, int, void *);
extern void ArrayFree(void *);
extern void *ArrayNth(void *, int);
extern int ArrayAppend(void *, void *);
extern void ArrayDeleteAt(void *, int);
extern void *TableNew2(int, int, int, void *, void *, void *);
extern void TableFree(void *);
extern void TableEnter(void *, void *);
extern void TableRemove(void *, void *);
extern void *TableLookup(void *, void *);
extern int TableMapSafe2(void *, void *, void *);
extern u32 current_time(void);
extern void SocketStartUp(void);
extern void SocketShutDown(void);
extern int CanSendOnSocket(int sock);
extern int gsSocketCreate(int, int, int);
extern void gsSocketClose(int);
extern int gsSocketBind(int, void *, int);
extern int gsSocketSendTo(int sock, const u8 *data, int len, int flags, GTSockAddr *to, int toLen);
extern int gsSocketGetSockName(int, void *, int *);
extern u32 gsSocketInetAddr(const char *);
extern int gsSocketGetLastError(int sock);
extern void gti2MessageCheck(const u8 **data, int *len);

int gti2AddOutgoingMessage(GTConn *conn, int serial, int len);
int gti2StartReliableMessage(GTConn *conn, int type, int len, int *overflow);
int gti2EndReliableMessage(GTConn *conn);
int gti2SendAppReliable(GTConn *conn, void *msg, int len);
int gti2SendClientChallenge(GTConn *conn, void *challenge);
int gti2SendServerChallenge(GTConn *conn, void *response, void *challenge);
int gti2SendClientResponse(GTConn *conn, void *response, void *msg, int len);
int gti2SendAccept(GTConn *conn);
int gti2SendReject(GTConn *conn, void *msg, int len);
int gti2SendClose(GTConn *conn);
int gti2SendKeepAlive(GTConn *conn);
int gti2SendAppUnreliable(GTConn *conn, u8 *msg, int len);
int gti2SendAck(GTConn *conn);
int gti2SendNack(GTConn *conn, u16 first, u16 last);
int gti2SendPong(GTConn *conn, u8 *msg, int len);
int gti2SendClosed(GTConn *conn);
int gti2SendClosedToAddress(GTSocket *socket, u32 ip, u16 port);
int gti2ResendMessage(GTConn *conn, GTOutMsg *msg);
int gti2Send(GTConn *conn, void *msg, int len, int reliable);
u32 gti2ConnectionHash(GTConn **conn, u32 numBuckets);
int gti2ConnectionCompare(GTConn **a, GTConn **b);
void gti2ConnectionFree(GTConn **conn);
GTConn *gti2SocketFindConnection(GTSocket *socket, u32 ip, int port);
int gti2CreateSocket(GTSocket **out, char *address, int outgoingBufferSize, int incomingBufferSize, void *socketErrorCB);
void gti2CloseSocket(GTSocket *socket);
void gti2Listen(GTSocket *socket, int (*connectAttemptCB)());
GTConn *gti2AllocConnection(void);
int gti2NewSocketConnection(GTSocket *socket, GTConn **out, u32 ip, int port);
void gti2FreeSocketConnection(GTConn *conn);
int gti2SocketSend(GTSocket *socket, u32 ip, u16 port, const u8 *data, int len);
int gti2SocketConnectionsThinkMap(GTConn **conn, u32 *now);
int gti2SocketConnectionsThink(GTSocket *socket);
void gti2FreeClosedConnections(GTSocket *socket);
void gti2SocketError(GTSocket *socket);
char *gt2AddressToString(u32 ip, u16 port, char *buf);
int gt2StringToAddress(const char *text, u32 *ip, u16 *port);

static inline u16 GTSwap16(u16 value) {
    int v = value;
    return ((v >> 8) & 255) | ((v << 8) & 0xff00);
}

void gti2Listen(GTSocket *socket, int (*connectAttemptCB)()) {
    socket->connectAttemptCB = connectAttemptCB;
}

static inline int GTIsDigit(int c) {
    return (c < 0 || c >= 128) ? 0 : (__msl_digit[c] & 8);
}
