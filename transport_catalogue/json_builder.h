#pragma once
#include "json.h"


namespace json {

    class Builder final {
        class ItemContext;
        class KeyItemContext;
        class ValueItemContext;
        class DictItemContext;
        class ArrayItemContext;

    public:
        Builder() = default;

        // выбрасывает исключение std::logic_error если на момент вызова объект некорректен
        const Node& Build() const;

        KeyItemContext Key(std::string key);
        Builder& Value(Node::Value value);

        DictItemContext StartDict();
        Builder& EndDict();

        ArrayItemContext StartArray();
        Builder& EndArray();

    private:
        Node root_;
        std::vector<Node*> nodes_stack_;
        // состояние по умолчанию при создании словаря
        bool is_empty_ = true;
        // наличие введенного ключа
        bool has_key_ = false;
        std::string key_;
        // добавляет указатель на новый узел в nodes_stack_ в зависимости от типа value
        void AddRef(const Node& value);
    };

    // ---- Вспомогательные классы для проверки корректности времени компиляции ----

    class Builder::ItemContext {
    public:
        ItemContext(Builder& builder) : builder_{ builder } {}
    protected:
        KeyItemContext Key(std::string key);
        DictItemContext StartDict();
        Builder& EndDict();
        ArrayItemContext StartArray();
        Builder& EndArray();

        Builder& builder_;
    };

    class Builder::DictItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        using ItemContext::Key;
        using ItemContext::EndDict;
    };

    class Builder::KeyItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        DictItemContext Value(Node::Value value);
        using ItemContext::StartDict;
        using ItemContext::StartArray;
    };

    class Builder::ArrayItemContext final : public ItemContext {
    public:
        using ItemContext::ItemContext;
        ArrayItemContext Value(Node::Value value);
        using ItemContext::StartDict;
        using ItemContext::StartArray;
        using ItemContext::EndArray;
    };

} // namespace json

