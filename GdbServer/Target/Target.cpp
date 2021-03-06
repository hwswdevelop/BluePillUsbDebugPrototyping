
#include "Target.h"

class Target {
	virtual bool attach() = 0;
	virtual void detach() = 0;
	virtual bool checkError() = 0;
	virtual void memRead(uint8_t* memDataBuffer, TargetMemoryAddress address, size_t byteCount) = 0;
	virtual void memWrite(const uint8_t* memDataBuffer, TargetMemoryAddress address, size_t byteCount) = 0;
	virtual void regsRead(uint8_t* regDataBuffer) = 0;
	virtual void regsWrite(const void* regDataBuffer) = 0;
	virtual size_t regRead(uint32_t regIndex, uint8_t* regsDataBuffer, size_t maxByteCount) = 0;
	virtual size_t regWrite(uint32_t regIndex, const void* regsDataBuffer, size_t byteCount) = 0;
	virtual void reset() = 0;
	virtual void extendedReset() = 0;
	virtual void haltRequest() = 0;
	virtual void haltResume(bool step) = 0;
	virtual bool breakWatchSet() = 0;
	virtual bool breakWatchClear() = 0;
	~Target() = default;
};
