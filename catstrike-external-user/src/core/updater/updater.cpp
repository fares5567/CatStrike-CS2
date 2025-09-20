#include <include/global.hpp>

void updater_c::deploy( )
{
	using namespace std::chrono;

	auto maintenance = std::thread( [ this ]( ) {
		auto last_clean = std::chrono::steady_clock::now( );

		while ( true )
		{
			try
			{
				this->update_roots( g->udata );

				{
					// Map update logic removed - raycasting was removed
				}

				const auto now = steady_clock::now( );
				if ( duration_cast< seconds >( now - last_clean ).count( ) >= 10 )
				{
					this->cleanup( );
					last_clean = now;
				}

				std::this_thread::sleep_for( milliseconds( 1000 ) );
			}
			catch ( ... )
			{
				std::this_thread::sleep_for( milliseconds( 1000 ) );
			}
		}
		} );

	auto local_player = std::thread( [ this ]( ) {
		while ( true ) {
			try {
				this->update_owning_player_entity( g->udata );
				std::this_thread::sleep_for( milliseconds( 100 ) );
			}
			catch ( ... ) {
				std::this_thread::sleep_for( milliseconds( 100 ) );
			}
		}
		} );

	auto player_entities = std::thread( [ this ]( ) {
		while ( true ) {
			try {
				this->process_player_entities( g->udata );
				std::this_thread::sleep_for( milliseconds( 5 ) );
			}
			catch ( ... ) {
				std::this_thread::sleep_for( milliseconds( 5 ) );
			}
		}
		} );

	auto misc_entities = std::thread( [ this ]( )
		{
			while ( true )
			{
				try {
					this->process_misc_entities( g->udata );
					std::this_thread::sleep_for( milliseconds( 15 ) );
				}
				catch ( ... ) {
					std::this_thread::sleep_for( milliseconds( 15 ) );
				}
			}
		} );

	maintenance.detach( );
	local_player.detach( );
	player_entities.detach( );
	misc_entities.detach( );
}

std::vector<udata_c::player_entity_t> updater_c::get_player_entities( ) const
{
	this->player_entity_buffer.update_read_buffer( );
	return this->player_entity_buffer.get_read_buffer( );
}

std::vector<udata_c::misc_entity_t> updater_c::get_misc_entities( ) const
{
	this->misc_entity_buffer.update_read_buffer( );
	return this->misc_entity_buffer.get_read_buffer( );
}

std::uintptr_t updater_c::resolve_entity_handle( std::uint32_t handle, udata_c& udata ) const
{
	if ( !handle )
	{
		return 0;
	}

	const auto list_entry = g->memory.read<std::uintptr_t>( udata.get_roots( ).dw_entity_list + 0x8ull * ( ( handle & 0x7FFF ) >> 9 ) + 0x10 );
	if ( !list_entry )
	{
		return 0;
	}

	return g->memory.read<std::uintptr_t>( list_entry + 120ull * ( handle & 0x1FF ) );
}

std::uintptr_t updater_c::get_entity_at_index( int index, udata_c& udata ) const
{
	const auto list_entry = g->memory.read<std::uintptr_t>( udata.get_roots( ).dw_entity_list + 0x8ull * ( index >> 9 ) + 0x10 );
	if ( !list_entry )
	{
		return 0;
	}

	return g->memory.read<std::uintptr_t>( list_entry + 120ull * ( index & 0x1FF ) );
}

void updater_c::update_roots( udata_c& udata )
{
	udata.get_roots( ).dw_entity_list = g->memory.read<std::uintptr_t>( g->core.get_process_info( ).client_base + offsets::dw_entity_list );
	udata.get_roots( ).dw_local_player_controller = g->memory.read<std::uintptr_t>( g->core.get_process_info( ).client_base + offsets::dw_local_player_controller );
}

