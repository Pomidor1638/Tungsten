#ifndef MEGAGAME_CMD_H
#define MEGAGAME_CMD_H

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <functional>
#include <array>
#include <sstream>
#include <variant>
#include <memory>
#include <type_traits>
#include <format>
#include <optional>

namespace cmd
{

    using floating = double;

    struct vec3
    {
        floating v[3];

        floating& operator[](size_t index) {
            return v[index];
        }

        const floating& operator[](size_t index) const {
            return v[index];
        }
    };

    template<typename T>
    concept IsAllowedType = std::disjunction_v
    <
        std::is_same<T, int>,
        std::is_same<T, floating>,
        std::is_same<T, bool>,
        std::is_same<T, std::string>,
        std::is_same<T, vec3>
    >;


    class Сonsole
    {
    protected:
        class Variable
        {
        public:
            enum class VariableType
            {
                var_base,
                var_dynamic,
                var_static,
                var_pointer,
                var_ref 
            };
            enum class ValueType
            {
                none,
                integer,
                floating,
                vec3,
                boolean,
                string,
            };

            static constexpr const char* ValueTypeToString(const ValueType& type);

            template <IsAllowedType T>
            static constexpr ValueType getValueType()
            {
                if constexpr (std::is_same_v<T, int>) { return ValueType::integer;  }
                else if constexpr (std::is_same_v<T, floating>) { return ValueType::floating; }
                else if constexpr (std::is_same_v<T, vec3>) { return ValueType::vec3;  }
                else if constexpr (std::is_same_v<T, bool>) { return ValueType::boolean;  }
                else if constexpr (std::is_same_v<T, std::string>) { return ValueType::string; }
                else return ValueType::none;
            }

            virtual ~Variable() = default;
            [[nodiscard]] virtual std::string  toString() const = 0;
            [[nodiscard]] virtual VariableType getType () const = 0;
            [[nodiscard]] virtual ValueType    getValueType() const = 0;
        };
        template <IsAllowedType T>
        class StaticVariable : public Variable
        {
        private:
            T value;
        public:
            StaticVariable() = default;
            StaticVariable(const T& val) : value(val) {}
            StaticVariable(const StaticVariable& o) = default;
            StaticVariable(StaticVariable&& o) = default;

            void setValue(const T& v) { value = v; }
            const T& getValue() const { return value; }

            [[nodiscard]] std::string toString() const override
            {
                if constexpr (std::is_same_v<T, vec3>)
                {
                    return std::format("{{ {:.4f}, {:.4f}, {:.4f} }}",
                        value.v[0], value.v[1], value.v[2]);
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return value ? "true" : "false";
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return value;
                }
                else
                {
                    return std::to_string(value);
                }
            }

            [[nodiscard]] VariableType getType() const override
            {
                return VariableType::var_static;
            }

            [[nodiscard]] ValueType getValueType() const override
            {
                return Variable::getValueType<T>();
            }
        };
        template <IsAllowedType T>
        class VariableRef : public Variable
        {
        private:
            T& ref_value;
        public:
            VariableRef() = delete;
            VariableRef(const VariableRef&) = delete;
            explicit VariableRef(T& ref) noexcept : ref_value(ref) {}
            VariableRef& operator=(const VariableRef&) = delete;
            VariableRef(VariableRef&& o) = delete;

            void setValue(const T& v) { ref_value = v; }
            const T& getValue() const { return ref_value; }

            [[nodiscard]] std::string toString() const override
            {
                if constexpr (std::is_same_v<T, vec3>)
                {
                    return std::format("{{ {:.4f}, {:.4f}, {:.4f} }}",
                        ref_value.v[0], ref_value.v[1], ref_value.v[2]);
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return ref_value ? "true" : "false";
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    return ref_value;
                }
                else
                {
                    return std::to_string(ref_value);
                }
            }

            [[nodiscard]] VariableType getType() const override
            {
                return VariableType::var_ref;
            }

            [[nodiscard]] ValueType getValueType() const override
            {
                return Variable::getValueType<T>();
            }
        };
        class DynamicVariable : public Variable
        {
        private:
            using AllowedType = std::variant
            <
                int,
                floating,
                vec3,
                bool,
                std::string
            >;

