#include <include/global.hpp>

#include <include/util/external/fonts/proggy_tiny.hpp>
#include <include/util/external/fonts/tahoma_bold.hpp>

bool overlay_c::initialize( )
{
	if ( !this->setup_window( ) )
	{
		return false;
	}

	if ( !this->setup_d3d11( ) )
	{
		return false;
	}

	return true;
}

void overlay_c::loop( )
{
	call_function( &SetPriorityClass, call_function( &GetCurrentProcess ), HIGH_PRIORITY_CLASS );
	call_function( &SetThreadPriority, call_function( &GetCurrentThread ), THREAD_PRIORITY_HIGHEST );

	constexpr float clear_color[ 4 ] = { 0.0f, 0.0f, 0.0f, 0.0f };
	MSG msg{};

	auto last_refresh_time = std::chrono::steady_clock::now( );

	while ( msg.message != WM_QUIT )
	{
		while ( PeekMessage( &msg, this->hwnd, 0, 0, PM_REMOVE ) )
		{
			if ( msg.message == WM_QUIT )
			{
				break;
			}

			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}

		auto& io = ImGui::GetIO( );
		io.DeltaTime = 1.0f / 60.0f;

		if ( g->menu.is_running )
		{
			POINT cursor_pos{};
			GetCursorPos( &cursor_pos );
			io.MousePos = ImVec2( static_cast< float >( cursor_pos.x ), static_cast< float >( cursor_pos.y ) );
			io.MouseDown[ 0 ] = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;
		}

		{
			ImGui_ImplDX11_NewFrame( );
			ImGui_ImplWin32_NewFrame( );
		}

		ImGui::NewFrame( );
		{
			const auto drawlist = ImGui::GetBackgroundDrawList( );

			g->dispatch.run( drawlist );
			g->modern_menu.run( );
		}
		ImGui::Render( );

		{
			this->device_context->OMSetRenderTargets( 1, &this->render_target_view, nullptr );
			this->device_context->ClearRenderTargetView( this->render_target_view, clear_color );
		}

		{
			ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );
		}

		if ( g->core.get_settings( ).misc.do_no_wait )
		{
			this->swap_chain->Present( g->core.get_settings( ).misc.vsync, DXGI_PRESENT_DO_NOT_WAIT );
		}
		else
		{
			this->swap_chain->Present( g->core.get_settings( ).misc.vsync, 0 );
		}
	}

	ImGui_ImplDX11_Shutdown( );
	ImGui_ImplWin32_Shutdown( );
	ImGui::DestroyContext( );
}

bool overlay_c::setup_d3d11( )
{
	DXGI_SWAP_CHAIN_DESC sc_desc{};
	sc_desc.BufferCount = 2;
	sc_desc.BufferDesc.Width = 0;
	sc_desc.BufferDesc.Height = 0;
	sc_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sc_desc.BufferDesc.RefreshRate.Numerator = 0;
	sc_desc.BufferDesc.RefreshRate.Denominator = 0;
	sc_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sc_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sc_desc.OutputWindow = this->hwnd;
	sc_desc.SampleDesc.Count = 1;
	sc_desc.SampleDesc.Quality = 0;
	sc_desc.Windowed = TRUE;
	sc_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	const D3D_FEATURE_LEVEL feature_levels[ ] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	D3D_FEATURE_LEVEL selected_feature_level;

	constexpr std::uint32_t device_flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_PREVENT_INTERNAL_THREADING_OPTIMIZATIONS;

	auto result = call_function( &D3D11CreateDeviceAndSwapChain, nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags, feature_levels, _countof( feature_levels ), D3D11_SDK_VERSION, &sc_desc, &this->swap_chain, &this->device, &selected_feature_level, &this->device_context );
	if ( FAILED( result ) )
	{
		return false;
	}

	ID3D11Texture2D* back_buffer = nullptr;
	result = this->swap_chain->GetBuffer( 0, IID_PPV_ARGS( &back_buffer ) );
	if ( FAILED( result ) || back_buffer == nullptr )
	{
		return false;
	}

	result = this->device->CreateRenderTargetView( back_buffer, nullptr, &this->render_target_view );
	back_buffer->Release( );

	if ( FAILED( result ) )
	{
		return false;
	}

	IDXGIDevice1* dxgi_device = nullptr;
	if ( SUCCEEDED( this->device->QueryInterface( __uuidof( IDXGIDevice1 ), ( void** )&dxgi_device ) ) )
	{
		dxgi_device->SetMaximumFrameLatency( 1 );
		dxgi_device->Release( );
	}

	IDXGISwapChain2* swap_chain2 = nullptr;
	if ( SUCCEEDED( this->swap_chain->QueryInterface( IID_PPV_ARGS( &swap_chain2 ) ) ) )
	{
		swap_chain2->SetMaximumFrameLatency( 1 );
		swap_chain2->Release( );
	}

	ImGui::CreateContext( );

	auto& io = ImGui::GetIO( );
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	io.Fonts->AddFontFromMemoryTTF( tahoma_bold, sizeof( tahoma_bold ), 13.0f, nullptr, nullptr );

	ImGui_ImplWin32_Init( this->hwnd );
	ImGui_ImplDX11_Init( this->device, this->device_context );

	return true;
}

bool overlay_c::setup_window( )
{
	const auto game = call_function( &FindWindowA, nullptr, ecrypt( "Counter-Strike 2" ) );
	if ( !game )
	{
		return false;
	}

	if ( !this->find_ime( ) )
	{
		return false;
	}

	if ( !this->align_to_game( game ) )
	{
		return false;
	}

	this->set_attributes( );

	// g->util.console.space( );

	return true;
}

bool overlay_c::find_ime( )
{
	const auto desktop = call_function( &GetDesktopWindow );
	const auto target = g->core.get_process_info( ).id;

	HWND current = nullptr;
	while ( ( current = call_function( &FindWindowExA, desktop, current, ecrypt( "IME" ), ecrypt( "Default IME" ) ) ) != nullptr )
	{
		DWORD window = 0;
		call_function( &GetWindowThreadProcessId, current, &window );

		if ( window == target )
		{
			this->hwnd = current;
			return true;
		}
	}

	return false;
}

bool overlay_c::align_to_game( HWND hwnd ) const
{
	RECT rect{};
	if ( !call_function( &GetClientRect, hwnd, &rect ) )
	{
		return false;
	}

	auto top_left = POINT( rect.left, rect.top );
	auto bottom_right = POINT( rect.right, rect.bottom );

	call_function( &ClientToScreen, hwnd, &top_left );
	call_function( &ClientToScreen, hwnd, &bottom_right );

	const auto width = bottom_right.x - top_left.x;
	const auto height = bottom_right.y - top_left.y;

	return call_function( &SetWindowPos, this->hwnd, nullptr, top_left.x, top_left.y, width, height, SWP_NOZORDER );
}

void overlay_c::set_attributes( ) const
{
	call_function( &SetWindowLongA, this->hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW );

	const MARGINS margins = { -1 };
	call_function( &DwmExtendFrameIntoClientArea, this->hwnd, &margins );

	call_function( &SetLayeredWindowAttributes, this->hwnd, 0, 255, LWA_ALPHA );

	call_function( &UpdateWindow, this->hwnd );
	call_function( &ShowWindow, this->hwnd, SW_SHOW );
}