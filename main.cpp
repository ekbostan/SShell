#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_NUM_ARGS 15
#define MAX_NUM_PIPE_ARGS 3
#define MAX_CMD_SIZE 512

struct CommandLine {
    std::vector<std::string> argv;
    CommandLine() {}
};

enum ParsingErrors {
    ARGS,
    MISSING,
    NO_OUTPUT,
    PERMISSIONS,
    OUTPUT_LOCATION,
    NONE
};

void executeCommand(const CommandLine& cmd) {
    // Convert std::vector<std::string> to char* array for execvp
    std::vector<char*> args;
    for (auto& arg : cmd.argv) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);  

    if (execvp(args[0], args.data()) == -1) {
        perror("execvp");
    }
}
bool executeBuiltinCd(const std::vector<std::string>& argv) {
    if (argv.size() > 1) {
        if (chdir(argv[1].c_str()) != 0) {
            perror("chdir");
            return false;
        }
        return true;
    } else {
        std::cerr << "Error: cd command requires a path argument" << std::endl;
        return false;
    }
}

void executeBuiltinPwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        std::cout << cwd << std::endl;
    } else {
        perror("getcwd");
    }
}

bool executeBuiltinExit() {
    std::exit(0);
    return true;  
}


int main() {
    while (true) {
        std::string command_line;
        std::cout << "sshell$ ";
        std::cout.flush();

        if (!std::getline(std::cin, command_line)) {
            if (std::cin.eof()) {
                break;
            }
            continue;
        }

        std::istringstream iss(command_line);
        std::string token;
        std::vector<CommandLine> commands;
        CommandLine currentCommand;

        while (iss >> token) {
            if (token == "|") {
                if (!currentCommand.argv.empty()) {
                    commands.push_back(currentCommand);
                    currentCommand.argv.clear();
                }
            } else {
                currentCommand.argv.push_back(token);
            }
        }
        if (!currentCommand.argv.empty()) {
            commands.push_back(currentCommand);
        }

        int numCommands = commands.size();
        if (numCommands == 0) {
            std::cerr << "Error: no command entered\n";
            continue;
        }
          for (const auto& command : commands) {
            if (command.argv.empty()) continue;

            const std::string& cmd = command.argv[0];
            if (cmd == "exit") {
                executeBuiltinExit();
            } else if (cmd == "cd") {
                executeBuiltinCd(command.argv);
                continue;
            } else if (cmd == "pwd") {
                executeBuiltinPwd();
                continue;
            }

        int pipes[2 * (numCommands - 1)];  // Create pipes
        for (int i = 0; i < numCommands - 1; i++) {
            if (pipe(pipes + 2 * i) < 0) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }
        }

        for (int i = 0; i < numCommands; i++) {
            pid_t pid = fork();
            if (pid == 0) { 
                if (i > 0) { 
                    dup2(pipes[2 * (i - 1)], STDIN_FILENO);
                }
                if (i < numCommands - 1) {
                    dup2(pipes[2 * i + 1], STDOUT_FILENO);
                }
                for (int j = 0; j < 2 * (numCommands - 1); j++) {
                    close(pipes[j]);
                }
                executeCommand(commands[i]);
                exit(EXIT_FAILURE);
            }
        }
        for (int i = 0; i < 2 * (numCommands - 1); i++) {
            close(pipes[i]);
        }
        for (int i = 0; i < numCommands; i++) {
            wait(NULL);
        }
        continue;
    }
    return 0;
}
}