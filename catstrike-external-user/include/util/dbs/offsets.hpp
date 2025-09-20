#ifndef OFFSETS_HPP
#define OFFSETS_HPP

namespace offsets {

	inline std::uintptr_t dw_csgo_input;
	inline std::uintptr_t dw_entity_list;
	inline std::uintptr_t dw_global_vars;
	inline std::uintptr_t dw_local_player_controller;
	inline std::uintptr_t dw_local_player_pawn;
	inline std::uintptr_t dw_view_matrix;
	inline std::uintptr_t dw_view_angles;

	inline std::uintptr_t dw_network_game_client;
	inline std::uintptr_t dw_network_game_client_delta_tick;

	inline std::uintptr_t m_i_health;
	inline std::uintptr_t m_i_team_num;
	inline std::uintptr_t m_i_shots_fired;
	inline std::uintptr_t m_h_player_pawn;
	inline std::uintptr_t m_p_entity;
	inline std::uintptr_t m_s_sanitized_player_name;
	inline std::uintptr_t m_p_collision;

	inline std::uintptr_t m_vec_origin;
	inline std::uintptr_t m_vec_view_offset;
	inline std::uintptr_t m_ang_eye_angles;
	inline std::uintptr_t m_ang_rotation;
	inline std::uintptr_t v_angle;

	inline std::uintptr_t m_vec_mins;
	inline std::uintptr_t m_vec_maxs;

	inline std::uintptr_t m_p_game_scene_node;
	inline std::uintptr_t m_model_state;

	inline std::uintptr_t m_i_rarity_override;

	inline std::uintptr_t m_p_clipping_weapon;
	inline std::uintptr_t m_p_bullet_services;
	inline std::uintptr_t m_p_weapon_services;
	inline std::uintptr_t m_total_hits_on_server;
	inline std::uintptr_t m_aim_punch_cache;


	inline std::uintptr_t m_v_smoke_color;
	inline std::uintptr_t m_v_smoke_detonation_pos;
	inline std::uintptr_t m_b_smoke_effect_spawned;
	inline std::uintptr_t m_n_smoke_effect_tick_begin;

	inline std::uintptr_t m_fire_count;
	inline std::uintptr_t m_b_fire_is_burning;
	inline std::uintptr_t m_fire_positions;

	inline std::uintptr_t m_b_exposure_control;
	inline std::uintptr_t m_fl_min_exposure;
	inline std::uintptr_t m_fl_max_exposure;

	
	inline std::uintptr_t m_h_pawn; 
	inline std::uintptr_t m_h_observer_target;
	inline std::uintptr_t m_p_observer_services;

} // namespace offsets

#endif // !OFFSETS_HPP
