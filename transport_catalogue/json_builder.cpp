#include "json_builder.h"


namespace json {

    using namespace std;

    const Node& Builder::Build() const {
        // Если строитель пустой или есть незакрытые контейнеры
        if (is_empty_ || !nodes_stack_.empty()) {
            throw std::logic_error("Incorrect Builder");
        }
        return root_;
    }

    Builder::KeyItemContext Builder::Key(std::string key) {
        // если внутри словаря и ключ ещё не добавлен
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && !has_key_) {
            has_key_ = true;
            key_ = std::move(key);
            return KeyItemContext(*this);
        }
        throw std::logic_error("Incorrect Key");
    }

    Builder& Builder::Value(Node::Value value) {

        Node new_node = std::visit([](auto val) {return Node(val);}, value);

        // Если класс еще пустой
        if (is_empty_) {
            root_ = new_node;
            is_empty_ = false;
            return *this;
        }

        // Если внутри словаря и ключ уже добавлен
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && has_key_) {
            const_cast<Dict&>(nodes_stack_.back()->AsMap()).insert({ key_, new_node });
            has_key_ = false;
            return *this;
        }

        // Если внутри массива
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(new_node);
            return *this;
        }

        throw std::logic_error("Incorrect Value");
    }

    Builder::DictItemContext Builder::StartDict() {
        Value(Dict{});
        AddRef(Node(Dict{}));
        return DictItemContext(*this);
    }

    Builder& Builder::EndDict() {
        // если внутри словаря и нет незакрытого ключа
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsMap() && !has_key_) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw std::logic_error("Incorrect EndDict");
    }

    Builder::ArrayItemContext Builder::StartArray() {
        Value(Array{});
        AddRef(Node(Array{}));
        return ArrayItemContext(*this);
    }

    Builder& Builder::EndArray() {
        // если внутри массива
        if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
            return *this;
        }
        throw std::logic_error("Incorrect EndArray");
    }

    void Builder::AddRef(const Node& value) {
        // если value - Array или Map, нужно добавить указатель на новую структуру в стек nodes_stack_
        if (value.IsArray() || value.IsMap()) {
            // если вне структуры
            if (nodes_stack_.empty()) {
                nodes_stack_.push_back(&root_);
                return;
            }
            // если массив
            if (nodes_stack_.back()->IsArray()) {
                auto p = &nodes_stack_.back()->AsArray().back();
                nodes_stack_.push_back(const_cast<Node*>(p));
                return;
            }
            // если словарь
            if (nodes_stack_.back()->IsMap()) {
                auto p = &nodes_stack_.back()->AsMap().at(key_);
                nodes_stack_.push_back(const_cast<Node*>(p));
                return;
            }
        }
    }

    //Реализация дополнительных классов
    Builder::DictItemContext Builder::KeyItemContext::Value(Node::Value value) {
        builder_.Value(std::move(value));
        return DictItemContext{ builder_ };
    }

    Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node::Value value) {
        builder_.Value(std::move(value));
        return ArrayItemContext{ builder_ };
    }

    Builder::KeyItemContext Builder::ItemContext::Key(std::string key) {
        return builder_.Key(std::move(key));
    }

    Builder::DictItemContext Builder::ItemContext::StartDict() {
        return builder_.StartDict();
    }

    Builder& Builder::ItemContext::EndDict() {
        return builder_.EndDict();
    }

    Builder::ArrayItemContext Builder::ItemContext::StartArray() {
        return builder_.StartArray();
    }

    Builder& Builder::ItemContext::EndArray() {
        return builder_.EndArray();
    }

}