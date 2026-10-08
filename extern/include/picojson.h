#ifndef PICOJSON_H_
#define PICOJSON_H_

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <string>
#include <vector>

namespace picojson {

  enum {
    null_type,
    boolean_type,
    number_type,
    string_type,
    array_type,
    object_type
  };

  class value;
  typedef std::vector<value> array;
  typedef std::map<std::string, value> object;

  class value {
  public:
    union _storage {
      bool boolean_;
      double number_;
      std::string* string_;
      array* array_;
      object* object_;
    };

  protected:
    int type_;
    _storage u_;

  public:
    value() : type_(null_type) {}
    value(int type, bool) : type_(type) {
      switch (type) {
      case boolean_type: u_.boolean_ = false; break;
      case number_type: u_.number_ = 0.0; break;
      case string_type: u_.string_ = new std::string(); break;
      case array_type: u_.array_ = new array(); break;
      case object_type: u_.object_ = new object(); break;
      default: break;
      }
    }
    explicit value(bool b) : type_(boolean_type) { u_.boolean_ = b; }
    explicit value(double n) : type_(number_type) { u_.number_ = n; }
    explicit value(const std::string& s) : type_(string_type) { u_.string_ = new std::string(s); }
    explicit value(const array& a) : type_(array_type) { u_.array_ = new array(a); }
    explicit value(const object& o) : type_(object_type) { u_.object_ = new object(o); }
    explicit value(const char* s) : type_(string_type) { u_.string_ = new std::string(s); }
    explicit value(const char* s, size_t len) : type_(string_type) { u_.string_ = new std::string(s, len); }
    ~value() { clear(); }
    value(const value& x) : type_(null_type) { *this = x; }
    value& operator=(const value& x) {
      if (this != &x) {
        clear();
        type_ = x.type_;
        switch (type_) {
        case string_type: u_.string_ = new std::string(*x.u_.string_); break;
        case array_type: u_.array_ = new array(*x.u_.array_); break;
        case object_type: u_.object_ = new object(*x.u_.object_); break;
        default: u_ = x.u_; break;
        }
      }
      return *this;
    }
    void clear() {
      switch (type_) {
      case string_type: delete u_.string_; break;
      case array_type: delete u_.array_; break;
      case object_type: delete u_.object_; break;
      default: break;
      }
      type_ = null_type;
    }

    template <typename T> bool is() const;
    template <typename T> const T& get() const;
    template <typename T> T& get();

    bool is_null() const { return type_ == null_type; }
    bool is_bool() const { return type_ == boolean_type; }
    bool is_number() const { return type_ == number_type; }
    bool is_string() const { return type_ == string_type; }
    bool is_array() const { return type_ == array_type; }
    bool is_object() const { return type_ == object_type; }

    bool get_bool() const { return u_.boolean_; }
    double get_number() const { return u_.number_; }
    int get_int() const { return static_cast<int>(u_.number_); }
    unsigned int get_uint() const { return static_cast<unsigned int>(u_.number_); }
    const std::string& get_string() const { return *u_.string_; }
    const array& get_array() const { return *u_.array_; }
    const object& get_object() const { return *u_.object_; }

    const value& get(size_t idx) const {
      static value null_val;
      return (type_ == array_type && idx < u_.array_->size()) ? (*u_.array_)[idx] : null_val;
    }
    const value& get(const std::string& key) const {
      static value null_val;
      if (type_ != object_type) return null_val;
      object::const_iterator i = u_.object_->find(key);
      return i != u_.object_->end() ? i->second : null_val;
    }
    bool contains(const std::string& key) const {
      return (type_ == object_type) && (u_.object_->find(key) != u_.object_->end());
    }
    bool contains(size_t idx) const {
      return (type_ == array_type) && (idx < u_.array_->size());
    }
  };

  template <> inline bool value::is<bool>() const { return type_ == boolean_type; }
  template <> inline bool value::is<double>() const { return type_ == number_type; }
  template <> inline bool value::is<int>() const { return type_ == number_type; }
  template <> inline bool value::is<unsigned int>() const { return type_ == number_type; }
  template <> inline bool value::is<std::string>() const { return type_ == string_type; }
  template <> inline bool value::is<array>() const { return type_ == array_type; }
  template <> inline bool value::is<object>() const { return type_ == object_type; }

  template <> inline const bool& value::get<bool>() const { return u_.boolean_; }
  template <> inline const double& value::get<double>() const { return u_.number_; }
  template <> inline const std::string& value::get<std::string>() const { return *u_.string_; }
  template <> inline const array& value::get<array>() const { return *u_.array_; }
  template <> inline const object& value::get<object>() const { return *u_.object_; }

  template <> inline bool& value::get<bool>() { return u_.boolean_; }
  template <> inline double& value::get<double>() { return u_.number_; }
  template <> inline std::string& value::get<std::string>() { return *u_.string_; }
  template <> inline array& value::get<array>() { return *u_.array_; }
  template <> inline object& value::get<object>() { return *u_.object_; }

