#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <format>
#include <vector>

using namespace std;
using namespace filesystem;

struct package
{
	string remote_name;
	string url;
	string branch;
	vector<string> files;
};

vector<package> packages = {
	{		 "c7h16",			 "https://github.com/ENDESGA/c7h16.git",	 "main", { "c7h16.h", ".clang-format" }},
	{"Hephaestus", "https://github.com/ENDESGA/Hephaestus.git",	 "main",						 { "Hephaestus.h" }},
	{			"hept",				"https://github.com/ENDESGA/hept.git",	 "main", { "hept.h", "CMakeLists.txt" }},
#ifdef _WIN32
	{		 "altar",			 "https://github.com/ENDESGA/altar.git",	 "main",		 { "out\\ninja\\ninja.exe" }},
#else
	{ "altar", "https://github.com/ENDESGA/altar.git", "main", { "out/ninja/ninja" } },
#endif
	{			"volk",					"https://github.com/zeux/volk.git", "master",					{ "volk.h", "volk.c" }}
};

//

void replace_in_file(const string& filepath, const string& target, const string& replacement) {
	path path(filepath);
	if (!exists(path)) return;

	string file_content;
	{
		ifstream file(filepath, ios::in);
		if (!file.is_open()) return;

		file_content.assign(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
		file.close();
	}

	size_t pos = file_content.find(target);
	if (pos != string::npos) {
		file_content.replace(pos, target.length(), replacement);
	}

	{
		ofstream file(filepath, ios::out | ios::trunc);
		if (!file.is_open()) return;

		file << file_content;
		file.close();
	}
}

//

int main()
{
	cout << " :::.                    __    __" << endl;
	cout << " v1.5           ____    / /   / /_   ____     ___" << endl;
	cout << "               _\\__ \\  / /   / __/  _\\__ \\   / _ \\" << endl;
	cout << "              / __  / / /_  / /__  / __  /  / //_/" << endl;
	cout << "              \\__/\\_\\ \\__/  \\___/  \\__/\\_\\ /_/" << endl;
	cout << "    ________________________________________________________" << endl;
	cout << "   / ______________________________________________________ \\" << endl;
	cout << "  / /                                                      \\ \\" << endl;
	cout << " /_/    hept project construction and manifestation tool    \\_\\" << endl;
	cout << endl;
	cout << " inc\\ ___ hept" << endl;
	cout << "        \\___ Hephaestus" << endl;
	cout << "        |  \\___ Volk 1.3" << endl;
	cout << "        \\___ c7h16" << endl;
	cout << "                                               by @ENDESGA 2023" << endl;

	//

	cout << " enter hept project name: ";
	string project_name_str = "";
	getline( cin, project_name_str );

	path project_name = path(project_name_str);
	path project_dir = current_path() / project_name;

#ifdef _WIN32
	path ninja_path = project_dir / "out" / "ninja" / "ninja.exe";
#else
	path ninja_path = project_dir / "out" / "ninja" / "ninja";
#endif

	create_directories( project_name / "inc" / "Volk" );
	create_directories( project_name / "src" );
	create_directories( project_name / "out" );

	system( ( "cd " + project_name.string() + " && git init" ).c_str() );

	ofstream( project_name / "src" / "main.c" );

	// get packages
	for( const auto &pkg: packages )
	{
		system( format( "cd {} && git remote add {} {}", project_dir.string(), pkg.remote_name, pkg.url ).c_str() );
		system( format( "cd {} && git fetch {} {}", project_dir.string(), pkg.remote_name, pkg.branch ).c_str() );

		for( const auto &file: pkg.files )
		{
			system( format( "cd {} && git checkout {}/{} -- {}", project_dir.string(), pkg.remote_name, pkg.branch, file ).c_str() );
			system( format( "cd {} && echo {} >> .git/info/sparse-checkout", project_dir.string(), file ).c_str() );
		}
	}

	// move files
	rename( project_name / "c7h16.h", project_name / "inc" / "c7h16.h" );
	rename( project_name / "Hephaestus.h", project_name / "inc" / "Hephaestus.h" );
	rename( project_name / "hept.h", project_name / "inc" / "hept.h" );
	rename( project_name / "volk.h", project_name / "inc" / "Volk" / "volk.h" );
	rename( project_name / "volk.c", project_name / "inc" / "Volk" / "volk.c" );

	// get Vulkan for Windows
#ifdef _WIN32
	system(format("cd {} && xcopy /E /I %VULKAN_SDK%\\Include\\vulkan inc\\vulkan && xcopy /E /I %VULKAN_SDK%\\Include\\vk_video inc\\vk_video", project_name.string()).c_str());
#endif

	// edit CMakeLists
	replace_in_file((project_name / "CMakeLists.txt").string(), "hept", project_name.string());

	// start CMake
#ifdef _WIN32
		system(format("cd {} && cmake -DCMAKE_MAKE_PROGRAM={} -G \"Ninja Multi-Config\" -B out", project_name.string(), ninja_path.string()).c_str());
		system(format("cd {} && echo cmake --build out --config Debug > build_debug.bat", project_name.string()).c_str());
		system(format("cd {} && echo cmake --build out --config Release > build_release.bat", project_name.string()).c_str());
#else
		system(format("cd {} && chmod +x {} && cmake -DCMAKE_MAKE_PROGRAM={} -G \"Ninja Multi-Config\" -B out", project_name.string(), ninja_path.string(), ninja_path.string()).c_str());
		system(format("cd {} && echo 'cmake --build out --config Debug' > build_debug.sh", project_name.string()).c_str());
		system(format("cd {} && echo 'cmake --build out --config Release' > build_release.sh", project_name.string()).c_str());
		system(format("cd {} && chmod +x build_debug.sh build_release.sh", project_name.string()).c_str());
#endif

	cout << " project creation complete." << endl;
	cout << format(" {} project made in {}\\out\\", project_name.string(), project_name.string()) << endl;
	cout << " press Enter to exit..." << endl;
	cin.get();
}