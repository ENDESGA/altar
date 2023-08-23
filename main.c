#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_BUF_SIZE 2048
#define PATH_BUF_SIZE 512
#define MAX_FILE_SIZE 8192
#define NUM_PACKAGES 5

typedef struct {
	const char* remote_name;
	const char* url;
	const char* branch;
	const char* files[ 5 ];
} Package;

#define run_command( _ ) system( _ )

char* optimized_strstr( const char* haystack, const char* needle ) {
	if( !*needle ) return (char*)haystack;
	for( ; *haystack; ++haystack ) {
		const char *h = haystack, *n = needle;
		while( *h && *n && *h == *n ) {
			++h;
			++n;
		}
		if( !*n ) return (char*)haystack;
	}
	return NULL;
}

int count_substrings( const char* str, const char* sub ) {
	int count = 0;
	const char* temp = str;

	while( ( temp = optimized_strstr( temp, sub ) ) != NULL ) {
		count++;
		temp += strlen( sub );
	}

	return count;
}



void replace_in_file( const char* filename, const char* find, const char* replace ) {
	FILE* file = fopen( filename, "rb" );
	if( !file ) {
		printf( "Failed to open file: %s\n", filename );
		return;
	}

	fseek( file, 0, SEEK_END );
	long length = ftell( file );
	fseek( file, 0, SEEK_SET );

	char* buffer = malloc( length + 1 );
	fread( buffer, 1, length, file );
	buffer[ length ] = '\0';
	fclose( file );

	int occurrences = count_substrings( buffer, find );
	size_t find_len = strlen( find );
	size_t replace_len = strlen( replace );
	size_t diff = replace_len - find_len;
	long new_length = (long)( length + occurrences * diff );
	char* new_buffer = malloc( new_length + 1 );

	char *src = buffer, *dst = new_buffer, *pos;
	while( ( pos = optimized_strstr( src, find ) ) != NULL ) {
		size_t bytes_to_copy = pos - src;
		memcpy( dst, src, bytes_to_copy );
		memcpy( dst + bytes_to_copy, replace, replace_len );

		src += bytes_to_copy + find_len;
		dst += bytes_to_copy + replace_len;
	}
	strcpy( dst, src );

	file = fopen( filename, "wb" );
	if( !file ) {
		printf( "Failed to open file: %s\n", filename );
		return;
	}
	fwrite( new_buffer, 1, new_length, file );

	free( buffer );
	free( new_buffer );
	fclose( file );
}

#ifdef _WIN32
	#define PATH_SEPARATOR "\\"
#else
	#define PATH_SEPARATOR "/"
#endif

void add_packages( const Package packages[], int num_packages, const char* project_dir ) {
	for( int i = 0; i < num_packages; ++i ) {
		char cmd[ CMD_BUF_SIZE ];

		snprintf( cmd, sizeof( cmd ), "cd %s && git remote add %s %s",
		          project_dir, packages[ i ].remote_name, packages[ i ].url );
		run_command( cmd );

		snprintf( cmd, sizeof( cmd ), "cd %s && git fetch %s %s",
		          project_dir, packages[ i ].remote_name, packages[ i ].branch );
		run_command( cmd );

		for( int j = 0; packages[ i ].files[ j ]; ++j ) {
			snprintf( cmd, sizeof( cmd ), "cd %s && git checkout %s/%s -- %s",
			          project_dir, packages[ i ].remote_name, packages[ i ].branch, packages[ i ].files[ j ] );
			run_command( cmd );

			snprintf( cmd, sizeof( cmd ), "cd %s && echo %s>> .git%sinfo%ssparse-checkout",
			          project_dir, packages[ i ].files[ j ], PATH_SEPARATOR, PATH_SEPARATOR );
			run_command( cmd );
		}
	}
}

#if defined( _WIN32 ) || defined( _WIN64 )
	#include <windows.h>
	#define RED FOREGROUND_RED
	#define GREEN FOREGROUND_GREEN
	#define BLUE FOREGROUND_BLUE
	#define CYAN ( FOREGROUND_GREEN | FOREGROUND_BLUE )
	#define YELLOW ( FOREGROUND_RED | FOREGROUND_GREEN )
	#define MAGENTA ( FOREGROUND_RED | FOREGROUND_BLUE )
	#define WHITE ( FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE )

void set_color( unsigned int color ) {
	HANDLE hConsole = GetStdHandle( STD_OUTPUT_HANDLE );
	SetConsoleTextAttribute( hConsole, color | FOREGROUND_INTENSITY );
}
#else

	#include <libgen.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <unistd.h>

	#define RED "\x1B[31m"
	#define GREEN "\x1B[32m"
	#define BLUE "\x1B[34m"
	#define CYAN "\x1B[36m"
	#define YELLOW "\x1B[33m"
	#define MAGENTA "\x1B[35m"
	#define WHITE "\x1B[0m"

void set_color( const char* color ) {
	printf( "%s", color );
}
#endif

char* get_directory() {
	char path[ 1024 ];
	char* parent_dir = malloc( 1024 * sizeof( char ) );

#ifdef _WIN32
	GetModuleFileNameA( NULL, path, sizeof( path ) );
	char* last_slash = strrchr( path, '\\' );
	if( last_slash ) {
		*last_slash = '\0';// remove everything after last slash (including it)
	}
	strncpy( parent_dir, path, 1024 );
#else
	ssize_t len = readlink( "/proc/self/exe", path, sizeof( path ) - 1 );
	if( len != -1 ) {
		path[ len ] = '\0';
		strncpy( parent_dir, dirname( path ), 1024 );
	} else {
		// handle error here
		free( parent_dir );
		return NULL;
	}
#endif
	return parent_dir;
}

int directory_exists( const char* directoryName ) {
#ifdef _WIN32
	DWORD attribs = GetFileAttributesA( directoryName );
	if( attribs == INVALID_FILE_ATTRIBUTES ) {
		return 0;
	}
	return ( attribs & FILE_ATTRIBUTE_DIRECTORY ) != 0;
#else
	struct stat info;
	if( stat( directoryName, &info ) != 0 ) {
		return 0;// stat error, might be due to non-existence
	} else if( info.st_mode & S_IFDIR ) {
		return 1;// it's a directory
	} else {
		return 0;// it's not a directory
	}
#endif
}

#ifdef _WIN32
void restart_self() {
	char executable_path[ MAX_PATH ] = { 0 };
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };

	si.cb = sizeof( si );

	// get current executable path
	if( GetModuleFileNameA( NULL, executable_path, MAX_PATH ) == 0 ) {
		printf( "Error getting module filename\n" );
		return;
	}

	// create new process
	if( !CreateProcess( NULL, executable_path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi ) ) {
		printf( "Error creating process: %lu\n", GetLastError() );
		return;
	}

	// close process and thread handles
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );

	// terminate current process
	exit( 0 );
}
#else
void restart_self() {
	char executable_path[ 1024 ];
	ssize_t len = readlink( "/proc/self/exe", executable_path, sizeof( executable_path ) - 1 );
	if( len == -1 ) {
		perror( "Error getting executable path" );
		return;
	}
	executable_path[ len ] = '\0';

	// Create a new process
	pid_t pid = fork();
	if( pid == -1 ) {
		perror( "Error forking process" );
		return;
	} else if( pid > 0 ) {
		int status;
		waitpid( pid, &status, 0 );

		exit( 0 );
	} else {
		if( execl( executable_path, executable_path, NULL ) == -1 ) {
			perror( "Error executing new process" );
			return;
		}
	}
}
#endif

int delete_file( const char* file_path ) {
	int status = remove( file_path );

	if( status == 0 )
		printf( "File deleted successfully\n" );
	else {
		printf( "Unable to delete the file\n" );
		perror( "Following error occurred" );
	}

	return status;
}

int MAIN();

void CLEAR() {
#ifdef _WIN32
	system( "cls" );
#else
	system( "clear" );
#endif
}

