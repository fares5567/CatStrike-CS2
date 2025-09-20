#ifndef CORE_HPP
#define CORE_HPP

class core_c
{
public:
	class display_c;
	class settings_c;
	class process_info_c;

	display_c& get_display( ) { return this->display; }
	settings_c& get_settings( ) { return this->settings; }
	process_info_c& get_process_info( ) { return this->process_info; }
private:
	class display_c
	{
	public:
		int width;
		int height;
		float aspect;

		display_c( ) : width( GetSystemMetrics( SM_CXSCREEN ) ), height( GetSystemMetrics( SM_CYSCREEN ) ), aspect( width ? static_cast< float >( width ) / height : 0.0f ) { }
	};

	class settings_c
	{
	public:
		bool debug_mode{ true };
		struct aim_t
		{
			bool enabled{ true };
			bool rcs{ true };

			int key{ VK_XBUTTON2 };
			int type{ 0 }; // mouse, angles, silent

			float smooth{ 10.0f };
			float fov{ 100.0f };

			struct visualization_t
			{
				bool fov{ true };
				bool line{ true };
				bool dot{ false };
			} visualization;
		};

		struct visuals_t
		{
			struct player_t
			{
				bool box{ true };
				bool skeleton{ true };

				bool health{ true };

				bool name{ true };
				bool weapon{ true };
				bool distance{ true };

				bool oof_arrows{ true };

				bool chamies{ false };

				struct glow_t
				{
					bool enabled{ false };
					bool rainbow{ false };
					float rainbow_speed{ 0.1f };
					
					ImColor enemy_color{ 1.0f, 0.0f, 0.0f, 0.8f }; 
					ImColor team_color{ 0.0f, 0.5f, 1.0f, 0.8f }; 
					ImColor local_color{ 0.0f, 1.0f, 0.0f, 0.8f }; 
					
					bool health_based{ false };
					bool team_based{ true };  
					
					float intensity{ 1.0f };
					float alpha{ 0.8f };
				} glow;

				struct radar_t
				{
					bool enabled{ false };

					bool distance{ false };
					bool bomb{ false };
				} radar;
			} player;

			struct world_t
			{
				bool smoke{ false };
				bool smoke_dome{ false };

				bool molotov{ false };
				bool molotov_bounds{ false };

				bool drops{ false };
				bool drops_bounds{ true };

				bool bomb{ false };
			} world;

			struct spectator_t
			{
				bool enabled{ false };

				bool name{ true };
			} spectator;

			struct movement_tracers_t
			{
				bool local_trail{ false };
				bool enemy_trail{ false };
			} movement_tracers;
		};

		struct exploits_t
		{
			bool night_mode{ false };
			float night_mode_strength{ 1.0f };

			bool third_person{ false };
		};

		struct misc_t
		{
			bool vsync{ false };
			bool do_no_wait{ false };
			bool watermark{ true }; 
			bool team_check{ true }; 
		};

		aim_t aim;
		visuals_t visuals;
		exploits_t exploits;
		misc_t misc;
	};

	class process_info_c
	{
	public:
		std::uint32_t id;
		std::uintptr_t base;
		std::uintptr_t dtb;
		std::uintptr_t client_base;
		std::uintptr_t engine2_base;
	};

	display_c display;
	settings_c settings;
	process_info_c process_info;
};

#endif
