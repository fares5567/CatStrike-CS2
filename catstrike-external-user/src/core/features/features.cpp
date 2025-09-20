#include <include/global.hpp>
#include <include/util/external/widgets_colors.h>

void features_c::aim_c::reset_closest_target( )
{
	this->closest_distance = std::numeric_limits<float>::max( );
	this->has_closest_target = false;

	this->is_aimbotting = false;
}

void features_c::aim_c::find_closest_target( const math::vector2& position_projected, const target_t& player )
{
	const auto center = math::vector2{ g->core.get_display( ).width * 0.5f, g->core.get_display( ).height * 0.5f };
	const auto delta = position_projected - center;
	const auto distance_squared = delta.x * delta.x + delta.y * delta.y;

	const auto max_distance_squared = this->fov * this->fov;

	if ( distance_squared < this->closest_distance && distance_squared < max_distance_squared )
	{
		this->closest_distance = distance_squared;
		this->closest_target = player;
		this->has_closest_target = true;
	}
}

bool features_c::aim_c::is_outside_fov( const math::vector2& position_projected ) const
{
	const auto center = math::vector2{ g->core.get_display( ).width * 0.5f, g->core.get_display( ).height * 0.5f };
	const auto delta = position_projected - center;
	const auto distance_squared = delta.x * delta.x + delta.y * delta.y;

	return distance_squared > ( this->fov * this->fov );
}

const features_c::aim_c::target_t* features_c::aim_c::get_closest_target( ) const
{
	return this->has_closest_target ? &this->closest_target : nullptr;
}

float& features_c::aim_c::get_fov( )
{
	return this->fov;
}

void features_c::aim_c::apply_aim_toward( const math::vector3& world_position )
{
	this->is_aimbotting = true;

	using namespace std::chrono;

	auto screen_target = g->sdk.w2s.project( world_position );
	auto punch_angle = math::vector3{};
	bool has_punch = false;

	if ( g->core.get_settings( ).aim.rcs )
	{
		const auto local_player = g->udata.get_owning_player( ).cs_player_pawn;
		if ( local_player )
		{
			const auto shots_fired = g->memory.read<int>( local_player + offsets::m_i_shots_fired );
			if ( shots_fired > 1 )
			{
				struct c_utl_vector
				{
					std::uintptr_t count;
					std::uintptr_t data;
				};

				const auto punch_cache = g->memory.read<c_utl_vector>( local_player + offsets::m_aim_punch_cache );
				if ( punch_cache.count > 0 && punch_cache.count < 0xFFFF )
				{
					punch_angle = g->memory.read<math::vector3>( punch_cache.data + ( punch_cache.count - 1 ) * sizeof( math::vector3 ) );
					has_punch = true;

					constexpr auto punch_scale = 22.0f;
					const auto screen_x = std::clamp( punch_angle.y * punch_scale, -100.f, 100.f );
					const auto screen_y = std::clamp( punch_angle.x * punch_scale, -100.f, 100.f );

					screen_target.x += screen_x;
					screen_target.y -= screen_y;
				}
			}
		}
	}

	const auto delta = world_position - g->sdk.camera.get_data( ).position;
	const auto hyp = std::sqrt( delta.x * delta.x + delta.y * delta.y );

	math::vector3 final_angles{};
	final_angles.x = -std::atan2( delta.z, hyp ) * 180.0f / std::numbers::pi;
	final_angles.y = std::atan2( delta.y, delta.x ) * 180.0f / std::numbers::pi;
	final_angles.z = 0.0f;

	if ( has_punch )
	{
		constexpr auto punch_scale = 2.0f;
		final_angles.x -= punch_angle.x * punch_scale;
		final_angles.y -= punch_angle.y * punch_scale;
	}

	if ( g->core.get_settings( ).aim.type == 0 )
	{
		static auto last_injection_time = high_resolution_clock::now( );
		static auto accumulated_mouse_x_error = 0.0f;
		static auto accumulated_mouse_y_error = 0.0f;

		const auto delta = screen_target - math::vector2{ g->core.get_display( ).width * 0.5f, g->core.get_display( ).height * 0.5f };
		const auto move_x = delta.x / g->core.get_settings( ).aim.smooth;
		const auto move_y = delta.y / g->core.get_settings( ).aim.smooth;

		accumulated_mouse_x_error += move_x;
		accumulated_mouse_y_error += move_y;

		const auto mouse_dx = static_cast< int >( accumulated_mouse_x_error );
		const auto mouse_dy = static_cast< int >( accumulated_mouse_y_error );

		accumulated_mouse_x_error -= mouse_dx;
		accumulated_mouse_y_error -= mouse_dy;

		const auto now = high_resolution_clock::now( );
		const auto elapsed = duration_cast< milliseconds >( now - last_injection_time ).count( );

		if ( ( mouse_dx != 0 || mouse_dy != 0 ) && elapsed >= 5 )
		{
			g->memory.inject_mouse( mouse_dx, mouse_dy, 0 );
			last_injection_time = now;
		}
	}
	else if ( g->core.get_settings( ).aim.type == 1 )
	{
		static auto current_angles = math::vector3{};
		static auto last_update_time = high_resolution_clock::now( );

		current_angles = g->memory.read<math::vector3>( g->core.get_process_info( ).client_base + offsets::dw_csgo_input + 0x7b0 ); // CCSGOInput::ViewAngles

		auto angle_delta = final_angles - current_angles;

		while ( angle_delta.y > 180.0f ) angle_delta.y -= 360.0f;
		while ( angle_delta.y < -180.0f ) angle_delta.y += 360.0f;

		const auto smooth_factor = g->core.get_settings( ).aim.smooth * 15;
		auto smoothed_angles = current_angles + ( angle_delta / smooth_factor );

		while ( smoothed_angles.y > 180.0f ) smoothed_angles.y -= 360.0f;
		while ( smoothed_angles.y < -180.0f ) smoothed_angles.y += 360.0f;

		smoothed_angles.x = std::clamp( smoothed_angles.x, -89.0f, 89.0f );
		smoothed_angles.z = 0.0f;

		g->memory.write<math::vector3>( g->core.get_process_info( ).client_base + offsets::dw_csgo_input + 0x7b0, smoothed_angles ); // CCSGOInput::ViewAngles

		current_angles = smoothed_angles;
	}
	else if ( g->core.get_settings( ).aim.type == 2 )
	{
		g->memory.write<math::vector3>( g->udata.get_owning_player( ).cs_player_pawn + offsets::v_angle, final_angles );
	}
}

void features_c::visuals_c::player_c::box( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& b, ImColor color )
{
	const auto x = std::round( b.min_x );
	const auto y = std::round( b.min_y );
	const auto w = std::round( b.max_x - b.min_x );
	const auto h = std::round( b.max_y - b.min_y );

	const auto top_left = ImVec2( x, y );
	const auto bottom_right = ImVec2( x + w, y + h );

	
	drawlist->AddRect( top_left, bottom_right, ImColor( 0, 0, 0, 255 ), 0.0f, 0, 2.0f );
	
	
	drawlist->AddRect( top_left, bottom_right, color, 0.0f, 0, 1.0f );
}

void features_c::visuals_c::player_c::skeleton( ImDrawList* drawlist, const sdk_c::bones_c::data_t& bone_data, ImColor color )
{
	using bone = sdk_c::bones_c::bone_id;

	const std::vector<std::pair<bone, bone>> boner_chains = {
		{ bone::head, bone::neck },
		{ bone::neck, bone::spine_3 },
		{ bone::spine_3, bone::spine_2 },
		{ bone::spine_2, bone::spine_1 },
		{ bone::spine_1, bone::spine_0 },
		{ bone::spine_0, bone::pelvis },

		{ bone::neck, bone::left_shoulder },
		{ bone::left_shoulder, bone::left_upper_arm },
		{ bone::left_upper_arm, bone::left_hand },

		{ bone::neck, bone::right_shoulder },
		{ bone::right_shoulder, bone::right_upper_arm },
		{ bone::right_upper_arm, bone::right_hand },

		{ bone::pelvis, bone::left_hip },
		{ bone::left_hip, bone::left_knee },
		{ bone::left_knee, bone::left_foot },

		{ bone::pelvis, bone::right_hip },
		{ bone::right_hip, bone::right_knee },
		{ bone::right_knee, bone::right_foot }
	};

	for ( const auto& [from, to] : boner_chains )
	{
		const auto from_projected = g->sdk.w2s.project( bone_data.get_position( from ) );
		const auto to_projected = g->sdk.w2s.project( bone_data.get_position( to ) );

		
		if ( !g->sdk.w2s.is_valid( from_projected ) || !g->sdk.w2s.is_valid( to_projected ) )
			continue;

		
		const auto dx = to_projected.x - from_projected.x;
		const auto dy = to_projected.y - from_projected.y;
		const auto length = std::sqrt( dx * dx + dy * dy );

		
		if ( length <= 75.0f && length > 0.1f )
		{
			
			drawlist->AddLine( from_projected.vec( ), to_projected.vec( ), ImColor( 0, 0, 0, 255 ), 2.0f );
			
			
			drawlist->AddLine( from_projected.vec( ), to_projected.vec( ), color, 1.0f );
		}
	}
}

