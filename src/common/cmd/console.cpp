#include "console.h"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>

namespace cmd
{
    // ==================== DynamicVariable ====================

    constexpr const char* Сonsole::Variable::ValueTypeToString(const ValueType& type)
    {
        switch (type)
        {
            case ValueType::floating:
                return "floating";
            case ValueType::integer:
                return "integer";
            case ValueType::boolean:
                return "boolean";
            case ValueType::string:
                return "string";
            case ValueType::vec3:
                return "vec3";
            case ValueType::none:
            default:
                return "none";
        }
    }

    void Сonsole::DynamicVariable::setTypeFromValue() {
        std::visit([this](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int>) {
                currentType = ValueType::integer;
            }
            else if constexpr (std::is_same_v<T, floating>) {
                currentType = ValueType::floating;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                currentType = ValueType::boolean;
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                currentType = ValueType::string;
            }
            else if constexpr (std::is_same_v<T, vec3>) {
                currentType = ValueType::vec3;
            }
        }, value);
    }


    void Сonsole::log(const std::string& msg)
    {
        if (log_callback)
            log_callback(msg);
    }

    std::vector<std::string> Сonsole::tokenize(const std::string& input) {
        std::vector<std::string> tokens;
        std::string token;
        bool inQuotes = false;
        bool escapeNext = false;

        for (char c : input) {
            if (escapeNext) {
                token += c;
                escapeNext = false;
            }
            else if (c == '\\') {
                escapeNext = true;
            }
            else if (c == '"') {
                inQuotes = !inQuotes;
                if (!inQuotes && !token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else if (std::isspace(c) && !inQuotes) {
                if (!token.empty()) {
                    tokens.push_back(token);
                    token.clear();
                }
            }
            else {
                token += c;
            }
        }

        if (!token.empty()) {
            tokens.push_back(token);
        }

        return tokens;
    }

    void Сonsole::addToHistory(const std::string& command) {
        if (command.empty()) return;

        command_history.push_front(command);
        if (command_history.size() > MAX_HISTORY_SIZE) {
            command_history.pop_back();
        }
        resetHistoryNavigation();
    }

    void Сonsole::writeToBuffer(const std::string& message)
    {
        console_buffer << message;
        if (write_to_buf_callback)
            write_to_buf_callback(message);
    }

    void Сonsole::helpCommand(const std::vector<std::string>& args) {
        if (args.empty())
        {
            println("Available commands:");
            for (const auto& [name, desc] : command_descriptions)
            {
                println("  " + name + " - " + desc);
            }
        }
        else {
            auto it = command_descriptions.find(args[0]);
            if (it != command_descriptions.end()) {
                print("Command: " + args[0]);
                print("Description: " + it->second);
            }
            else {
                printError("Command not found: " + args[0]);
            }
        }
    }

    void Сonsole::historyCommand(const std::vector<std::string>& args) {
        int count = 10;
        if (!args.empty()) {
            try {
                count = std::stoi(args[0]);
            }
            catch (...) {
                printError("Invalid number: " + args[0]);
                return;
            }
        }

        int i = 1;
        auto it = command_history.begin();
        while (it != command_history.end() && i <= count) {
            println(std::to_string(i) + ": " + *it);
            ++it;
            ++i;
        }
    }

    void Сonsole::clearCommand(const std::vector<std::string>& args) {
        clearBuffer();
        printSuccess("Console cleared");
    }

    void Сonsole::echoCommand(const std::vector<std::string>& args) {
        for (const auto& arg : args) {
            print(arg + " ");
        }
        println("");
    }

void Сonsole::setCommand(const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        printError("Usage: set <variable> <value>");
        return;
    }

    const std::string& varName = args[0];
    const std::string& valueStr = args[1];

    auto it = variables.find(varName);
    if (it == variables.end())
    {
        printError("Variable not found: " + varName);
        return;
    }

    auto* var = it->second.get();

    if (auto* dynamicVar = dynamic_cast<DynamicVariable*>(var))
    {
        return;
    }

    bool success = false;
    if (!success) success = trySetValue<int>(var, valueStr, varName);
    if (!success) success = trySetValue<floating>(var, valueStr, varName);
    if (!success) success = trySetValue<bool>(var, valueStr, varName);
    if (!success) success = trySetValue<std::string>(var, valueStr, varName);
    if (!success) success = trySetValue<vec3>(var, valueStr, varName);
    if (success)
    {

    }
    else
    {
        printError("Variable type not supported for set operation");
    }
}

    void Сonsole::getCommand(const std::vector<std::string>& args) {
        bool verbose = false;
        size_t varNameIndex = 0;

        if (args.empty()) {
            printError("Usage: get <variable>");
            return;
        }

        // Проверяем первый аргумент на флаг -v
        if (args[0] == "-v") {
            verbose = true;

            if (args.size() < 2) {
                printError("Usage: get -v <variable>");
                return;
            }

            varNameIndex = 1; // Имя переменной после флага
        } else {
            if (args.size() != 1) {
                printError("Usage: get <variable>");
                return;
            }
            varNameIndex = 0; // Имя переменной - первый аргумент
        }

        const std::string& varName = args[varNameIndex];

        auto it = variables.find(varName);
        if (it == variables.end()) {
            printError("Variable not found: " + varName);
            return;
        }

        auto* var = it->second.get();
        auto descIt = variable_descriptions.find(varName);

        if (verbose) {
            println("Variable: " + varName);
            if (descIt != variable_descriptions.end() && !descIt->second.empty()) {
                println("Description: " + descIt->second);
            }
            println(std::format("Type: {}", Variable::ValueTypeToString(var->getValueType())));
            println("Value: " + var->toString());
        } else {
            println(var->toString());
        }
    }

    void Сonsole::VarsCommand(const std::vector<std::string>& args) {
        if (variables.empty())
        {
            println("No variables registered");
            return;
        }

        println("Registered variables (" + std::to_string(variables.size()) + "):");
        for (const auto& [name, var] : variables) {
            auto descIt = variable_descriptions.find(name);
            std::string desc = (descIt != variable_descriptions.end() && !descIt->second.empty())
                               ? " - " + descIt->second : "";

            println("  " + name + " = " + var->toString() + desc);
        }
    }


    void Сonsole::RegisterCommand(const std::vector<std::string> &args)
    {
        if (int args_count = args.size(); args_count < 2 || args_count > 3)
        {
            println("Usage: register (type) <variable> <value>");
            return;
        }

        if (args[0] == "boolean")
        {
            if (bool value = false; tryParseValue(args[2], value))
            {
                registerStaticVariable(args[1], value);
            }
            else
            {
                println("Invalid boolean value");
            }
        }
        else if (args[0] == "floating")
        {
            if (floating value = 0; tryParseValue(args[2], value))
            {
                registerStaticVariable(args[1], value);
            }
            else
            {
                println("Invalid floating value");
            }
        }
        else if (args[0] == "string")
        {
            if (std::string str; tryParseValue(args[2], str))
            {
                registerStaticVariable(args[1], str);
            }
            else
            {
                println("Invalid string");
            }
        }
        else if (args[0] == "vec3")
        {
            if (vec3 vec{}; tryParseValue(args[2], vec))
            {
                registerStaticVariable(args[1], vec);
            }
            else
            {
                println("Invalid vec3");
            }
        }
        else
        {
            // it's dynamic var,
            // trying to find value type
            printError("Can't register variable: " + args[0]);
        }
    }


    bool Сonsole::parseBoolean(const std::string& str)
    {
        if (str == "true" || str == "1" || str == "yes" || str == "on") return true;
        if (str == "false" || str == "0" || str == "no" || str == "off") return false;
        throw std::runtime_error("Invalid boolean value: " + str);
    }

    bool Сonsole::parseVec3(const std::string& str, vec3& result) {
        std::string processed = str;
        if (processed.front() == '[' && processed.back() == ']')
        {
            processed = processed.substr(1, processed.size() - 2);
        }

        std::stringstream ss(processed);
        char comma;
        ss >> result.v[0] >> comma >> result.v[1] >> comma >> result.v[2];
        return !ss.fail();
    }

    Сonsole::Сonsole()
    {
        registerCommand("help", [this](const auto& args) { helpCommand(args); },
                        "Show help for commands");
        registerCommand("history", [this](const auto& args) { historyCommand(args); },
                        "Show command history");
        registerCommand("clear", [this](const auto& args) { clearCommand(args); },
                        "Clear console buffer");
        registerCommand("echo", [this](const auto& args) { echoCommand(args); },
                        "Echo arguments");
        registerCommand("set", [this](const auto& args) { setCommand(args); },
                        "Set variable value");
        registerCommand("get", [this](const auto& args) { getCommand(args); },
                        "Get variable value");
        registerCommand("vars", [this](const auto& args) { VarsCommand(args); },
                        "List all variables");
        registerCommand("register", [this](const auto& args) { RegisterCommand(args); },
                        "List all variables");
        resetHistoryNavigation();
    }

    void Сonsole::execute(const std::string& command)
    {
        if (command.empty())
            return;

        addToHistory(command);

        auto tokens = tokenize(command);
        if (tokens.empty()) return;

        std::string cmdName = tokens[0];
        std::vector<std::string> args(tokens.begin() + 1, tokens.end());

        auto it = commands.find(cmdName);
        if (it != commands.end())
        {
            try
            {
                it->second(args);
            }
            catch (const std::exception& e)
            {
                printError("Error executing command: " + std::string(e.what()));
            }
            catch (...)
            {
                printError("Unknown error executing command");
            }
        }
        else
        {
            printError("Unknown command: " + cmdName);
        }
    }

    bool Сonsole::registerCommand
    (
        const std::string& name,
        std::function<void(const std::vector<std::string>&)> handler,
        const std::string& description
    ) {
        if (commands.find(name) != commands.end())
            {
            printError("Command already registered: " + name);
            return false;
        }

        commands[name] = std::move(handler);
        command_descriptions[name] = description;
        return true;
    }

    bool Сonsole::registerDynamicVariable
    (
        const std::string& name,
        const DynamicVariable& var,
        const std::string& description
    ) {
        if (variables.find(name) != variables.end())
        {
            printError("Variable already registered: " + name);
            return false;
        }
        variables[name] = std::make_unique<DynamicVariable>(var);
        variable_descriptions[name] = description;
        return true;
    }

    std::string Сonsole::getBuffer() const
    {
        return console_buffer.str();
    }

    void Сonsole::clearBuffer()
    {
        console_buffer.str("");
        console_buffer.clear();
    }

    std::string Сonsole::getPreviousCommand()
    {
        if (command_history.empty()) return "";

        if (history_iterator == command_history.end())
        {
            history_iterator = command_history.begin();
        }
        else {
            ++history_iterator;
            if (history_iterator == command_history.end())
            {
                --history_iterator;
            }
        }

        return *history_iterator;
    }

    std::string Сonsole::getNextCommand()
    {
        if (command_history.empty() || history_iterator == command_history.begin())
        {
            return "";
        }

        --history_iterator;
        return *history_iterator;
    }

    void Сonsole::resetHistoryNavigation()
    {
        history_iterator = command_history.end();
    }

    void Сonsole::print(const std::string& message)
    {
        writeToBuffer(message);
    }

    void Сonsole::printError(const std::string& error)
    {
        println("[ERROR] " + error);
    }

    void Сonsole::printSuccess(const std::string& msg)
    {
        println("[SUCCESS] " + msg);
    }

    void Сonsole::printWarning(const std::string& warning)
    {
        println("[WARNING] " + warning);
    }

    void Сonsole::println(const std::string& message)
    {
        print(message + '\n');
    }

    size_t Сonsole::getCommandCount() const
    {
        return commands.size();
    }

    size_t Сonsole::getVariableCount() const
    {
        return variables.size();
    }

    std::vector<std::string> Сonsole::getAllCommands() const
    {
        std::vector<std::string> result;
        for (const auto& [name, _] : commands)
        {
            result.push_back(name);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<std::string> Сonsole::getAllVariables() const
    {
        std::vector<std::string> result;
        for (const auto& [name, _] : variables)
        {
            result.push_back(name);
        }
        std::sort(result.begin(), result.end());
        return result;
    }


    void Сonsole::setLogCallback(const std::function<void(const std::string&)>& callback)
    {
        log_callback = callback;
    }
    void Сonsole::setWriteToBufferCallback(const std::function<void(const std::string&)>& callback)
    {
        write_to_buf_callback = callback;
    }

}