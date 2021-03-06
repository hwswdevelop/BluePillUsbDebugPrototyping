#include "stm32DbgCommands.h"
#include <cstring>

static constexpr const size_t usbEpBufferSize = 64;

extern "C" size_t usbSendAndReceive(uint8_t* outBuf, size_t outBufSize, uint8_t* inBuf, size_t inBufSize);

extern "C" size_t stm32MemRead(uint32_t address, uint8_t * data, size_t size) {
#if 0
    uint32_t offset = 0;
    uint8_t* dptr = (uint8_t*)data;
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];

    while (offset < size) {
        size_t remain = size - offset;
        size_t txSize = readMemCmd(address + offset, remain, out);
        size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
        if (rxSize < sizeof(struct MemReadWriteAnsHead)) return offset;
        struct MemReadWriteAnsHead* rxHead = (struct MemReadWriteAnsHead*)out;
        if (rxHead->cmd != DBG_CMD_MEM_READ) return offset;
        memcpy(dptr + offset, &in[sizeof(struct MemReadWriteAnsHead)], rxHead->byteCount);
        offset += rxHead->byteCount;
    }

    return offset;
#endif
    return {};
}

extern "C" size_t stm32MemWrite(uint32_t address, uint8_t * data, size_t size) {
#if 0
    uint32_t offset = 0;
    uint8_t* dptr = (uint8_t*)data;
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];

    while (offset < size) {
        size_t txHeadSize = writeMemCmd(address, size, out);
        struct MemReadWriteCmdHead* txHead = (struct MemReadWriteCmdHead*)out;
        size_t bytesToWrite = txHead->byteCount;
        memcpy( &out[txHeadSize], dptr + offset, bytesToWrite );
        offset += bytesToWrite;
        size_t txSize = txHeadSize + bytesToWrite;
        size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
        if (rxSize < sizeof(struct MemReadWriteAnsHead)) break;
        struct MemReadWriteAnsHead* rxHead = (struct MemReadWriteAnsHead*)out;
        if (rxHead->cmd != DBG_CMD_MEM_WRITE) break;
        offset += rxHead->byteCount;
    }

    return offset;
#endif
    return {};
}

extern "C" size_t stm32RegsRead( uint8_t regIndex, uint8_t count, uint8_t* buf ) {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    size_t txSize = readRegCmd(regIndex, count, out);
    size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
    if ( rxSize < sizeof(struct RegReadWriteAnsHead) ) return 0;
    struct RegReadWriteCmdHead* cmdHead = (struct RegReadWriteCmdHead*)out;
    struct RegReadWriteAnsHead* ansHead = (struct RegReadWriteAnsHead*)in;
    if ( ansHead->cmd != cmdHead->cmd ) return 0;
    if ( ansHead->regCount == 0 ) return 0;
    memcpy( buf, &in[sizeof(struct RegReadWriteAnsHead)], ansHead->regCount * sizeof(uint32_t) );
    return ansHead->regCount;
#endif
    return {};
}

extern "C" size_t stm32RegsWrite(uint8_t regIndex, uint8_t count, uint32_t values[]) {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    size_t txSize = writeRegCmd(regIndex, 1, out);
    memcpy(&out[sizeof(struct RegReadWriteCmdHead)], &values[0], sizeof(uint32_t) * count);
    size_t rxSize = usbSendAndReceive(out, txSize + sizeof(uint32_t), in, sizeof(in));
    struct RegReadWriteCmdHead* cmdHead = (struct RegReadWriteCmdHead*)out;
    struct RegReadWriteAnsHead* ansHead = (struct RegReadWriteAnsHead*)in;
    if (ansHead->cmd != cmdHead->cmd) return 0;
    if (ansHead->regCount == 0) return 0;
    return 1;
#endif
    return {};
}

extern "C" size_t stm32HaltResume(uint8_t step) {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    size_t txSize = haltResumeCmd(step, out);
    size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
    struct HaltResumeCmdHead* cmdHead = (struct HaltResumeCmdHead*)out;
    struct HaltResumeAnsHead* ansHead = (struct HaltResumeAnsHead*)in;
    if (ansHead->cmd != cmdHead->cmd) return 0;
    return ansHead->step;
#endif
    return {};
}

