#include "json.h"

using namespace std;

namespace json {

    namespace {

        //------Доп. функции для парсинга Node-------------

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
            if (int ch = input.peek(); ch == 'e' || ch == 'E') {
                read_char();
                if (ch = input.peek(); ch == '+' || ch == '-') {
                    read_char();
                }
                read_digits();
                is_int = false;
            }

            try {
                if (is_int) {
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
        // Функцию следует использовать после считывания открывающего символа ":
        std::string ParseString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        // ------------------- Функции парсинга Node -------------------------

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            char c;
            while (input >> c) {
                if (c == ']') {
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            // проверяем, что массив заканчивается на ]
            if (c != ']') {
                throw ParsingError("Failed to parse array node");
            }
            return Node(move(result));
        }

        Node LoadDict(istream& input) {
            Dict result;
            char c;
            input >> c;

            // проверяем, если словарь пустой
            if (c == '}') {
                return Node(Dict{});
            }
            else {
                input.putback(c);
            }

            while (input >> c) {
                // считываем ключ
                input.putback(c);
                string key;
                auto first_node = LoadNode(input);
                if (first_node.IsString()) {
                    key = first_node.AsString();
                }
                else {
                    throw ParsingError("Failed to parse dict key");
                }

                // считываем разделитель
                input >> c;
                if (c != ':') {
                    throw ParsingError("Failed to parse dict node");
                }

                // считываем значение и записываем в словарь
                result.insert({ move(key), LoadNode(input) });

                // считываем следующий символ (должен быть либо "}" либо ","
                input >> c;
                if (c == '}') {
                    break;
                }
                else if (c != ',') {
                    throw ParsingError("Failed to parse dict");
                }
            }

            // проверяем, что если поток закончился, то последний символ был }
            if (c != '}') {
                throw ParsingError("Failed to parse dict node");
            }


            return Node(move(result));
        }

        Node LoadString(istream& input) {
            string line = ParseString(input);
            return Node(move(line));
        }

        Node LoadNull(istream& input) {
            std::string res;

            char c;

            for (int i = 0; i < 4; ++i) {
                if (input.get(c)) {
                    res += c;
                }
            }

            if (res != "null") {
                throw ParsingError("Failed to parse null node");
            }

            return Node();
        }

        Node LoadBool(istream& input) {
            std::string res;
            char c;

            //определяю сколько символов считывать
            c = static_cast<char>(input.peek());
            int length = c == 't' ? 4 : 5;

            for (int i = 0; i < length; ++i) {
                if (input.get(c)) {
                    res += c;
                }
            }

            if (res != "true"s && res != "false"s) {
                throw ParsingError("Failed to parse bool node");
            }

            if (res == "true") {
                return Node(true);
            }
            else {
                return Node(false);
            }
        }

        Node LoadNum(istream& input) {
            auto num = LoadNumber(input);
            if (std::holds_alternative<double>(num)) {
                return Node(std::get<double>(num));
            }
            else {
                return Node(std::get<int>(num));
            }
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadString(input);
            }
            else if (c == 'n') {
                input.putback(c);
                return LoadNull(input);
            }
            else if (std::isdigit(c) || c == '-') {
                input.putback(c);
                return LoadNum(input);
            }
            else if (c == 't' || c == 'f') {
                input.putback(c);
                return LoadBool(input);
            }
            else {
                throw ParsingError("Failed to parse document"s);
            }
        }

    }  // namespace

    // ------------ методы проверки на тип значения ---------------------

    bool Node::IsNull() const noexcept {
        return std::holds_alternative<std::nullptr_t>(*this);
    }
    bool Node::IsBool() const noexcept {
        return std::holds_alternative<bool>(*this);
    }
    bool Node::IsInt() const noexcept {
        return std::holds_alternative<int>(*this);
    }
    bool Node::IsDouble() const noexcept {
        return (std::holds_alternative<int>(*this) ||
            std::holds_alternative<double>(*this));
    }
    bool Node::IsPureDouble() const noexcept {
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsString() const noexcept {
        return std::holds_alternative<std::string>(*this);
    }
    bool Node::IsArray() const noexcept {
        return std::holds_alternative<Array>(*this);
    }
    bool Node::IsMap() const noexcept {
        return std::holds_alternative<Dict>(*this);
    }

    // -------- возврат значения определенного типа по ссылке -----------

    const Array& Node::AsArray() const {
        if (IsArray()) {
            return std::get<Array>(*this);
        }
        else {
            throw std::logic_error("Node data is not array"s);
        }
    }

    const Dict& Node::AsMap() const {
        if (IsMap()) {
            return std::get<Dict>(*this);
        }
        else {
            throw std::logic_error("Node data is not map"s);
        }
    }

    bool Node::AsBool() const {
        if (IsBool()) {
            return std::get<bool>(*this);
        }
        else {
            throw std::logic_error("Node data is not bool"s);
        }
    }

    int Node::AsInt() const {
        if (IsInt()) {
            return std::get<int>(*this);
        }
        else {
            throw std::logic_error("Node data is not int"s);
        }
    }

    double Node::AsDouble() const {
        if (IsInt()) {
            return static_cast<double>(std::get<int>(*this));
        }
        else if (IsPureDouble()) {
            return std::get<double>(*this);
        }
        else {
            throw std::logic_error("Node data is not double"s);
        }
    }

    const string& Node::AsString() const {
        if (IsString()) {
            return std::get<std::string>(*this);
        }
        else {
            throw std::logic_error("Node data is not string"s);
        }
    }

    // ------------------------------------------------------------------

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(istream& input) {
        Document result{ LoadNode(input) };
        // проверить что после считывания в потоке не осталось лишних символов
        if (char c; input >> c) {
            throw ParsingError("Failed to parse document"s);
        }
        return result;
    }

    namespace {

        // -------------------------- печать нод ----------------------------

        void PrintNode(const Node& node, PrintContext print_context);

        void PrintNullNode(const Node& node, PrintContext print_context) {
            Node a = node;
            print_context.out << "null";
        }

        void PrintBoolNode(const Node& node, PrintContext print_context) {
            if (node.AsBool()) {
                print_context.out << "true";
            }
            else {
                print_context.out << "false";
            }
        }

        void PrintIntNode(const Node& node, PrintContext print_context) {
            print_context.out << node.AsInt();
        }

        void PrintDoubleNode(const Node& node, PrintContext print_context) {
            print_context.out << node.AsDouble();
        }

        std::string SpecialSimvilForString(const std::string& str) {
            std::string result;
            for (auto c : str) {
                if (c == '\"') {
                    result += "\\\"";
                }
                else if (c == '\r') {
                    result += "\\r";
                }
                else if (c == '\n') {
                    result += "\\n";
                }
                else if (c == '\\') {
                    result += "\\\\";
                }
                else {
                    result += c;
                }
            }
            return result;
        }

        void PrintStringNode(const Node& node, PrintContext print_context) {
            print_context.out << "\"" << SpecialSimvilForString(node.AsString()) << "\"";
        }

        void PrintArrayNode(const Node& node, PrintContext print_context) {
            auto arr = node.AsArray();
            auto size = arr.size();
            if (size != 0) {
                print_context.out << "[";
                PrintNode(arr.at(0), print_context);
                for (int i = 1; i < static_cast<int>(size); ++i) {
                    print_context.out << ", ";
                    PrintNode(arr.at(i), print_context);
                }
                print_context.out << "]";
            }
            else {
                print_context.out << "[]";
            }
        }

        void PrintMapNode(const Node& node, PrintContext print_context) {
            auto map = node.AsMap();
            auto size = map.size();
            if (size != 0) {
                print_context.out << "{" << std::endl;

                PrintContext map_print_context(print_context.out, print_context.indent + 2);
                map_print_context.Indented();
                // вывожу первую пару вне цикла, чтобы не было лишнего переноса строки в начале или в конце
                PrintNode(map.begin()->first, map_print_context);
                map_print_context.out << ": ";
                PrintNode(map.begin()->second, map_print_context);
                for (auto it = std::next(map.begin()); it != map.end(); ++it) {
                    map_print_context.out << "," << std::endl;
                    map_print_context.Indented();
                    PrintNode(it->first, map_print_context);
                    map_print_context.out << ": ";
                    PrintNode(it->second, map_print_context);
                }
                print_context.out << std::endl;
                print_context.Indented();
                print_context.out << "}";
            }
            else {
                print_context.out << "{}";
            }
        }

        void PrintNode(const Node& node, PrintContext print_context) {
            if (node.IsNull()) {
                PrintNullNode(node, print_context);
            }
            else if (node.IsBool()) {
                PrintBoolNode(node, print_context);
            }
            else if (node.IsInt()) {
                PrintIntNode(node, print_context);
            }
            else if (node.IsPureDouble()) {
                PrintDoubleNode(node, print_context);
            }
            else if (node.IsString()) {
                PrintStringNode(node, print_context);
            }
            else if (node.IsArray()) {
                PrintArrayNode(node, print_context);
            }
            else if (node.IsMap()) {
                PrintMapNode(node, print_context);
            }
        }
    }
    void Print(const Document& doc, std::ostream& output) {
        PrintContext print_context(output, 0);
        PrintNode(doc.GetRoot(), print_context);
    }
}  // namespace json