// -------------------------------------------------------------------------
//                              mp_task_rk.cc
// 
// MP Master Process - methods for rubik cube solver task sporadic coordination
// 
// Ostatnia modyfikacja: 16.04.98
// -------------------------------------------------------------------------


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <list>
#include <map>

#include "common/typedefs.h"
#include "common/impconst.h"
#include "common/com_buf.h"

#include "lib/srlib.h"
#include "mp/mp.h"
#include "mp/mp_g_force.h"
#include "mp/mp_g_vis.h"
#include "mp/mp_t_rcs.h"
#include "ecp_mp/ecp_mp_s_schunk.h"
#include "ecp_mp/ecp_mp_s_vis.h"
#include "ecp_mp/ecp_mp_tr_rc_windows.h"
#include "ecp_mp/ecp_mp_s_rcs_kociemba.h"
#include "ecp_mp/ecp_mp_s_rcs_korf.h"


//extern bool debug_tmp;

CUBE_COLORS read_cube_color(char input_char)
{
	switch (input_char)
	{
		case 'Y':
			return YELLOW;
		break;
		case 'W':
			return WHITE;
		break;
		case 'R':
			return RED;
		break;
		case 'O':
			return ORANGE;
		break;
		case 'G':
			return GREEN;
		break;
		case 'B':
			return BLUE;
		break;
		default:
			return UKNOWN_CUBE_COLOR;
		break;			
	}	
};
 

CUBE_TURN_ANGLE read_cube_turn_angle(char input_char)
{
	switch (input_char)
	{
		case '0':
			return CL_0;
		break;
		case '1':
			return CL_90;
		break;
		case '2':
			return CL_180;
		break;
		case '3':
			return CCL_90;
		break;
		default:
			return UKNOWN_TURN_ANGLE;
		break;			
	}	
};


// single_manipulation_class

void single_manipulation_class::set_state (CUBE_COLORS face_to_turn_l, CUBE_TURN_ANGLE turn_angle_l)
	{ face_to_turn = face_to_turn_l; turn_angle = turn_angle_l;};

// konstruktory
single_manipulation_class::single_manipulation_class(void){};
single_manipulation_class::single_manipulation_class(CUBE_COLORS face_to_turn_l, CUBE_TURN_ANGLE turn_angle_l)
	{ };
single_manipulation_class::single_manipulation_class (const single_manipulation_class& cs)
	{ set_state(cs.face_to_turn, cs.turn_angle);};


// CUBE_STATE_CLASS

cube_state_class::cube_state_class ()
{
	up = UKNOWN_CUBE_COLOR;
	down = UKNOWN_CUBE_COLOR;
	front = UKNOWN_CUBE_COLOR;
	rear = UKNOWN_CUBE_COLOR;
	left = UKNOWN_CUBE_COLOR;
	right = UKNOWN_CUBE_COLOR;
};


cube_state_class::cube_state_class (CUBE_COLORS up_is, CUBE_COLORS down_is, CUBE_COLORS front_is, 
	CUBE_COLORS rear_is, CUBE_COLORS left_is, CUBE_COLORS right_is)
{
	set_state(up_is, down_is, front_is, rear_is, left_is, right_is);
};


cube_state_class::cube_state_class (const cube_state_class& cs)
{
	set_state(cs.up, cs.down, cs.front, cs.rear, cs.left, cs.right);
};


cube_state_class& cube_state_class::operator= (const cube_state_class& cs)
{
	if (this != &cs)
		this->set_state(cs.up, cs.down, cs.front, cs.rear, cs.left, cs.right);

	return *this;
};


void cube_state_class::set_state(CUBE_COLORS up_is, CUBE_COLORS down_is, CUBE_COLORS front_is, 
		CUBE_COLORS rear_is, CUBE_COLORS left_is, CUBE_COLORS right_is)
{
	up = up_is;
	down = down_is;
	front = front_is;
	rear = rear_is;
	left = left_is;
	right = right_is;
};


void cube_state_class::print_face_color(CUBE_COLORS face_name)
{
	switch (face_name)
	{
		case YELLOW:
			printf("YELLOW");
		break;
		case WHITE:
			printf("WHITE");
		break;
		case RED:
			printf("RED");
		break;
		case ORANGE:
			printf("ORANGE");
		break;
		case GREEN:
			printf("GREEN");
		break;
		case BLUE:
			printf("BLUE");
		break;
		default:
		break;			
	}	
};


void cube_state_class::print_cube_colors()
{
	
	printf("up: ");
	print_face_color(up);
	printf(", down: ");
	print_face_color(down);
	printf(", front: ");
	print_face_color(front);
	printf(", rear: ");
	print_face_color(rear);
	printf(", left: ");
	print_face_color(left);
	printf(", right: ");
	print_face_color(right);
	printf("\n");
	
};




// MP_RUBIK_CUBE_SOLVER_CLASS


void mp_task_rubik_cube_solver::initiate(CUBE_COLORS up_is, CUBE_COLORS down_is, CUBE_COLORS front_is, 
		CUBE_COLORS rear_is, CUBE_COLORS left_is, CUBE_COLORS right_is)
{
	cube_state = new cube_state_class(up_is, down_is, front_is, rear_is, left_is, right_is);
	
	cube_initial_state = NULL;
	
};


mp_task_rubik_cube_solver::mp_task_rubik_cube_solver() : mp_task()
{

};

mp_task_rubik_cube_solver::~mp_task_rubik_cube_solver()
{
    		delete cube_state;
}	
	


