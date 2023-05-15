#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_BUF_SIZE 2048
#define PATH_BUF_SIZE 512
#define MAX_FILE_SIZE 8192
#define NUM_PACKAGES 4

typedef struct {
    const char* remote_name;
    const char* url;
    const char* branch;
    const char* files[5];
} Package;

#define run_command( _ ) system( _ )

int count_substrings(const char *str, const char *sub) {
    int count = 0;
    const char *temp = str;

    while ((temp = strstr(temp, sub)) != NULL) {
        count++;
        temp += strlen(sub);
    }

    return count;
}

char *optimized_strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; ++haystack) {
        const char *h = haystack, *n = needle;
        while (*h && *n && *h == *n) {
            ++h;
            ++n;
        }
        if (!*n) return (char *)haystack;
    }
    return NULL;
}

void replace_in_file(const char *filename, const char *find, const char *replace) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(length + 1);
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);

    int occurrences = count_substrings(buffer, find);
    size_t find_len = strlen(find);
    size_t replace_len = strlen(replace);
    size_t diff = replace_len - find_len;
    long new_length = (long)(length + occurrences * diff);
    char *new_buffer = malloc(new_length + 1);

    char *src = buffer, *dst = new_buffer, *pos;
    while ((pos = optimized_strstr(src, find)) != NULL) {
        size_t bytes_to_copy = pos - src;
        memcpy(dst, src, bytes_to_copy);
        memcpy(dst + bytes_to_copy, replace, replace_len);

        src += bytes_to_copy + find_len;
        dst += bytes_to_copy + replace_len;
    }
    strcpy(dst, src);

    file = fopen(filename, "wb");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return;
    }
    fwrite(new_buffer, 1, new_length, file);

    free(buffer);
    free(new_buffer);
    fclose(file);
}

void add_packages(const Package packages[], int num_packages, const char* project_dir) {
    for (int i = 0; i < num_packages; ++i) {
        char cmd[CMD_BUF_SIZE];
        snprintf(cmd, sizeof(cmd), "cd %s && git remote add %s %s", project_dir, packages[i].remote_name, packages[i].url);
        run_command(cmd);

        snprintf(cmd, sizeof(cmd), "cd %s && git fetch %s %s", project_dir, packages[i].remote_name, packages[i].branch);
        run_command(cmd);

        for (int j = 0; packages[i].files[j]; ++j) {
            snprintf(cmd, sizeof(cmd), "cd %s && git checkout %s/%s -- %s", project_dir, packages[i].remote_name, packages[i].branch, packages[i].files[j]);
            run_command(cmd);

            snprintf(cmd, sizeof(cmd), "cd %s && echo %s>> .git\\info\\sparse-checkout", project_dir, packages[i].files[j]);
            run_command(cmd);
        }
    }
}

#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #define RED     FOREGROUND_RED
    #define GREEN   FOREGROUND_GREEN
    #define BLUE    FOREGROUND_BLUE
    #define CYAN    (FOREGROUND_GREEN | FOREGROUND_BLUE)
    #define YELLOW  (FOREGROUND_RED | FOREGROUND_GREEN)
    #define MAGENTA (FOREGROUND_RED | FOREGROUND_BLUE)
    #define WHITE   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

    void set_color(unsigned int color) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color | FOREGROUND_INTENSITY);
    }
#else
    #define RED     "\x1B[31m"
    #define GREEN   "\x1B[32m"
    #define BLUE    "\x1B[34m"
    #define CYAN    "\x1B[36m"
    #define YELLOW  "\x1B[33m"
    #define MAGENTA "\x1B[35m"
    #define WHITE   "\x1B[0m"

    void set_color(const char* color) {
        printf("%s", color);
    }
#endif

