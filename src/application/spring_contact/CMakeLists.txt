add_executable(ecp_spring_contact
	ecp_g_spring_contact.cc
	ecp_t_spring_contact.cc
	ecp_st_spring_contact.cc
)

target_link_libraries(ecp_spring_contact ecp_generators ecp_subtasks ecp_r_irp6ot_m ecp_r_irp6p_m)

add_executable(mp_spring_contact
	mp_t_spring_contact.cc
)

target_link_libraries(mp_spring_contact mp)

install(TARGETS ecp_spring_contact mp_spring_contact DESTINATION bin)
