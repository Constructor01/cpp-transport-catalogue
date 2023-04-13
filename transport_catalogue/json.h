#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {
    class Node;
    // Сохраните объявления Dict и Array без изменения
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    struct PrintContext {
        PrintContext(std::ostream& out, int indent = 0) : out(out), indent(indent) {}
        void Indented() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }
        std::ostream& out;
        int indent = 0;
    };

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node final
        : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
    public:
        using variant::variant;
        using Value = variant;

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        bool AsBool() const;
        double AsDouble() const;

        bool IsNull() const noexcept;
        bool IsBool() const noexcept;
        bool IsInt() const noexcept;
        bool IsDouble() const noexcept;
        bool IsPureDouble() const noexcept;
        bool IsString() const noexcept;
        bool IsArray() const noexcept;
        bool IsMap() const noexcept;

        friend bool operator==(const Node& lhs, const Node& rhs) {
            return static_cast<Value>(lhs) == static_cast<Value>(rhs);
        }
        friend bool operator!=(const Node& lhs, const Node& rhs) {
            return !(lhs == rhs);
        }
    };

    class Document final {
    public:
        explicit Document(Node root);
        const Node& GetRoot() const;

        friend bool operator==(const Document& lhs, const Document& rhs) {
            return lhs.root_ == rhs.root_;
        }
        friend bool operator!=(const Document& lhs, const Document& rhs) {
            return !(lhs == rhs);
        }
    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json

