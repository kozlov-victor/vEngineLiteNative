#pragma once

#include "VArrayList.h"
#include "VHashTable.h"
#include "string"

// =====================================
// util
// =====================================
class JsonHelper {
public:
    static std::string escapeJsonString(const std::string& s) {
        std::string out;
        out.reserve(s.length() + 4);

        for (size_t i = 0; i < s.length(); i++) {
            const char c = s[i];
            switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if ((uint8_t)c < 0x20) {
                    // control chars — пропускаємо або замінюємо
                }
                else {
                    out += c;
                }
            }
        }
        return out;
    }
};

// =====================================
// TOKENIZER
// =====================================
enum VTokenType {
    T_LBRACE, T_RBRACE, T_LBRACKET, T_RBRACKET,
    T_COMMA, T_COLON,
    T_STRING, T_INT, T_FLOAT,
    T_TRUE, T_FALSE, T_NULL,
    T_END, T_ERROR
};

struct Token {
    VTokenType type;
    std::string value;
};

class Tokenizer {
private:
    const char* input;
    int pos = 0;

    char peek() const { return input[pos] ? input[pos] : '\0'; }
    char advance() { return input[pos] ? input[pos++] : '\0'; }

    static bool isWsOrNewLine(const char c) {
        return c == '\n' || c == '\r' || c == ' ';
    }

    static bool isDigit(const char c) {
        return
            c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' ||
            c == '6' || c == '7' || c == '8' || c == '9';
    }

    void skipWS() { while (isWsOrNewLine(peek())) advance(); }

    bool match(const char* kw) {
        int i = 0;
        while (kw[i]) {
            if (input[pos + i] != kw[i]) return false;
            i++;
        }
        pos += i;
        return true;
    }

    std::string parseString() {
        std::string s;
        advance(); // "
        while (true) {
            const char c = advance();
            if (c == '\0') return "";
            if (c == '"') break;
            if (c == '\\') {
                const char e = advance();
                if (e == 'n') s += '\n';
                else if (e == 't') s += '\t';
                else s += e;
            }
            else s += c;
        }
        return s;
    }

    std::string parseNumber() {
        std::string s;
        if (peek() == '-') s += advance();
        while (isdigit(peek())) s += advance();
        if (peek() == '.') {
            s += advance();
            while (isdigit(peek())) s += advance();
        }
        return s;
    }

public:
    explicit Tokenizer(const char* str) : input(str) {}

    void tokenize(VArrayList<Token>& tokens) {
        while (true) {
            skipWS();
            const char c = peek();

            if (c == '\0') { tokens.add({ T_END,"" }); break; }

            if (c == '{') { advance(); tokens.add({ T_LBRACE,"{" }); }
            else if (c == '}') { advance(); tokens.add({ T_RBRACE,"}" }); }
            else if (c == '[') { advance(); tokens.add({ T_LBRACKET,"[" }); }
            else if (c == ']') { advance(); tokens.add({ T_RBRACKET,"]" }); }
            else if (c == ',') { advance(); tokens.add({ T_COMMA,"," }); }
            else if (c == ':') { advance(); tokens.add({ T_COLON,":" }); }
            else if (c == '"') tokens.add({ T_STRING,parseString() });
            else if (isdigit(c) || c == '-') {
                std::string val = parseNumber();
                if (val.find_first_of(".") > -1) {
                    tokens.add({ T_FLOAT,val });
                }
                else {
                    tokens.add({ T_INT,val });
                }
            }
            else if (match("true")) tokens.add({ T_TRUE,"true" });
            else if (match("false")) tokens.add({ T_FALSE,"false" });
            else if (match("null")) tokens.add({ T_NULL,"null" });
            else {
                std::string err{c};
                tokens.add({ T_ERROR,err}); 
            }
        }
    }
};

// =====================================
// JSON VALUE
// =====================================
enum JsonType { J_NULL, J_BOOL, J_INT, J_FLOAT, J_STRING, J_OBJECT, J_ARRAY };
class JsonParser;
class JsonPool;
class JsonObjectBuilder;

class  JsonValue {
private:
    JsonType type;
    bool boolVal;
    double floatVal;
    int intVal;
    std::string strVal;
    VHashTable<JsonValue*> object;
    VArrayList<JsonValue*> array;
    friend JsonParser;
    friend JsonPool;
    friend JsonObjectBuilder;

    void clear() {
        boolVal = false;
        floatVal = 0;
        intVal = 0;
        strVal = "";
        object.clear();
        array.clear();
    }

public:

    JsonValue() : type(J_NULL), boolVal(false), floatVal(0), intVal(0) {

    }

