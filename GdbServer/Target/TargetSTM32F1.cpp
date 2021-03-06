#include "TargetSTM32F1.h"
#include "../DbgCommands.h"


TargetSTM32F1::TargetSTM32F1(TargetInterface& targetInterface) : _interface(targetInterface) {
	size_t bytesCount = _interface.getMaxPacketSize();
	_maxPacketSize = bytesCount;
	size_t wordsCount = (bytesCount + (sizeof(uint32_t) - 1)) / sizeof(uint32_t);
	_out = reinterpret_cast<uint8_t*>(new uint32_t[wordsCount]);
	_in = reinterpret_cast<uint8_t*>(new uint32_t[wordsCount]);
}

TargetSTM32F1::~TargetSTM32F1() {
	delete[] _out;
	delete[] _in;
}

struct MakeHead {

	MakeHead(uint8_t* buf, size_t bufferSize) : _buf(buf), _rxBufferSize(bufferSize), _txBufferSize(bufferSize) {};

	size_t memReadCmd(const MemoryAddress address, const AccessSize accessSizeBytes, const size_t elementCount) const {
		if ((_buf == nullptr) || (elementCount == 0) || (accessSizeBytes == 0)) return 0;
		// Check that accessSizeBytes is power of 2, and non zero
		bool accessSizeIsInvalid = ((accessSizeBytes - 1) & accessSizeBytes) || (accessSizeBytes == 0);
		if (accessSizeIsInvalid) return 0;
		const size_t requiredBytesToRead = elementCount * accessSizeBytes;
		const size_t maxBytesToRead = (_rxBufferSize - sizeof(MemReadWriteAnsHead));
		size_t readBytesCount = ( maxBytesToRead < requiredBytesToRead) ? maxBytesToRead : requiredBytesToRead;
		size_t accessBitMask = ~(accessSizeBytes - 1);
		readBytesCount &= accessBitMask;

		MemReadWriteCmdHead* const cmdHead = reinterpret_cast<MemReadWriteCmdHead* const>(_buf);
		cmdHead->cmd = static_cast<uint8_t>( DebugCommand::MemoryRead );
		cmdHead->byteCount = static_cast<uint16_t>(readBytesCount);
		cmdHead->align = accessSizeBytes;
		cmdHead->address = address;
		return sizeof(MemReadWriteCmdHead);
	}

	size_t memWriteCmd(const MemoryAddress address, const AccessSize accessSizeBytes, const size_t elementCount) const {
		if ((_buf == nullptr) || (elementCount == 0) || (accessSizeBytes == 0)) return 0;
		// Check that accessSizeBytes is power of 2, and non zero
		const bool accessSizeIsInvalid = ((accessSizeBytes - 1) & accessSizeBytes) || (accessSizeBytes == 0);
		if (accessSizeIsInvalid) return 0;
		const size_t requiredBytesToRead = elementCount * accessSizeBytes;
		const size_t maxBytesToWrite = ( _txBufferSize - sizeof(MemReadWriteCmdHead) );
		const size_t accessBitMask = ~(accessSizeBytes - 1);
		size_t writeBytesCount = ( maxBytesToWrite < requiredBytesToRead ) ? maxBytesToWrite : requiredBytesToRead;
		writeBytesCount &= accessBitMask;

		MemReadWriteCmdHead* const cmdHead = reinterpret_cast<MemReadWriteCmdHead* const>(_buf);
		cmdHead->cmd = static_cast<uint8_t>( DebugCommand::MemoryWrite);
		cmdHead->byteCount = static_cast<uint16_t>(writeBytesCount);
		cmdHead->align = accessSizeBytes;
		cmdHead->address = address;
		return sizeof(MemReadWriteCmdHead);
	}

	size_t regsRead(const size_t regIndex, const size_t regCount) {
		if ((_buf == nullptr) || (regCount == 0)) return 0;
		const size_t regSize = sizeof(TargetRegister);
		const size_t requiredBytesToRead = regCount * regSize;
		const size_t maxBytesToRead = (_rxBufferSize - sizeof(RegReadWriteAnsHead));
		const size_t accessBitMask = ~(regSize - 1);
		size_t bytesToRead = (maxBytesToRead < requiredBytesToRead) ? maxBytesToRead : requiredBytesToRead;
		bytesToRead &= accessBitMask;
		const size_t regCountToRead = bytesToRead / regSize;
		RegReadWriteCmdHead* const cmdHead = reinterpret_cast<RegReadWriteCmdHead* const>(_buf);
		cmdHead->cmd = static_cast<uint8_t>(DebugCommand::RegsRead);
		cmdHead->regCount = static_cast<uint8_t>(regCountToRead);
		cmdHead->regIndex = static_cast<uint16_t>(regIndex);
		return sizeof(RegReadWriteCmdHead);
	}