extern "C" size_t stm32HaltRequest() {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    out[0] = DBG_CMD_HALT_REQUEST;
    size_t rxSize = usbSendAndReceive(out, 1, in, sizeof(in));
    if (rxSize == 0) return 0;
    return (in[0] == out[0]);
#endif
    return {};
}

extern "C" uint8_t stm32HaltPoll() {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    out[0] = DBG_CMD_HALT_POLL;
    size_t rxSize = usbSendAndReceive(out, 1, in, sizeof(in));
    if (rxSize != 2) return 0;
    if (in[0] != out[0]) return (uint8_t)(HALT_RUNNING);
    return (uint8_t)(in[1]);
#endif
    return {};
}

extern "C" uint8_t stm32BreakpointSetClear(uint32_t address, uint8_t type, uint8_t set) {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    size_t txSize = breakpointCmd(address, type, set, out);
    size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
    struct BreakpointCmdHead* cmdHead = (struct BreakpointCmdHead*)out;
    struct BreakpointAnsHead* ansHead = (struct BreakpointAnsHead*)in;
    if ( rxSize != sizeof(BreakpointAnsHead) ) return 0;
    if (ansHead->cmd != cmdHead->cmd) return 0;
    //if (ansHead->type != cmdHead->type) return 0;
    if (ansHead->set != cmdHead->set) return 0;
    if (ansHead->status == 0) return 0;
    return 1;
#endif
    return {};
}

extern "C" size_t stm32FlashEraseCmd(uint32_t address, uint16_t size) {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    size_t txSize = flashEraseCmd(address, size, out);
    size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
    struct FlashEraseCmdHead* cmdHead = (struct FlashEraseCmdHead*)out;
    struct FlashEraseAnsHead* ansHead = (struct FlashEraseAnsHead*)in;
    if (rxSize < sizeof(struct FlashEraseAnsHead)) return 0;
    return ( cmdHead->cmd != ansHead->cmd );
#endif
    return {};
}

extern "C" size_t stm32FlashWriteCmd(uint32_t address, uint8_t* data, uint16_t size) {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    size_t headSize = flashWriteCmd(address, data, size, out);
    struct FlashWriteCmdHead* cmdHead = (struct FlashWriteCmdHead*)out;
    memcpy(&out[headSize], data, cmdHead->size);
    size_t txSize = headSize + cmdHead->size;
    size_t rxSize = usbSendAndReceive(out, txSize, in, sizeof(in));
    struct FlashWriteAnsHead* ansHead = (struct FlashWriteAnsHead*)in;
    if ( rxSize < sizeof(struct FlashWriteAnsHead) ) return 0;
    if ( cmdHead->cmd != ansHead->cmd ) return 0;
    if (cmdHead->address != ansHead->address) return 0;
    if (ansHead->status == 0) return 0;
    return ansHead->size;
#endif
    return {};
}


extern "C" bool  stm32AttachCmd() {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    out[0] = DBG_CMD_ATTACH;
    size_t rxSize = usbSendAndReceive(out,1, in, sizeof(in));
    if (rxSize != 1) return false;
    if (out[0] != in[0]) return false;
    return true;
#endif
    return {};
}

extern "C" void  stm32DetachCmd() {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    out[0] = DBG_CMD_DETACH;
    size_t rxSize = usbSendAndReceive(out, 1, in, sizeof(in));
#endif
}


extern "C" void  stm32FlashDone() {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    out[0] = DBG_CMD_FLASH_DONE;
    size_t rxSize = usbSendAndReceive(out, 1, in, sizeof(in));
#endif
}

extern "C" void  stm32Reset() {
#if 0
    uint8_t  out[usbEpBufferSize];
    uint8_t  in[usbEpBufferSize];
    out[0] = DBG_CMD_RESET;
    size_t rxSize = usbSendAndReceive(out, 1, in, sizeof(in));
#endif
}