            AllowedType value;
            ValueType currentType = ValueType::none;

            void setTypeFromValue();

        public:
            DynamicVariable() : value(int{0}) { setTypeFromValue(); }
            explicit DynamicVariable(const DynamicVariable& o) = default;
            explicit DynamicVariable(DynamicVariable&& o) noexcept = default;

            DynamicVariable& operator=(const DynamicVariable& o) = default;
            DynamicVariable& operator=(DynamicVariable&& o) noexcept = default;

            template <IsAllowedType T>
            explicit DynamicVariable(const T& val) noexcept : value(val) {
                setTypeFromValue();
            }

            template<IsAllowedType T>
            [[nodiscard]] bool holdsType() const
            {
                return std::holds_alternative<T>(value);
            }

            template<IsAllowedType T>
            [[nodiscard]] std::optional<T> getAs() const
            {
                if (holdsType<T>())
                {
                    return std::get<T>(value);
                }
                return std::nullopt;
            }

            template <IsAllowedType T>
            void setValue(const T& val)
            {
                value = val;
                setTypeFromValue();
            }

            [[nodiscard]] const AllowedType& getValue() const { return value; }

            [[nodiscard]] std::string toString() const override
            {
                return std::visit([]<typename T0>(T0&& arg) -> std::string {
                    using T = std::decay_t<T0>;

                    if constexpr (std::is_same_v<T, int>) {
                        return std::to_string(arg);
                    }
                    else if constexpr (std::is_same_v<T, floating>) {
                        return std::format("{:.6f}", arg);
                    }
                    else if constexpr (std::is_same_v<T, bool>) {
                        return arg ? "true" : "false";
                    }
                    else if constexpr (std::is_same_v<T, std::string>) {
                        return arg;
                    }
                    else if constexpr (std::is_same_v<T, vec3>) {
                        return std::format("{{ {:.4f}, {:.4f}, {:.4f} }}",
                            arg.v[0], arg.v[1], arg.v[2]);
                    }
                    else {
                        return "unknown";
                    }
                }, value);
            }

            [[nodiscard]] VariableType getType() const override
            {
                return VariableType::var_dynamic;
            }

            [[nodiscard]] ValueType getValueType() const override
            {
                return currentType;
            }

            [[nodiscard]] std::string getValueTypeName() const
            {
                switch (currentType) {
                    case ValueType::integer: return "int";
                    case ValueType::floating: return "floating";
                    case ValueType::vec3: return "vec3";
                    case ValueType::boolean: return "bool";
                    case ValueType::string: return "string";
                    default: return "none";
                }
            }
        };

    private:

        std::ostringstream console_buffer;

        std::unordered_map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
        std::unordered_map<std::string, std::string> command_descriptions;
        std::unordered_map<std::string, std::unique_ptr<Variable>> variables;
        std::unordered_map<std::string, std::string> variable_descriptions;

        std::list<std::string> command_history;
        std::list<std::string>::const_iterator history_iterator;

        std::function<void(const std::string&)> log_callback = nullptr;
        std::function<void(const std::string&)> write_to_buf_callback = nullptr;

        static const size_t MAX_HISTORY_SIZE = 256;

        void log(const std::string& msg);

        std::vector<std::string> tokenize(const std::string& input);
        void addToHistory(const std::string& command);
        void writeToBuffer(const std::string& message);

        void helpCommand(const std::vector<std::string>& args);
        void historyCommand(const std::vector<std::string>& args);
        void clearCommand(const std::vector<std::string>& args);
        void echoCommand(const std::vector<std::string>& args);
        void setCommand(const std::vector<std::string>& args);
        void getCommand(const std::vector<std::string>& args);
        void VarsCommand(const std::vector<std::string>& args);
        void RegisterCommand(const std::vector<std::string>& args);

        template<IsAllowedType T>
        static bool tryParseValue(const std::string& str, T& value);

        template<IsAllowedType T>
        bool trySetValue(Variable* var, const std::string& valueStr, const std::string& varName);

