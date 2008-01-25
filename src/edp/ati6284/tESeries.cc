/*	! \file src/edp/ati6284/tESeries.cc
    * \brief NI*/

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file is autogenerated!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!


#include "tESeries.h"


tESeries::tESeries(tBusSpaceReference addrSpace, nMDBG::tStatus2* s)

{
	if (s && s->isFatal()) return;

	_addressOffset = 0;
	_addrSpace = addrSpace;
	_deallocateBus = kFalse;
	_initialize(s);
}


void tESeries::reset(nMDBG::tStatus2* s)
{
	if (s && s->isFatal()) return;

	AIAO_Select.setRegister(0x0, s);
	AIFifoData.setRegister(0x0, s);
	ConfigFifoLow.setRegister(0x0, s);
	ConfigFifoHigh.setRegister(0x0, s);
	AO_Config.setRegister(0x0, s);
	DAC0_DirectData.setRegister(0x0, s);
	DAC1_DirectData.setRegister(0x0, s);
	AO_FIFOData.setRegister(0x0, s);
	SerialCommandRegister.setRegister(0x0, s);
	SerialStatus.setRegister(0x0, s);
}

void tESeries::_initialize(nMDBG::tStatus2* s)
{
	if (s && s->isFatal()) return;

	//!< set register maps of all registers

	AIAO_Select.setRegisterMap(this);
	AIFifoData.setRegisterMap(this);
	ConfigFifoLow.setRegisterMap(this);
	ConfigFifoHigh.setRegisterMap(this);
	AO_Config.setRegisterMap(this);
	DAC0_DirectData.setRegisterMap(this);
	DAC1_DirectData.setRegisterMap(this);
	AO_FIFOData.setRegisterMap(this);
	SerialCommandRegister.setRegisterMap(this);
	SerialStatus.setRegisterMap(this);

	reset(s);
}

tESeries::~tESeries()
{
}



// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// This file is autogenerated!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