void features_c::visuals_c::player_c::health_bar( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& b, int health )
{
	const auto bar_width = 3.0f;
	const auto bar_offset = 2.0f;

	const auto x = std::round( b.min_x - bar_width - bar_offset );
	const auto y = std::round( b.min_y );
	const auto h = std::round( b.max_y - b.min_y );

	const auto chealth = std::clamp( health, 0, 100 );
	const auto fraction = chealth / 100.0f;

	const auto filled_height = std::round( h * fraction );
	const auto filled_bottom = y + h;
	const auto filled_top = filled_bottom - filled_height;

	// Verbesserte Health-Color-Berechnung wie im anderen Projekt
	ImColor color;
	if ( health > 98 )
		color = ImColor( 0.0f, 1.0f, 0.0f, 0.8f );
	else if ( health > 95 )
		color = ImColor( 0.1f, 1.0f, 0.1f, 0.8f );
	else if ( health > 90 )
		color = ImColor( 0.2f, 1.0f, 0.2f, 0.8f );
	else if ( health > 80 )
		color = ImColor( 0.4f, 1.0f, 0.0f, 0.8f );
	else if ( health > 70 )
		color = ImColor( 0.6f, 1.0f, 0.0f, 0.8f );
	else if ( health > 60 )
		color = ImColor( 0.8f, 1.0f, 0.0f, 0.8f );
	else if ( health > 50 )
		color = ImColor( 1.0f, 1.0f, 0.0f, 0.8f );
	else if ( health > 40 )
		color = ImColor( 1.0f, 0.8f, 0.0f, 0.8f );
	else if ( health > 30 )
		color = ImColor( 1.0f, 0.6f, 0.0f, 0.8f );
	else if ( health > 25 )
		color = ImColor( 1.0f, 0.4f, 0.0f, 0.8f );
	else if ( health > 15 )
		color = ImColor( 1.0f, 0.2f, 0.0f, 0.8f );
	else if ( health > 5 )
		color = ImColor( 1.0f, 0.0f, 0.0f, 0.8f );
	else
		color = ImColor( 0.5f, 0.0f, 0.0f, 0.8f );

	
	drawlist->AddRectFilled( ImVec2( x, y ), ImVec2( x + bar_width, y + h ), ImColor( 40, 40, 40, 255 ) );

	
	if ( filled_height > 0 )
	{
		drawlist->AddRectFilled( ImVec2( x, filled_top ), ImVec2( x + bar_width, filled_bottom ), color );
	}

	
	drawlist->AddRect( ImVec2( x, y ), ImVec2( x + bar_width, y + h ), ImColor( 0, 0, 0, 255 ), 0.0f, 0, 1.0f );
}

void features_c::visuals_c::player_c::distance( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& b, int distance )
{
	const auto text = std::to_string( distance ) + "m";
	const auto text_size = ImGui::CalcTextSize( text.c_str( ) );

	const auto x = std::round( ( b.min_x + b.max_x ) * 0.5f - text_size.x * 0.5f );
	const auto y = std::round( b.max_y + 2.0f );

	ImColor color;
	if ( distance < 5 )
	{
		color = ImColor( 255, 100, 100, 255 );
	}
	else if ( distance < 10 )
	{
		color = ImColor( 255, 140, 80, 255 );
	}
	else if ( distance < 25 )
	{
		color = ImColor( 255, 200, 60, 255 );
	}
	else if ( distance < 50 )
	{
		color = ImColor( 255, 255, 255, 255 );
	}
	else if ( distance < 100 )
	{
		color = ImColor( 200, 200, 200, 220 );
	}
	else
	{
		color = ImColor( 150, 150, 150, 180 );
	}

	drawlist->AddText( ImVec2( x, y ), color, text.c_str( ) );
}

void features_c::visuals_c::player_c::name( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, const std::string& name )
{
	const auto center_x = std::round( ( bound_data.min_x + bound_data.max_x ) * 0.5f );
	const auto text_size = ImGui::CalcTextSize( name.c_str( ) );

	auto offset_y = bound_data.min_y;

	if ( g->core.get_settings( ).visuals.player.health )
	{
		offset_y -= 0.0f;
	}

	offset_y -= text_size.y;

	drawlist->AddText( ImVec2( center_x - text_size.x * 0.5f, offset_y - 2.0f ), ImColor( 240, 240, 240, 255 ), name.c_str( ) );
}

void features_c::visuals_c::player_c::weapon( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, const std::string& weapon )
{
	const auto center_x = std::round( ( bound_data.min_x + bound_data.max_x ) * 0.5f );
	const auto text_size = ImGui::CalcTextSize( weapon.c_str( ) );

	auto offset_y = bound_data.max_y + 2.0f;

	if ( g->core.get_settings( ).visuals.player.distance )
	{
		offset_y += text_size.y + 2.0f;
	}

	drawlist->AddText( ImVec2( center_x - text_size.x * 0.5f, offset_y ), ImColor( 200, 200, 200, 255 ), weapon.c_str( ) );
}

void features_c::visuals_c::player_c::line( ImDrawList* drawlist, const math::vector2& target, int distance )
{
	const auto start = ImVec2( g->core.get_display( ).width * 0.5f, g->core.get_display( ).height * 0.5f );
	const auto end = ImVec2( target.x, target.y );

	ImColor color;
	if ( distance < 5 )
	{
		color = ImColor( 255, 100, 100, 255 );
	}
	else if ( distance < 10 )
	{
		color = ImColor( 255, 140, 80, 255 );
	}
	else if ( distance < 25 )
	{
		color = ImColor( 255, 200, 60, 255 );
	}
	else if ( distance < 50 )
	{
		color = ImColor( 255, 255, 255, 255 );
	}
	else if ( distance < 100 )
	{
		color = ImColor( 200, 200, 200, 220 );
	}
	else
	{
		color = ImColor( 150, 150, 150, 180 );
	}

	const auto thickness = distance < 25 ? 1.5f : 1.0f;
	drawlist->AddLine( start, end, color, thickness );
}

void features_c::visuals_c::player_c::dot( ImDrawList* drawlist, const math::vector2& target, int distance )
{
	auto radius = 28.0f / std::powf( distance + 1, 0.75f );
	radius = std::clamp( radius, 2.0f, 8.0f );

	ImColor color;
	if ( distance < 5 )
	{
		color = ImColor( 255, 100, 100, 255 );
	}
	else if ( distance < 10 )
	{
		color = ImColor( 255, 140, 80, 255 );
	}
	else if ( distance < 25 )
	{
		color = ImColor( 255, 200, 60, 255 );
	}
	else if ( distance < 50 )
	{
		color = ImColor( 255, 255, 255, 255 );
	}
	else if ( distance < 100 )
	{
		color = ImColor( 200, 200, 200, 220 );
	}
	else
	{
		color = ImColor( 150, 150, 150, 180 );
	}

	drawlist->AddCircleFilled( target.vec( ), radius, color, 32 );
	drawlist->AddCircle( target.vec( ), radius, ImColor( 0, 0, 0, 200 ), 32, 1.5f );
}

void features_c::visuals_c::player_c::chamies_c::run( ImDrawList* drawlist, const sdk_c::hitboxes_c::data_t& hitbox_data, const sdk_c::bones_c::data_t& bone_data, ImColor color )
{
	for ( const auto& hbox : hitbox_data )
	{
		if ( hbox.bone_index == -1 )
		{
			continue;
		}

		const auto bone_id = static_cast< sdk_c::bones_c::bone_id >( hbox.bone_index );
		const auto position = bone_data.get_position( bone_id );
		const auto rotation = bone_data.get_rotation( bone_id );

		this->draw_capsule( drawlist, hbox.bb_max, hbox.bb_min, hbox.radius, rotation, position, color, 24 );
	}
}

