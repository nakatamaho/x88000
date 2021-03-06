////////////////////////////////////////////////////////////
// Parallel Null Device Emulator
//
// Written by Manuke

////////////////////////////////////////////////////////////
// include

#include "StdHeader.h"

#include "ParallelNull.h"

////////////////////////////////////////////////////////////
// implementation of CParallelNull

////////////////////////////////////////////////////////////
// attribute

// get device id

std::string CParallelNull::GetDeviceID() const {
	return "null";
}

////////////////////////////////////////////////////////////
// create & destroy

// default constructor

CParallelNull::CParallelNull() :
	CParallelDevice(),
	m_bPause(false)
{
}

// destructor

CParallelNull::~CParallelNull() {
}

////////////////////////////////////////////////////////////
// initialize

// initialize at first

void CParallelNull::Initialize() {
	m_bPause = false;
}

// reset

void CParallelNull::Reset() {
}

////////////////////////////////////////////////////////////
// communication

// is busy

bool CParallelNull::IsBusy() {
	return m_bPause;
}

// set strobe(negative logic)

void CParallelNull::SetStrobe(bool /*bNStrobe*/) {
}

// output data

void CParallelNull::SetData(uint8_t /*btData*/) {
}

////////////////////////////////////////////////////////////
// implementation

// is pause mode

bool CParallelNull::IsPause() {
	return m_bPause;
}

// set pause mode

void CParallelNull::SetPause(bool bPause) {
	m_bPause = bPause;
}

// flush data

void CParallelNull::Flush() {
}

// idle process

void CParallelNull::OnIdle(int /*nMilliSecond*/) {
}
