#ifndef CONSOLE_CONSOLE_HEADER_FILE
#define CONSOLE_CONSOLE_HEADER_FILE

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace CppReadline {
    class Console {
        public:
            /**
             * @brief This is the function type that is used to interface with the Console class.
             * 
             * These are the functions that are going to get called by Console when the user
             * types in a message. The vector will hold the command elements, and the function
             * needs to return its result. The result can either be OK (0), or an arbitrary error
             * (>=1). Normal functions cannot directly issue the console to quit, which is a
             * return code that can only be issued by the 'quit' and 'exit' commands which are
             * hardcoded in the Console (you can change those in the cpp file).
             */
            using CommandFunction = std::function<unsigned(const std::vector<std::string> &)>;

            enum ReturnCode {
                Quit = -1,
                Ok = 0,
                Error = 1 // Or greater!
            };

            /**
             * @brief Basic constructor.
             *
             * @param greeting This represents the prompt of the Console.
             */
            Console(std::string greeting);

            /**
             * @brief Basic destructor.
             * 
             * Frees the history which is been produced by GNU readline.
             */
            ~Console();

            /**
             * @brief This function registers a new command within the Console.
             *
             * @param s The name of the command as inserted by the user.
             * @param f The function that will be called once the user writes the command.
             */
            void registerCommand(const std::string & s, CommandFunction f);

            /**
             * @brief This function returns a list with the currently available commands.
             *
             * @return A vector containing all registered commands names.
             */
            std::vector<std::string> getRegisteredCommands() const;

            /**
             * @brief This function executes an arbitrary string as if it was inserted via stdin.
             *
             * @param command The command that needs to be executed.
             *
             * @return The result of the operation.
             */
            int executeCommand(const std::string & command);

            /**
             * @brief This function calls an external script and executes all commands inside.
             * 
             * This function stops execution as soon as any single command returns something
             * different from 0, be it a quit code or an error code.
             *
             * @param filename The pathname of the script.
             *
             * @return What the last command executed returned.
             */
            int executeFile(const std::string & filename);

            /**
             * @brief This function executes a single command from the user via stdin.
             *
             * @return The result of the operation.
             */
            int readLine();
        private:
            using RegisteredCommands = std::unordered_map<std::string,CommandFunction>;

            std::string greeting_;
            RegisteredCommands commands_;
            // This is just to avoid importing library names in here
            void * history_;

            /**
             * @brief This function saves the current state so that some other Console can make use of the GNU readline facilities.
             */
            void saveState();
            /**
             * @brief This function reserves the use of the GNU readline facilities to the calling Console instance.
             */
            void reserveConsole();
            static Console * currentConsole_;
            static void * emptyHistory_;

            // GNU newline interface to our commands.
            using commandCompleterFunction = char**(const char * text, int start, int end);
            using commandIteratorFunction = char*(const char * text, int state);

            static commandCompleterFunction getCommandCompletions;
            static commandIteratorFunction commandIterator;
    };
}

#endif
