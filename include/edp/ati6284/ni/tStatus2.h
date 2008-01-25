/* ! \file include/edp/ati6284/ni/tStatus2.h
    * \brief plik nagłówkowy NI
    * Ostatnia modyfikacja: 04.2006 */
#ifndef  ___osiTypes_h___
#include "osiTypes.h"
#endif

#ifndef ___nimdbg_tStatus2_h___
#define ___nimdbg_tStatus2_h___

class nMDBG{
	public:
	class tStatus2{
		public:
		i32 tStatus;
		i32 isFatal() {return (tStatus<0);};
		operator i32*() {return &tStatus;};

		operator i32&() {return tStatus;};

	};
};

#define NIM_TRACE_LOAD(a,b,c)   ;
#define NIM_TRACE_LOAD_AT_LEVEL(a,b,c,d)   ;
#define NIM_TRACE_STORE(a,b,c)   ;
#define NIM_TRACE_STORE_AT_LEVEL(a,b,c,d)   ;

#endif