int main() {
    run_command("mode con: cols=64 lines=16");

    set_color(CYAN);
    printf(" :::.                    __    __\n");
    printf("                ____    / /   / /_   ____     ___\n");
    printf("               _\\__ \\  / /   / __/  _\\__ \\   / _ \\\n");
    printf("              / __  / / /_  / /__  / __  /  / //_/\n");
    printf("              \\____/  \\__/  \\___/  \\____/  /_/\n");
    set_color(MAGENTA);
    printf("    ________________________________________________________\n");
    printf("   / ______________________________________________________ \\\n");
    printf("  / /   ");set_color(YELLOW);printf("hept project construction and manifestation tool   ");set_color(MAGENTA);printf("\\ \\\n");
    printf(" /_/                                                        \\_\\\n");
    //printf("\n                            ///////\n");
    set_color(YELLOW);                                                  //-
    //printf(" -  an explicit laconically esoteric C abstraction framework  -\n");
    set_color(CYAN);
    printf(" inc\\ ___ hept\n");
    printf("        \\___ Hephaestus\n");
    printf("        |  \\___ Volk 1.3\n");
    printf("        \\___ c7h16\n");
    set_color(MAGENTA);
    printf("                                               by @ENDESGA 2023\n");
    set_color(YELLOW);
    printf(" enter the project name: ");
    set_color(WHITE);
    char projectName[PATH_BUF_SIZE];
    fgets(projectName, sizeof(projectName), stdin);
    projectName[strcspn(projectName, "\n")] = 0;

    char cmd[CMD_BUF_SIZE];

    set_color(CYAN);
    snprintf(cmd, sizeof(cmd), "mkdir %s && cd %s && git init && mkdir inc src && echo.> README.md && echo.> src\\main.c", projectName, projectName);
    run_command(cmd);

    Package packages[] = {
        {"c7h16", "https://github.com/ENDESGA/c7h16.git", "main", {"c7h16.h", NULL}},
        {"Hephaestus", "https://github.com/ENDESGA/Hephaestus.git", "main", {"hephaestus.h", NULL}},
        {"hept", "https://github.com/ENDESGA/hept.git", "main", {"hept.h", "CMakeLists.txt", NULL}},
        {"volk", "https://github.com/zeux/volk.git", "master", {"volk.h", "volk.c", NULL}}
    };

    char project_dir[PATH_BUF_SIZE];
    snprintf(project_dir, sizeof(project_dir), "%s", projectName);
    add_packages(packages, sizeof(packages) / sizeof(packages[0]), project_dir);

    set_color(MAGENTA);
    snprintf(cmd, sizeof(cmd), "cd %s && move c7h16.h inc\\c7h16.h && move hephaestus.h inc\\hephaestus.h && move hept.h inc\\hept.h && mkdir inc\\Volk && move volk.h inc\\Volk\\volk.h && move volk.c inc\\Volk\\volk.c", projectName);
    run_command(cmd);

    set_color(YELLOW);
    snprintf(cmd, sizeof(cmd), "cd %s && xcopy /E /I %%VULKAN_SDK%%\\Include\\vulkan inc\\vulkan && xcopy /E /I %%VULKAN_SDK%%\\Include\\vk_video inc\\vk_video && xcopy /E /I %%VULKAN_SDK%%\\Include\\shaderc inc\\shaderc && xcopy /E /I %%VULKAN_SDK%%\\Lib\\shaderc_shared.lib inc", projectName);
    run_command(cmd);

    set_color(CYAN);
    snprintf(cmd, sizeof(cmd), "cd %s && rd /s /q .git", projectName);
    run_command(cmd);

    snprintf(cmd, sizeof(cmd), "%s\\CMakeLists.txt", projectName);
    replace_in_file(cmd, "enframe", projectName);

    set_color(MAGENTA);
    snprintf(cmd, sizeof(cmd), "cd %s && mkdir build && cd build && cmake ..", projectName);
    run_command(cmd);

    set_color(YELLOW);
    printf(" project creation complete.\n");

    set_color(CYAN);
    printf(" press Enter to exit...\n");
    getchar();

    return 0;
}