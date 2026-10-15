#pragma force_active on
#include "global.h"

#include "dwc/gt_internal.h"

// The code below is compiled as ARM (32-bit) code.
#include <nitro/code32.h>

// Reliable UDP sockets: the callbacks into the user code (connected, received, closed, ping, filters,
// unrecognized messages), creating / closing sockets and connections, sending, and processing the
// received messages (serial numbers, acknowledgements, buffered out-of-order messages, keep-alives).

extern char sStr_gti2DumpCallback[]; // "gti2DumpCallback"
extern char sStr_gti2PingCallback[]; // "gti2PingCallback"
extern char sStr_gti2ClosedCallback[]; // "gti2ClosedCallback"
extern char sStr_gti2ReceivedCallback[]; // "gti2ReceivedCallback"
extern char sStr_gti2ConnectedCallback[]; // "gti2ConnectedCallback"
extern char sStr_gti2SendFilterCallback[]; // "gti2SendFilterCallback"
extern char sStr_gti2ReceiveFilterCallback[]; // "gti2ReceiveFilterCallback"
extern char sStr_gti2UnrecognizedMessageCallback[]; // "gti2UnrecognizedMessageCallback"
extern char sStr_socket[]; // "socket"
extern char sStr_Gt2CallbackC[]; // "gt2Callback.c"
extern char sStr_connection[]; // "connection"
extern char sStr_gt2SetConnectionData[]; // "gt2SetConnectionData"
extern char sStr_gt2GetConnectionData[]; // "gt2GetConnectionData"
extern char sStr_Gt2MainC[]; // "gt2Main.c"
extern char sStr_connection_2[]; // "connection"
extern char sStr_time_2[]; // "time"
extern char gti2ProtocolMagic[]; // 0xFE 0xFE
extern int gti2ClosingSocket;    // set while a socket is being closed

extern void __msl_assertion_failed(const char *expr, const char *file, const char *func, int line);
extern void *memcpy(void *, const void *, u32);
extern int memcmp(const void *, const void *, u32);
extern void gti2CloseSocket(GTSocket *socket);
extern void *ArrayNth(void *array, int n);
extern int ArrayLength(void *array);
extern void ArrayDeleteAt(void *array, int n);
extern void ArrayFree(void *array);
extern void ArrayAppend(void *array, void *elem);
extern void ArrayInsertSorted(void *array, void *elem, void *cmp);
extern void TableRemove(void *table, void *elem);
extern void TableMapSafe(void *table, void *fn, void *user);
extern u32 current_time(void);
extern void msleep(int ms);
extern int CanReceiveOnSocket(int sock);
extern int gsSocketRecvFrom(int sock, void *buf, int len, int flags, void *addr, int *addrlen);
extern int gsSocketGetLastError(int sock);
extern void *DWCi_GsMalloc(int size);
extern void DWCi_GsFree(void *ptr);
extern int gti2NewSocketConnection(GTSocket *socket, GTConn **conn, u32 ip, u16 port);
extern void gti2MessageCheck(u8 **msg, int *len);
extern int gti2ConnectAttemptCallback(GTSocket *socket, GTConn *conn, u32 ip, u16 port, int latency, u8 *msg, int len);
extern int gti2SendClientChallenge(GTConn *conn, u8 *challenge);
extern int gti2SendServerChallenge(GTConn *conn, u8 *response, u8 *challenge);
extern int gti2SendClientResponse(GTConn *conn, u8 *response, u8 *msg, int len);
extern int gti2SendAccept(GTConn *conn);
extern int gti2SendReject(GTConn *conn, u8 *msg, int len);
extern int gti2SendClose(GTConn *conn);
extern int gti2SendKeepAlive(GTConn *conn);
extern int gti2SendAck(GTConn *conn);
extern int gti2SendNack(GTConn *conn, u16 a, u16 b);
extern int gti2SendPong(GTConn *conn, u8 *msg, int len);
extern int gti2SendClosed(GTConn *conn);
extern int gti2SendClosedToAddress(GTSocket *socket, u32 ip, u16 port);
extern int gti2ResendMessage(GTConn *conn, GTOutMsg *msg);
extern int gti2Send(GTConn *conn, u8 *msg, int len, int reliable);
extern GTConn *gti2SocketFindConnection(GTSocket *socket, u32 ip, u16 port);
extern int gti2CreateSocket(void *a, void *b, void *c, void *d, void *e);
extern int gti2Listen(void *a, void *b, void *c, void *d);
extern void gti2FreeSocketConnection(GTConn *conn);
extern int gti2SocketSend(GTSocket *socket, u32 ip, u16 port, u8 *msg, int len);
extern int gti2SocketConnectionsThink(GTSocket *socket);
extern void gti2FreeClosedConnections(GTSocket *socket);
extern void gti2SocketError(GTSocket *socket);
extern int gt2StringToAddress(const char *addr, u32 *ip, u16 *port);

void gt2SetUnrecognizedMessageCallback(GTSocket *socket, GTUnrecCB cb) {
    socket->unrecognizedCB = cb;
}
