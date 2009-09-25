// ------------------------------------------------------------------------
//                            int_hand.cc
//
// Funkcja obslugi przerwania -- odczyt i zapis rejestrow sprzetowych dla robota irp6 postument
//
// Ostatnia modyfikacja: 2005
// ------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#ifdef __QNXNTO__
#include <sys/neutrino.h>
#include <sys/sched.h>
#include <hw/inout.h>
#endif

#include "lib/typedefs.h"
#include "lib/impconst.h"
#include "lib/com_buf.h"

// Klasa edp_irp6p_effector.
#include "edp/irp6_postument/edp_irp6p_effector.h"
// Klasa hardware_interface.
#include "edp/irp6_postument/hi_irp6p.h"

namespace mrrocpp {
namespace edp {
namespace common {

extern irp6p::effector* master;   // Bufor polecen i odpowiedzi EDP_MASTER

}

namespace irp6p {

// ------------------------------------------------------------------------

// Obsluga przerwania sprzetowego

// UWAGA - zmienna ilosc serwomechanizmow w zaleznosci od tego czy gripper jest dolaczony czy nie
#ifdef __QNXNTO__
const struct sigevent *
int_handler (void *arg, int int_id)
{
	common::irq_data_t *irq_data = (common::irq_data_t *) arg;
	common::motor_data & md = irq_data->md;
	struct sigevent & event = irq_data->event;

	common::status_of_a_dof robot_status[IRP6_POSTUMENT_NUM_OF_SERVOS];
	short int low_word, high_word;

	md.hardware_error = (uint64_t) lib::ALL_RIGHT; // Nie ma bledow sprzetowych

	if(common::master->test_mode)
	{
		return (&event); // by Y&W
	}

	// INT_EMPTY obluga pusta
	// z zalozenia to pierwszy tryb w ktorym jest uruchomiona fukcja obslugi przewania  ze wzgledu na synchronizacje
	if (md.interrupt_mode == INT_EMPTY)
	{
		// konieczne dla skasowania przyczyny przerwania
		out8(ADR_OF_SERVO_PTR, INTERRUPT_GENERATOR_SERVO_PTR);
		in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
		in16(SERVO_REPLY_INT_ADR);

		md.is_synchronised = true;
		for (int i = 0; i < common::master->number_of_servos; i++ )
		{
			out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
			md.robot_status[i].adr_offset_plus_0 = robot_status[i].adr_offset_plus_0 = in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
			md.robot_status[i].adr_offset_plus_2 = robot_status[i].adr_offset_plus_2 = in16(SERVO_REPLY_INT_ADR);

			// jesli ktorakolwiek os jest niezsynchronizwana to i robot jest niezsynchronizowany
			if (!(robot_status[i].adr_offset_plus_0 & 0x0040))
			{
				md.is_synchronised = false;
			}
		}

		return (&event);
	}

	// INT_SERVOING tryb regulacji osi
	else if (md.interrupt_mode == INT_SERVOING)
	{

		// konieczne dla skasowania przyczyny przerwania
		out8(ADR_OF_SERVO_PTR, INTERRUPT_GENERATOR_SERVO_PTR);
		in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
		in16(SERVO_REPLY_INT_ADR);

		for (int i = 0; i < common::master->number_of_servos; i++ )
		{
			// Odczyty stanu osi, polozenia oraz pradu wirnikow
			out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
			md.robot_status[i].adr_offset_plus_0 = robot_status[i].adr_offset_plus_0 = in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow

			md.robot_status[i].adr_offset_plus_2 = robot_status[i].adr_offset_plus_2 = in16(SERVO_REPLY_INT_ADR);


			// Odczyt polozenia osi: slowo 32 bitowe - negacja licznikow 16-bitowych
			robot_status[i].adr_offset_plus_4 = 0xFFFF ^ in16(SERVO_REPLY_POS_LOW_ADR); // Mlodsze slowo 16-bitowe
			robot_status[i].adr_offset_plus_6 = 0xFFFF ^ in16(SERVO_REPLY_POS_HIGH_ADR);// Starsze slowo 16-bitowe

			md.robot_status[i].adr_offset_plus_4 = robot_status[i].adr_offset_plus_4;
			md.robot_status[i].adr_offset_plus_6 = robot_status[i].adr_offset_plus_6;

			low_word  = robot_status[i].adr_offset_plus_4;
			high_word = robot_status[i].adr_offset_plus_6;

			// Obliczenie polozenia
			md.current_absolute_position[i] =  (((uint32_t) (high_word<<16)) & (0xFFFF0000)) | ((uint16_t) low_word);

			//   md.robot_status[i].adr_offset_plus_6 = robot_status[i].adr_offset_plus_6;
			//   md.high_word = high_word;

			//  md.robot_status[i].adr_offset_plus_8 = robot_status[i].adr_offset_plus_8 = in16(SERVO_REPLY_REG_1_ADR); // Niewykorzystane
			//  md.robot_status[i].adr_offset_plus_a = robot_status[i].adr_offset_plus_a = in16(SERVO_REPLY_REG_2_ADR); // Niewykorzystane


			// Obsluga bledow
			if ( robot_status[i].adr_offset_plus_0 & 0x0100 )
				md.hardware_error |= (uint64_t) (lib::SYNCHRO_ZERO << (5*i)); // Impuls zera rezolwera

			if (i>=5) //  sterowniki osi z mechatroniki z przekaznikami
			{
				if ( ~(robot_status[i].adr_offset_plus_0) & 0x4000 )
					md.hardware_error |= (uint64_t) (lib::SYNCHRO_SWITCH_ON << (5*i)); // Zadzialal wylacznik synchronizacji
			 }  else
			{
				if ( robot_status[i].adr_offset_plus_0 & 0x4000 )
					md.hardware_error |= (uint64_t) (lib::SYNCHRO_SWITCH_ON << (5*i)); // Zadzialal wylacznik synchronizacji
			 }

			// wylaczniki krancowe
			if ( ~(robot_status[i].adr_offset_plus_0) & 0x1000 ) {
			//	out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
			//	out16(SERVO_COMMAND1_ADR, RESET_ALARM); // Skasowanie alarmu i umozliwienie ruchu osi
				md.hardware_error |= (uint64_t) (lib::UPPER_LIMIT_SWITCH << (5*i)); // Zadzialal wylacznik "gorny" krancowy
			}
			else if ( ~(robot_status[i].adr_offset_plus_0) & 0x2000 ) {
				md.hardware_error |= (uint64_t) (lib::LOWER_LIMIT_SWITCH << (5*i)); // Zadzialal wylacznik "dolny" krancowy
			}


			if ( robot_status[i].adr_offset_plus_0 & 0x0400 )
			{
				md.hardware_error |= (uint64_t) (lib::OVER_CURRENT << (5*i));
				//     out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
				//     out16(SERVO_COMMAND1_ADR, RESET_ALARM); // Skasowanie alarmu i umozliwienie ruchu osi
			}
		}

		if (robot_status[1].adr_offset_plus_0 & 0x0080) // czy wlaczono moc
		{
			md.is_power_on = true;
		} else {
			md.is_robot_blocked = true;
			md.is_power_on = false;
		}


		if ( md.hardware_error & lib::HARDWARE_ERROR_MASK ) // wyciecie SYNCHRO_ZERO i SYNCHRO_SWITCH_ON
		{
			for (int i = 0; i < common::master->number_of_servos; i++ ) {
				// Zapis wartosci zadanej wypelnienia PWM
				out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
				out16(SERVO_COMMAND1_ADR, STOP_MOTORS);
			}
			return (&event); // Yoyek & 7
		}

		for (int i = 0; i < common::master->number_of_servos; i++ ) {
			// Zapis wartosci zadanej wypelnienia PWM
			out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
			if (md.is_robot_blocked) md.robot_control[i].adr_offset_plus_0 &= 0xff00;
			out16(SERVO_COMMAND1_ADR, md.robot_control[i].adr_offset_plus_0);
		}

		return (&event);
	} // end INT_SERVOING

	// INT_SINGLE_COMMAND do synchronizacji, inicjacji, etc.
	else if (md.interrupt_mode == INT_SINGLE_COMMAND)
	{

		// konieczne dla skasowania przyczyny przerwania
		out8(ADR_OF_SERVO_PTR, INTERRUPT_GENERATOR_SERVO_PTR);
		in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
		in16(SERVO_REPLY_INT_ADR);

		out8(ADR_OF_SERVO_PTR, md.card_adress);
		out16(md	.register_adress, md.value);

		for (int i = 0; i < common::master->number_of_servos; i++ )
		{
			out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
			md.robot_status[i].adr_offset_plus_0 = robot_status[i].adr_offset_plus_0 = in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
			md.robot_status[i].adr_offset_plus_2 = robot_status[i].adr_offset_plus_2 = in16(SERVO_REPLY_INT_ADR);
		}
		md.interrupt_mode=INT_EMPTY; // aby tylko raz wyslac polecenie

		return (&event);
	}

	// INT_CHECK_STATE do odczytu stanu z adresu 0x220
	else if (md.interrupt_mode == INT_CHECK_STATE)
	{

		// konieczne dla skasowania przyczyny przerwania
		out8(ADR_OF_SERVO_PTR, INTERRUPT_GENERATOR_SERVO_PTR);
		in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
		in16(SERVO_REPLY_INT_ADR);

		for (int i = 0; i < common::master->number_of_servos; i++ )
		{
			out8(ADR_OF_SERVO_PTR, FIRST_SERVO_PTR + (uint8_t)i);
			md.robot_status[i].adr_offset_plus_0 = robot_status[i].adr_offset_plus_0 = in16(SERVO_REPLY_STATUS_ADR); // Odczyt stanu wylacznikow
			md.robot_status[i].adr_offset_plus_2 = robot_status[i].adr_offset_plus_2 = in16(SERVO_REPLY_INT_ADR);
		}
		md.interrupt_mode=INT_EMPTY; // aby tylko raz sprawdzic stan

		return (&event);
	}

	return (&event);
}
#endif /*__QNXNTO__ */


// ------------------------------------------------------------------------

} // namespace common
} // namespace edp
} // namespace mrrocpp