  template <typename Iter> class input {
  protected:
    Iter cur_, end_;
    int line_;
  public:
    input(const Iter& first, const Iter& last) : cur_(first), end_(last), line_(1) {}
    int getc() {
      if (cur_ == end_) return -1;
      int ch = *cur_++ & 0xff;
      if (ch == '\n') line_++;
      return ch;
    }
    void ungetc() {
      --cur_;
      if (*cur_ == '\n') --line_;
    }
    void skip_ws() {
      while (cur_ != end_) {
        int ch = *cur_ & 0xff;
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
          if (ch == '\n') line_++;
          ++cur_;
        } else {
          break;
        }
      }
    }
  };

  template <typename Iter> inline std::string parse(value& out, input<Iter>& in) {
    in.skip_ws();
    int ch = in.getc();
    switch (ch) {
    case 't': {
      if (in.getc() == 'r' && in.getc() == 'u' && in.getc() == 'e') {
        out = value(true);
        return std::string();
      }
      return "syntax error";
    }
    case 'f': {
      if (in.getc() == 'a' && in.getc() == 'l' && in.getc() == 's' && in.getc() == 'e') {
        out = value(false);
        return std::string();
      }
      return "syntax error";
    }
    case 'n': {
      if (in.getc() == 'u' && in.getc() == 'l' && in.getc() == 'l') {
        out = value();
        return std::string();
      }
      return "syntax error";
    }
    case '"': {
      std::string s;
      while (1) {
        ch = in.getc();
        if (ch < 0) return "unexpected end of input in string";
        if (ch == '"') {
          out = value(s);
          return std::string();
        }
        if (ch == '\\') {
          ch = in.getc();
          if (ch < 0) return "unexpected end of input in escape sequence";
          switch (ch) {
          case '"': s.push_back('"'); break;
          case '\\': s.push_back('\\'); break;
          case '/': s.push_back('/'); break;
          case 'b': s.push_back('\b'); break;
          case 'f': s.push_back('\f'); break;
          case 'n': s.push_back('\n'); break;
          case 'r': s.push_back('\r'); break;
          case 't': s.push_back('\t'); break;
          default: s.push_back(static_cast<char>(ch)); break;
          }
        } else {
          s.push_back(static_cast<char>(ch));
        }
      }
    }
    case '[': {
      out = value(array_type, false);
      array& a = out.get<array>();
      in.skip_ws();
      if ((ch = in.getc()) == ']') return std::string();
      in.ungetc();
      while (1) {
        value v;
        std::string err = parse(v, in);
        if (!err.empty()) return err;
        a.push_back(v);
        in.skip_ws();
        ch = in.getc();
        if (ch == ']') return std::string();
        if (ch != ',') return "syntax error in array";
      }
    }
    case '{': {
      out = value(object_type, false);
      object& o = out.get<object>();
      in.skip_ws();
      if ((ch = in.getc()) == '}') return std::string();
      in.ungetc();
      while (1) {
        in.skip_ws();
        if (in.getc() != '"') return "syntax error: key must be string";
        std::string k;
        while (1) {
          ch = in.getc();
          if (ch < 0) return "unexpected end of input in key";
          if (ch == '"') break;
          if (ch == '\\') {
            ch = in.getc();
            if (ch < 0) return "unexpected end of input in key escape";
            k.push_back(static_cast<char>(ch));
          } else {
            k.push_back(static_cast<char>(ch));
          }
        }
        in.skip_ws();
        if (in.getc() != ':') return "syntax error: missing ':'";
        value v;
        std::string err = parse(v, in);
        if (!err.empty()) return err;
        o[k] = v;
        in.skip_ws();
        ch = in.getc();
        if (ch == '}') return std::string();
        if (ch != ',') return "syntax error in object";
      }
    }
    default:
      if (ch == '-' || ('0' <= ch && ch <= '9')) {
        std::string num_str;
        num_str.push_back(static_cast<char>(ch));
        while (1) {
          ch = in.getc();
          if (('0' <= ch && ch <= '9') || ch == '.' || ch == 'e' || ch == 'E' || ch == '+' || ch == '-') {
            num_str.push_back(static_cast<char>(ch));
          } else {
            if (ch >= 0) in.ungetc();
            break;
          }
        }
        char* endp;
        double val = strtod(num_str.c_str(), &endp);
        out = value(val);
        return std::string();
      }
      return "syntax error";
    }
  }

  inline std::string parse(value& out, const std::string& str) {
    input<std::string::const_iterator> in(str.begin(), str.end());
    return parse(out, in);
  }

  inline std::string parse(value& out, const char* first, const char* last) {
    input<const char*> in(first, last);
    return parse(out, in);
  }

  inline std::string parse(value& out, const char* str, size_t len) {
    input<const char*> in(str, str + len);
    return parse(out, in);
  }
}

#endif
