#include <deque>
#ifndef FEATURES_HPP
#define FEATURES_HPP

class features_c
{
public:
	class aim_c
	{
	public:
		struct target_t
		{
			math::vector3 position{};
			std::uintptr_t cs_player_pawn{};
	 
		};

		void reset_closest_target( );
		void find_closest_target( const math::vector2& position_projected, const target_t& player );
		bool is_outside_fov( const math::vector2& position_projected ) const;
		const target_t* get_closest_target( ) const;
		float& get_fov( );

		void apply_aim_toward( const math::vector3& world_position );

		bool is_aimbotting;

	private:
		float fov;
		float closest_distance;
		bool has_closest_target;
		target_t closest_target;
	};

	class visuals_c
	{
	public:
		class player_c
		{
		public:
			void box( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, ImColor color );
			void skeleton( ImDrawList* drawlist, const sdk_c::bones_c::data_t& bone_data, ImColor color );
			void health_bar( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, int health );
			void distance( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, int distance );
			void name( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, const std::string& name );
			void weapon( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, const std::string& weapon );

			void line( ImDrawList* drawlist, const math::vector2& target, int distance );
			void dot( ImDrawList* drawlist, const math::vector2& target, int distance );

			class glow_c
			{
			public:
				void render( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, int team, int health, bool is_local_player = false );
				ImColor get_glow_color( int team, int health, bool is_local_player = false );
				ImColor rainbow_glow();

			private:
				static float rainbow_time;
			};

			class chamies_c
			{
			public:
				void run( ImDrawList* drawlist, const sdk_c::hitboxes_c::data_t& hitbox_data, const sdk_c::bones_c::data_t& bone_data, ImColor color );

			private:
				void draw_capsule( ImDrawList* drawlist, const math::vector3& start, const math::vector3& end, float radius, const math::quaternion& rotation, const math::vector3& origin, ImColor color, int segments_max );

				void draw_capsule_filled( ImDrawList* drawlist, const math::vector3& top, const math::vector3& bottom, const math::vector3& axis, const math::vector3& u, const math::vector3& v, float radius, ImColor color, const std::vector<float>& sin_cache, const std::vector<float>& cos_cache, int segments, float distance );
				void draw_capsule_outline( ImDrawList* drawlist, const math::vector3& top, const math::vector3& bottom, const math::vector3& axis, const math::vector3& u, const math::vector3& v, float radius, ImColor color, const std::vector<float>& sin_cache, const std::vector<float>& cos_cache, int segments );

				void precompute_sincos( int segments, std::vector<float>& sin_cache, std::vector<float>& cos_cache );
				void create_circle( const math::vector3& center, const math::vector3& u, const math::vector3& v, float radius, const std::vector<float>& sin_cache, const std::vector<float>& cos_cache, std::vector<math::vector3>& out, int segments );
			};

			chamies_c chamies;
			glow_c glow;
		};

		class indicators_c
		{
		public:
			void oof_arrow( ImDrawList* drawlist, const math::vector2& position_projected, ImColor color );
		};

		class world_c
		{
		public:
			void molotov( ImDrawList* drawlist, const math::vector3& center, int distance );
			void molotov_bounds( ImDrawList* drawlist, const std::vector<math::vector3>& fire_points );

			void smoke( ImDrawList* drawlist, const math::vector3& detonation_pos, int distance );
			//void smoke_dome( ImDrawList* drawlist, const std::vector<math::vector3>& voxel_points );

			void bomb( ImDrawList* drawlist, const math::vector3& position, int distance );

			void drop( ImDrawList* drawlist, const math::vector3& position, const std::string& name, int distance, ImColor color );
			void drop_bounds( ImDrawList* drawlist, const math::transform& node_transform, const math::vector3& collision_min, const math::vector3& collision_ma );

		private:
			std::vector<ImVec2> compute_convex_hull( std::vector<ImVec2>& points ) const;
		};

		class hud_c
		{
		public:
			void ring( ImDrawList* drawlist );
		};

		class radar_c
		{
		public:
			void render( ImDrawList* drawlist, const std::vector<udata_c::player_entity_t>& players, const udata_c::owning_player_entity_t& local_player, const std::vector<udata_c::misc_entity_t>& misc_entities );

		private:
			ImVec2 revolve_coordinates_system( float revolve_angle, const ImVec2& origin_pos, const ImVec2& dest_pos );
			ImVec2 world_to_radar_improved( const math::vector3& world_pos, const math::vector3& local_pos, float local_yaw, const ImVec2& radar_center, float radar_radius );
			ImColor get_player_color( int team, int health );
			ImColor get_entity_color( const std::string& entity_type );
			
			float proportion = 2680.0f;     
			float circle_size = 4.0f;        
			float arrow_size = 11.0f;        
			float arc_arrow_size = 7.0f;  
			float render_range = 250.0f;     
			bool show_cross_line = true;     
			ImColor cross_color = ImColor(255, 255, 255, 255);
		};

		class movement_tracers_c
		{
		public:
			void update_trails( ImDrawList* drawlist );
		};

		class spectator_c
		{
		public:
			void render_list( ImDrawList* drawlist );
			
		private:
			std::uintptr_t get_observer_target( std::uintptr_t pawn_address );
		};

		player_c player;
		indicators_c indicators;
		world_c world;
		hud_c hud;
		radar_c radar;
		movement_tracers_c movement_tracers;
		spectator_c spectator;
	};

	class exploits_c
	{
	public:
		class anti_aim_c 
		{
		public:
			struct aa_data_t 
			{
				math::vector3 real_angles;
				math::vector3 fake_angles;
			} data;

			void run( );
		};

		class night_mode_c
		{
		public:
			void run( std::uintptr_t post_processing_volume );

		private:
			bool has_original;
			float original_min;
			float original_max;
		};

		class third_person_c
		{
		public:
			void run( );

		private:
			static constexpr std::uintptr_t jne_patch = 0x7E3697;
			static constexpr std::array<std::uint8_t, 2> original_jne = { 0x75, 0x10 };
			static constexpr std::array<std::uint8_t, 2> patched_jne = { 0xEB, 0x10 };

			bool currently_active;
			bool patch_applied;
			std::uintptr_t cached_player_pawn;

			void apply_patch( );
			void remove_patch( );
			void set_third_person_state( bool enabled );
			bool needs_reapply( std::uintptr_t current_player_pawn ) const;
		};

		anti_aim_c anti_aim;
		night_mode_c night_mode;
		third_person_c third_person;
	};

	aim_c aim;
	visuals_c visuals;
	exploits_c exploits;
};

#endif // !FEATURES_HPP