void updater_c::update_owning_player_entity( udata_c& udata )
{
	const auto m_h_player_pawn = g->memory.read<std::uint32_t>( udata.get_roots( ).dw_local_player_controller + offsets::m_h_player_pawn );
	if ( !m_h_player_pawn )
	{
		return;
	}

	const auto pawn = this->resolve_entity_handle( m_h_player_pawn, udata );
	if ( !pawn )
	{
		return;
	}

	auto& owning_player = udata.get_owning_player( );
	owning_player.cs_player_pawn = pawn;
	owning_player.m_p_bullet_services = g->memory.read<std::uintptr_t>( pawn + offsets::m_p_bullet_services );
	owning_player.m_p_weapon_services = g->memory.read<std::uintptr_t>( pawn + offsets::m_p_weapon_services );
	owning_player.m_p_game_scene_node = g->memory.read<std::uintptr_t>( pawn + offsets::m_p_game_scene_node );
	owning_player.team = g->memory.read<int>( pawn + offsets::m_i_team_num );
	owning_player.weapon_data = g->sdk.weapon.get_data( pawn );
}

void updater_c::process_player_entities( udata_c& udata )
{
	if ( !udata.get_roots( ).dw_entity_list && !udata.get_owning_player( ).cs_player_pawn )
	{
		return;
	}

	auto& temp_player_entities = this->player_entity_buffer.get_write_buffer( );
	temp_player_entities.clear( );
	temp_player_entities.reserve( 64 );

	for ( int i = 0; i < 64; ++i )
	{
		const auto entity = this->get_entity_at_index( i, udata );
		if ( !entity )
		{
			continue;
		}

		const auto pawn_opt = this->resolve_player_entity( entity );
		if ( !pawn_opt.has_value( ) )
		{
			continue;
		}

		const auto pawn = pawn_opt.value( );
		if ( !pawn || pawn == udata.get_owning_player( ).cs_player_pawn )
		{
			continue;
		}

		const auto health = g->memory.read<int>( pawn + offsets::m_i_health );
		if ( health <= 0 || health > 1337 )
		{
			continue;
		}

		const auto team = g->memory.read<int>( pawn + offsets::m_i_team_num );
		if ( g->core.get_settings( ).misc.team_check && team == udata.get_owning_player( ).team )
		{
			continue;
		}

		const auto m_p_game_scene_node = g->memory.read<std::uintptr_t>( pawn + offsets::m_p_game_scene_node );
		if ( !m_p_game_scene_node )
		{
			continue;
		}

		const auto bone_array = g->memory.read<std::uintptr_t>( m_p_game_scene_node + offsets::m_model_state + 0x80 );
		if ( !bone_array )
		{
			continue;
		}

		udata_c::player_entity_t player{};
		player.cs_player_controller = entity;
		player.cs_player_pawn = pawn;
		player.m_p_game_scene_node = m_p_game_scene_node;
		player.bone_array = bone_array;
		player.team = team;
		player.health = health;

		if ( g->core.get_settings( ).visuals.player.chamies )
		{
			player.hitbox_data = g->sdk.hitboxes.get_data( m_p_game_scene_node );
		}

		if ( g->core.get_settings( ).visuals.player.name )
		{
			player.name_data = g->sdk.name.get_data( entity );
		}

		if ( g->core.get_settings( ).visuals.player.weapon )
		{
			player.weapon_data = g->sdk.weapon.get_data( pawn );
		}

		temp_player_entities.emplace_back( std::move( player ) );
	}

	this->player_entity_buffer.swap_write_buffer( );
}

