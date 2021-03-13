#pragma once

#include "../TargetInterface/TargetInterface.h"
#include <list>
#include <vector>
#include <string>

typedef uint32_t MemoryAddress;
typedef uint32_t TargetRegister;
typedef uint32_t  AccessSize;


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

struct Target {
	virtual bool probe() = 0;
	virtual bool attachDetach(const AttachCommand command) = 0;
	virtual std::vector<uint8_t> memRead(const MemoryAddress address, const AccessSize accessSize, const size_t elementCount ) = 0;
	virtual size_t memWrite( const MemoryAddress address, const AccessSize accessSize, const std::vector<uint8_t> dataToWrite) = 0;
	virtual std::vector<TargetRegister> regsRead( const uint16_t regIndex, const uint16_t regCount) = 0;
	virtual size_t regsWrite(const uint16_t regIndex, const std::vector<TargetRegister>& regsToWrite ) = 0;
	virtual size_t breakpointsSetClear(const std::vector<Breakpoint> breakpoints, const bool set) = 0;
	virtual ResetHaltState resetHaltResume(const ResetHaltCommand command) = 0;
	virtual bool flashCmd(const FlashCommand command, const MemoryAddress address, const uint32_t blockSizeBytes) = 0;
	virtual std::string getSerialData() = 0;
	~Target() = default;
};


