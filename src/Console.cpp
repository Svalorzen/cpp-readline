#include "Console.hpp"

#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>
#include <iterator>
#include <sstream>

#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>

namespace CppReadline {
    Console * Console::currentConsole_  = nullptr;
    void * Console::emptyHistory_       = static_cast<void*>(history_get_history_state());

    // Here we set default commands, they do nothing since we quit with them
    // Quitting behaviour is hardcoded in readLine()
    Console::Console(std::string greeting) : greeting_(greeting),

    // These are hardcoded commands. They do not do anything and are catched manually in the executeCommand function.
    commands_({   {"quit",{}}, {"exit",{}}   }),
    history_(nullptr)
    {
        // Init readline basics
        rl_attempted_completion_function = &Console::getCommandCompletions;
        // These are other two hardcoded commands, but they are more readable
        // here rather than in the initialization list.
        // Help command lists available commands.
        commands_["help"] = [this](const std::vector<std::string>&){
            auto commands = getRegisteredCommands();
            std::cout << "Available commands are:\n";
            for ( auto & command : commands ) std::cout << "\t" << command << "\n";
            return 0;
        };
        // Run command executes all commands in an external file.
        commands_["run"] =  [this](const std::vector<std::string>& input) {
            if ( input.size() < 2 ) { std::cout << "Usage: " << input[0] << " script_filename\n"; return 1; }
            return executeFile(input[1]);
        };
    }

    Console::~Console() {
        free(history_);
    }

    void Console::registerCommand(const std::string & s, CommandFunction f) {
        commands_[s] = f;
    }

    std::vector<std::string> Console::getRegisteredCommands() const {
        std::vector<std::string> allCommands;
        for ( auto & pair : commands_ ) allCommands.push_back(pair.first);

        return allCommands;
    }

    void Console::saveState() {
        free(history_);
        history_ = static_cast<void*>(history_get_history_state());
    }

    void Console::reserveConsole() {
        if ( currentConsole_ == this ) return;

        // Save state of other Console
        if ( currentConsole_ )
            currentConsole_->saveState();

        // Else we swap state
        if ( ! history_ )
            history_set_history_state(static_cast<HISTORY_STATE*>(emptyHistory_));
        else
            history_set_history_state(static_cast<HISTORY_STATE*>(history_));

        // Tell others we are using the console
        currentConsole_ = this;
    }

    int Console::executeCommand(const std::string & command) {
        // Convert input to vector
        std::vector<std::string> inputs;
        {
            std::istringstream iss(command);
            std::copy(std::istream_iterator<std::string>(iss),
                    std::istream_iterator<std::string>(),
                    std::back_inserter(inputs));
        }

        if ( inputs.size() == 0 ) return ReturnCode::Ok;
        if ( inputs[0] == "quit" || inputs[0] == "exit" ) return ReturnCode::Quit;

        RegisteredCommands::iterator it;
        if ( ( it = commands_.find(inputs[0]) ) != end(commands_) ) {
            return static_cast<int>((it->second)(inputs));
        }

        std::cout << "Command '" << inputs[0] << "' not found.\n";
        return ReturnCode::Error;
    }

    int Console::executeFile(const std::string & filename) {
        std::ifstream input(filename);
        if ( ! input ) {
            std::cout << "Could not find the specified file to execute.\n";
            return ReturnCode::Error;
        }
        std::string command;
        int counter = 0, result;

        while ( std::getline(input, command)  ) {
            if ( command[0] == '#' ) continue; // Ignore comments
            // Report what the Console is executing.
            std::cout << "[" << counter << "] " << command << '\n';
            if ( (result = executeCommand(command)) ) return result;
            ++counter; std::cout << '\n';
        }

        // If we arrived successfully at the end, all is ok
        return ReturnCode::Ok;
    }

    int Console::readLine() {
        reserveConsole();

        char * buffer = readline(greeting_.c_str());
        if ( !buffer ) {
            std::cout << '\n'; // EOF doesn't put last endline so we put that so that it looks uniform.
            return ReturnCode::Quit;
        }

        // TODO: Maybe add commands to history only if succeeded?
        if ( buffer[0] != '\0' )
            add_history(buffer);

        std::string line(buffer);
        free(buffer);

        return executeCommand(line);
    }

    char ** Console::getCommandCompletions(const char * text, int start, int) {
        char ** completionList = nullptr;

        if ( start == 0 )
            completionList = rl_completion_matches(text, &Console::commandIterator);

        return completionList;
    }

    char * Console::commandIterator(const char * text, int state) {
        static RegisteredCommands::iterator it;
        auto & commands_ = currentConsole_->commands_;

        if ( state == 0 ) it = begin(commands_);

        while ( it != end(commands_ ) ) {
            auto & command = it->first;
            ++it;
            if ( command.find(text) != std::string::npos ) {
                char * completion = new char[command.size()];
                strcpy(completion, command.c_str());
                return completion;
            }
        }
        return nullptr;
    }
}