void updater_c::process_misc_entities( udata_c& udata ) // shit coded
{
	if ( !udata.get_roots( ).dw_entity_list && !udata.get_owning_player( ).cs_player_pawn )
	{
		return;
	}

	std::vector<const char*> allowed_types;

	if ( g->core.get_settings( ).visuals.world.smoke || g->core.get_settings( ).visuals.world.smoke_dome )
	{
		allowed_types.emplace_back( "C_SmokeGrenadeProjectile" );
	}
	if ( g->core.get_settings( ).visuals.world.molotov || g->core.get_settings( ).visuals.world.molotov_bounds )
	{
		allowed_types.emplace_back( "C_Inferno" );
	}
	if ( g->core.get_settings( ).exploits.night_mode )
	{
		allowed_types.emplace_back( "C_PostProcessingVolume" );
	}
	if ( g->core.get_settings( ).visuals.world.bomb )
	{
		allowed_types.emplace_back( "C_PlantedC4" );
	}
	if ( g->core.get_settings( ).visuals.world.drops )
	{
		g->sdk.drops.add_to_allowed( allowed_types );
	}

	if ( allowed_types.empty( ) )
	{
		return;
	}

	auto& temp_misc_entities = this->misc_entity_buffer.get_write_buffer( );
	temp_misc_entities.clear( );
	temp_misc_entities.reserve( 1000 ); // eh

	for ( int i = 64; i < 1024; ++i )
	{
		const auto entity = this->get_entity_at_index( i, udata );
		if ( !entity )
		{
			continue;
		}

		const auto entity_type_opt = this->resolve_misc_entity_type( entity, allowed_types );
		if ( !entity_type_opt.has_value( ) )
		{
			continue;
		}

		const auto& entity_type = entity_type_opt.value( );
		if ( entity_type.empty( ) )
		{
			continue;
		}

		auto misc_type = udata_c::misc_entity_t::type_t::invalid;
		if ( _stricmp( entity_type.c_str( ), "C_SmokeGrenadeProjectile" ) == 0 )
		{
			misc_type = udata_c::misc_entity_t::type_t::smoke;
		}
		else if ( _stricmp( entity_type.c_str( ), "C_Inferno" ) == 0 )
		{
			misc_type = udata_c::misc_entity_t::type_t::molotov;
		}
		else if ( _stricmp( entity_type.c_str( ), "C_PostProcessingVolume" ) == 0 )
		{
			misc_type = udata_c::misc_entity_t::type_t::post_processing_volume;
		}
		else if ( _stricmp( entity_type.c_str( ), "C_EnvSky" ) == 0 )
		{
			misc_type = udata_c::misc_entity_t::type_t::env_sky;
		}
		else if ( _stricmp( entity_type.c_str( ), "C_PlantedC4" ) == 0 )
		{
			misc_type = udata_c::misc_entity_t::type_t::bomb;
		}
		else if ( g->sdk.drops.is( entity_type ) )
		{
			misc_type = udata_c::misc_entity_t::type_t::drop;
		}
		else
		{
			continue;
		}

		udata_c::misc_entity_t misc{};

		misc.entity = entity;
		misc.type = misc_type;

		if ( misc_type == udata_c::misc_entity_t::type_t::smoke )
		{
			if ( g->memory.read<bool>( entity + offsets::m_b_smoke_effect_spawned ) )
			{
				misc.data = udata_c::misc_entity_t::smoke_data_t{ .detonation_pos = g->memory.read<math::vector3>( entity + offsets::m_v_smoke_detonation_pos ) };
			}
		}
		else if ( misc_type == udata_c::misc_entity_t::type_t::molotov )
		{
			const auto fire_count = g->memory.read<int>( entity + offsets::m_fire_count );
			if ( fire_count <= 0 || fire_count > 64 )
			{
				continue;
			}

			udata_c::misc_entity_t::molotov_data_t molotov_data{};
			bool any_burning = false;

			for ( int j = 0; j < fire_count; ++j )
			{
				const auto is_burning = g->memory.read<bool>( entity + offsets::m_b_fire_is_burning + j );
				if ( !is_burning )
				{
					continue;
				}

				any_burning = true;
				const auto fire_pos = g->memory.read<math::vector3>( entity + offsets::m_fire_positions + sizeof( math::vector3 ) * j );
				molotov_data.fire_points.push_back( fire_pos );
			}

			if ( any_burning && !molotov_data.fire_points.empty( ) )
			{
				math::vector3 sum{};
				for ( const auto& pt : molotov_data.fire_points )
				{
					sum += pt;
				}

				molotov_data.center = sum / static_cast< float >( molotov_data.fire_points.size( ) );
				misc.data = std::move( molotov_data );
			}
			else
			{
				continue;
			}
		}
		else if ( misc_type == udata_c::misc_entity_t::type_t::bomb )
		{
			const auto m_p_game_scene_node = g->memory.read<std::uintptr_t>( entity + offsets::m_p_game_scene_node );
			if ( m_p_game_scene_node )
			{
				misc.data = udata_c::misc_entity_t::bomb_data_t{ .position = g->memory.read<math::vector3>( m_p_game_scene_node + offsets::m_vec_origin ) };
			}
		}
		else if ( misc_type == udata_c::misc_entity_t::type_t::drop ) // meh
		{
			const auto m_p_game_scene_node = g->memory.read<std::uintptr_t>( entity + offsets::m_p_game_scene_node );
			if ( m_p_game_scene_node )
			{
				const auto m_p_collision = g->memory.read<std::uintptr_t>( entity + offsets::m_p_collision );
				if ( m_p_collision )
				{
					const auto collision_min = g->memory.read<math::vector3>( m_p_collision + 0x40 ); // CCollisionProperty::m_vecMins
					const auto collision_max = g->memory.read<math::vector3>( m_p_collision + 0x4C ); // CCollisionProperty::m_vecMaxs

					const auto node_to_world = g->memory.read<math::transform>( m_p_game_scene_node + 0x10 );

					misc.data = udata_c::misc_entity_t::drop_data_t{
						.position = g->memory.read<math::vector3>( m_p_game_scene_node + offsets::m_vec_origin ),
						.name = g->sdk.drops.get_display_name( entity_type ),
						.mins = collision_min,
						.maxs = collision_max,
						.node_transform = node_to_world,
					};
				}
			}
		}

		temp_misc_entities.emplace_back( std::move( misc ) );
	}

	this->misc_entity_buffer.swap_write_buffer( );
}