void features_c::visuals_c::player_c::chamies_c::draw_capsule( ImDrawList* drawlist, const math::vector3& start, const math::vector3& end, float radius, const math::quaternion& rotation, const math::vector3& origin, ImColor color, int segments_max )
{
	const auto top = rotation.rotate_vector( start ) + origin;
	const auto bottom = rotation.rotate_vector( end ) + origin;

	const auto axis = ( bottom - top ).normalized( );
	const auto arbitrary = std::abs( axis.x ) < 0.99f ? math::vector3( 1, 0, 0 ) : math::vector3( 0, 1, 0 );
	const auto u = axis.cross( arbitrary ).normalized( );
	const auto v = axis.cross( u );

	const auto capsule_mid_point = ( top + bottom ) * 0.5f;
	const auto distance_meters = capsule_mid_point.distance( g->sdk.camera.get_data( ).position ) / 52.5f;

	const auto start_reduction_distance = 15.0f;
	const auto end_reduction_distance = 70.0f;

	int min_segments = 4;
	int current_segments;

	if ( distance_meters <= start_reduction_distance )
	{
		current_segments = segments_max;
	}
	else if ( distance_meters >= end_reduction_distance )
	{
		current_segments = min_segments;
	}
	else
	{
		auto normalized_distance = ( distance_meters - start_reduction_distance ) / ( end_reduction_distance - start_reduction_distance );
		normalized_distance = std::clamp( normalized_distance, 0.0f, 1.0f );
		current_segments = static_cast< int >( std::lerp( static_cast< float >( segments_max ), static_cast< float >( min_segments ), normalized_distance ) );
		current_segments = std::max( current_segments, min_segments );
	}

	std::vector<float> sin_cache, cos_cache;
	this->precompute_sincos( current_segments, sin_cache, cos_cache );

	if ( distance_meters > 20.0f )
	{
		this->draw_capsule_outline( drawlist, top, bottom, axis, u, v, radius, color, sin_cache, cos_cache, current_segments );
	}
	else
	{
		this->draw_capsule_filled( drawlist, top, bottom, axis, u, v, radius, color, sin_cache, cos_cache, current_segments, distance_meters );
	}
}

void features_c::visuals_c::player_c::chamies_c::draw_capsule_filled( ImDrawList* drawlist, const math::vector3& top, const math::vector3& bottom, const math::vector3& axis, const math::vector3& u, const math::vector3& v, float radius, ImColor color, const std::vector<float>& sin_cache, const std::vector<float>& cos_cache, int segments, float distance )
{
	const auto alpha_multiplier = std::clamp( 1.2f - ( distance / 25.0f ), 0.4f, 1.0f );
	const auto edge_alpha = std::min( alpha_multiplier * 0.8f, 0.9f );

	auto center_color = ImColor( std::min( 255, static_cast< int >( color.Value.x * 255 * 1.15f ) ), std::min( 255, static_cast< int >( color.Value.y * 255 * 1.15f ) ), std::min( 255, static_cast< int >( color.Value.z * 255 * 1.15f ) ), static_cast< int >( color.Value.w * 255 * alpha_multiplier ) );
	auto edge_color = ImColor( static_cast< int >( color.Value.x * 255 * 0.7f ), static_cast< int >( color.Value.y * 255 * 0.7f ), static_cast< int >( color.Value.z * 255 * 0.7f ), static_cast< int >( color.Value.w * 255 * edge_alpha ) );

	const auto hemisphere_segments = std::max( 4, segments / 2 );
	std::vector<std::vector<math::vector3>> top_hemisphere_rings, bottom_hemisphere_rings;

	for ( int ring = 0; ring <= hemisphere_segments; ++ring )
	{
		const auto phi = ( std::numbers::pi_v<float> / 2.0f ) * ( static_cast< float >( ring ) / hemisphere_segments );
		const auto ring_radius = radius * std::cos( phi );
		const auto ring_height = radius * std::sin( phi );

		std::vector<math::vector3> ring_points;
		const auto ring_center = top - axis * ring_height;

		this->create_circle( ring_center, u, v, ring_radius, sin_cache, cos_cache, ring_points, segments );
		top_hemisphere_rings.push_back( ring_points );
	}

	for ( int ring = 0; ring <= hemisphere_segments; ++ring )
	{
		const auto phi = ( std::numbers::pi_v<float> / 2.0f ) * ( static_cast< float >( ring ) / hemisphere_segments );
		const auto ring_radius = radius * std::cos( phi );
		const auto ring_height = radius * std::sin( phi );

		std::vector<math::vector3> ring_points;
		const auto ring_center = bottom + axis * ring_height;

		this->create_circle( ring_center, u, v, ring_radius, sin_cache, cos_cache, ring_points, segments );
		bottom_hemisphere_rings.push_back( ring_points );
	}

	std::vector<math::vector3> top_circle, bottom_circle;
	this->create_circle( top, u, v, radius, sin_cache, cos_cache, top_circle, segments );
	this->create_circle( bottom, u, v, radius, sin_cache, cos_cache, bottom_circle, segments );

	std::vector<std::vector<ImVec2>> wtop_hemisphere_rings, wbottom_hemisphere_rings;
	std::vector<ImVec2> wtop_circle( segments + 1 ), wbottom_circle( segments + 1 );

	for ( const auto& ring : top_hemisphere_rings )
	{
		std::vector<ImVec2> projected_ring( segments + 1 );
		for ( int i = 0; i <= segments; ++i )
		{
			projected_ring[ i ] = g->sdk.w2s.project( ring[ i ] ).vec( );
		}

		wtop_hemisphere_rings.push_back( projected_ring );
	}

	for ( const auto& ring : bottom_hemisphere_rings )
	{
		std::vector<ImVec2> projected_ring( segments + 1 );
		for ( int i = 0; i <= segments; ++i )
		{
			projected_ring[ i ] = g->sdk.w2s.project( ring[ i ] ).vec( );
		}

		wbottom_hemisphere_rings.push_back( projected_ring );
	}

	for ( int i = 0; i <= segments; ++i )
	{
		wtop_circle[ i ] = g->sdk.w2s.project( top_circle[ i ] ).vec( );
		wbottom_circle[ i ] = g->sdk.w2s.project( bottom_circle[ i ] ).vec( );
	}

	for ( int i = 0; i < segments; ++i )
	{
		const auto next_i = i + 1;

		ImVec2 body_points[ ] = { wtop_circle[ i ], wtop_circle[ next_i ], wbottom_circle[ next_i ], wbottom_circle[ i ] };

		const auto current_color = ( i % 3 == 0 ) ? center_color : color;
		drawlist->AddConvexPolyFilled( body_points, 4, current_color );
	}

	for ( int ring = 0; ring < static_cast< int >( wtop_hemisphere_rings.size( ) ) - 1; ++ring )
	{
		for ( int i = 0; i < segments; ++i )
		{
			const auto next_i = i + 1;

			const auto ring_fact = static_cast< float >( ring ) / ( wtop_hemisphere_rings.size( ) - 1 );
			const auto ring_color = ImColor( static_cast< int >( color.Value.x * 255 * ( 1.0f + ring_fact * 0.2f ) ), static_cast< int >( color.Value.y * 255 * ( 1.0f + ring_fact * 0.2f ) ), static_cast< int >( color.Value.z * 255 * ( 1.0f + ring_fact * 0.2f ) ), static_cast< int >( color.Value.w * 255 * alpha_multiplier ) );

			ImVec2 hemisphere_points[ ] = { wtop_hemisphere_rings[ ring ][ i ], wtop_hemisphere_rings[ ring ][ next_i ], wtop_hemisphere_rings[ ring + 1 ][ next_i ], wtop_hemisphere_rings[ ring + 1 ][ i ] };
			drawlist->AddConvexPolyFilled( hemisphere_points, 4, ring_color );
		}
	}

	for ( int ring = 0; ring < static_cast< int >( wbottom_hemisphere_rings.size( ) ) - 1; ++ring )
	{
		for ( int i = 0; i < segments; ++i )
		{
			const auto next_i = i + 1;

			const auto ring_fact = static_cast< float >( ring ) / ( wbottom_hemisphere_rings.size( ) - 1 );
			const auto ring_color = ImColor( static_cast< int >( color.Value.x * 255 * ( 1.0f + ring_fact * 0.2f ) ), static_cast< int >( color.Value.y * 255 * ( 1.0f + ring_fact * 0.2f ) ), static_cast< int >( color.Value.z * 255 * ( 1.0f + ring_fact * 0.2f ) ), static_cast< int >( color.Value.w * 255 * alpha_multiplier ) );

			ImVec2 hemisphere_points[ ] = { wbottom_hemisphere_rings[ ring ][ i ], wbottom_hemisphere_rings[ ring ][ next_i ], wbottom_hemisphere_rings[ ring + 1 ][ next_i ], wbottom_hemisphere_rings[ ring + 1 ][ i ] };
			drawlist->AddConvexPolyFilled( hemisphere_points, 4, ring_color );
		}
	}

	if ( distance < 15.0f )
	{
		const auto outline_color = ImColor( static_cast< int >( color.Value.x * 255 * 0.3f ), static_cast< int >( color.Value.y * 255 * 0.3f ), static_cast< int >( color.Value.z * 255 * 0.3f ), static_cast< int >( color.Value.w * 255 * 0.6f ) );

		for ( int i = 0; i < segments; ++i )
		{
			drawlist->AddLine( wtop_circle[ i ], wtop_circle[ static_cast< std::vector<ImVec2, std::allocator<ImVec2>>::size_type >( i ) + 1 ], outline_color, 0.8f );
			drawlist->AddLine( wbottom_circle[ i ], wbottom_circle[ static_cast< std::vector<ImVec2, std::allocator<ImVec2>>::size_type >( i ) + 1 ], outline_color, 0.8f );
		}

		const auto quarter = segments / 4;
		const auto half = segments / 2;
		const auto three_quarter = ( 3 * segments ) / 4;

		drawlist->AddLine( wtop_circle[ 0 ], wbottom_circle[ 0 ], outline_color, 0.6f );
		drawlist->AddLine( wtop_circle[ quarter ], wbottom_circle[ quarter ], outline_color, 0.6f );
		drawlist->AddLine( wtop_circle[ half ], wbottom_circle[ half ], outline_color, 0.6f );
		drawlist->AddLine( wtop_circle[ three_quarter ], wbottom_circle[ three_quarter ], outline_color, 0.6f );
	}
}

