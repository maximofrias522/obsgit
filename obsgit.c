#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define REPO_PATH "/home/mfrias/Obsidian"
#define GIT_COMMAND "git"
#define REMOTE_BRANCH "origin/main"

// Códigos de color ANSI
#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_BOLD    "\x1b[1m"

// Función para ejecutar comandos y mostrar su salida
int execute_command(const char *command, bool show_output) {
    FILE *fp;
    char buffer[1024];
    
    if (show_output) {
        printf(COLOR_BLUE "\nEjecutando: " COLOR_YELLOW "%s" COLOR_RESET "\n", command);
    }
    
    fp = popen(command, "r");
    if (fp == NULL) {
        printf(COLOR_RED "Error al ejecutar el comando" COLOR_RESET "\n");
        return -1;
    }
    
    if (show_output) {
        while (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("%s\n", buffer);
        }
    }
    
    return pclose(fp);
}

// Función para verificar si el directorio es un repositorio git
bool is_git_repo(const char *path) {
    char git_dir[1024];
    snprintf(git_dir, sizeof(git_dir), "%s/.git", path);
    
    if (access(git_dir, F_OK) == 0) {
        return true;
    }
    return false;
}

// Función para mostrar información de los commits
void show_commit_info(const char *repo_path) {
    char command[2048];
    
    // Cambiar al directorio del repositorio
    if (chdir(repo_path) != 0) {
        printf(COLOR_RED "Error al cambiar al directorio del repositorio" COLOR_RESET "\n");
        exit(EXIT_FAILURE);
    }
    
    // Mostrar commit local
    printf(COLOR_CYAN COLOR_BOLD "\n=== Último commit local ===" COLOR_RESET);
    snprintf(command, sizeof(command), "%s log -n 1 --pretty=format:'%%h - %%an (%%cd): %%s'", GIT_COMMAND);
    execute_command(command, true);
    
    // Mostrar commit remoto
    printf(COLOR_CYAN COLOR_BOLD "\n=== Último commit remoto ===" COLOR_RESET);
    snprintf(command, sizeof(command), "%s fetch origin", GIT_COMMAND);
    execute_command(command, false);
    snprintf(command, sizeof(command), "%s log -n 1 --pretty=format:'%%h - %%an (%%cd): %%s' origin/main", GIT_COMMAND);
    execute_command(command, true);
}

// Función para realizar la actualización
void perform_update() {
    char command[2048];
    
    printf(COLOR_GREEN COLOR_BOLD "\n=== Realizando actualización ===" COLOR_RESET);
    
    // Git fetch (sin salida)
    snprintf(command, sizeof(command), "%s fetch origin", GIT_COMMAND);
    printf(COLOR_BLUE "\nEjecutando: " COLOR_YELLOW "%s" COLOR_RESET, command);
    int fetch_result = system(command);
    if (fetch_result != 0) {
        printf(COLOR_RED "\nError al ejecutar git fetch" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    
    // Git reset (con salida)
    snprintf(command, sizeof(command), "%s reset --hard %s", GIT_COMMAND, REMOTE_BRANCH);
    printf(COLOR_BLUE "\nEjecutando: " COLOR_YELLOW "%s" COLOR_RESET "\n", command);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf(COLOR_RED "Error al ejecutar git reset" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }
    pclose(fp);
    
    printf(COLOR_GREEN "Actualización completada con éxito.\n" COLOR_RESET);
}

// Menú principal
void show_menu() {
    printf(COLOR_MAGENTA COLOR_BOLD "\n=== Gestor de Actualización Git ===" COLOR_RESET "\n");
    printf(COLOR_BOLD "1." COLOR_RESET " Realizar actualización (fetch + reset)\n");
    printf(COLOR_BOLD "2." COLOR_RESET " Salir\n");
    printf(COLOR_YELLOW "Seleccione una opción: " COLOR_RESET);
}

void perform_update_and_exit() {
    perform_update();
    printf("\n");
    exit(EXIT_SUCCESS);
}

int main() {
    // Verificar que el directorio es un repo git
    if (!is_git_repo(REPO_PATH)) {
        printf(COLOR_RED "Error: %s no es un repositorio git válido." COLOR_RESET "\n", REPO_PATH);
        return EXIT_FAILURE;
    }
    
    int option;
    bool running = true;
    
    while (running) {
        show_menu();
        scanf("%d", &option);
        
        switch (option) {
            case 1:
                show_commit_info(REPO_PATH);
                printf(COLOR_YELLOW "\n¿Está seguro que desea actualizar el repositorio local "
                       "para que coincida con el remoto? (s/n): " COLOR_RESET);
                
                char confirm;
                scanf(" %c", &confirm);
                if (confirm == 's' || confirm == 'S') {
                    perform_update_and_exit(); // Esta función ahora maneja la salida
                } else {
                    printf(COLOR_YELLOW "Actualización cancelada." COLOR_RESET "\n");
                }
                break;
            case 2:
                running = false;
                break;
            default:
                printf(COLOR_RED "Opción no válida. Intente nuevamente." COLOR_RESET "\n");
        }
    }
    
    printf("\n");
    return EXIT_SUCCESS;
}