bool mp_task_rubik_cube_solver::identify_colors() //DO WIZJI (przekladanie i ogladanie scian)
{

	//sekwencja poczatkowa w kolejnosci: UP, DOWN, FRONT, BACK, LEFT, RIGHT
	//cube_initial_state=BGROWY
	

	// manianka		
	cube_state->set_state(BLUE, GREEN, RED, ORANGE, WHITE, YELLOW);
	
	CUBE_TURN_ANGLE changing_order[]={CL_0, CL_0, CL_180, CL_0, CL_180, CL_0};
				
	for(int k=0; k<6; k++)
	{
		    
		if(face_turn_op(CL_0))
			return true;
		
		if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "oglAdam kolory na Sciance", 1, ROBOT_SPEAKER)) {  return true;  }
	
	    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
	   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
	 		(1, 1, ROBOT_SPEAKER, 
			ROBOT_SPEAKER)) {  return true;  }	

	    
		if (wait_ms(5000)) {  return true;  } //30 000 - OK //unrem		   //3000 - na lato na zime 5000 
		sensor_m[SENSOR_CAMERA_ON_TRACK]->initiate_reading();			
		if (wait_ms(1000)) {  return true;  }	
		sensor_m[SENSOR_CAMERA_ON_TRACK]->get_reading();
		
		for(int i=0; i<3; i++)
			for(int j=0; j<3; j++)	
				cube_state->cube_tab[k][3*i+j]=(char)sensor_m[SENSOR_CAMERA_ON_TRACK]->image.cube_face.colors[3*i+j];

				
		printf("\nFACE FACE %d:\n",k);	
		for(int i=0; i<9; i++)
		{
			switch (cube_state->cube_tab[k][i])
		 	{
				case 1: cube_state->cube_tab[k][i]='r';printf("R"); break;
				case 2: cube_state->cube_tab[k][i]='o';printf("O"); break;
				case 3: cube_state->cube_tab[k][i]='y'; printf("Y"); break;
				case 4: cube_state->cube_tab[k][i]='g'; printf("G"); break;
				case 5: cube_state->cube_tab[k][i]='b'; printf("B"); break;
				case 6: cube_state->cube_tab[k][i]='w'; printf("W"); break;
				default: cube_state->cube_tab[k][i]='o'; printf("?"); break;
		 	}
		}
		printf("\n");			
			
		if (wait_ms(1000)) {  return true;  }
		if(face_change_op(changing_order[k]))
			return true;
		    
	} //for
	return false;
}

/*
bool mp_task_rubik_cube_solver::communicate_with_windows_solver()
{
	char c_up;
	char c_right;
	char c_front;
	char c_down;
	char c_left;
	char c_back;
//	char face_c, rot_c, curr_c;
	int s;
	int str_size;
	char cube_tab_send[55];
	char manipulation_sequence[200];

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[2*9+3*i+j]=cube_state->cube_tab[0][3*i+j]; //rot cl 0 
			
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[1*9+3*j+2-i]=cube_state->cube_tab[1][3*i+j]; //rot cl 90
	
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[3*9+3*(2-j)+i]=cube_state->cube_tab[2][3*i+j]; //rot ccl 90
	
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[5*9+3*i+j]=cube_state->cube_tab[3][3*i+j]; //rot cl 0
 
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[4*9+3*j+2-i]=cube_state->cube_tab[4][3*i+j]; //rot cl 90
	
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[0*9+3*j+2-i]=cube_state->cube_tab[5][3*i+j]; //rot cl 90
	
	printf("SEQ IN COLOR : %s\n",cube_tab_send);
		
	c_up=cube_tab_send[4];
	c_right=cube_tab_send[13];
	c_front=cube_tab_send[22];
	c_down=cube_tab_send[31];
	c_left=cube_tab_send[40];
	c_back=cube_tab_send[49];
			
	printf("%c %c %c %c %c %c\n", c_up, c_right, c_front, c_down, c_left, c_back);
		
	for(int i=0; i<54; i++)
	{
		  	if (cube_tab_send[i] == c_up) 
				cube_tab_send[i]='u';
			else if (cube_tab_send[i] == c_down)
				 cube_tab_send[i]='d';
			else if (cube_tab_send[i] == c_front)
				 cube_tab_send[i]='f';
			else if (cube_tab_send[i] == c_back) 
				cube_tab_send[i]='b';
			else if (cube_tab_send[i] == c_right)
				cube_tab_send[i]='r';
			else if (cube_tab_send[i] == c_left) 
				cube_tab_send[i]='l';		
	}	
	cube_tab_send[54]='\0';
			
	printf("SEQ FROM VIS : %s\n",cube_tab_send);
			
	//reszta
	// struktura pomiocnicza
	single_manipulation_class single_manipulation;
	
	// czyszczenie listy
	manipulation_list.clear();
	
	for(int i=0; i<54; i++)
	{
		transmitter_m[TRANSMITTER_RC_WINDOWS]->to_va.rc_windows.rc_state[i]=cube_tab_send[i];   	
	}
	//mp_object.transmitter_m[TRANSMITTER_RC_WINDOWS]->to_va.rc_windows.rc_state[i]=patternx[i];
	transmitter_m[TRANSMITTER_RC_WINDOWS]->to_va.rc_windows.rc_state[54]='\0';
	
	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "mySlE", 1, ROBOT_SPEAKER)) {  return true;  }

    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(1, 1, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }	
	
	transmitter_m[TRANSMITTER_RC_WINDOWS]->t_write();
	
	
	
	transmitter_m[TRANSMITTER_RC_WINDOWS]->t_read(true);
	
	printf ("OPS: %s", transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence);
	
	strcpy (manipulation_sequence,transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence);
		    
	if ((manipulation_sequence[0]=='C') && (manipulation_sequence[1]=='u') && (manipulation_sequence[2]=='b') && (manipulation_sequence[3]=='e'))
	{
		printf("Jam jest daltonista. ktory Ci nie ulozy kostki\n");
		return true;
	}
	
	//sekwencja poczatkowa w kolejnosci: UP, DOWN, FRONT, BACK, LEFT, RIGHT
	//cube_initial_state=BGROWY

	
    s=0;
    str_size=0;
    for (unsigned int char_i=0; char_i < strlen(transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence)-1; char_i ++)
    {
		if (s==0)
		{
			switch (transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence[char_i]) 
			{
				case 'U': manipulation_sequence[str_size] = 'B'; break;
				case 'D': manipulation_sequence[str_size] = 'G'; break;
				case 'F': manipulation_sequence[str_size] = 'O'; break;
				case 'B': manipulation_sequence[str_size] = 'R'; break;
				case 'L': manipulation_sequence[str_size] = 'W'; break;
				case 'R': manipulation_sequence[str_size] = 'Y'; break;
			}
			s=1;
			str_size++;
		} 			
		else if (s==1)
		{
			switch (transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence[char_i]) 
			{
				case ' ': manipulation_sequence[str_size] = '1'; s=0; break;
				case '2': manipulation_sequence[str_size] = '2'; s=2; break;
				case '\'': manipulation_sequence[str_size] = '3'; s=2; break;
			}
			str_size++;
		}
		else if (s==2)
		{
			s=0;
		}
			
    }
			     
	if (s==1)
	{
		str_size--;
		manipulation_sequence[str_size] = '1';
		str_size++;		    
	}
	manipulation_sequence[str_size]='\0';
    
	printf ("\n%d %d\n",str_size,strlen(manipulation_sequence));
	printf ("SEQ from win %s\n",transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence);
	printf ("\nSEQ2 %s\n",manipulation_sequence);
		   	
	//pocztaek ukladania
	// dodawanie manipulacji do listy
	for (unsigned int char_i=0; char_i < strlen(manipulation_sequence)-1; char_i += 2)
	{
		single_manipulation.set_state(read_cube_color(manipulation_sequence[char_i]),
		read_cube_turn_angle(manipulation_sequence[char_i+1]));
		manipulation_list.push_back(single_manipulation);
	}

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "juZ ukLadam", 1, ROBOT_SPEAKER)) {  return true;  }

    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(1, 1, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }	


			    
	return false;
}
*/