void features_c::visuals_c::player_c::chamies_c::draw_capsule_outline( ImDrawList* drawlist, const math::vector3& top, const math::vector3& bottom, const math::vector3& axis, const math::vector3& u, const math::vector3& v, float radius, ImColor color, const std::vector<float>& sin_cache, const std::vector<float>& cos_cache, int segments )
{
	std::vector<math::vector3> top_circle, bottom_circle;
	this->create_circle( top, u, v, radius, sin_cache, cos_cache, top_circle, segments );
	this->create_circle( bottom, u, v, radius, sin_cache, cos_cache, bottom_circle, segments );

	const auto hemisphere_segments = std::max( 3, segments / 3 );

	std::vector<ImVec2> wtop( segments + 1 ), wbottom( segments + 1 );
	for ( int i = 0; i <= segments; ++i )
	{
		wtop[ i ] = g->sdk.w2s.project( top_circle[ i ] ).vec( );
		wbottom[ i ] = g->sdk.w2s.project( bottom_circle[ i ] ).vec( );
	}

	const auto thickness = 1.5f;
	auto new_color = ImColor( std::min( 255, static_cast< int >( color.Value.x * 255 * 1.1f ) ), std::min( 255, static_cast< int >( color.Value.y * 255 * 1.1f ) ), std::min( 255, static_cast< int >( color.Value.z * 255 * 1.1f ) ), static_cast< int >( color.Value.w * 255 * 0.9f ) );

	for ( int i = 0; i < segments; ++i )
	{
		drawlist->AddLine( wtop[ i ], wtop[ static_cast< std::vector<ImVec2, std::allocator<ImVec2>>::size_type >( i ) + 1 ], new_color, thickness );
		drawlist->AddLine( wbottom[ i ], wbottom[ static_cast< std::vector<ImVec2, std::allocator<ImVec2>>::size_type >( i ) + 1 ], new_color, thickness );
	}

	for ( int h = 0; h < hemisphere_segments; ++h )
	{
		const auto phi = ( std::numbers::pi_v<float> / 2.0f ) * ( static_cast< float >( h + 1 ) / hemisphere_segments );
		const auto ring_radius = radius * std::cos( phi );
		const auto ring_height = radius * std::sin( phi );

		std::vector<math::vector3> top_arc, bottom_arc;
		const auto top_ring_center = top - axis * ring_height;
		const auto bottom_ring_center = bottom + axis * ring_height;

		this->create_circle( top_ring_center, u, v, ring_radius, sin_cache, cos_cache, top_arc, segments );
		this->create_circle( bottom_ring_center, u, v, ring_radius, sin_cache, cos_cache, bottom_arc, segments );

		for ( int i = 0; i < segments; ++i )
		{
			const auto top_p1 = g->sdk.w2s.project( top_arc[ i ] ).vec( );
			const auto top_p2 = g->sdk.w2s.project( top_arc[ i + 1 ] ).vec( );
			const auto bottom_p1 = g->sdk.w2s.project( bottom_arc[ i ] ).vec( );
			const auto bottom_p2 = g->sdk.w2s.project( bottom_arc[ i + 1 ] ).vec( );

			drawlist->AddLine( top_p1, top_p2, new_color, thickness * 0.7f );
			drawlist->AddLine( bottom_p1, bottom_p2, new_color, thickness * 0.7f );
		}
	}

	const auto quarter = segments / 4;
	const auto half = segments / 2;
	const auto three_quarter = ( 3 * segments ) / 4;

	drawlist->AddLine( wtop[ 0 ], wbottom[ 0 ], new_color, thickness );
	drawlist->AddLine( wtop[ half ], wbottom[ half ], new_color, thickness );
	drawlist->AddLine( wtop[ quarter ], wbottom[ quarter ], new_color, thickness * 0.8f );
	drawlist->AddLine( wtop[ three_quarter ], wbottom[ three_quarter ], new_color, thickness * 0.8f );
}

void features_c::visuals_c::player_c::chamies_c::precompute_sincos( int segments, std::vector<float>& sin_cache, std::vector<float>& cos_cache )
{
	sin_cache.resize( static_cast< std::vector<float, std::allocator<float>>::size_type >( segments ) + 1 );
	cos_cache.resize( static_cast< std::vector<float, std::allocator<float>>::size_type >( segments ) + 1 );

	const auto angle_step = 2.0f * std::numbers::pi_v<float> / segments;
	for ( int i = 0; i <= segments; ++i )
	{
		const auto angle = angle_step * i;
		sin_cache[ i ] = std::sin( angle );
		cos_cache[ i ] = std::cos( angle );
	}
}

void features_c::visuals_c::player_c::chamies_c::create_circle( const math::vector3& center, const math::vector3& u, const math::vector3& v, float radius, const std::vector<float>& sin_cache, const std::vector<float>& cos_cache, std::vector<math::vector3>& out, int segments )
{
	out.clear( );
	out.reserve( static_cast< std::vector<math::vector3, std::allocator<math::vector3>>::size_type >( segments ) + 1 );

	for ( int i = 0; i <= segments; ++i )
	{
		out.push_back( center + ( u * cos_cache[ i ] + v * sin_cache[ i ] ) * radius );
	}
}

void features_c::visuals_c::indicators_c::oof_arrow( ImDrawList* drawlist, const math::vector2& position_projected, ImColor color )
{
	const auto center = math::vector2{ g->core.get_display( ).width * 0.5f, g->core.get_display( ).height * 0.5f };

	const auto angle = std::atan2( position_projected.y - center.y, position_projected.x - center.x );
	const auto radius = g->core.get_settings( ).aim.fov + 10;

	const auto arrow_center = center + math::func::from_angle( angle, radius );

	const auto length = 18.0f;
	const auto width = 14.0f;

	const auto tip = arrow_center + math::func::from_angle( angle, length );
	const auto left = arrow_center + math::func::from_angle( angle + std::numbers::pi / 2.0f, width * 0.5f );
	const auto right = arrow_center + math::func::from_angle( angle - std::numbers::pi / 2.0f, width * 0.5f );

	drawlist->AddTriangleFilled( tip.vec( ), left.vec( ), right.vec( ), color );
	drawlist->AddTriangle( tip.vec( ), left.vec( ), right.vec( ), ImColor( 10, 10, 10, 255 ), 1.0f );
}

void features_c::visuals_c::world_c::molotov( ImDrawList* drawlist, const math::vector3& center, int distance )
{
	const auto projected = g->sdk.w2s.project( center );
	if ( !g->sdk.w2s.is_valid( projected ) )
	{
		return;
	}

	char label[ 64 ];
	std::snprintf( label, sizeof( label ), "molotov [%dm]", distance );

	const auto text_size = ImGui::CalcTextSize( label );

	drawlist->AddText( projected.vec( ) - ImVec2( text_size.x * 0.5f, text_size.y * 0.5f ), ImColor( 255, 255, 255, 255 ), label );
}

