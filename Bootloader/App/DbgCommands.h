#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __clpusplus
extern "C" {
#endif

struct MemReadWriteCmdHead {
	uint8_t  cmd;
	uint8_t  align;
	uint16_t byteCount;
	uint32_t address;
};

struct MemReadWriteAnsHead {
	uint8_t  cmd;
	uint8_t  align;
	uint16_t byteCount;
	uint32_t address;
};

struct RegReadWriteCmdHead {
	uint8_t cmd;
	uint8_t regCount;
	uint16_t regIndex;
};

struct RegReadWriteAnsHead {
	uint8_t cmd;
	uint8_t regCount;
	uint16_t regIndex;
};

struct ResetHaltResumeCmdHead {
	uint8_t  cmd;
	uint8_t  haltCommand;
	uint16_t reserved;
};

struct ResetHaltResumeAnsHead {
	uint8_t cmd;
	uint8_t haltCommand;
	uint8_t haltState;
	uint8_t reserved;
};

struct AttachDetachCmdHead {
	uint8_t  cmd;
	uint8_t  attach;
	uint16_t reserved;
};

struct AttachDetachAnsHead {
	uint8_t cmd;
	uint8_t attach;
	uint8_t state;
	uint8_t reserved;
};

struct FlashCmdHead {
	uint8_t  cmd;
	uint8_t  flashCommand;
	uint16_t reserved;
	uint32_t address;
	uint32_t size;
};

struct FlashAnsHead {
	uint8_t cmd;
	uint8_t flashCommand;
	uint8_t status;
	uint8_t reserved;
};

struct BreakpointCmdHead {
	uint8_t cmd;
	uint8_t set;
	uint16_t count;
};

struct BreakpointAnsHead {
	uint8_t cmd;
	uint8_t set;
	uint16_t count;
};


typedef uint32_t MemoryAddress;
typedef uint32_t TargetRegister;
typedef uint8_t  AccessSize;

enum class DebugCommand : uint8_t {
	MemoryRead,
	MemoryWrite,
	RegsRead,
	RegsWrite,
	ResetHalt,
	Breakpoint,
	Flash,
	Attach
};

enum class ResetHaltCommand : uint8_t {
	ResetOnly,
	HaltOnly,
	ResetAndHalt,
	Resume,
	ResumeStep,
	GetState
};

enum class ResetHaltState {
	Running,
	Error,
	Request,
	Stepping,
	Breakpoint,
	Watchpoint,
	Fault,
	Undefined
};

enum class FlashCommand {
	Unlock,
	Lock,
	Erase,
	Detach
};

enum class AttachCommand : uint8_t {
	Attach,
	Detach
};

enum class BreakpointType : uint8_t {
	BreakSoft,
	BreakHard,
	WatchWrite,
	WatchRead,
	WatchAccess
};

struct Breakpoint {
	MemoryAddress	address;
	BreakpointType  type;
	uint8_t			size;
	uint16_t		reserved;
};



#ifdef __clpusplus
}
#endif