bool mp_task_rubik_cube_solver::find_rcs()
{
	char c_up, c_right, c_front, c_down, c_left, c_back;
	int s;
	int str_size;
	char cube_tab_send[55];
	char cube_sol_rec[200];
	char manipulation_sequence[200];

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[2*9+3*i+j]=cube_state->cube_tab[0][3*i+j]; //rot cl 0 

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[1*9+3*j+2-i]=cube_state->cube_tab[1][3*i+j]; //rot cl 90

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[3*9+3*(2-j)+i]=cube_state->cube_tab[2][3*i+j]; //rot ccl 90

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[5*9+3*i+j]=cube_state->cube_tab[3][3*i+j]; //rot cl 0

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[4*9+3*j+2-i]=cube_state->cube_tab[4][3*i+j]; //rot cl 90

	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)	 
			cube_tab_send[0*9+3*j+2-i]=cube_state->cube_tab[5][3*i+j]; //rot cl 90

	printf("SEQ IN COLOR : %s\n",cube_tab_send);

	c_up=cube_tab_send[4];
	c_right=cube_tab_send[13];
	c_front=cube_tab_send[22];
	c_down=cube_tab_send[31];
	c_left=cube_tab_send[40];
	c_back=cube_tab_send[49];

//	cube_state->set_state(c_up, c_down, c_front, c_back, c_left, c_right);
	printf("%c %c %c %c %c %c\n", c_up, c_right, c_front, c_down, c_left, c_back);
		
	for(int i=0; i<54; i++)
	{
			if (cube_tab_send[i] == c_up) 
				cube_tab_send[i]='u';
			else if (cube_tab_send[i] == c_down)
				 cube_tab_send[i]='d';
			else if (cube_tab_send[i] == c_front)
				 cube_tab_send[i]='f';
			else if (cube_tab_send[i] == c_back) 
				cube_tab_send[i]='b';
			else if (cube_tab_send[i] == c_right)
				cube_tab_send[i]='r';
			else if (cube_tab_send[i] == c_left) 
				cube_tab_send[i]='l';		
	}
	cube_tab_send[54]='\0';

	printf("SEQ FROM VIS : %s\n",cube_tab_send);

	//reszta
	// struktura pomiocnicza
	single_manipulation_class single_manipulation;
	
	// czyszczenie listy
	manipulation_list.clear();
	
	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "mySlE", 1, ROBOT_SPEAKER)) {  return true;  }

    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(1, 1, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }	

	// Pobiera metoda znajdywania rozwiazania z pliku konfiguracyjnego.
	char* solver = config->return_string_value("solver");

	// znalezienie rozwiazania zadana metoda
	int sol_status;
	
	if (!strcmp(solver, "VSP"))
		sol_status = find_rcs_with_VSP(cube_tab_send, cube_sol_rec);
	else if (!strcmp(solver, "windows_solver"))
		sol_status = find_rcs_with_windows_solver(cube_tab_send, cube_sol_rec);
	else
		sol_status = RCS_SOLUTION_NOTPOSSIBLE;

	if (sol_status != RCS_SOLUTION_FOUND || cube_sol_rec == NULL)
	{
		printf("Jam jest daltonista. ktory Ci nie ulozy kostki\n");
		return true;
	} else {
		printf ("OPS: %s\n", cube_sol_rec);
		printf("Znalazlem rozwiazanie\n");
	}
	

	for (int i=0; i<strlen(cube_sol_rec); i++) {
		switch (cube_sol_rec[i]) 
		{
			case 'U': manipulation_sequence[i] = 'B'; break;
			case 'D': manipulation_sequence[i] = 'G'; break;
			case 'F': manipulation_sequence[i] = 'O'; break;
			case 'B': manipulation_sequence[i] = 'R'; break;
			case 'L': manipulation_sequence[i] = 'W'; break;
			case 'R': manipulation_sequence[i] = 'Y'; break;
/*			case 'U': manipulation_sequence[i] = c_up; break;
			case 'D': manipulation_sequence[i] = c_down; break;
			case 'F': manipulation_sequence[i] = c_front; break;
			case 'B': manipulation_sequence[i] = c_rear; break;
			case 'L': manipulation_sequence[i] = c_left; break;
			case 'R': manipulation_sequence[i] = c_right; break;
*/			default: manipulation_sequence[i] = cube_sol_rec[i]; break;
		}
	}
	manipulation_sequence[strlen(cube_sol_rec)] = '\0';
	
	printf ("\nSEQ2: %s\n",manipulation_sequence);

	//pocztaek ukladania
	// dodawanie manipulacji do listy
	for (unsigned int char_i=0; char_i < strlen(manipulation_sequence)-1; char_i += 2)
	{
		single_manipulation.set_state(read_cube_color(manipulation_sequence[char_i]),
		read_cube_turn_angle(manipulation_sequence[char_i+1]));
		manipulation_list.push_back(single_manipulation);
	}

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "juZ ukLadam", 1, ROBOT_SPEAKER)) {  return true;  }

    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(1, 1, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }	

	return false;
}