int MAIN() {
	CLEAR();

	if( system( "git --version" ) != 0 ) {
		CLEAR();
		set_color( CYAN );
		printf( " GIT is not installed\n" );
		set_color( MAGENTA );
		printf( " please install it to use altar\n" );
		set_color( YELLOW );
		printf( " press Enter to exit...\n" );
		getchar();
		return 0;
	} else
		CLEAR();

	if( system( "cmake --version" ) != 0 ) {
		CLEAR();
		set_color( CYAN );
		printf( " CMAKE is not installed\n" );
		set_color( MAGENTA );
		printf( " please install it to use altar\n" );
		set_color( YELLOW );
		printf( " press Enter to exit...\n" );
		getchar();
		return 0;
	} else
		CLEAR();

#ifdef _WIN32
	if( getenv( "VULKAN_SDK" ) == NULL ) {
		CLEAR();
		set_color( CYAN );
		printf( " VULKAN is not installed\n" );
		set_color( MAGENTA );
		printf( " please install VULKAN to use hept\n" );
		set_color( YELLOW );
		printf( " press Enter to exit...\n" );
		getchar();
		return 0;
	} else
		CLEAR();
#endif

	set_color( CYAN );
	printf( " :::.                    __    __\n" );
	set_color( MAGENTA );
	printf( " v1.4" );
	set_color( CYAN );
	printf( "           ____    / /   / /_   ____     ___\n" );
	printf( "               _\\__ \\  / /   / __/  _\\__ \\   / _ \\\n" );
	printf( "              / __  / / /_  / /__  / __  /  / //_/\n" );
	printf( "              \\__/\\_\\ \\__/  \\___/  \\__/\\_\\ /_/\n" );
	set_color( MAGENTA );
	printf( "    ________________________________________________________\n" );
	printf( "   / ______________________________________________________ \\\n" );
	printf( "  / /   " );
	set_color( YELLOW );
	printf( "hept project construction and manifestation tool   " );
	set_color( MAGENTA );
	printf( "\\ \\\n" );
	printf( " /_/                                                        \\_\\\n" );

	set_color( CYAN );
	printf( " inc\\ ___ hept\n" );
	printf( "        \\___ Hephaestus\n" );
	printf( "        |  \\___ Volk 1.3\n" );
	printf( "        \\___ c7h16\n" );
	set_color( MAGENTA );
	printf( "                                               by @ENDESGA 2023\n" );

	//

	set_color( YELLOW );
	printf( " enter hept project name: " );
	set_color( WHITE );
	char projectName[ PATH_BUF_SIZE ];
	fgets( projectName, sizeof( projectName ), stdin );
	projectName[ strcspn( projectName, "\n" ) ] = 0;

	char ninja_path[ CMD_BUF_SIZE ];
	char name[ CMD_BUF_SIZE ];
#ifdef _WIN32
	snprintf( name, sizeof( name ), "%s\\%s", get_directory(), projectName );
	snprintf( ninja_path, sizeof( ninja_path ), "%s\\%s\\out\\ninja\\ninja.exe", get_directory(), projectName );
#else
	snprintf( name, sizeof( name ), "%s/%s", get_directory(), projectName );
	snprintf( ninja_path, sizeof( ninja_path ), "%s/%s/out/ninja/ninja", get_directory(), projectName );
#endif

	char cmd[ CMD_BUF_SIZE ];

	if( directory_exists( name ) ) {
		set_color( MAGENTA );
		printf( " hept project already exists!\n" );
		set_color( CYAN );
		printf( " rebuild " );
		set_color( WHITE );
		printf( "%s", projectName );
		set_color( CYAN );
		printf( "?" );
		set_color( YELLOW );
		printf( " y" );
		set_color( MAGENTA );
		printf( "es / " );
		set_color( YELLOW );
		printf( "n" );
		set_color( MAGENTA );
		printf( "o\n" );

		char choice[ 2 ];
		fgets( choice, sizeof( choice ), stdin );

		switch( choice[ 0 ] ) {
			case 'y': {
				char cmake_delete[ CMD_BUF_SIZE ];
#ifdef _WIN32
				snprintf( cmake_delete, sizeof( cmake_delete ), "%s\\%s\\out\\CMakeCache.txt", get_directory(), projectName );
#else
				snprintf( cmake_delete, sizeof( cmake_delete ), "%s/%s/out/CMakeCache.txt", get_directory(), projectName );
#endif
				printf( "%s\n", cmake_delete );
				delete_file( cmake_delete );
				goto REBUILD;
				break;
			}

			case 'n': {
				CLEAR();
				restart_self();
				break;
			}
		}
	}

	set_color( CYAN );
#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "mkdir %s && cd %s && git init && mkdir inc src out && echo.> src\\main.c", projectName, projectName );
	run_command( cmd );
#else
	// Arch Linux
	snprintf( cmd, sizeof( cmd ), "mkdir %s && cd %s && git init && mkdir inc src out && touch README.md && touch src/main.c", projectName, projectName );
	run_command(
	  cmd );
#endif

	Package packages[] = {
	  { "c7h16", "https://github.com/ENDESGA/c7h16.git", "main", { "c7h16.h", ".clang-format", NULL } },
	  { "Hephaestus", "https://github.com/ENDESGA/Hephaestus.git", "main", { "Hephaestus.h", NULL } },
	  { "hept", "https://github.com/ENDESGA/hept.git", "main", { "hept.h", "CMakeLists.txt", NULL } },
#ifdef _WIN32
	  { "altar", "https://github.com/ENDESGA/altar.git", "main", { "out\\ninja\\ninja.exe", NULL } },
#else
	  { "altar", "https://github.com/ENDESGA/altar.git", "main", { "out/ninja/ninja", NULL } },
#endif
	  { "volk", "https://github.com/zeux/volk.git", "master", { "volk.h", "volk.c", NULL } } };

	char project_dir[ PATH_BUF_SIZE ];
	snprintf( project_dir, sizeof( project_dir ), "%s", projectName );
	add_packages( packages, sizeof( packages ) / sizeof( packages[ 0 ] ), project_dir );

	set_color( MAGENTA );
#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "cd %s && move c7h16.h inc\\c7h16.h && move Hephaestus.h inc\\Hephaestus.h && move hept.h inc\\hept.h && mkdir inc\\Volk && move volk.h inc\\Volk\\volk.h && move volk.c inc\\Volk\\volk.c", projectName );
	run_command( cmd );
#else
	snprintf( cmd, sizeof( cmd ), "cd %s && mv c7h16.h inc/c7h16.h && mv Hephaestus.h inc/Hephaestus.h && mv hept.h inc/hept.h && mkdir -p inc/Volk && mv volk.h inc/Volk/volk.h && mv volk.c inc/Volk/volk.c", projectName );
	run_command( cmd );

	//

#endif
	set_color( YELLOW );
#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "cd %s && xcopy /E /I %%VULKAN_SDK%%\\Include\\vulkan inc\\vulkan && xcopy /E /I %%VULKAN_SDK%%\\Include\\vk_video inc\\vk_video", projectName );
	run_command( cmd );
#endif

	set_color( CYAN );
#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "cd %s && rd /s /q .git", projectName );
#else
	snprintf( cmd, sizeof( cmd ), "cd %s && rm -rf .git", projectName );
#endif
	run_command( cmd );

#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "%s\\CMakeLists.txt", projectName );
#else
	snprintf( cmd, sizeof( cmd ), "%s/CMakeLists.txt", projectName );
#endif
	replace_in_file( cmd, "hept", projectName );

REBUILD:

#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "cd %s && cmake -DCMAKE_MAKE_PROGRAM=%s -G \"Ninja Multi-Config\" -B out", projectName, ninja_path );
#else
	snprintf( cmd, sizeof( cmd ), "cd %s && chmod +x %s && cmake -DCMAKE_MAKE_PROGRAM=%s -G \"Ninja Multi-Config\" -B out", projectName, ninja_path, ninja_path );
#endif
	run_command( cmd );

#ifdef _WIN32
	snprintf( cmd, sizeof( cmd ), "cd %s && echo cmake --build out --config Debug > build_debug.bat", projectName );
	run_command( cmd );
	snprintf( cmd, sizeof( cmd ), "cd %s && echo cmake --build out --config Release > build_release.bat", projectName );
	run_command( cmd );
#else
	// Arch Linux
	snprintf( cmd, sizeof( cmd ), "cd %s && echo 'cmake --build out --config Debug' > build_debug.sh", projectName );
	run_command( cmd );
	snprintf( cmd, sizeof( cmd ), "cd %s && echo 'cmake --build out --config Release' > build_release.sh", projectName );
	run_command( cmd );
	snprintf( cmd, sizeof( cmd ), "cd %s && chmod +x build_debug.sh build_release.sh", projectName );
	run_command( cmd );
#endif

	set_color( YELLOW );
	printf( " project creation complete.\n" );

	set_color( CYAN );
	printf( " %s project made in %s\\out\\\n", projectName, projectName );

	set_color( MAGENTA );
	printf( " press Enter to exit...\n" );
	getchar();
}

int main() {
	MAIN();

	return 0;
}