	size_t regsWrite(const size_t regIndex, const std::vector<TargetRegister>& regsToWrite) {
		const size_t regCount = regsToWrite.size();
		if ((_buf == nullptr) || (regCount == 0)) return 0;
		const size_t regSize = sizeof(TargetRegister);
		const size_t requiredBytesToWrite = regCount * regSize;
		const size_t maxBytesToWrite = (_txBufferSize - sizeof(RegReadWriteCmdHead));
		const size_t accessBitMask = ~(regSize - 1);
		size_t bytesToRead = (maxBytesToWrite < requiredBytesToWrite) ? maxBytesToWrite : requiredBytesToWrite;
		bytesToRead &= accessBitMask;
		const size_t regCountToWrite = bytesToRead / regSize;
		RegReadWriteCmdHead* const cmdHead = reinterpret_cast<RegReadWriteCmdHead* const>(_buf);
		cmdHead->cmd = static_cast<uint8_t>(DebugCommand::RegsWrite);
		cmdHead->regCount = static_cast<uint8_t>(regCountToWrite);
		cmdHead->regIndex = static_cast<uint8_t>(regIndex);
		return sizeof(RegReadWriteCmdHead);
	}

	size_t breakpointSetClear(const std::vector<Breakpoint> breakpoints, const bool set) {
		const size_t breakpointCount = breakpoints.size();
		const size_t breakpointSize = sizeof(Breakpoint);
		const size_t bytesCount = breakpointCount * breakpointSize;
		const size_t maxSendBytesCount = _txBufferSize - breakpointSize;
		size_t sendBreakpointsCount = maxSendBytesCount / sizeof(BreakpointCmdHead);
		sendBreakpointsCount = (sendBreakpointsCount < breakpointCount) ? sendBreakpointsCount : breakpointCount;
		BreakpointCmdHead* cmdHead = reinterpret_cast<BreakpointCmdHead* const>(_buf);
		cmdHead->cmd = static_cast<uint8_t>(DebugCommand::Breakpoint);
		cmdHead->set = set;
		cmdHead->count = static_cast<uint16_t>(sendBreakpointsCount);
		return sizeof(BreakpointCmdHead);
	}

	size_t resetHaltResume(const ResetHaltCommand command) {
		ResetHaltResumeCmdHead* const cmdHead = reinterpret_cast<ResetHaltResumeCmdHead*>(_buf);
		cmdHead->cmd = static_cast<uint8_t>(DebugCommand::ResetHalt);
		cmdHead->haltCommand = static_cast<uint8_t>(command);
		return sizeof(ResetHaltResumeCmdHead);
	};

	size_t flashCmd( const FlashCommand command, const MemoryAddress address, const uint32_t blockSizeBytes) {
		FlashCmdHead* const cmdHead = reinterpret_cast<FlashCmdHead* const>(_buf);
		cmdHead->cmd = static_cast<uint8_t>(DebugCommand::Flash);
		cmdHead->address = address;
		cmdHead->flashCommand = static_cast<uint8_t>(command);
		cmdHead->size = blockSizeBytes;
		cmdHead->reserved = 0;
		return sizeof(FlashCmdHead);
	}

private:
	uint8_t* _buf = nullptr;
	size_t	_rxBufferSize = 64;
	size_t	_txBufferSize = 64;

};

size_t TargetSTM32F1::sendAndReceive(size_t sendSize) {
	size_t txSize = _interface.writePacket( _out, sendSize );
	if ( txSize != sendSize ) return 0;
	size_t rxSize = _interface.readPacket( _in, _maxPacketSize );
	return rxSize;
}

bool TargetSTM32F1::probe() {
	return{};
}

bool TargetSTM32F1::attachDetach(const AttachCommand command) {
	AttachDetachCmdHead* const cmdHead = reinterpret_cast<AttachDetachCmdHead* const>(_out);
	cmdHead->cmd = static_cast<uint8_t>(DebugCommand::Attach);
	cmdHead->attach = static_cast<uint8_t>(command);
	size_t rxSize = sendAndReceive(sizeof(AttachDetachCmdHead));
	if (rxSize < sizeof(AttachDetachAnsHead)) return false;
	AttachDetachAnsHead* const ansHead = reinterpret_cast<AttachDetachAnsHead* const>(_in);
	if (ansHead->cmd != cmdHead->cmd) return false;
	if (ansHead->attach != cmdHead->attach) return false;
	return ansHead->state;
}