    static JsonValue* JSON_NULL_VALUE() {
        static JsonValue jsonValue;
        jsonValue.type = J_NULL;
        return &jsonValue;
    }

    JsonValue* get(const std::string& key) {
        if (type != J_OBJECT) return this;
        if (object.has(key)) return object.get(key);
        // якщо не знайдено — повертаємо null (this)
        return this;
    }

    JsonValue* get(const int index) {
        if (type != J_ARRAY) return this;
        if (index < 0 || index >= array.size()) return this;
        return array.getAt(index);
    }

    double asDouble(double defaultValue = 0) const {
        if (type == J_FLOAT) return floatVal;
        if (type == J_INT) return intVal;
        return defaultValue;
    }

    int asInt(int defaultValue = 0) const {
        if (type == J_FLOAT) return floatVal;
        if (type == J_INT) return intVal;
        return defaultValue;
    }

    bool asBool(bool defaultValue = false) const {
        if (type == J_BOOL) return boolVal;
        return defaultValue;
    }

    std::string asString(std::string defaultValue = "") const {
        if (type == J_STRING) return strVal;
        return defaultValue;
    }

    bool isNull() const {
        return type == J_NULL;
    }

    bool isArray() const {
        return type == J_ARRAY;
    }

    bool isObject() const {
        return type == J_OBJECT;
    }

    int getArraySize() {
        return array.size();
    }

    std::string toString() {
        switch (type) {
        case J_NULL: return "null";
        case J_BOOL: return boolVal ? "true" : "false";
        case J_FLOAT: return std::to_string(floatVal);
        case J_INT: return std::to_string(intVal);
        case J_STRING: return "\"" + JsonHelper::escapeJsonString(strVal) + "\"";

        case J_OBJECT: {
            std::string s = "{";
            int i = 0;
            object.forEach([&i, &s](const std::string& key, JsonValue* val) {
                if (i > 0)s += ",";
                i++;
                s += "\"" + key + "\":" + val->toString();
                });
            return s + "}";
        }

        case J_ARRAY: {
            std::string s = "[";
            for (int i = 0; i < array.size(); i++) {
                if (i > 0) s += ",";
                s += array.getAt(i)->toString();
            }
            return s + "]";
        }
        default:
            return "";
        }
    }
};

// =====================================
// PARSER
// =====================================
class JsonPool {
private:
    JsonValue* pool;
    int capacity;
    int used = 0;

public:
    JsonPool(JsonValue* buffer, const int cap)
        : pool(buffer), capacity(cap) {}

    JsonValue* alloc() {
        if (used >= capacity) {
            printf("ERROR: JsonPool is empty!!");
            return JsonValue::JSON_NULL_VALUE();
        }
        return &pool[used++];
    }

    void reset() {
        for (auto i = 0; i < capacity; i++) {
            pool[i].clear();
        }
        used = 0;
    }

    int free() const { return capacity - used; }
};

JsonValue defaultBuffer[128];
JsonPool defaultPool(defaultBuffer, 128);


class JsonParser {
private:
    VArrayList<Token>* tokens;
    int pos = 0;
    JsonPool* pool;

    Token& lookNext() const { return tokens->getAt(pos); }
    Token& getNext() { return tokens->getAt(pos++); }

    bool eat(VTokenType t) {
        if (lookNext().type == t) { pos++; return true; }
        return false;
    }

    JsonValue* parseValue() {
        const Token& t = lookNext();

        if (t.type == T_STRING) {
            JsonValue* v = pool->alloc();
            v->type = J_STRING;
            v->strVal = t.value;
            getNext();
            return v;
        }

        if (t.type == T_INT) {
            JsonValue* v = pool->alloc();
            v->type = J_INT;
            v->intVal = std::stoi(t.value);
            getNext();
            return v;
        }

        if (t.type == T_FLOAT) {
            JsonValue* v = pool->alloc();
            v->type = J_FLOAT;
            v->floatVal = std::stod(t.value);
            getNext();
            return v;
        }

        if (t.type == T_TRUE) {
            JsonValue* v = pool->alloc();
            v->type = J_BOOL;
            v->boolVal = true;
            getNext();
            return v;
        }

        if (t.type == T_FALSE) {
            JsonValue* v = pool->alloc();
            v->type = J_BOOL;
            v->boolVal = false;
            getNext();
            return v;
        }

        if (t.type == T_NULL) {
            JsonValue* v = pool->alloc();
            v->type = J_NULL;
            getNext();
            return v;
        }

        if (t.type == T_LBRACE) return parseObject();
        if (t.type == T_LBRACKET) return parseArray();

        return JsonValue::JSON_NULL_VALUE();
    }