int mp_task_rubik_cube_solver::find_rcs_with_VSP(char* cube_state, char* cube_solution)
{
	// Zmienne na rozwiazania
	char *sol_korf = NULL, *sol_kociemba = NULL;
	bool sol_state = RCS_SOLUTION_NOTFOUND;

	// petle ustawiajace stan kostki w czujnikach, az do skutku
	bool korf_configured = false;
	bool kociemba_configured = false;
	
	// konfiguruje czujnik dla algorytmu Kociemby, ten od razu rozpoczyna prace	
	sensor_m[SENSOR_RCS_KOCIEMBA]->to_vsp.rcs.configure_mode = RCS_CUBE_STATE;
	strncpy(sensor_m[SENSOR_RCS_KOCIEMBA]->to_vsp.rcs.cube_state, cube_state, 54);
	sensor_m[SENSOR_RCS_KOCIEMBA]->configure_sensor(); 

	// inicjuje odczyt z czujnika dla algorytmu Korfa, az do skutku
	while (!korf_configured) {
		strncpy(sensor_m[SENSOR_RCS_KORF]->to_vsp.rcs.cube_state, cube_state, 54);
		sensor_m[SENSOR_RCS_KORF]->initiate_reading();
		if (sensor_m[SENSOR_RCS_KORF]->image.rcs.init_mode == RCS_INIT_SUCCESS)
			korf_configured = true;
		else
			sleep(1);
	}


	// petle aktywnego czekania na rozwiazanie
	bool sol_possible = true;
	bool sol_needed = true;
	bool korf_found = false;
	bool kociemba_found = false;
	bool time_elapsed;

	// zapewnienie ze znalezione rozwiazanie Kociemby
	while (!kociemba_found && sol_possible && sol_needed) {

		time_t t1, t2;
		int timeout = config->return_int_value("korf_timeout");
		t1 = time(NULL);
		time_elapsed = false;
		while (!korf_found && !time_elapsed && sol_possible && sol_needed) {

		    // odczytuje wynik z czujnika dla algorytmu Korfa
			sensor_m[SENSOR_RCS_KORF]->get_reading();
			sleep(1);
			if (sensor_m[SENSOR_RCS_KORF]->image.rcs.reading_mode == RCS_SOLUTION_NOTPOSSIBLE) {
				sol_possible = false;
			} else if (sensor_m[SENSOR_RCS_KORF]->image.rcs.reading_mode == RCS_SOLUTION_NOTNEEDED) {
				sol_needed = false;
			} else if (sensor_m[SENSOR_RCS_KORF]->image.rcs.reading_mode == RCS_SOLUTION_FOUND) {
				sol_korf = new char[200];
				strcpy(sol_korf, (char*) sensor_m[SENSOR_RCS_KORF]->image.rcs.cube_solution);
				printf("MP KR: %s\n", sol_korf);
				korf_found = true;
			}
	
			t2 = time(NULL);
			if (t2-t1 > timeout) {
				printf("MP KR Timeot\n");
			    time_elapsed = true;
			} else {
				//printf("MP KR still searching: %d\n", (t2-t1));
			}
			
		}

		// odczytuje ostanio znalezione rozwiazanie z czujnika dla algorytmu Kociemby
		sensor_m[SENSOR_RCS_KOCIEMBA]->get_reading();
		sleep(1);
		if (sensor_m[SENSOR_RCS_KOCIEMBA]->image.rcs.reading_mode == RCS_SOLUTION_NOTPOSSIBLE) {
			sol_possible = false;
		} else if (sensor_m[SENSOR_RCS_KOCIEMBA]->image.rcs.reading_mode == RCS_SOLUTION_NOTNEEDED) {
			sol_needed = false;
		} else if (sensor_m[SENSOR_RCS_KOCIEMBA]->image.rcs.reading_mode == RCS_SOLUTION_FOUND)  {
			sol_kociemba = new char[200];
			strcpy(sol_kociemba, (char*) sensor_m[SENSOR_RCS_KOCIEMBA]->image.rcs.cube_solution);
			printf("MP KC: %s\n", sol_kociemba);
			kociemba_found = true;
		}

	}

	// sprawdza czy mozliwe i konieczne ukladanie
	if (!sol_possible)
		sol_state = RCS_SOLUTION_NOTPOSSIBLE;
	else if (!sol_needed)
		sol_state = RCS_SOLUTION_NOTNEEDED;

	// wybiera najkrotsze rozwiazanie i informuje o nim
	else {
		int sol_len_korf = ( sol_korf == NULL ? 200 : strlen(sol_korf) );
		int sol_len_kociemba = ( sol_kociemba == NULL ? 200 : strlen(sol_kociemba) );
		if (sol_len_korf <= sol_len_kociemba) {
			strcpy(cube_solution, sol_korf);
			printf("MP KR SOL: %s\n", sol_korf);
		} else {
			strcpy(cube_solution, sol_kociemba);
			printf("MP KC SOL: %s\n", sol_kociemba);
		}
	
		// Usuniecie rozwiazan
		if (sol_korf) { delete[] sol_korf; sol_korf = NULL; }
		if (sol_kociemba) { delete[] sol_kociemba; sol_kociemba = NULL; }
	
		sol_state = RCS_SOLUTION_FOUND;
	}

	return sol_state;
}


