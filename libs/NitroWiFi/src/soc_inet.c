#include "nitrowifi/socl_internal.h"

// SOC_* interface, second part: socket name, host address, listen / accept, fcntl, start-up with the
// caller's allocator, address conversion and polling.

#pragma force_active on


u32 SOC_GetHostID(void) {
    return Socl_Swap32(SOCL_GetHostID());
}

int SOC_SetResolver(u32 *primary, u32 *secondary) {
    return SOCL_SetResolver(Socl_Swap32(*primary), Socl_Swap32(*secondary));
}

// 3: get flags (4: non-blocking), 4: set flags
int SOC_Fcntl(int socket, int command, int flags) {
    SoclSocket *soclSocket = (SoclSocket *)socket;
    if (!socket) {
        return -1;
    }
    switch (command) {
    case 3:
        return soclSocket->blocking == 1 ? 0 : 4;
    case 4:
        if (flags & 4) {
            soclSocket->blocking = 0;
        } else {
            soclSocket->blocking = 1;
        }
        break;
    }
    return 0;
}

// SOCL allocator on top of the caller's: the block size is kept in front of the block.
void *AllocFunc_SOCL(u32 size) {
    u32 total = size + 4;
    u32 *block = gSocState.alloc(0, total);
    if (block) {
        *block = total;
        block++;
    }
    return block;
}

void FreeFunc_SOCL(u32 *block) {
    if (block) {
        gSocState.free(0, block - 1, block[-1]);
    }
}

#define SOC_STATICS (*(SocStatics *)&gSocState)

// Builds the SOCL configuration from the caller's (addresses in network byte order).
int SOC_Startup(SocConfig *config) {
    SOC_STATICS.soclConfig.useDhcp = config->mode == 1;
    SOC_STATICS.soclConfig.hostIp = Socl_Swap32(config->hostIp);
    SOC_STATICS.soclConfig.netmask = Socl_Swap32(config->netmask);
    SOC_STATICS.soclConfig.gateway = Socl_Swap32(config->gateway);
    SOC_STATICS.soclConfig.dns[0] = Socl_Swap32(config->dns[0]);
    SOC_STATICS.soclConfig.dns[1] = Socl_Swap32(config->dns[1]);
    SOC_STATICS.soclConfig.alloc = AllocFunc_SOCL;
    SOC_STATICS.soclConfig.free = (void (*)(void *))FreeFunc_SOCL;
    SOC_STATICS.state.alloc = config->alloc;
    SOC_STATICS.state.free = config->free;
    SOC_STATICS.soclConfig.commandPacketCount = 64;
    SOC_STATICS.soclConfig.mtu = config->mtu;
    SOC_STATICS.soclConfig.recvBufSize = config->recvBufSize;
    return SOCL_Startup(&gSocSoclConfig);
}

int SOC_Cleanup(void) {
    return SOCL_Cleanup();
}

int SOC_InetAtoN(const char *name, u32 *ip) {
    u32 hostIp = SOCL_InetAtoH(name);
    if (hostIp == 0) {
        return 0;
    }
    *ip = Socl_Swap32(hostIp);
    return 1;
}

void SOC_U32to4U8(u32 value, u8 *bytes) {
    bytes[0] = value >> 24;
    bytes[1] = value >> 16;
    bytes[2] = value >> 8;
    bytes[3] = value;
}

// Polls the sockets' status until one has an event or the timeout (OS ticks, -1: none) has passed.
int SOC_Poll(SocPollFd *fds, u32 count, s64 timeout) {
    int finite = timeout != -1;
    SocPollFd *fd;
    u32 i;
    int hits;
    for (;;) {
        hits = 0;
        fd = fds;
        for (i = 0; i < count;) {
            int mask = fd->events | 0xE0;
            int status = SOCL_GetStatus(fd->fd) & mask;
            fd->revents = status;
            i++;
            fd++;
            if (status) {
                hits++;
            }
        }
        if (hits > 0 || (finite && timeout <= 0)) {
            break;
        }
        OS_Sleep(1);
        timeout -= 523; // ticks per millisecond
    }
    return hits;
}