void features_c::visuals_c::world_c::molotov_bounds( ImDrawList* drawlist, const std::vector<math::vector3>& fire_points )
{
	std::vector<math::vector3> boundary_points;

	constexpr auto fire_radius = 60.0f;
	constexpr auto points_per_fire = 16;

	for ( const auto& point : fire_points )
	{
		for ( int i = 0; i < points_per_fire; ++i )
		{
			const auto angle = static_cast< float >( i ) / points_per_fire * std::numbers::pi_v<float> *2.0f;
			boundary_points.emplace_back( point + math::vector3{ std::cos( angle ) * fire_radius, std::sin( angle ) * fire_radius, 0.0f } );
		}
	}

	std::vector<ImVec2> screen_points;
	screen_points.reserve( boundary_points.size( ) );

	for ( const auto& point : boundary_points )
	{
		const auto projected = g->sdk.w2s.project( point );
		if ( g->sdk.w2s.is_valid( projected ) )
		{
			screen_points.push_back( projected.vec( ) );
		}
	}

	if ( screen_points.size( ) >= 3 )
	{
		const auto hull = this->compute_convex_hull( screen_points );
		if ( hull.size( ) >= 3 )
		{
			drawlist->AddConvexPolyFilled( hull.data( ), hull.size( ), ImColor( 255, 50, 0, 80 ) );
			drawlist->AddPolyline( hull.data( ), hull.size( ), ImColor( 255, 80, 0, 150 ), ImDrawFlags_Closed, 2.0f );
		}
	}
}

void features_c::visuals_c::world_c::smoke( ImDrawList* drawlist, const math::vector3& detonation_pos, int distance )
{
	const auto projected = g->sdk.w2s.project( detonation_pos );
	if ( !g->sdk.w2s.is_valid( projected ) )
	{
		return;
	}

	char label[ 64 ];
	std::snprintf( label, sizeof( label ), "smoke [%dm]", distance );

	const auto text_size = ImGui::CalcTextSize( label );

	drawlist->AddText( projected.vec( ) - ImVec2( text_size.x * 0.5f, text_size.y * 0.5f ), ImColor( 255, 255, 255, 255 ), label );
}

void features_c::visuals_c::world_c::bomb( ImDrawList* drawlist, const math::vector3& position, int distance )
{
	const auto projected = g->sdk.w2s.project( position );
	if ( !g->sdk.w2s.is_valid( projected ) )
	{
		return;
	}

	char label[ 64 ];
	std::snprintf( label, sizeof( label ), "C4 [%dm]", distance );

	const auto text_size = ImGui::CalcTextSize( label );

	drawlist->AddText( projected.vec( ) - ImVec2( text_size.x * 0.5f, text_size.y * 0.5f ), ImColor( 255, 50, 50, 255 ), label );
}

void features_c::visuals_c::world_c::drop( ImDrawList* drawlist, const math::vector3& position, const std::string& name, int distance, ImColor color )
{
	const auto projected = g->sdk.w2s.project( position );
	if ( !g->sdk.w2s.is_valid( projected ) )
	{
		return;
	}

	char label[ 128 ];
	std::snprintf( label, sizeof( label ), "%s [%dm]", name.c_str( ), distance );

	const auto text_size = ImGui::CalcTextSize( label );

	drawlist->AddText( projected.vec( ) - ImVec2( text_size.x * 0.5f, text_size.y * 0.5f ), color, label );
}

void features_c::visuals_c::world_c::drop_bounds( ImDrawList* drawlist, const math::transform& node_transform, const math::vector3& collision_min, const math::vector3& collision_max )
{
	const auto transform_matrix = math::func::transform_to_matrix3x4( node_transform );

	std::vector<ImVec2> screen_points;
	screen_points.reserve( 8 );

	for ( size_t i = 0; i < 8; ++i )
	{
		const auto point = math::vector3( i & 1 ? collision_max.x : collision_min.x, i & 2 ? collision_max.y : collision_min.y, i & 4 ? collision_max.z : collision_min.z ).transform( transform_matrix );
		const auto projected = g->sdk.w2s.project( point );
		if ( !g->sdk.w2s.is_valid( projected ) )
		{
			return;
		}

		screen_points.push_back( projected.vec( ) );
	}

	if ( screen_points.size( ) >= 3 )
	{
		const auto hull_points = this->compute_convex_hull( screen_points );
		if ( hull_points.size( ) >= 3 )
		{
			drawlist->AddConvexPolyFilled( hull_points.data( ), hull_points.size( ), ImColor( 0, 255, 0, 60 ) );
			drawlist->AddPolyline( hull_points.data( ), hull_points.size( ), ImColor( 0, 255, 0, 120 ), ImDrawFlags_Closed, 1.5f );
		}
	}
}

std::vector<ImVec2> features_c::visuals_c::world_c::compute_convex_hull( std::vector<ImVec2>& points ) const
{
	std::sort( points.begin( ), points.end( ), [ ]( const ImVec2& a, const ImVec2& b ) { return a.x < b.x || ( a.x == b.x && a.y < b.y ); } );

	std::vector<ImVec2> lower, upper;

	for ( const auto& p : points )
	{
		while ( lower.size( ) >= 2 )
		{
			const auto& p1 = lower[ lower.size( ) - 2 ];
			const auto& p2 = lower[ lower.size( ) - 1 ];
			if ( ( p2.x - p1.x ) * ( p.y - p1.y ) - ( p2.y - p1.y ) * ( p.x - p1.x ) > 0 )
			{
				break;
			}

			lower.pop_back( );
		}

		lower.push_back( p );
	}

	for ( auto it = points.rbegin( ); it != points.rend( ); ++it )
	{
		while ( upper.size( ) >= 2 )
		{
			const auto& p1 = upper[ upper.size( ) - 2 ];
			const auto& p2 = upper[ upper.size( ) - 1 ];
			if ( ( p2.x - p1.x ) * ( it->y - p1.y ) - ( p2.y - p1.y ) * ( it->x - p1.x ) > 0 )
			{
				break;
			}

			upper.pop_back( );
		}

		upper.push_back( *it );
	}

	lower.pop_back( );
	upper.pop_back( );
	lower.insert( lower.end( ), upper.begin( ), upper.end( ) );

	return lower;
}

void features_c::visuals_c::hud_c::ring( ImDrawList* drawlist )
{
	const auto center = ImVec2( g->core.get_display( ).width * 0.5f, g->core.get_display( ).height * 0.5f );

	drawlist->AddCircle( center, g->core.get_settings( ).aim.fov, ImColor( 10, 10, 10, 255 ), 128, 3.0f );
			drawlist->AddCircle( center, g->core.get_settings( ).aim.fov, ImColor( 255, 255, 255, 255 ), 128, 1.5f );
}

void features_c::exploits_c::anti_aim_c::run( ) // isnt server sided -> very useless -> use if you wanna flex about your external "antiaim"
{
	g->features.exploits.anti_aim.data.real_angles = g->memory.read<math::vector3>( g->core.get_process_info( ).client_base + offsets::dw_view_angles );

	static bool side = false;
	static auto last_switch = std::chrono::steady_clock::now( );
	static auto time = 0.0f;

	auto now = std::chrono::steady_clock::now( );
	auto elapsed = std::chrono::duration_cast< std::chrono::milliseconds >( now - last_switch );

	time += 0.016f;

	const auto sway = std::sin( time * 2.5f ) * 40.0f;
	const auto quick_sway = std::sin( time * 8.0f ) * 15.0f;
	const auto big = std::sin( time * 12.0f ) * 5.0f;
	const auto drift = std::sin( time * 0.8f ) * 25.0f;
	const auto noisy = std::sin( time * 15.0f ) * 8.0f;

	if ( elapsed.count( ) > 80 )
	{
		side = !side;
		last_switch = now;
	}

	this->data.fake_angles.x = 90.0f + std::sin( time * 1.0f ) * 2.0f + std::sin( time * 6.0f ) * 4.0f;
	this->data.fake_angles.y = this->data.real_angles.y + 180.0f + sway + quick_sway + big + drift + noisy + ( side ? 22.0f : -22.0f );
	this->data.fake_angles.z = this->data.real_angles.z;

	if ( g->udata.get_owning_player( ).cs_player_pawn )
	{
		if ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 )
		{
			g->memory.write<math::vector3>( g->udata.get_owning_player( ).cs_player_pawn + offsets::v_angle, this->data.real_angles );
		}
		else
		{
			g->memory.write<math::vector3>( g->udata.get_owning_player( ).cs_player_pawn + offsets::v_angle, this->data.fake_angles );
		}
	}
}

