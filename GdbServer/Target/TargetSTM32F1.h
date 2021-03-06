#pragma once

#include "Target.h"


struct TargetSTM32F1 : public Target {
	TargetSTM32F1(TargetInterface& targetInterface);
	bool probe() final;
	bool attachDetach(const AttachCommand command) final;
	std::vector<uint8_t> memRead(const MemoryAddress address, const AccessSize accessSize, const size_t elementCount) final;
	size_t memWrite(const MemoryAddress address, const AccessSize accessSize, const std::vector<uint8_t> dataToWrite) final;
	std::vector<TargetRegister> regsRead(const uint16_t regIndex, const uint16_t regCount) final;
	size_t regsWrite(const uint16_t regIndex, const std::vector<TargetRegister>& regsToWrite) final;
	size_t breakpointsSetClear(const std::vector<Breakpoint> breakpoints, const bool set) final;
	ResetHaltState resetHaltResume(const ResetHaltCommand command) final;
	bool flashCmd(const FlashCommand command, const MemoryAddress address, const uint32_t blockSizeBytes) final;
	std::string getSerialData() final;
	~TargetSTM32F1();
private:
	size_t sendAndReceive(size_t sendSize);
private:
	TargetInterface& _interface;
	size_t	_maxPacketSize = 0;
	uint8_t* _out = nullptr;
	uint8_t* _in = nullptr;
};

