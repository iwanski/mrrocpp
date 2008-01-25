// -------------------------------------------------------------------------
//                            ecp.h dla QNX6
// Zadanie przelewania - ECP dla IRP6_ON_TRACK
//
// Ostatnia modyfikacja: styczen 2008
// autor: Przemek Pilacinski
// -------------------------------------------------------------------------

#if !defined(_ECP_T_POURING_IRP6OT_H)
#define _ECP_T_POURING_IRP6OT_H

#include "ecp/common/ecp_task.h"
#include "ecp/common/ecp_generator_t.h"

class ecp_task_pouring_irp6ot: public ecp_task  {
protected:
	ecp_smooth_generator* sg;
	ecp_smooth_pouring_generator* spg;

public:
	// KONSTRUKTORY
	ecp_task_pouring_irp6ot();
	~ecp_task_pouring_irp6ot();
	
	// methods for ECP template to redefine in concrete classes
	void task_initialization(void);
	void main_task_algorithm(void);
	void grip(double gripper_increment, int motion_time);
	
};

#endif