int mp_task_rubik_cube_solver::find_rcs_with_windows_solver(char* cube_state, char* cube_solution)
{
	// przepisanie stanu kostki do bufora transmisyjnego
	for(int i=0; i<54; i++)
		transmitter_m[TRANSMITTER_RC_WINDOWS]->to_va.rc_windows.rc_state[i]=cube_state[i];
	transmitter_m[TRANSMITTER_RC_WINDOWS]->to_va.rc_windows.rc_state[54]='\0';

	// wyslanie stanu kostki do serwera windows
	transmitter_m[TRANSMITTER_RC_WINDOWS]->t_write();

	// odebranie rozwiazania kostki Rubika
	transmitter_m[TRANSMITTER_RC_WINDOWS]->t_read(true);

	// przepisanie i przeformatowanie rozwiazania kostki z bufora transmisyjnego
    int s=0;
    int str_size=0;
    for (unsigned int char_i=0; char_i < strlen(transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence)-1; char_i ++) {
		if (s==0) {
			cube_solution[str_size] = transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence[char_i];
			s=1;
			str_size++;
		} else if (s==1) {
			switch (transmitter_m[TRANSMITTER_RC_WINDOWS]->from_va.rc_windows.sequence[char_i]) 	{
				case ' ': cube_solution[str_size] = '1'; s=0; break;
				case '2': cube_solution[str_size] = '2'; s=2; break;
				case '\'': cube_solution[str_size] = '3'; s=2; break;
			}
			str_size++;
		} else if (s==2) {
			s=0;
		}
    }
			     
	if (s==1)
	{
		str_size--;
		cube_solution[str_size] = '1';
		str_size++;		    
	}
	cube_solution[str_size]='\0';

	// zwrocenie wlasciwego statusu rozwiazania
	if ((cube_solution[0]=='C') && (cube_solution[1]=='u') && (cube_solution[2]=='b') && (cube_solution[3]=='e'))
		return RCS_SOLUTION_NOTPOSSIBLE;
	return RCS_SOLUTION_FOUND;
}


bool mp_task_rubik_cube_solver::execute_manipulation_sequence()
{
	for(std::list<single_manipulation_class>::iterator manipulation_list_iterator = manipulation_list.begin(); 
		 manipulation_list_iterator != manipulation_list.end(); manipulation_list_iterator++)
	{
		if (manipulate(manipulation_list_iterator->face_to_turn, manipulation_list_iterator->turn_angle))
		{
				return true;
		}
	}
	return false;
}



bool mp_task_rubik_cube_solver::manipulate(CUBE_COLORS face_to_turn, CUBE_TURN_ANGLE turn_angle )
{

	if (face_to_turn == cube_state->up)
	{
		// printf("cube_state->up\n");
		if (face_change_op(CL_90))
		{
			return true;
		}
		if (face_turn_op(turn_angle))
		{
			return true;
		}
	} else if (face_to_turn == cube_state->down)
	{
		// printf("cube_state->down\n");
		if (face_change_op(CCL_90))
		{
			return true;
		}
		if (face_turn_op(turn_angle))
		{
			return true;
		}
		
	} else if (face_to_turn == cube_state->front)
	{
		// printf("cube_state->front\n");
		if (face_change_op(CL_0))
		{
			return true;
		}
		if (face_turn_op(CL_0))
		{
			return true;
		}
		if (face_change_op(CL_90))
		{
			return true;
		}
		if (face_turn_op(turn_angle))
		{
			return true;
		}
	} else if (face_to_turn == cube_state->rear)
	{
		// printf("cube_state->rear\n");
		if (face_change_op(CL_0))
		{
			return true;
		}
		if (face_turn_op(CL_0))
		{
			return true;
		}
		if (face_change_op(CCL_90))
		{
			return true;
		}
		if (face_turn_op(turn_angle))
		{
			return true;
		}
	} else if (face_to_turn == cube_state->left)
	{
		// printf("cube_state->left\n");
		if (face_change_op(CL_0))
		{
			return true;
		}
		if (face_turn_op(turn_angle))
		{
			return true;
		}
	} else if (face_to_turn == cube_state->right)
	{
		// printf("cube_state->right\n");
		if (face_change_op(CL_180))
		{
			return true;
		}
		if (face_turn_op(turn_angle))
		{
			return true;
		}
	}
	return false;
};



// obrot sciany
bool mp_task_rubik_cube_solver::face_turn_op(CUBE_TURN_ANGLE turn_angle)
{

	// zblizenie chwytakow


  	// wlaczenie generatora uczacego w obu robotach
	switch (turn_angle)
	{
		case CL_90:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fturn_ap_cl_90.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fturn_ap_cl_90.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		case CL_0:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fturn_ap_cl_0.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fturn_ap_cl_0.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		case CCL_90:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fturn_ap_ccl_90.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fturn_ap_ccl_90.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;	
		case CL_180:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fturn_ap_cl_180.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fturn_ap_cl_180.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		default:
		break;			
	}

    	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
	 	(3, 2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }


	// zacisniecie postumenta na kostce
	
	// Konfiguracja czujnikow sily
	sensor_m[SENSOR_FORCE_ON_TRACK]->to_vsp.parameters=1; // biasowanie czujnika
	sensor_m[SENSOR_FORCE_ON_TRACK]->configure_sensor();
	sensor_m[SENSOR_FORCE_POSTUMENT]->to_vsp.parameters=1; // biasowanie czujnika
	sensor_m[SENSOR_FORCE_POSTUMENT]->configure_sensor();
/*
	for (std::map <SENSOR_ENUM, sensor*>::iterator sensor_m_iterator = sensor_m.begin();
			 sensor_m_iterator != sensor_m.end(); sensor_m_iterator++)
	{
		sensor_m_iterator->second->to_vsp.parameters=1; // biasowanie czujnika
		sensor_m_iterator->second->configure_sensor();
	}
*/


		
    	// wlaczenie generatora zacisku na kostce w robocie irp6p
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FROM_OPEARTOR_PHASE_1, "", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FROM_OPEARTOR_PHASE_2, "", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }

	// obrot kostki

	switch (turn_angle)
	{
		case CL_90:
			if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "obracam kostkE", 1, ROBOT_SPEAKER)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_FACE_ROTATE, (int) RCSC_CL_90, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }

			// uruchomienie generatora empty_gen
		    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
			 	(2, 2, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER)) {  return true;  }

		break;
		case CL_0:
		break;
		case CCL_90:
			if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "obracam kostkE", 1, ROBOT_SPEAKER)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_FACE_ROTATE, (int) RCSC_CCL_90, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }

			// uruchomienie generatora empty_gen
		    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
			 	(2, 2, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER)) {  return true;  }
		break;	
		case CL_180:
			if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "obracam kostkE", 1, ROBOT_SPEAKER)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_FACE_ROTATE, (int) RCSC_CL_180, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }	

			// uruchomienie generatora empty_gen
		    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
			 	(2, 2, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER)) {  return true;  }

		break;
		default:
		break;			
	}	
	
	// rozwarcie chwytaka tracka

    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) RCSC_GRIPPER_OPENING, (int) RCSC_GO_VAR_2, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
    	// wlaczenie generatora zacisku na kostce w robocie irp6ot

	// odejscie tracka od postumenta
	
	// wlaczenie generatora uczacego  robocie irp6ot
	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fturn_de.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
 	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	
	return false;
};