void features_c::exploits_c::night_mode_c::run( std::uintptr_t post_processing_volume )
{
	const auto target = std::lerp( 1.0f, 0.075f, g->core.get_settings( ).exploits.night_mode_strength );

	const auto our_min = g->memory.read<float>( post_processing_volume + offsets::m_fl_min_exposure );
	const auto our_max = g->memory.read<float>( post_processing_volume + offsets::m_fl_max_exposure );

	if ( !this->has_original )
	{
		this->original_min = our_min;
		this->original_max = our_max;
		this->has_original = true;
	}

	if ( g->core.get_settings( ).exploits.night_mode_strength > 0.0f )
	{
		if ( our_min != target || our_max != target )
		{
			g->memory.write<bool>( post_processing_volume + offsets::m_b_exposure_control, true );
			g->memory.write<float>( post_processing_volume + offsets::m_fl_min_exposure, target );
			g->memory.write<float>( post_processing_volume + offsets::m_fl_max_exposure, target );
		}
	}
	else if ( our_min != this->original_min || our_max != this->original_max )
	{
		g->memory.write<float>( post_processing_volume + offsets::m_fl_min_exposure, this->original_min );
		g->memory.write<float>( post_processing_volume + offsets::m_fl_max_exposure, this->original_max );
	}
}

void features_c::exploits_c::third_person_c::run( )
{
	if ( this->needs_reapply( g->udata.get_owning_player( ).cs_player_pawn ) )
	{
		this->currently_active = false;
		this->cached_player_pawn = g->udata.get_owning_player( ).cs_player_pawn;
	}

	if ( g->core.get_settings( ).exploits.third_person && !this->currently_active )
	{
		this->apply_patch( );
		this->set_third_person_state( true );
		this->currently_active = true;
	}
	else if ( !g->core.get_settings( ).exploits.third_person && this->currently_active )
	{
		this->set_third_person_state( false );
		this->remove_patch( );
		this->currently_active = false;
	}
}

void features_c::exploits_c::third_person_c::apply_patch( )
{
	if ( this->patch_applied )
	{
		return;
	}

	g->memory.write_array<std::uint8_t>( g->core.get_process_info( ).client_base + this->jne_patch, this->patched_jne );
	this->patch_applied = true;
}

void features_c::exploits_c::third_person_c::remove_patch( )
{
	if ( !this->patch_applied )
	{
		return;
	}

	g->memory.write_array<std::uint8_t>( g->core.get_process_info( ).client_base + this->jne_patch, this->original_jne );
	this->patch_applied = false;
}

void features_c::exploits_c::third_person_c::set_third_person_state( bool enabled )
{
	g->memory.write<std::uint8_t>( g->core.get_process_info( ).client_base + offsets::dw_csgo_input + 0x251, enabled ? 1 : 0 ); // CCSGOInput::ThirdPerson
}

bool features_c::exploits_c::third_person_c::needs_reapply( std::uintptr_t current_player_pawn ) const
{
	if ( current_player_pawn != this->cached_player_pawn && current_player_pawn != 0 )
	{
		return true;
	}

	if ( this->currently_active && g->core.get_settings( ).exploits.third_person )
	{
		const auto cur_value = g->memory.read<std::uint8_t>( g->core.get_process_info( ).client_base + offsets::dw_csgo_input + 0x251 );
		if ( cur_value == 0 )
		{
			return true;
		}
	}

	return false;
}


void features_c::visuals_c::radar_c::render( ImDrawList* drawlist, const std::vector<udata_c::player_entity_t>& players, const udata_c::owning_player_entity_t& local_player, const std::vector<udata_c::misc_entity_t>& misc_entities )
{
	if ( !g->core.get_settings( ).visuals.player.radar.enabled )
	{
		return;
	}

	
	const float radar_size = 250.0f;
	const ImVec2 radar_pos = ImVec2( 50.0f, 50.0f );
	const ImVec2 radar_center = ImVec2( radar_pos.x + radar_size * 0.5f, radar_pos.y + radar_size * 0.5f );
	const float radar_radius = radar_size * 0.5f;

	
	const auto local_pos = g->sdk.camera.get_data( ).position;
	const auto local_rotation = g->sdk.camera.get_data( ).rotation;
	const auto local_euler = math::func::quaternion_to_euler( local_rotation );
	const float local_yaw = local_euler.y;

	
	drawlist->AddRectFilled( radar_pos, ImVec2( radar_pos.x + radar_size, radar_pos.y + radar_size ), 
		ImColor( 0.0f, 0.0f, 0.0f, 0.7f ) );
	drawlist->AddRect( radar_pos, ImVec2( radar_pos.x + radar_size, radar_pos.y + radar_size ), 
		ImColor( 1.0f, 1.0f, 1.0f, 0.3f ), 0.0f, 0, 1.0f );

	
	if ( this->show_cross_line )
	{
		const ImVec2 cross_line1_start = ImVec2( radar_center.x - radar_radius, radar_center.y );
		const ImVec2 cross_line1_end = ImVec2( radar_center.x + radar_radius, radar_center.y );
		const ImVec2 cross_line2_start = ImVec2( radar_center.x, radar_center.y - radar_radius );
		const ImVec2 cross_line2_end = ImVec2( radar_center.x, radar_center.y + radar_radius );

		drawlist->AddLine( cross_line1_start, cross_line1_end, this->cross_color, 1.0f );
		drawlist->AddLine( cross_line2_start, cross_line2_end, this->cross_color, 1.0f );
	}

	
	const auto local_radar_pos = this->world_to_radar_improved( local_pos, local_pos, local_yaw, radar_center, radar_radius );
	drawlist->AddCircleFilled( local_radar_pos, this->circle_size, ImColor( 0.0f, 1.0f, 0.0f, 1.0f ) );
	drawlist->AddCircle( local_radar_pos, this->circle_size, ImColor( 0.0f, 0.0f, 0.0f, 0.5f ), 0, 1.0f );

	
	for ( const auto& player : players )
	{
		if ( !player.cs_player_pawn || !player.m_p_game_scene_node )
		{
			continue;
		}

		
		const auto player_pos = g->memory.read<math::vector3>( player.m_p_game_scene_node + offsets::m_vec_origin );
		const auto player_rotation = g->memory.read<math::quaternion>( player.m_p_game_scene_node + offsets::m_ang_rotation );
		const auto player_euler = math::func::quaternion_to_euler( player_rotation );
		const float player_yaw = player_euler.y;
		
		
		const auto radar_pos_2d = this->world_to_radar_improved( player_pos, local_pos, local_yaw, radar_center, radar_radius );
		
		
		const float dx = radar_pos_2d.x - radar_center.x;
		const float dy = radar_pos_2d.y - radar_center.y;
		if ( dx * dx + dy * dy > radar_radius * radar_radius )
		{
			continue;
		}

		
		const auto color = this->get_player_color( player.team, player.health );
		
		
		if ( g->core.get_settings( ).visuals.player.radar.distance )
		{
		
			drawlist->AddCircleFilled( radar_pos_2d, this->circle_size, color );
			drawlist->AddCircle( radar_pos_2d, this->circle_size, ImColor( 0.0f, 0.0f, 0.0f, 0.5f ), 0, 1.0f );
			
			
			const auto distance = local_pos.distance( player_pos );
			const auto distance_text = std::to_string( static_cast<int>( distance ) );
			drawlist->AddText( ImVec2( radar_pos_2d.x + 5.0f, radar_pos_2d.y - 5.0f ), 
				ImColor( 1.0f, 1.0f, 1.0f, 1.0f ), distance_text.c_str( ) );
		}
		else
		{
			
			const float angle = ( local_yaw - player_yaw ) + 180.0f;
			const ImVec2 re_point = this->revolve_coordinates_system( angle, radar_center, radar_pos_2d );

			const ImVec2 re_a( re_point.x, re_point.y + this->arrow_size );
			const ImVec2 re_b( re_point.x - this->arrow_size / 1.5f, re_point.y - this->arrow_size / 2.0f );
			const ImVec2 re_c( re_point.x + this->arrow_size / 1.5f, re_point.y - this->arrow_size / 2.0f );

			const ImVec2 a = this->revolve_coordinates_system( -angle, radar_center, re_a );
			const ImVec2 b = this->revolve_coordinates_system( -angle, radar_center, re_b );
			const ImVec2 c = this->revolve_coordinates_system( -angle, radar_center, re_c );

			drawlist->AddQuadFilled( a, b, radar_pos_2d, c, color );
			drawlist->AddQuad( a, b, radar_pos_2d, c, ImColor( 0, 0, 0, 150 ), 0.1f );
		}
	}

	if ( g->core.get_settings( ).visuals.player.radar.bomb )
	{
		for ( const auto& entity : misc_entities )
		{
			if ( entity.type != udata_c::misc_entity_t::type_t::bomb )
			{
				continue;
			}

			if ( const auto* bomb = std::get_if<udata_c::misc_entity_t::bomb_data_t>( &entity.data ) )
			{
				if ( bomb->position.x == 0.0f && bomb->position.y == 0.0f && bomb->position.z == 0.0f )
				{
					continue;
				}

				const auto radar_pos_2d = this->world_to_radar_improved( bomb->position, local_pos, local_yaw, radar_center, radar_radius );
				
				const float dx = radar_pos_2d.x - radar_center.x;
				const float dy = radar_pos_2d.y - radar_center.y;
				if ( dx * dx + dy * dy > radar_radius * radar_radius )
				{
					continue;
				}

				const float size = 6.0f;
					drawlist->AddTriangleFilled( 
						ImVec2( radar_pos_2d.x, radar_pos_2d.y - size ),
						ImVec2( radar_pos_2d.x - size, radar_pos_2d.y + size ),
						ImVec2( radar_pos_2d.x + size, radar_pos_2d.y + size ),
						ImColor( 1.0f, 0.0f, 0.0f, 1.0f ) );
				}
			}
		}
	}

