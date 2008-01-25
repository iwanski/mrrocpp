// -------------------------------------------------------------------------
//                            ecp.cc
//            Effector Control Process (ECP) - methods
// Funkcje do tworzenia procesow ECP
// robot - speaker
// 
// Ostatnia modyfikacja: 16.04.98
// -------------------------------------------------------------------------

#include "common/impconst.h"
#include "common/com_buf.h"
#include "lib/mis_fun.h"

#include "ecp/speaker/ecp_local.h"


// ####################################################################################################
// ####################################################################################################
// ####################################################################################################

ecp_speaker_robot::ecp_speaker_robot (configurator &_config, sr_ecp *_sr_ecp)
	:	ecp_robot (ROBOT_SPEAKER, _config, _sr_ecp)
{};
ecp_speaker_robot::ecp_speaker_robot (ecp_task& _ecp_object)
	:	ecp_robot (ROBOT_SPEAKER, _ecp_object)
{ };

// --------------------------------------------------------------------------
void ecp_speaker_robot::create_command (void) {
  // wypelnia bufor wysylkowy do EDP na podstawie danych
  // zawartych w skladowych generatora lub warunku

  EDP_command_and_reply_buffer.instruction.instruction_type = EDP_data.instruction_type;

  switch (EDP_data.instruction_type) {
    case SET:
    case SET_GET:
    	strcpy(EDP_command_and_reply_buffer.instruction.arm.text_def.text, EDP_data.text);
	strcpy(EDP_command_and_reply_buffer.instruction.arm.text_def.prosody, EDP_data.prosody);
     break;
    case GET:
    case QUERY:
         break;
     case SYNCHRO:
    default: // blad: nieprawidlowe polecenie
      throw ECP_error (NON_FATAL_ERROR, INVALID_ECP_COMMAND);
  }; // end: switch (instruction_type)

}; // end: ecp_speaker_robot::create_command
// ---------------------------------------------------------------

/*---------------------------------------------------------------------*/
void ecp_speaker_robot::get_reply (void) {
  // pobiera z pakietu przeslanego z EDP informacje i wstawia je do
  // odpowiednich skladowych generatora lub warunku
EDP_data.reply_type = EDP_command_and_reply_buffer.reply_package.reply_type;

 switch (EDP_data.reply_type) {
   case ERROR:
     EDP_data.error_no.error0 = EDP_command_and_reply_buffer.reply_package.error_no.error0;
     EDP_data.error_no.error1 = EDP_command_and_reply_buffer.reply_package.error_no.error1;
     break;
   case  ACKNOWLEDGE:
       EDP_data.speaking = EDP_command_and_reply_buffer.reply_package.arm.text_def.speaking;
   break;
   default:  // bledna przesylka
     throw ECP_error (NON_FATAL_ERROR, INVALID_EDP_REPLY);
 }; // end: switch (EDP_data.reply_type)
}; // end: ecp_speaker_robot::get_reply ()