        static bool parseBoolean(const std::string& str);
        static bool parseVec3(const std::string& str, vec3& result);

    public:
        Сonsole();
        virtual ~Сonsole() = default;

        void execute(const std::string& command);

        bool registerCommand
        (
            const std::string& name,
            std::function<void(const std::vector<std::string>&)> handler,
            const std::string& description = ""
        );

        template<IsAllowedType T>
        bool registerVariable
        (
            const std::string& name,
            T* variable_ptr,
            const std::string& description = ""
        )
        {
            if (variables.find(name) != variables.end())
            {
                printError("Variable already registered: " + name);
                return false;
            }

            if (!variable_ptr)
            {
                printError("Cannot register null pointer variable: " + name);
                return false;
            }

            variables[name] = std::make_unique<VariableRef<T>>(*variable_ptr);
            variable_descriptions[name] = description;
            return true;
        }

        template<IsAllowedType T>
        bool registerStaticVariable
        (
            const std::string& name,
            const T& value,
            const std::string& description = ""
        )
        {
            if (variables.find(name) != variables.end()) {
                printError("Variable already registered: " + name);
                return false;
            }

            variables[name] = std::make_unique<StaticVariable<T>>(value);
            variable_descriptions[name] = description;
            return true;
        }

        bool registerDynamicVariable
        (
            const std::string& name,
            const DynamicVariable& var,
            const std::string& description = ""
        );

        [[nodiscard]] std::string getBuffer() const;
        void clearBuffer();
        std::string getPreviousCommand();
        std::string getNextCommand();
        void resetHistoryNavigation();
        [[nodiscard]] const std::list<std::string>& getHistory() const { return command_history; }

        void print       (const std::string& message);
        void printError  (const std::string& error  );
        void printSuccess(const std::string& msg    );
        void printWarning(const std::string& warning);
        void println     (const std::string& message);

        [[nodiscard]] size_t getCommandCount () const;
        [[nodiscard]] size_t getVariableCount() const;
        [[nodiscard]] std::vector<std::string> getAllCommands() const;
        [[nodiscard]] std::vector<std::string> getAllVariables() const;

        void setLogCallback(const std::function<void(const std::string&)>& callback);
        void setWriteToBufferCallback(const std::function<void(const std::string&)>& callback);

    };



    template<IsAllowedType T>
    bool Сonsole::tryParseValue(const std::string& str, T& value)
    {
        std::stringstream ss(str);
        ss >> value;
        return !ss.fail() && ss.eof();
    }

    template<>
    inline bool Сonsole::tryParseValue<vec3>(const std::string& str, vec3& value)
    {
        return parseVec3(str, value);
    }


    template<IsAllowedType T>
    bool Сonsole::trySetValue(Variable* var, const std::string& valueStr, const std::string& varName)
    {
        if (auto* staticVar = dynamic_cast<StaticVariable<T>*>(var))
        {
            T value;
            if constexpr (std::is_same_v<T, bool>)
            {
                try
                {
                    value = parseBoolean(valueStr);
                }
                catch (...)
                {
                    return false;
                }
            }
            else if constexpr (std::is_same_v<T, vec3>)
            {
                if (!parseVec3(valueStr, value))
                {
                    return false;
                }
            }
            else
            {
                if (!tryParseValue(valueStr, value))
                {
                    return false;
                }
            }

            staticVar->setValue(value);
            return true;
        }
        else if (auto* refVar = dynamic_cast<VariableRef<T>*>(var))
        {
            T value;
            if constexpr (std::is_same_v<T, bool>)
            {
                try
                {
                    value = parseBoolean(valueStr);
                }
                catch (...)
                {
                    return false;
                }
            }
            else if constexpr (std::is_same_v<T, vec3>)
            {
                if (!parseVec3(valueStr, value))
                {
                    return false;
                }
            }
            else
            {
                if (!tryParseValue(valueStr, value))
                {
                    return false;
                }
            }
            refVar->setValue(value);
            return true;
        }
        return false;
    }

}

#endif // MEGAGAME_CMD_H