#include <include/global.hpp>

void dispatch_c::run( ImDrawList* drawlist )
{
	g->sdk.w2s.update( );
	g->sdk.camera.update( );

	{
		g->features.aim.get_fov( ) = g->core.get_settings( ).aim.fov;

		if ( g->core.get_settings( ).aim.enabled )
		{
			g->features.aim.reset_closest_target( );
		}
	}

	{
		for ( const auto& player : g->updater.get_player_entities( ) )
		{
			const auto bone_data = g->sdk.bones.get_data( player.bone_array );
			if ( !bone_data.is_valid( ) )
			{
				continue;
			}

			const auto head = bone_data.get_position( sdk_c::bones_c::bone_id::head );
			const auto head_projected = g->sdk.w2s.project( head );

			if ( !g->sdk.w2s.is_valid( head_projected ) )
			{
				continue;
			}

			const auto visible = g->sdk.bones.is_any_major_bone_visible( bone_data );

			const auto color_primary = visible ? ImColor( 255, 255, 255, 255 ) : ImColor( 255, 105, 105, 255 );
			const auto color_secondary = visible ? ImColor( 255, 255, 255, 255 ) : ImColor( 220, 85, 85, 255 );

			if ( g->sdk.w2s.is_on_screen( head_projected ) )
			{
				const auto bound_data = g->sdk.bounds.get_data( player.m_p_game_scene_node );
				const auto distance = static_cast< int >( g->sdk.camera.get_data( ).position.distance( head ) / 52.5f );

				if ( g->core.get_settings( ).visuals.player.box )
				{
					g->features.visuals.player.box( drawlist, bound_data, color_primary );
				}

				if ( g->core.get_settings( ).visuals.player.skeleton )
				{
					g->features.visuals.player.skeleton( drawlist, bone_data, color_secondary );
				}

				if ( g->core.get_settings( ).visuals.player.health )
				{
					g->features.visuals.player.health_bar( drawlist, bound_data, player.health );
				}

				if ( g->core.get_settings( ).visuals.player.name )
				{
					g->features.visuals.player.name( drawlist, bound_data, player.name_data.name );
				}

				if ( g->core.get_settings( ).visuals.player.weapon )
				{
					g->features.visuals.player.weapon( drawlist, bound_data, player.weapon_data.name );
				}

				if ( g->core.get_settings( ).visuals.player.distance )
				{
					g->features.visuals.player.distance( drawlist, bound_data, distance );
				}

				if ( g->core.get_settings( ).visuals.player.chamies )
				{
					const auto cham_color = visible ? ImColor( 255, 255, 255, 75 ) : ImColor( 220, 85, 85, 75 );

					g->features.visuals.player.chamies.run( drawlist, player.hitbox_data, bone_data, cham_color );
				}

				// Render Glow/Chams effect
				g->features.visuals.player.glow.render( drawlist, bound_data, player.team, player.health, false );
			}

			if ( g->core.get_settings( ).visuals.player.oof_arrows && g->features.aim.is_outside_fov( head_projected ) )
			{
				g->features.visuals.indicators.oof_arrow( drawlist, head_projected, color_primary );
			}

			if ( g->core.get_settings( ).aim.enabled && visible )
			{
				features_c::aim_c::target_t target{};

				{
					target.position = head;
					target.cs_player_pawn = player.cs_player_pawn;
				}



				g->features.aim.find_closest_target( head_projected, target );
			}
		}
	}

	{
		for ( const auto& misc : g->updater.get_misc_entities( ) )
		{
			if ( !misc.entity )
			{
				continue;
			}

			if ( misc.type == udata_c::misc_entity_t::type_t::smoke )
			{
				if ( const auto* smoke = std::get_if<udata_c::misc_entity_t::smoke_data_t>( &misc.data ) )
				{
					if ( smoke->detonation_pos.x == 0.0f && smoke->detonation_pos.y == 0.0f && smoke->detonation_pos.z == 0.0f )
					{
						continue;
					}

					const auto distance = static_cast< int >( g->sdk.camera.get_data( ).position.distance( smoke->detonation_pos ) / 52.5f );

					if ( g->core.get_settings( ).visuals.world.smoke_dome )
					{
						// havent finished voxel data parser so nothing yet
					}

					if ( g->core.get_settings( ).visuals.world.smoke )
					{
						g->features.visuals.world.smoke( drawlist, smoke->detonation_pos, distance );
					}
				}
			}

			if ( misc.type == udata_c::misc_entity_t::type_t::molotov )
			{
				if ( const auto* molotov = std::get_if<udata_c::misc_entity_t::molotov_data_t>( &misc.data ) )
				{
					if ( molotov->center.x == 0.0f && molotov->center.y == 0.0f && molotov->center.z == 0.0f )
					{
						continue;
					}

					const auto distance = static_cast< int >( g->sdk.camera.get_data( ).position.distance( molotov->center ) / 52.5f );

					if ( g->core.get_settings( ).visuals.world.molotov_bounds && !molotov->fire_points.empty( ) )
					{
						g->features.visuals.world.molotov_bounds( drawlist, molotov->fire_points );
					}

					if ( g->core.get_settings( ).visuals.world.molotov )
					{
						g->features.visuals.world.molotov( drawlist, molotov->center, distance );
					}
				}
			}

			if ( misc.type == udata_c::misc_entity_t::type_t::bomb )
			{
				if ( const auto* bomb = std::get_if<udata_c::misc_entity_t::bomb_data_t>( &misc.data ) )
				{
					if ( bomb->position.x == 0.0f && bomb->position.y == 0.0f && bomb->position.z == 0.0f )
					{
						continue;
					}

					const auto distance = static_cast< int >( g->sdk.camera.get_data( ).position.distance( bomb->position ) / 52.5f );

					if ( g->core.get_settings( ).visuals.world.bomb )
					{
						g->features.visuals.world.bomb( drawlist, bomb->position, distance );
					}
				}
			}

			if ( misc.type == udata_c::misc_entity_t::type_t::drop )
			{
				if ( const auto* drop = std::get_if<udata_c::misc_entity_t::drop_data_t>( &misc.data ) )
				{
					if ( drop->position.x == 0.0f && drop->position.y == 0.0f && drop->position.z == 0.0f )
					{
						continue;
					}

					const auto distance = static_cast< int >( g->sdk.camera.get_data( ).position.distance( drop->position ) / 52.5f );

					if ( g->core.get_settings( ).visuals.world.drops )
					{
						const auto color = g->sdk.drops.get_color( drop->name );

						if ( g->core.get_settings( ).visuals.world.drops_bounds )
						{
							g->features.visuals.world.drop_bounds( drawlist, drop->node_transform, drop->mins, drop->maxs );
						}

						g->features.visuals.world.drop( drawlist, drop->position, drop->name, distance, color );
					}
				}
			}

			if ( misc.type == udata_c::misc_entity_t::type_t::post_processing_volume )
			{
				if ( g->core.get_settings( ).exploits.night_mode )
				{
					g->features.exploits.night_mode.run( misc.entity );
				}
			}
		}
	}

	{
		g->features.exploits.third_person.run( ); // man why am i still formatting my shit like this like its honestly insufferable 
	}

	if ( g->core.get_settings( ).aim.enabled )
	{
		const auto& closest_target = g->features.aim.get_closest_target( );
		if ( closest_target )
		{
			const auto position_projected = g->sdk.w2s.project( closest_target->position );
			const auto distance = g->sdk.camera.get_data( ).position.distance( closest_target->position ) / 52.5f;

	
			if ( g->core.get_settings( ).aim.visualization.line )
			{
				g->features.visuals.player.line( drawlist, position_projected, distance );
			}

			if ( g->core.get_settings( ).aim.visualization.dot )
			{
				g->features.visuals.player.dot( drawlist, position_projected, distance );
			}

			if ( GetAsyncKeyState( g->core.get_settings( ).aim.key ) & 0x8000 )
			{
				g->features.aim.apply_aim_toward( closest_target->position );
			}
		}
	}

	if ( g->core.get_settings( ).aim.visualization.fov )
	{
		g->features.visuals.hud.ring( drawlist );
	}

	g->features.visuals.movement_tracers.update_trails( drawlist );

	g->features.visuals.spectator.render_list( drawlist );

	const auto& players = g->updater.get_player_entities( );
	const auto& local_player = g->udata.get_owning_player( );
	const auto& misc_entities = g->updater.get_misc_entities( );
	g->features.visuals.radar.render( drawlist, players, local_player, misc_entities );
}