std::vector<uint8_t> TargetSTM32F1::memRead(const MemoryAddress address, const AccessSize accessSize, const size_t elementCount) {
	if ( (accessSize == 0) || (elementCount == 0) ) return {};
	const size_t totalBytesToRead = elementCount * accessSize;
	std::vector<uint8_t> data(totalBytesToRead);
	size_t offset = 0;
	while( offset < totalBytesToRead ){
		const size_t remain = totalBytesToRead - offset;
		size_t cmdHeadSize = MakeHead( _out, _maxPacketSize ).memReadCmd( address + offset, accessSize, remain / accessSize);
		if (0 == cmdHeadSize) break;
		size_t rxSize = sendAndReceive(cmdHeadSize);
		if ( rxSize < sizeof(MemReadWriteAnsHead) ) break;
		MemReadWriteCmdHead* const cmdHead = reinterpret_cast<MemReadWriteCmdHead*>(_out);
		MemReadWriteAnsHead* const ansHead = reinterpret_cast<MemReadWriteAnsHead*>(_in);
		if ( cmdHead->cmd != ansHead->cmd ) break;
		if ( cmdHead->address != ansHead->address ) break;
		if ( cmdHead->align != ansHead->align ) break;
		if ( 0 == ansHead->byteCount ) break;
		size_t readCount = (ansHead->byteCount < remain) ? ansHead->byteCount : remain;
		memcpy(data.data() + offset, _in + sizeof(MemReadWriteAnsHead), readCount);
		offset += ansHead->byteCount;
	}
	if ( offset < data.size() ) data.resize(offset);
	return data;
}

size_t TargetSTM32F1::memWrite(const MemoryAddress address, const AccessSize accessSize, const std::vector<uint8_t> dataToWrite) {
	const size_t totalBytesToWrite =  dataToWrite.size();
	if (totalBytesToWrite == 0 ) return false;
	size_t offset = 0;
	while ( offset < totalBytesToWrite ) {
		const size_t remain = totalBytesToWrite - offset;
		size_t cmdHeadSize = MakeHead( _out, _maxPacketSize ).memWriteCmd(address + offset, accessSize, remain / accessSize);
		if (0 == cmdHeadSize) break;
		MemReadWriteCmdHead* const cmdHead = reinterpret_cast<MemReadWriteCmdHead*>(_out);
		memcpy(_out + cmdHeadSize, dataToWrite.data() + offset, cmdHead->byteCount);
		const size_t rxSize = sendAndReceive(cmdHeadSize + cmdHead->byteCount);		
		MemReadWriteAnsHead* const ansHead = reinterpret_cast<MemReadWriteAnsHead*>(_in);
		if (cmdHead->cmd != ansHead->cmd) break;
		if (cmdHead->address != ansHead->address) break;
		if (cmdHead->align != ansHead->align) break;
		if ( 0 == ansHead->byteCount) break;
		if ( ansHead->byteCount > cmdHead->byteCount) break;
		offset += ansHead->byteCount;
	}
	return offset;
}

std::vector<TargetRegister> TargetSTM32F1::regsRead(const uint16_t regIndex, const uint16_t regCount) {
	if (regCount == 0) return {};
	const size_t regSize = sizeof(TargetRegister);
	const size_t totalBytesToRead = regSize * regCount;
	size_t offset = 0;
	std::vector<TargetRegister> regs(regCount);
	while (offset < totalBytesToRead) {
		const size_t remain = totalBytesToRead - offset;
		const size_t cmdHeadSize = MakeHead(_out, _maxPacketSize).regsRead(offset / regSize + regIndex, remain / regSize);
		const size_t rxSize = sendAndReceive(cmdHeadSize);
		if ( rxSize < sizeof(RegReadWriteAnsHead) ) break;
		RegReadWriteCmdHead* const cmdHead = reinterpret_cast<RegReadWriteCmdHead*>(_out);
		RegReadWriteAnsHead* const ansHead = reinterpret_cast<RegReadWriteAnsHead*>(_in);
		if (cmdHead->cmd != ansHead->cmd) break;
		if (cmdHead->regIndex != ansHead->regIndex) break;
		if (cmdHead->regCount < ansHead->regCount) break;
		memcpy(reinterpret_cast<uint8_t*>(regs.data()) + offset, _in + sizeof(RegReadWriteAnsHead), ansHead->regCount * regSize);
		offset += ansHead->regCount * regSize;
	}
	if (offset < (regCount * regSize)) regs.resize( offset / regSize );
	return regs;
}

