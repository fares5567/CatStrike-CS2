#include <include/global.hpp>

bool memory_c::attach( std::uint32_t process_id )
{
    if ( this->process_handle )
    {
        CloseHandle( this->process_handle );
    }

    this->process_handle = OpenProcess( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, process_id );
    return this->process_handle != nullptr;
}

void memory_c::detach( )
{
    if ( this->process_handle )
    {
        CloseHandle( this->process_handle );
        this->process_handle = nullptr;
    }
}

std::uint32_t memory_c::get_process_id( const std::wstring& process_name )
{
    PROCESSENTRY32 pe{ sizeof( pe ) };

    auto snap = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
    if ( snap == INVALID_HANDLE_VALUE )
    {
        return 0;
    }

    if ( !Process32First( snap, &pe ) )
    {
        CloseHandle( snap );
        return 0;
    }

    do
    {
        if ( !std::wcscmp( pe.szExeFile, process_name.c_str( ) ) )
        {
            CloseHandle( snap );
            return pe.th32ProcessID;
        }
    } while ( Process32Next( snap, &pe ) );

    CloseHandle( snap );
    return 0;
}

std::uintptr_t memory_c::get_module_base( std::uint32_t process_id, const std::wstring& module_name )
{
    MODULEENTRY32 me{ sizeof( me ) };

    auto snap = CreateToolhelp32Snapshot( TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id );
    if ( snap == INVALID_HANDLE_VALUE )
    {
        return 0;
    }

    if ( !Module32First( snap, &me ) )
    {
        CloseHandle( snap );
        return 0;
    }

    do
    {
        if ( !std::wcscmp( me.szModule, module_name.c_str( ) ) )
        {
            CloseHandle( snap );
            return reinterpret_cast< std::uintptr_t >( me.modBaseAddr );
        }
    } while ( Module32Next( snap, &me ) );

    CloseHandle( snap );
    return 0;
}

std::uintptr_t memory_c::get_process_base( std::uint32_t process_id )
{
    return 0;
}

std::uintptr_t memory_c::get_process_dtb( std::uintptr_t process_base )
{
    return 0;
}

bool memory_c::read_process_memory( std::uintptr_t address, void* buffer, std::uintptr_t size )
{
    SIZE_T bytes_read = 0;
    return ReadProcessMemory( this->process_handle, reinterpret_cast< LPCVOID >( address ), buffer, size, &bytes_read ) && bytes_read == size;
}

bool memory_c::write_process_memory( std::uintptr_t address, const void* buffer, std::uintptr_t size )
{
    SIZE_T bytes_written = 0;
    return WriteProcessMemory( this->process_handle, reinterpret_cast< LPVOID >( address ), buffer, size, &bytes_written ) && bytes_written == size;
}

void memory_c::inject_mouse( int x, int y, std::uint8_t button_flags )
{
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dx = x;
    input.mi.dy = y;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;

    if ( button_flags & 1 )
    {
        input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
    }

    SendInput( 1, &input, sizeof( INPUT ) );
}