// zmiana sciany (przelozenie kostki)
bool mp_task_rubik_cube_solver::face_change_op(CUBE_TURN_ANGLE turn_angle)
{

	// zblizenie chwytakow

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "przekLadam kostkE", 1, ROBOT_SPEAKER)) {  return true;  }

 	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP



	// wlaczenie generatora uczacego w obu robotach
	switch (turn_angle)
	{
		case CL_90:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_ap_cl_90.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_ap_cl_90.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		case CL_0:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_ap_cl_0.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_ap_cl_0.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		case CCL_90:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_ap_ccl_90.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_ap_ccl_90.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;	
		case CL_180:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_ap_cl_180.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_ap_cl_180.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		default:
		break;			
	}	

    	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
	 	(3, 2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }

	// zacisniecie tracka na kostce

	// Konfiguracja czujnikow sily
	sensor_m[SENSOR_FORCE_ON_TRACK]->to_vsp.parameters=1; // biasowanie czujnika
	sensor_m[SENSOR_FORCE_ON_TRACK]->configure_sensor();
	sensor_m[SENSOR_FORCE_POSTUMENT]->to_vsp.parameters=1; // biasowanie czujnika
	sensor_m[SENSOR_FORCE_POSTUMENT]->configure_sensor();
/*
	for (std::map <SENSOR_ENUM, sensor*>::iterator sensor_m_iterator = sensor_m.begin();
			 sensor_m_iterator != sensor_m.end(); sensor_m_iterator++)
		{
			sensor_m_iterator->second->to_vsp.parameters=1; // biasowanie czujnika
			sensor_m_iterator->second->configure_sensor();
		}
*/


    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FCHANGE_PHASE_1, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FCHANGE_PHASE_2, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }

     
    	// docisniecie chwytaka tracka do kostki
    	
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_GRIPPER_APPROACH, (int) 0, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	

	// zacisniecie tracka na kostce
	
    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FCHANGE_PHASE_3, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	

	
	// wstepne rozwarcie chwytaka postumenta
    	if (set_next_ecps_state ((int) RCSC_GRIPPER_OPENING, (int) RCSC_GO_VAR_1, "", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	

  
	 // ostateczne zacisniecie tracka na kostce    
	     
    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FCHANGE_PHASE_4, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	     

	// dalsze rozwarcie chwytaka postumenta

    	if (set_next_ecps_state ((int) RCSC_GRIPPER_OPENING, (int) RCSC_GO_VAR_2, "", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen (false, 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	

	// odejscie tracka od postumenta

  	// wlaczenie generatora uczacego w obu robotach
	switch (turn_angle)
	{
		case CL_90:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_de_cl_90.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_de_cl_90.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		case CL_0:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_de_cl_0.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_de_cl_0.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		case CCL_90:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_de_ccl_90.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_de_ccl_90.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;	
		case CL_180:
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_fchange_de_cl_180.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
			if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_fchange_de_cl_180.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		break;
		default:
		break;			
	}	

    	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
	 	(3, 2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }
	

	// zmiana stanu kostki

	cube_state_class tmp_cube_state;

	switch (turn_angle)
	{
		case CL_90:
			tmp_cube_state.set_state(cube_state->left, cube_state->right, cube_state->up, cube_state->down, 
			cube_state->front, cube_state->rear);
		break;
		case CL_0:
			tmp_cube_state.set_state(cube_state->front, cube_state->rear, cube_state->left, cube_state->right, 
			cube_state->up, cube_state->down);
		break;
		case CCL_90:
			tmp_cube_state.set_state(cube_state->right, cube_state->left, cube_state->down, cube_state->up, 
			cube_state->front, cube_state->rear);
		break;	
		case CL_180:
			tmp_cube_state.set_state(cube_state->front, cube_state->rear, cube_state->right, cube_state->left, 
			cube_state->down, cube_state->up);
		break;
		default:
		break;			
	}

	*cube_state = tmp_cube_state;

	//	cube_state->print_cube_colors();
	return false;
};


 
// dojscie
bool mp_task_rubik_cube_solver::approach_op(int mode)
{
	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "jestem podatny", 1, ROBOT_SPEAKER)) {  return true;  }

    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(1, 1, ROBOT_SPEAKER, ROBOT_SPEAKER)) {  return true;  }	


	// wlaczenie genrator tff_nose_run_generator w tracku
	if (set_next_ecps_state ((int) ECP_GEN_TFF_NOSE_RUN, (int) 0, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }

	// uruchomienie generatora empty_gen	
	if (run_ext_empty_gen (true, 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }


  	// przerwanie pracy generatora w ECP
	if (send_end_motion_to_ecps (1, ROBOT_IRP6_ON_TRACK)) {  return true;  }



    	// wlaczenie generatora uczacego w obu robotach
	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_ap_1.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_ap_1.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }

    	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP

//	(1, 1, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }	
     
    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(3, 2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }
		
		
	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "jestem robotem usLugowym", 1, ROBOT_SPEAKER)) {  return true;  }     
   	// wlaczenie generatora uczacego  robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_ap_2.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }

   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(3, 1, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "uLoZe kostkE rubika", 1, ROBOT_SPEAKER)) {  return true;  }     
	
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(3, 1, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }
     
	// powiedzenie 
	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "podaj kostkE", 1, ROBOT_SPEAKER)) {  return true;  }


	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(2, 2, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER)) {  return true;  }


    	// wlaczenie generatora transparentnego w obu robotach
    	if (set_next_ecps_state ((int) ECP_GEN_TRANSPARENT, (int) 0, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
    	if (set_next_ecps_state ((int) ECP_GEN_TRANSPARENT, (int) 0, "", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }
     
	
	// opcjonalne serwo wizyjne
	if (mode)
	{
 		mp_seven_eye_generator eyegen(*this, 4);
 		eyegen.robot_m[ROBOT_IRP6_ON_TRACK] = robot_m[ROBOT_IRP6_ON_TRACK]; 
	    	eyegen.sensor_m[SENSOR_CAMERA_SA] = sensor_m[SENSOR_CAMERA_SA];
	
		if (Move ( eyegen) )
		{
		     return true;
	     }
	}
	
	if (send_end_motion_to_ecps (2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }

	

	// zacisniecie chwytaka na kostce

    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FROM_OPEARTOR_PHASE_1, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(2, 1, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK)) {  return true;  }
    	// wlaczenie generatora zacisku na kostce w robocie irp6ot
    	if (set_next_ecps_state ((int) ECP_GEN_TFF_RUBIK_GRAB, (int) RCSC_RG_FROM_OPEARTOR_PHASE_2, "", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	// uruchomienie generatora empty_gen
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(2, 2, ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_SPEAKER)) {  return true;  }

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "puSC kostkE", 1, ROBOT_SPEAKER)) {  return true;  }

    	   	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
   	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(1, 1, ROBOT_SPEAKER, ROBOT_SPEAKER)) {  return true;  }	

	if (wait_ms(1000)) {  return true;  }

 	// wlaczenie generatora uczacego w obu robotach
	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_ap_3.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }

    	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP

//	(1, 1, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }	
     
    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
 		(3, 2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }


	return false;
};


// odejscie
bool mp_task_rubik_cube_solver::departure_op()
{

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "skoNczyLem", 1, ROBOT_SPEAKER)) {  return true;  }

    	// wlaczenie generatora uczacego w obu robotach
	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6ot_de_1.trj", 1, ROBOT_IRP6_ON_TRACK)) {  return true;  }
	if (set_next_ecps_state ((int) ECP_GEN_TEACH_IN, 0, "../trj/rcsc/irp6p_de_1.trj", 1, ROBOT_IRP6_POSTUMENT)) {  return true;  }


    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
	 	(3, 1, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_SPEAKER)) {  return true;  }

	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "jadE pracowaC do angli.", 1, ROBOT_SPEAKER)) {  return true;  }
	//	if (set_next_ecps_state ((int) ECP_GEN_SPEAK, 0, "jak sie paNstwu podobaLo", 1, ROBOT_SPEAKER)) {  return true;  }

    	// uruchomienie generatora empty_gen i oczekiwanie na zakonczenie obydwu generatorow ECP
    	if (run_ext_empty_gen_for_set_of_robots_and_wait_for_task_termin_mess_of_another_set_of_robots
	 	(3, 2, ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT, ROBOT_SPEAKER, 
		ROBOT_IRP6_ON_TRACK, ROBOT_IRP6_POSTUMENT)) {  return true;  }

	return false;
};


bool mp_task_rubik_cube_solver::gripper_opening(double track_increment, double postument_increment, int motion_time)
{

	trajectory_description tdes;

	tdes.arm_type = XYZ_EULER_ZYZ;
	tdes.interpolation_node_no = 1;
	tdes.internode_step_no = motion_time;
	tdes.value_in_step_no = tdes.internode_step_no - 2;
	// Wspolrzedne kartezjanskie XYZ i katy Eulera ZYZ
	tdes.coordinate_delta[0] = 0.0; // przyrost wspolrzednej X
	tdes.coordinate_delta[1] = 0.0;// przyrost wspolrzednej Y
	tdes.coordinate_delta[2] = 0.0;   // przyrost wspolrzednej Z
	tdes.coordinate_delta[3] = 0.0;   // przyrost wspolrzednej FI
	tdes.coordinate_delta[4] = 0.0;   // przyrost wspolrzednej TETA
	tdes.coordinate_delta[5] = 0.0;   // przyrost wspolrzednej PSI
	//	tdes.coordinate_delta[6] = 0.0;   // przyrost wspolrzednej PSI		
	tdes.coordinate_delta[6] = track_increment;   // przyrost wspolrzednej PSI		
	
	trajectory_description tdes2;

	tdes2.arm_type = XYZ_EULER_ZYZ;
	tdes2.interpolation_node_no = 1;
	tdes2.internode_step_no = motion_time;
	tdes2.value_in_step_no = tdes.internode_step_no - 2;
	// Wspolrzedne kartezjanskie XYZ i katy Eulera ZYZ
	tdes2.coordinate_delta[0] = 0.0; // przyrost wspolrzednej X
	tdes2.coordinate_delta[1] = 0.0;// przyrost wspolrzednej Y
	tdes2.coordinate_delta[2] = 0.0;   // przyrost wspolrzednej Z
	tdes2.coordinate_delta[3] = 0.0;   // przyrost wspolrzednej FI
	tdes2.coordinate_delta[4] = 0.0;   // przyrost wspolrzednej TETA
	tdes2.coordinate_delta[5] = 0.0;   // przyrost wspolrzednej PSI
	//	tdes2.coordinate_delta[6] = 0.02;   // przyrost wspolrzednej PSI
	tdes2.coordinate_delta[6] = postument_increment;   // przyrost wspolrzednej PSI
		
	// Generator trajektorii prostoliniowej
	mp_tight_coop_generator tcg(*this, tdes, tdes2);
	tcg.robot_m = robot_m;

	if (Move ( tcg))
	{
	    return true;
     }

	return false;
}



mp_task* return_created_mp_task (void)
{
	return new mp_task_rubik_cube_solver();
}