size_t TargetSTM32F1::regsWrite(const uint16_t regIndex, const std::vector<TargetRegister>& regsToWrite) {
	const size_t regCount = regsToWrite.size();
	const size_t regSize = sizeof(TargetRegister);
	const size_t totalBytesToWrite = regSize * regCount;
	if (regCount == 0) return false;
	size_t offset = 0;
	while (offset < totalBytesToWrite) {
		const size_t remain = totalBytesToWrite - offset;
		const size_t cmdHeadSize = MakeHead( _out, _maxPacketSize ).regsWrite( offset / regSize + regIndex, regsToWrite );
		RegReadWriteCmdHead* const cmdHead = reinterpret_cast<RegReadWriteCmdHead*>(_out);
		memcpy(_out + cmdHeadSize, reinterpret_cast<const uint8_t*>(regsToWrite.data()) + offset, cmdHead->regCount * regSize);
		const size_t rxSize = sendAndReceive(cmdHeadSize + cmdHead->regCount * regSize);
		if ( rxSize < sizeof(RegReadWriteAnsHead) ) break;
		RegReadWriteAnsHead* const ansHead = reinterpret_cast<RegReadWriteAnsHead*>(_in);
		if (cmdHead->cmd != ansHead->cmd) break;
		if (cmdHead->regIndex != ansHead->regIndex) break;
		if (0 == ansHead->regCount) break;
		if (cmdHead->regCount < ansHead->regCount) break;
		offset += ansHead->regCount * regSize;
	}
	return ( offset / regSize );
}

size_t TargetSTM32F1::breakpointsSetClear( const std::vector<Breakpoint> breakpoints, const bool set) {
	size_t breakpointCount = breakpoints.size();
	size_t breakpointIndex = 0;
	auto it = breakpoints.begin();
	while (breakpointIndex < breakpointCount) {
		size_t cmdHeadSize = MakeHead(_out, _maxPacketSize).breakpointSetClear(breakpoints, set);
		BreakpointCmdHead* const cmdHead = reinterpret_cast<BreakpointCmdHead*>(_out);
		memcpy(_out + cmdHeadSize, breakpoints.data() + breakpointIndex, sizeof(Breakpoint) * cmdHead->count);
		BreakpointAnsHead* const ansHead = reinterpret_cast<BreakpointAnsHead*>(_in);
		const size_t rxSize = sendAndReceive(cmdHeadSize + cmdHead->count * sizeof(Breakpoint));
		if ( rxSize < sizeof(BreakpointAnsHead) ) break;
		if (ansHead->cmd != cmdHead->cmd) break;
		if (ansHead->set != cmdHead->set) break;
		if (ansHead->count > ansHead->count) break;
		breakpointIndex += ansHead->count;
	}
	return breakpointIndex;
}

ResetHaltState TargetSTM32F1::resetHaltResume(const ResetHaltCommand command) {
	size_t cmdHeadSize = MakeHead(_out, _maxPacketSize).resetHaltResume(command);
	ResetHaltResumeCmdHead* cmdHead = (ResetHaltResumeCmdHead*)_out;
	const size_t rxSize = sendAndReceive(cmdHeadSize);
	ResetHaltResumeAnsHead* ansHead = (ResetHaltResumeAnsHead*)_in;
	if (rxSize < sizeof(ResetHaltResumeAnsHead)) return ResetHaltState::Undefined;
	if (cmdHead->cmd != ansHead->cmd) return  ResetHaltState::Undefined;
	if (cmdHead->haltCommand != ansHead->haltCommand) return  ResetHaltState::Undefined;
	return static_cast<ResetHaltState>(ansHead->haltState);
}

bool TargetSTM32F1::flashCmd(const FlashCommand command, const MemoryAddress address, const uint32_t blockSizeBytes) {
	size_t cmdHeadSize = MakeHead(_out, _maxPacketSize).flashCmd(command, address, blockSizeBytes);
	const size_t rxSize = sendAndReceive(cmdHeadSize);
	FlashCmdHead* cmdHead = (FlashCmdHead*)_out;
	FlashAnsHead* ansHead = (FlashAnsHead*)_in;
	if (rxSize < sizeof(FlashAnsHead)) return false;
	if (cmdHead->cmd != ansHead->cmd) return  false;
	if (cmdHead->flashCommand != ansHead->flashCommand) return false;
	return ansHead->status;
}

#include <thread>
#include <chrono>
std::string TargetSTM32F1::getSerialData() {
	std::string str;
	str.resize(_maxPacketSize + 1, 0);
	const size_t rxSize = _interface.readSerial((uint8_t*)(str.c_str()), _maxPacketSize);
	str.resize(rxSize);
	return str;
}