ImVec2 features_c::visuals_c::radar_c::revolve_coordinates_system( float revolve_angle, const ImVec2& origin_pos, const ImVec2& dest_pos )
{
	if ( revolve_angle == 0.0f )
		return dest_pos;

	const float rad = revolve_angle * ( 3.14159f / 180.0f );
	const float cos_val = cos( rad );
	const float sin_val = sin( rad );

	const float dx = dest_pos.x - origin_pos.x;
	const float dy = dest_pos.y - origin_pos.y;

	ImVec2 result_pos;
	result_pos.x = origin_pos.x + dx * cos_val + dy * sin_val;
	result_pos.y = origin_pos.y - dx * sin_val + dy * cos_val;
	return result_pos;
}

ImVec2 features_c::visuals_c::radar_c::world_to_radar_improved( const math::vector3& world_pos, const math::vector3& local_pos, float local_yaw, const ImVec2& radar_center, float radar_radius )
{
	const float dx = local_pos.x - world_pos.x;
	const float dy = local_pos.y - world_pos.y;
	const float distance = sqrt( dx * dx + dy * dy );

	const float angle_rad = ( local_yaw * ( 3.14159f / 180.0f ) ) - atan2( world_pos.y - local_pos.y, world_pos.x - local_pos.x );

	const float scale = ( 2.0f * this->render_range ) / this->proportion;
	const float scaled_distance = distance * scale;

	ImVec2 point_pos;
	point_pos.x = radar_center.x + scaled_distance * sin( angle_rad );
	point_pos.y = radar_center.y - scaled_distance * cos( angle_rad );
	
	const float clamped_x = ( point_pos.x < radar_center.x - radar_radius ) ? radar_center.x - radar_radius : 
						   ( point_pos.x > radar_center.x + radar_radius ) ? radar_center.x + radar_radius : point_pos.x;
	const float clamped_y = ( point_pos.y < radar_center.y - radar_radius ) ? radar_center.y - radar_radius : 
						   ( point_pos.y > radar_center.y + radar_radius ) ? radar_center.y + radar_radius : point_pos.y;
	
	return ImVec2( clamped_x, clamped_y );
}

ImColor features_c::visuals_c::radar_c::get_player_color( int team, int health )
{
	if ( team != g->udata.get_owning_player( ).team )
	{
		if ( health < 50 )
		{
			return ImColor( 1.0f, 0.0f, 0.0f, 1.0f ); // Red
		}
		else if ( health < 75 )
		{
			return ImColor( 1.0f, 1.0f, 0.0f, 1.0f ); // Yellow
		}
		else
		{
			return ImColor( 0.0f, 1.0f, 0.0f, 1.0f ); // Green
		}
	}
	else
	{
		// Teammate - blue
		return ImColor( 0.0f, 0.5f, 1.0f, 1.0f );
	}
}

//useless i kept it as a placeholder if i want to add it 
ImColor features_c::visuals_c::radar_c::get_entity_color( const std::string& entity_type )
{
	return ImColor( 1.0f, 1.0f, 1.0f, 1.0f );
}


float features_c::visuals_c::player_c::glow_c::rainbow_time = 0.0f;

void features_c::visuals_c::player_c::glow_c::render( ImDrawList* drawlist, const sdk_c::bounds_c::data_t& bound_data, int team, int health, bool is_local_player )
{
	if ( !g->core.get_settings( ).visuals.player.glow.enabled )
	{
		return;
	}

	auto& glow_settings = g->core.get_settings( ).visuals.player.glow;
	
	
	ImColor glow_color = this->get_glow_color( team, health, is_local_player );
	
	glow_color.Value.w *= glow_settings.intensity * glow_settings.alpha;
	
	const ImVec2 min( bound_data.min_x, bound_data.min_y );
	const ImVec2 max( bound_data.max_x, bound_data.max_y );
	
	const float glow_offset = 3.0f;
	const ImVec2 glow_min( min.x - glow_offset, min.y - glow_offset );
	const ImVec2 glow_max( max.x + glow_offset, max.y + glow_offset );
	
	const int glow_layers = 3;
	for ( int i = 0; i < glow_layers; i++ )
	{
		const float layer_offset = glow_offset * ( i + 1 ) * 0.5f;
		const ImVec2 layer_min( min.x - layer_offset, min.y - layer_offset );
		const ImVec2 layer_max( max.x + layer_offset, max.y + layer_offset );
		
		ImColor layer_color = glow_color;
		layer_color.Value.w *= ( 1.0f - ( float )i / glow_layers ) * 0.3f;
		
		drawlist->AddRect( layer_min, layer_max, layer_color, 2.0f, 0, 1.0f );
	}
	
	drawlist->AddRect( min, max, glow_color, 2.0f, 0, 2.0f );
}

ImColor features_c::visuals_c::player_c::glow_c::get_glow_color( int team, int health, bool is_local_player )
{
	auto& glow_settings = g->core.get_settings( ).visuals.player.glow;
	
	if ( glow_settings.rainbow )
	{
		return this->rainbow_glow();
	}
	
	if ( is_local_player )
	{
		return glow_settings.local_color;
	}
	
	if ( glow_settings.team_based )
	{
		if ( team == g->udata.get_owning_player( ).team )
		{
			return glow_settings.team_color;
	}
	else
	{
			if ( glow_settings.health_based )
			{
				if ( health < 50 )
				{
					return ImColor( 1.0f, 0.0f, 0.0f, glow_settings.alpha ); // Red for low health
				}
				else if ( health < 75 )
				{
					return ImColor( 1.0f, 1.0f, 0.0f, glow_settings.alpha ); // Yellow for medium health
				}
				else
				{
					return ImColor( 0.0f, 1.0f, 0.0f, glow_settings.alpha ); // Green for high health
				}
			}
			else
			{
				return glow_settings.enemy_color;
			}
		}
	}
	
	return glow_settings.enemy_color;
}

ImColor features_c::visuals_c::player_c::glow_c::rainbow_glow()
{
	auto& glow_settings = g->core.get_settings( ).visuals.player.glow;
	
	rainbow_time += glow_settings.rainbow_speed * 0.01f;
	
	const float r = ( std::sin( rainbow_time * 2.0f * 3.14159f ) + 1.0f ) * 0.5f;
	const float g = ( std::sin( rainbow_time * 2.0f * 3.14159f + 2.0f * 3.14159f / 3.0f ) + 1.0f ) * 0.5f;
	const float b = ( std::sin( rainbow_time * 2.0f * 3.14159f + 4.0f * 3.14159f / 3.0f ) + 1.0f ) * 0.5f;
	
	return ImColor( r, g, b, glow_settings.alpha );
}

namespace trail_system {
	struct trail_point_t {
		math::vector3 position;
		float timestamp;
		ImColor color;
	};
	
	static std::vector<trail_point_t> local_trail_points;
	static std::vector<std::vector<trail_point_t>> enemy_trail_points;
	static constexpr float trail_duration = 3.5f; 
	static constexpr int max_trail_points = 100; 
}

