// ------------------------------------------------------------------------
// Proces:		EDP
// Plik:			kinematic_model_irp6m_with_wrist.h
// System:	QNX/MRROC++  v. 6.3
// Opis:		Model kinematyki robota IRp-6 na postumencie
//				- deklaracja klasy
//				- wykorzystanie nowego stopnia swobody  jako czynnego stopnia swobody
//
// Autor:		tkornuta
// Data:		31.01.2007
// ------------------------------------------------------------------------

#if !defined(_IRP6M_KIN_MODEL_WITH_WRIST)  
#define _IRP6M_KIN_MODEL_WITH_WRIST

// Definicja typu frame_tab.
#include "common/impconst.h"
// Definicja klasy kinematic_model.
#include "edp/common/kinematic_model.h" 

class kinematic_model_irp6m_with_wrist : public kinematic_model
{
protected:
  // Dlugosci czlonow
  double d1;
  double a2;
  double a3;
  double d5;
  double d6;
  double d7;

  // Zmienne opisujace przekladnie dla wszystkich stopni swobody.
  double gear[7];
  double theta[7];

  // Zmienne uzywane przy obliczeniach zwiazanych z ramieniami dolnym i gornym.
 double sl123;
 double mi1;
 double ni1;
 double mi2;
 double ni2;

  /* Zmienne zwiazane z obliczeniami zwarcia/rozwarcia chwytaka.
  double dir_a_6;
  double dir_b_6;
  double dir_c_6;
  double inv_a_6;
  double inv_b_6;
  double inv_c_6;
  double inv_d_6;
*/
  // Zakresy ruchu walow silnikow w radianach.
  double lower_limit_axis[7];
  double upper_limit_axis[7];
  // Zakresy ruchu poszczegolnych stopni swobody (w radianach lub milimetrach).
  double lower_limit_joint[7];
  double upper_limit_joint[7];

  // Polozenia synchronizacji (polozenia walow silnikow).
  double synchro_motor_position[7];
  // Polozenia synchronizacji (polozenia we wspolrzednych wewnetrznych).
  double synchro_joint_position[7];
  
  // Ustawienie parametrow kinematycznych.
  virtual void set_kinematic_parameters(void);

  // Sprawdzenie ograniczen na polozenia katowe walow silnikow
  virtual void check_motor_position(double motor_position[]);

  // Sprawdzenie ograniczen na wspolrzedne wewnetrzne
  virtual void check_joints(double q[]);

public:
  // Konstruktor.
  kinematic_model_irp6m_with_wrist ( void );

  // Przeliczenie polozenia walow silnikow na wspolrzedne wewnetrzne.
  virtual void mp2i_transform(double* local_current_motor_pos, double* local_current_joints); 

  // Przeliczenie wspolrzednych wewnetrznych na polozenia walow silnikow.
  virtual void i2mp_transform(double* local_desired_motor_pos_new, double* local_desired_joints);

  // Rozwiazanie prostego zagadnienia kinematyki.
  virtual void direct_kinematics_transform(double* local_current_joints, frame_tab* local_current_end_effector_frame_m);
  
  // Rozwiazanie odwrotnego zagadnienia kinematyki.
  virtual void inverse_kinematics_transform(double* local_desired_joints, double* local_current_joints, frame_tab* local_desired_end_effector_frame_m);

};//: kinematic_model_irp6m_with_wrist;

#endif					   

