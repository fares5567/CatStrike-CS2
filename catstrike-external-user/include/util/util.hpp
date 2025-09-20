#ifndef UTIL_HPP
#define UTIL_HPP

class util_c
{
public:
	class console_c
	{
	public:
		bool initialize( );
		void print( const char* text, ... );
		void print_success( const char* text, ... );
		void print_waiting( const char* text, ... );
		void print_found( const char* text, ... );
		void print_finished( const char* text, ... );
		void print_debug( const char* text, ... );
		void clear_console( );
		void space( );
		void sleep_ms( int milliseconds );
		void wait_for_input( );
	private:
		void* handle;
		void* input_handle;
	};

	console_c console;
};

#endif // !UTIL_HPP