    JsonValue* parseObject() {
        JsonValue* obj = pool->alloc();

        obj->type = J_OBJECT;

        getNext(); // {

        while (!eat(T_RBRACE)) {
            if (lookNext().type != T_STRING) return JsonValue::JSON_NULL_VALUE();

            std::string key = getNext().value;

            if (!eat(T_COLON)) return JsonValue::JSON_NULL_VALUE();

            JsonValue* val = parseValue();
            obj->object.put(key, val);

            if (!eat(T_COMMA)) {
                if (lookNext().type != T_RBRACE) {
                    return JsonValue::JSON_NULL_VALUE();
                }
            }
        }

        return obj;
    }

    JsonValue* parseArray() {
        JsonValue* arr = pool->alloc();
        if (!arr) return JsonValue::JSON_NULL_VALUE();

        arr->type = J_ARRAY;

        getNext(); // [

        while (lookNext().type != T_RBRACKET) {
            JsonValue* val = parseValue();
            if (!val) return JsonValue::JSON_NULL_VALUE();

            arr->array.add(val);

            if (!eat(T_COMMA)) break;
        }

        if (!eat(T_RBRACKET)) return JsonValue::JSON_NULL_VALUE();

        return arr;
    }

    explicit JsonParser(VArrayList<Token>* tokens, JsonPool* pool = &defaultPool) : tokens(tokens), pool(pool) {

    }

public:

    static JsonValue* parse(const char* str, JsonPool* pool = &defaultPool) {
        if (pool == &defaultPool) {
            pool->reset();
        }
        Tokenizer t(str);

        VArrayList<Token> tokens;
        t.tokenize(tokens);

        JsonParser p(&tokens, pool);
        return p.parseValue();
    }

    static JsonValue* parse(const std::string& str, JsonPool* pool = &defaultPool) {
        return parse(str.c_str(), pool);
    }

};

// =====================================
// BUILDER
// =====================================
class JsonObjectBuilder {
private:
    JsonValue root;
    JsonPool* pool;
public:
    explicit JsonObjectBuilder(const JsonType type = J_OBJECT, JsonPool* pool = &defaultPool) :pool(pool) {
        root.type = type;
        if (pool == &defaultPool) {
            pool->reset();
        }
    }

    void setInt(const std::string& key, const int v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_INT;
        j->intVal = v;
        target->object.put(key, j);
    }

    void setFloat(const std::string& key, const double v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_FLOAT;
        j->floatVal = v;
        target->object.put(key, j);
    }

    void putInt(const double v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_INT;
        j->intVal = v;
        target->array.add(j);
    }

    void putFloat(const double v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_FLOAT;
        j->floatVal = v;
        target->array.add(j);
    }

    void setBool(const std::string& key, const bool v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_BOOL;
        j->boolVal = v;
        target->object.put(key, j);
    }

    void putBool(const bool v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_BOOL;
        j->boolVal = v;
        target->array.add(j);
    }

    void setString(const std::string& key, const std::string& v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_STRING;
        j->strVal = v;
        target->object.put(key, j);
    }

    void putString(const std::string& v, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_STRING;
        j->strVal = v;
        target->array.add(j);
    }

    void setNull(const std::string& key, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return;
        JsonValue* j = pool->alloc();
        j->type = J_NULL;
        target->object.put(key, j);
    }

    void putNull(JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return;
        JsonValue* j = pool->alloc();
        j->type = J_NULL;
        target->array.add(j);
    }

    JsonValue* putArray(JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return JsonValue::JSON_NULL_VALUE();
        JsonValue* j = pool->alloc();
        j->type = J_ARRAY;
        target->array.add(j);
        return j;
    }

    JsonValue* putObject(JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_ARRAY) return JsonValue::JSON_NULL_VALUE();
        JsonValue* j = pool->alloc();
        j->type = J_OBJECT;
        target->array.add(j);
        return j;
    }

    JsonValue* setObject(const std::string& key, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return JsonValue::JSON_NULL_VALUE();
        JsonValue* j = pool->alloc();
        j->type = J_OBJECT;
        target->object.put(key, j);
        return j;
    }

    JsonValue* setArray(const std::string& key, JsonValue* target = nullptr) {
        if (!target) target = &root;
        if (target->type != J_OBJECT) return JsonValue::JSON_NULL_VALUE();
        JsonValue* j = pool->alloc();
        j->type = J_ARRAY;
        target->object.put(key, j);
        return j;
    }

    JsonValue& getRoot() {
        return root;
    }

    std::string toString() { return root.toString(); }
};