// methods fo mp template to redefine in concete class
void mp_task_rubik_cube_solver::task_initialization(void) 
{
	// Powolanie czujnikow
	sensor_m[SENSOR_FORCE_ON_TRACK] = 
		new ecp_mp_schunk_sensor (SENSOR_FORCE_ON_TRACK, "[vsp_force_irp6ot]", *this);
		
	sensor_m[SENSOR_FORCE_POSTUMENT] = 
		new ecp_mp_schunk_sensor (SENSOR_FORCE_POSTUMENT, "[vsp_force_irp6p]", *this);
		
	sensor_m[SENSOR_CAMERA_ON_TRACK] = 
		new ecp_mp_vis_sensor (SENSOR_CAMERA_ON_TRACK, "[vsp_vis_eih]", *this);
		
	sensor_m[SENSOR_CAMERA_SA] = 
		new ecp_mp_vis_sensor (SENSOR_CAMERA_SA, "[vsp_vis_sac]", *this);
	
	// Konfiguracja wszystkich czujnikow
	for (std::map <SENSOR_ENUM, sensor*>::iterator sensor_m_iterator = sensor_m.begin();
		 sensor_m_iterator != sensor_m.end(); sensor_m_iterator++)
	{
		sensor_m_iterator->second->to_vsp.parameters=1; // biasowanie czujnika
		sensor_m_iterator->second->configure_sensor();
	}
	usleep(1000*100);
	
	// dodanie transmitter'a
	transmitter_m[TRANSMITTER_RC_WINDOWS] = 
		new rc_windows_transmitter (TRANSMITTER_RC_WINDOWS, "[transmitter_rc_windows]", *this);
		

	// Powolanie czujnikow znajdujacych rozwiazanie kostki Rubika
	// Wywolanie osobno ze wzgledu na inny sposob ich konfiguracji.

	// tworzy i konfiguruje czujnik dla algorytmu Kociemby (w powloce nieinteraktywnej)
	sensor_m[SENSOR_RCS_KOCIEMBA] = new ecp_mp_rcs_kociemba(SENSOR_RCS_KOCIEMBA, "[vsp_rcs_kociemba]", *this);
	sensor_m[SENSOR_RCS_KOCIEMBA]->to_vsp.rcs.configure_mode = RCS_BUILD_TABLES;
	sensor_m[SENSOR_RCS_KOCIEMBA]->configure_sensor(); 

	// tworzy i konfiguruje czujnik dla algorytmu Korfa (w powloce interaktywnej bez oczekiwania)
	sensor_m[SENSOR_RCS_KORF] = new ecp_mp_rcs_korf(SENSOR_RCS_KORF, "[vsp_rcs_korf]", *this);
	sensor_m[SENSOR_RCS_KORF]->to_vsp.rcs.configure_mode = RCS_BUILD_TABLES;
	sensor_m[SENSOR_RCS_KORF]->configure_sensor(); 


	sr_ecp_msg->message("MP rcsc loaded");
};
 

void mp_task_rubik_cube_solver::main_task_algorithm(void)
{

	break_state = false;
  
    // odczyt konfiguracji manipulacji
    if (cube_initial_state) delete[] cube_initial_state;
	cube_initial_state = config->return_string_value("cube_initial_state");
	//	enum CUBE_COLORS {UKNOWN, RED, YELLOW, GREEN, BLUE, ORANGE, WHITE};
	//	 cube_state_class::set_state(CUBE_COLORS up_is, CUBE_COLORS down_is, CUBE_COLORS front_is, 
	//		CUBE_COLORS rear_is, CUBE_COLORS left_is, CUBE_COLORS right_is)

	initiate (read_cube_color(cube_initial_state[0]), 
		read_cube_color(cube_initial_state[1]), read_cube_color(cube_initial_state[2]),  read_cube_color(cube_initial_state[3]), 
		read_cube_color(cube_initial_state[4]), read_cube_color(cube_initial_state[5]));

	 // printf("przed wait for start \n");
      // Oczekiwanie na zlecenie START od UI  
	sr_ecp_msg->message("MP dla kostki Rubika - wcisnij start");
	wait_for_start ();
	// Wyslanie START do wszystkich ECP 
	start_all (robot_m);

	for (;;)
	{  // Wewnetrzna petla

       	// Zlecenie wykonania kolejnego makrokroku
		// printf("po start all \n");
		 for(;;)
		{
			sr_ecp_msg->message("Nowa seria");
			// Konfiguracja czujnikow sily
			sensor_m[SENSOR_FORCE_ON_TRACK]->to_vsp.parameters=1; // biasowanie czujnika
			sensor_m[SENSOR_FORCE_ON_TRACK]->configure_sensor();
			sensor_m[SENSOR_FORCE_POSTUMENT]->to_vsp.parameters=1; // biasowanie czujnika
			sensor_m[SENSOR_FORCE_POSTUMENT]->configure_sensor();

/*		
			for (std::map <SENSOR_ENUM, sensor*>::iterator sensor_m_iterator = sensor_m.begin();
				 sensor_m_iterator != sensor_m.end(); sensor_m_iterator++)
			{
				sensor_m_iterator->second->to_vsp.parameters=1; // biasowanie czujnika
				sensor_m_iterator->second->configure_sensor();
			}
*/

			// przechwycenie kostki
			if (approach_op( config->return_int_value("vis_servoing"))){
		        	break_state = true;
		          break;
		    }

			// IDENTIFY COLORS
			if (identify_colors()){
		        	break_state = true;
		          break;
		    }
		
		 	//if (communicate_with_windows_solver()){
		 	if (find_rcs()){
				if (departure_op()){
		        		break_state = true;
		          	break;
		 		}
			
			   	break_state = true;
		          break;
		    }
					    
		    // wykonanie sekwencji manipulacji
		     if (face_turn_op(CL_0)){
		        	break_state = true;
		          break;
		    }
		    
		    /*
		    	char* manipulation_sequence = config->return_string_value("manipulation_sequence");
		    	for (int char_i=0; char_i < strlen(manipulation_sequence)-1; char_i += 2)
			{
				single_manipulation.set_state(read_cube_color(manipulation_sequence[char_i]),
				read_cube_turn_angle(manipulation_sequence[char_i+1]));
				manipulation_list.push_back(single_manipulation);
			}
		  */  
		    
		    	if (execute_manipulation_sequence()){
		        	break_state = true;
		          break;
		    }
		    
		     // zakonczenie zadania
		    
		    printf ("przed face change\n");
		    
			if (face_change_op(CL_0)){
		        	break_state = true;
		          break;
		    }
		
		    	if (departure_op()){
		        	break_state = true;
		          break;
		    }

      	}
	 	
	 	
		if (break_state) 
		{
			break;
		}	
        // Oczekiwanie na STOP od UI
	  //          printf("w mp przed wait for stop\n");
        wait_for_stop (MP_THROW);// by Y - wlaczony tryb
  
        // Wyslanie STOP do wszystkich ECP po zakonczeniu programu uzytkownika
        terminate_all (robot_m);
        break; 
      } // koniec: for(;;) - wewnetrzna petla
};
