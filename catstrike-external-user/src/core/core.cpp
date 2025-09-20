#include <include/global.hpp>

int main( )
{
	if ( !g->util.console.initialize( ) )
	{
		g->util.console.print_debug( ecrypt( "Failed to initialize console" ) );
		g->util.console.wait_for_input( );
		return 0;
	}

	// debug
	bool debug_mode = g->core.get_settings( ).debug_mode;
	
	if ( debug_mode )
	{
		g->util.console.clear_console( );
		g->util.console.print_success( ecrypt( "debug mode" ) );
		g->util.console.print_success( ecrypt( "overlay gets initalized" ) );
		g->util.console.print_success( ecrypt( "debug information wilol be shown now" ) );
		g->util.console.sleep_ms( 2000 );
	}
	else
	{
		// not debug
		g->util.console.clear_console( );
		g->util.console.print_waiting( ecrypt( "Welcome to CatStrike. Getting ready for you" ) );
		g->util.console.sleep_ms( 3000 );
	}

	
	if ( !debug_mode )
	{
		g->util.console.clear_console( );
	}
	
	{
		if ( !g->offset_db.load_all( ) )
		{
			g->util.console.print_debug( ecrypt( "Failed to load offset database" ) );
			g->util.console.wait_for_input( );
			return 0;
		}

		
		offsets::dw_csgo_input = g->offset_db.get_flat( ecrypt( "dwCSGOInput" ) );
		offsets::dw_entity_list = g->offset_db.get_flat( ecrypt( "dwEntityList" ) );
		offsets::dw_global_vars = g->offset_db.get_flat( ecrypt( "dwGlobalVars" ) );
		offsets::dw_local_player_controller = g->offset_db.get_flat( ecrypt( "dwLocalPlayerController" ) );
		offsets::dw_local_player_pawn = g->offset_db.get_flat( ecrypt( "dwLocalPlayerPawn" ) );
		offsets::dw_view_matrix = g->offset_db.get_flat( ecrypt( "dwViewMatrix" ) );
		offsets::dw_view_angles = g->offset_db.get_flat( ecrypt( "dwViewAngles" ) );

		offsets::dw_network_game_client = g->offset_db.get_flat( ecrypt( "dwNetworkGameClient" ) );
		offsets::dw_network_game_client_delta_tick = g->offset_db.get_flat( ecrypt( "dwNetworkGameClient_deltaTick" ) );

		offsets::m_i_health = g->offset_db.get_flat( ecrypt( "m_iHealth" ) );
		offsets::m_i_team_num = g->offset_db.get_flat( ecrypt( "m_iTeamNum" ) );
		offsets::m_i_shots_fired = g->offset_db.get_flat( ecrypt( "m_iShotsFired" ) );
		offsets::m_h_player_pawn = g->offset_db.get_flat( ecrypt( "m_hPlayerPawn" ) );
		offsets::m_p_entity = g->offset_db.get_flat( ecrypt( "m_pEntity" ) );
		offsets::m_s_sanitized_player_name = g->offset_db.get_flat( ecrypt( "m_sSanitizedPlayerName" ) );
		offsets::m_p_collision = g->offset_db.get_flat( ecrypt( "m_pCollision" ) );

		offsets::m_vec_origin = g->offset_db.get( ecrypt( "CGameSceneNode" ), ecrypt( "m_vecOrigin" ) );
		offsets::m_vec_view_offset = g->offset_db.get_flat( ecrypt( "m_vecViewOffset" ) );
		offsets::m_ang_eye_angles = g->offset_db.get_flat( ecrypt( "m_angEyeAngles" ) );
		offsets::m_ang_rotation = g->offset_db.get_flat( ecrypt( "m_angRotation" ) );
		offsets::v_angle = g->offset_db.get_flat( ecrypt( "v_angle" ) );

		offsets::m_vec_mins = g->offset_db.get_flat( ecrypt( "m_vecMins" ) );
		offsets::m_vec_maxs = g->offset_db.get_flat( ecrypt( "m_vecMaxs" ) );

		offsets::m_p_game_scene_node = g->offset_db.get_flat( ecrypt( "m_pGameSceneNode" ) );
		offsets::m_model_state = g->offset_db.get_flat( ecrypt( "m_modelState" ) );

		offsets::m_i_rarity_override = g->offset_db.get_flat( ecrypt( "m_iRarityOverride" ) );

		offsets::m_p_clipping_weapon = g->offset_db.get_flat( ecrypt( "m_pClippingWeapon" ) );
		offsets::m_p_bullet_services = g->offset_db.get_flat( ecrypt( "m_pBulletServices" ) );
		offsets::m_p_weapon_services = g->offset_db.get_flat( ecrypt( "m_pWeaponServices" ) );
		offsets::m_total_hits_on_server = g->offset_db.get_flat( ecrypt( "m_totalHitsOnServer" ) );
		offsets::m_aim_punch_cache = g->offset_db.get_flat( ecrypt( "m_aimPunchCache" ) );


		offsets::m_v_smoke_color = g->offset_db.get( ecrypt( "C_SmokeGrenadeProjectile" ), ecrypt( "m_vSmokeColor" ) );
		offsets::m_v_smoke_detonation_pos = g->offset_db.get( ecrypt( "C_SmokeGrenadeProjectile" ), ecrypt( "m_vSmokeDetonationPos" ) );
		offsets::m_b_smoke_effect_spawned = g->offset_db.get( ecrypt( "C_SmokeGrenadeProjectile" ), ecrypt( "m_bSmokeEffectSpawned" ) );
		offsets::m_n_smoke_effect_tick_begin = g->offset_db.get( ecrypt( "C_SmokeGrenadeProjectile" ), ecrypt( "m_nSmokeEffectTickBegin" ) );

		offsets::m_fire_count = g->offset_db.get_flat( ecrypt( "m_fireCount" ) );
		offsets::m_b_fire_is_burning = g->offset_db.get_flat( ecrypt( "m_bFireIsBurning" ) );
		offsets::m_fire_positions = g->offset_db.get_flat( ecrypt( "m_firePositions" ) );

		offsets::m_b_exposure_control = g->offset_db.get( ecrypt( "C_PostProcessingVolume" ), ecrypt( "m_bExposureControl" ) );
		offsets::m_fl_min_exposure = g->offset_db.get( ecrypt( "C_PostProcessingVolume" ), ecrypt( "m_flMinExposure" ) );
		offsets::m_fl_max_exposure = g->offset_db.get( ecrypt( "C_PostProcessingVolume" ), ecrypt( "m_flMaxExposure" ) );

		
		offsets::m_h_pawn = g->offset_db.get_flat( ecrypt( "m_hPawn" ) );  // die eig sind  m_hPawn statt m_hObserverPawn
		offsets::m_h_observer_target = g->offset_db.get_flat( ecrypt( "m_hObserverTarget" ) );
		offsets::m_p_observer_services = g->offset_db.get_flat( ecrypt( "m_pObserverServices" ) );
		//ausgaben von den offstes fürn debug mode damit man auchg weiss ob die richtig sind lol
		if ( debug_mode )
		{
			g->util.console.print_success( ecrypt( "Offsets loaded successfully" ) );
			g->util.console.print_debug( ecrypt( "dwCSGOInput: 0x%08X" ), offsets::dw_csgo_input );
			g->util.console.print_debug( ecrypt( "dwEntityList: 0x%08X" ), offsets::dw_entity_list );

			g->util.console.print_debug( ecrypt( "Entity List Base: 0x%08X" ), offsets::dw_entity_list );
			
			
			g->util.console.print_debug( ecrypt( "m_hPawn: 0x%08X" ), offsets::m_h_pawn );
			g->util.console.print_debug( ecrypt( "m_hObserverTarget: 0x%08X" ), offsets::m_h_observer_target );
			g->util.console.print_debug( ecrypt( "m_pObserverServices: 0x%08X" ), offsets::m_p_observer_services );
		}
	}
	
	if ( !debug_mode )
	{
		g->util.console.sleep_ms( 4000 );
	}

	
	if ( !debug_mode )
	{
		g->util.console.clear_console( );
	}
	
	{
		if ( !g->skin_db.load_all( ) )
		{
			g->util.console.print_debug( ecrypt( "Failed to load skin database" ) );
			g->util.console.wait_for_input( );
			return 0;
		}

		skins::ak47_skin_id = g->skin_db.get( ecrypt( "AK-47 | Inheritance" ) ); // removed skin changed but this skin id loader is sorta useful  
		
		if ( debug_mode )
		{
			g->util.console.print_success( ecrypt( "Skins loaded successfully" ) );
			g->util.console.print_debug( ecrypt( "AK-47 Skin ID: %d" ), skins::ak47_skin_id );
		}
	}
	
	if ( !debug_mode )
	{
		g->util.console.sleep_ms( 4000 );
	}

	
	if ( !debug_mode )
	{
		g->util.console.clear_console( );
		g->util.console.print_waiting( ecrypt( "Waiting For Cs2 to open ..." ) );
	}
	else
	{
		g->util.console.print_debug( ecrypt( "Looking for CS2 process..." ) );
	}

	
	auto& process = g->core.get_process_info( );
	while ( true )
	{
		process.id = g->memory.get_process_id( ecrypt( L"cs2.exe" ) );
		if ( process.id != 0 )
		{
			break;
		}
		if ( debug_mode )
		{
			g->util.console.print_debug( ecrypt( "CS2 not found, retrying..." ) );
		}
		g->util.console.sleep_ms( 1000 );
	}

	
	if ( !debug_mode )
	{
		g->util.console.clear_console( );
		g->util.console.print_found( ecrypt( "Found Cs2 Exe Loading pls wait" ) );
	}
	else
	{
		g->util.console.print_success( ecrypt( "CS2 found! Process ID: %d" ), process.id );
	}

	
	if ( debug_mode )
	{
		g->util.console.print_debug( ecrypt( "Waiting for CS2 to fully load..." ) );
		g->util.console.sleep_ms( 5000 ); // 
	}
	else
	{
		g->util.console.sleep_ms( 20000 ); 
	}

	
	process.base = g->memory.get_process_base( process.id );
	process.dtb = g->memory.get_process_dtb( process.base );
	process.client_base = g->memory.get_module_base( process.id, ecrypt( L"client.dll" ) );
	process.engine2_base = g->memory.get_module_base( process.id, ecrypt( L"engine2.dll" ) );

	if ( debug_mode )
	{
		g->util.console.print_success( ecrypt( "Process info obtained:" ) );
		g->util.console.print_debug( ecrypt( "Base: 0x%08X" ), process.base );
		g->util.console.print_debug( ecrypt( "DTB: 0x%08X" ), process.dtb );
		g->util.console.print_debug( ecrypt( "Client Base: 0x%08X" ), process.client_base );
		g->util.console.print_debug( ecrypt( "Engine2 Base: 0x%08X" ), process.engine2_base );
	}

	{
		if ( !g->memory.attach( process.id ) )
		{
			g->util.console.print_debug( ecrypt( "Failed to attach to CS2 process" ) );
			g->util.console.wait_for_input( );
			return 0;
		}
		
		if ( debug_mode )
		{
			g->util.console.print_success( ecrypt( "Successfully attached to CS2 process" ) );
		}
	}

	
	if ( !debug_mode )
	{
		g->util.console.clear_console( );
	}
	else
	{
		g->util.console.print_debug( ecrypt( "Initializing overlay..." ) );
	}
	
	{
		if ( !g->overlay.initialize( ) )
		{
			g->util.console.print_debug( ecrypt( "Failed to initialize overlay" ) );
			g->util.console.wait_for_input( );
			return 0;
		}

		if ( debug_mode )
		{
			g->util.console.print_success( ecrypt( "Overlay initialized successfully!" ) );
			g->util.console.print_debug( ecrypt( "Starting updater and overlay loop..." ) );
		}

		g->updater.deploy( );
		
		if ( !debug_mode )
		{
			g->util.console.print_finished( ecrypt( "Have Fun Playing !" ) );
		}
		else
		{
			g->util.console.print_success( ecrypt( "overlay is fully initalized everythings working dude !" ) );
			g->util.console.print_debug( ecrypt( "all debug informations are fully givin out" ) );
		}
		
		g->overlay.loop( );
	}

	
	g->util.console.wait_for_input( );
	return 0;
}