std::optional<std::uintptr_t> updater_c::resolve_player_entity( std::uintptr_t entity )
{
	try
	{
		const auto handle = g->memory.read<std::uint32_t>( entity + offsets::m_h_player_pawn );
		if ( !handle )
		{
			return std::nullopt;
		}

		const auto pawn = this->resolve_entity_handle( handle, g->udata );
		if ( !pawn )
		{
			return std::nullopt;
		}

		return pawn;
	}
	catch ( ... )
	{
		return std::nullopt;
	}
}

std::optional<std::string> updater_c::resolve_misc_entity_type( std::uintptr_t entity, std::span<const char*> expected_classes )
{
	try
	{
		const auto ent_identity = g->memory.read<std::uintptr_t>( entity + 0x10 );
		if ( !ent_identity )
		{
			return std::nullopt;
		}

		const auto class_info = g->memory.read<std::uintptr_t>( ent_identity + 0x8 );
		if ( !class_info )
		{
			return std::nullopt;
		}

		const auto schema_info = g->memory.read<std::uintptr_t>( class_info + 0x30 );
		if ( !schema_info )
		{
			return std::nullopt;
		}

		const auto name_ptr = g->memory.read<std::uintptr_t>( schema_info + 0x8 );
		if ( !name_ptr )
		{
			return std::nullopt;
		}

		char ent_name[ 128 ]{};
		if ( !g->memory.read_process_memory( name_ptr, ent_name, sizeof( ent_name ) ) )
		{
			return std::nullopt;
		}

		ent_name[ 127 ] = '\0';

		for ( const auto& expected : expected_classes )
		{
			if ( _stricmp( ent_name, expected ) == 0 )
			{
				return std::string( ent_name );
			}
		}

		return std::nullopt;
	}
	catch ( ... )
	{
		return std::nullopt;
	}
}

void updater_c::cleanup( )
{
	g->sdk.hitboxes.clear_cache( );
	g->sdk.name.clear_cache( );
	g->sdk.weapon.clear_cache( );
}