void features_c::visuals_c::movement_tracers_c::update_trails( ImDrawList* drawlist )
{
	const float current_time = static_cast<float>( ImGui::GetTime() );
	
	if ( g->core.get_settings().visuals.movement_tracers.local_trail )
	{
		const auto local_player = g->udata.get_owning_player();
		const auto local_pos = g->memory.read<math::vector3>( local_player.m_p_game_scene_node + offsets::m_vec_origin );
		
		trail_system::local_trail_points.push_back({
			local_pos,
			static_cast<float>( current_time ),
			ImColor( 0.0f, 1.0f, 0.0f, 1.0f )
		
		while ( trail_system::local_trail_points.size() > trail_system::max_trail_points )
		{
			trail_system::local_trail_points.erase( trail_system::local_trail_points.begin() );
		}
		
		
		if ( trail_system::local_trail_points.size() > 1 )
		{
			for ( size_t i = 1; i < trail_system::local_trail_points.size(); ++i )
			{
				const auto& prev_point = trail_system::local_trail_points[i - 1];
				const auto& curr_point = trail_system::local_trail_points[i];
				
				
				const float age = current_time - curr_point.timestamp;
				const float alpha = 1.0f - ( age / trail_system::trail_duration );
				
				if ( alpha > 0.0f )
				{
					const auto prev_screen = g->sdk.w2s.project( prev_point.position );
					const auto curr_screen = g->sdk.w2s.project( curr_point.position );
					
					if ( g->sdk.w2s.is_valid( prev_screen ) && g->sdk.w2s.is_valid( curr_screen ) )
					{
						const float fade_progress = age / trail_system::trail_duration;
						const float r = 1.0f - ( fade_progress * 0.3f ); 
						const float g = 0.0f; 
						const float b = 0.0f; 
						
						const ImColor trail_color = ImColor( r, g, b, alpha );
						
						drawlist->AddLine( 
							ImVec2( prev_screen.x, prev_screen.y ), 
							ImVec2( curr_screen.x, curr_screen.y ), 
							trail_color, 
							3.0f 
						);
					}
				}
			}
		}
	}
	
	if ( g->core.get_settings().visuals.movement_tracers.enemy_trail )
	{
		const auto players = g->updater.get_player_entities();
		
		if ( trail_system::enemy_trail_points.size() != players.size() )
		{
			trail_system::enemy_trail_points.resize( players.size() );
		}
		
		for ( size_t i = 0; i < players.size(); ++i )
		{
			const auto& player = players[i];
			
			if ( player.team == g->udata.get_owning_player().team )
				continue;
				
			if ( g->core.get_settings().misc.team_check && player.team == g->udata.get_owning_player().team )
				continue;
			
			const auto player_pos = g->memory.read<math::vector3>( player.m_p_game_scene_node + offsets::m_vec_origin );
			
			trail_system::enemy_trail_points[i].push_back({
				player_pos,
				static_cast<float>( current_time ),
				ImColor( 1.0f, 0.0f, 0.0f, 1.0f ) // Red for enemies
			});
			
			
			while ( trail_system::enemy_trail_points[i].size() > trail_system::max_trail_points )
			{
				trail_system::enemy_trail_points[i].erase( trail_system::enemy_trail_points[i].begin() );
			}
			
			if ( trail_system::enemy_trail_points[i].size() > 1 )
			{
				for ( size_t j = 1; j < trail_system::enemy_trail_points[i].size(); ++j )
				{
					const auto& prev_point = trail_system::enemy_trail_points[i][j - 1];
					const auto& curr_point = trail_system::enemy_trail_points[i][j];
					
					const float age = current_time - curr_point.timestamp;
					const float alpha = 1.0f - ( age / trail_system::trail_duration );
					
					if ( alpha > 0.0f )
					{
						const auto prev_screen = g->sdk.w2s.project( prev_point.position );
						const auto curr_screen = g->sdk.w2s.project( curr_point.position );
						
						if ( g->sdk.w2s.is_valid( prev_screen ) && g->sdk.w2s.is_valid( curr_screen ) )
						{
							const float fade_progress = age / trail_system::trail_duration;
							const float r = 0.0f + ( fade_progress * 1.0f );
							const float g = 1.0f - ( fade_progress * 1.0f );
							const float b = 0.0f;
							
							const ImColor trail_color = ImColor( r, g, b, alpha );
							
							drawlist->AddLine( 
								ImVec2( prev_screen.x, prev_screen.y ), 
								ImVec2( curr_screen.x, curr_screen.y ), 
								trail_color, 
								3.0f 
							);
						}
					}
				}
			}
		}
	}
}

void features_c::visuals_c::spectator_c::render_list( ImDrawList* drawlist )
{
	if ( !g->core.get_settings().visuals.spectator.enabled )
	{
		return;
	}

	const auto players = g->updater.get_player_entities();
	const auto local_player = g->udata.get_owning_player();
	
	if ( !local_player.cs_player_pawn )
	{
		return;
	}
	
	std::vector<std::string> spectator_names;
	
	for ( const auto& player : players )
	{
		if ( !player.cs_player_controller || player.name_data.name.empty() )
		{
			continue;
		}
		
		if ( player.cs_player_controller == local_player.cs_player_pawn )
		{
			continue;
		}
		
		if ( local_player.cs_player_pawn && player.cs_player_pawn && player.health > 0 )
		{
			continue;
		}
		
		
		std::uint32_t pawn_handle = 0;
		pawn_handle = g->memory.read<std::uint32_t>( 
			player.cs_player_controller + offsets::m_h_pawn 
		);
		
		std::uintptr_t pawn_addr = 0;
		if ( pawn_handle != 0 )
		{
			pawn_addr = g->updater.resolve_entity_handle( pawn_handle, g->udata );
		}
		
		if ( !pawn_addr )
		{
			pawn_addr = player.cs_player_pawn;
		}
		
		if ( !pawn_addr )
		{
			continue;
		}
		
		std::uintptr_t spec_target = get_observer_target( pawn_addr );
		if ( !spec_target )
		{
			continue;
		}
		
		if ( spec_target == local_player.cs_player_pawn )
		{
			spectator_names.push_back( player.name_data.name );
		}
	}
	
	ImGui::SetNextWindowPos( ImVec2( g->core.get_display().width - 200.0f, 80.0f ), ImGuiCond_FirstUseEver );
	
	float required_height = spectator_names.size() * (ImGui::GetFontSize() + 5) + 40;
	if ( spectator_names.empty() )
		required_height = 60.0f;
		
	ImGui::SetNextWindowSize( ImVec2( 180.0f, required_height ), ImGuiCond_Always );
	
	ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 8.0f );
	ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 1.0f );
	ImGui::PushStyleColor( ImGuiCol_WindowBg, ImVec4( 0.08f, 0.08f, 0.08f, 0.95f ) );
	ImGui::PushStyleColor( ImGuiCol_Border, ImVec4( 0.58f, 0.20f, 0.92f, 0.8f ) ); 
	
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
	if ( !g->modern_menu.is_running )
	{
		window_flags |= ImGuiWindowFlags_NoMove; 
	}
	
	if ( ImGui::Begin( "Spectators", nullptr, window_flags ) )
	{
		if ( spectator_names.empty() )
		{
			ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.6f, 0.6f, 0.6f, 1.0f ) );
			ImGui::Text( "No spectators" );
			ImGui::PopStyleColor();
		}
		else
		{
			for ( const auto& name : spectator_names )
			{
				ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 5.0f );
				ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 0.9f, 0.9f, 0.9f, 1.0f ) );
				ImGui::Text( "%s", name.c_str() );
				ImGui::PopStyleColor();
			}
		}
	}
	ImGui::End();
	
	ImGui::PopStyleColor( 2 );
	ImGui::PopStyleVar( 2 );
}

std::uintptr_t features_c::visuals_c::spectator_c::get_observer_target( std::uintptr_t pawn_address )
{
	if ( !pawn_address )
		return 0;
		
	std::uintptr_t observer_services = 0;
	observer_services = g->memory.read<std::uintptr_t>( pawn_address + offsets::m_p_observer_services );
	
	if ( !observer_services )
		return 0;
		
	std::uint32_t observer_target_handle = 0;
	observer_target_handle = g->memory.read<std::uint32_t>( 
		observer_services + offsets::m_h_observer_target 
	);
	
	if ( !observer_target_handle )
		return 0;
		
	return g->updater.resolve_entity_handle( observer_target_handle, g->udata );
}