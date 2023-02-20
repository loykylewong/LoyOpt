/*
 * LoyOpt.h
 *
 *  Created on: Nov 28, 2022
 *      Author: loywong@gmail.com, github.com/loykylewong
 *     License: MIT License, Copyright (c) 2023 Loy Kyle Wong
 */

#ifndef __LOYOPT_H__
#define __LOYOPT_H__

#include <cstdint>
#include <cmath>
#include <limits>
#include <type_traits>
#include <variant>
#include <vector>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

// ==== LoyOption ===
//
// A single .h, easy to use, command line options/args parser, 
// with option status checking, default value, clamping and kinds of value type.
//
// Compile with -std=c++17
// 
// Features:
//   * support bool, string, and numeric options.
//   * support option default value.
//   * support option max, min value and clamping.
//   * support status check for:
//       * option(s) not parsed (unsupported option(s)).
//       * invalid or missing option value (not parsalbe).
// Class:
//   * OptionBase   : base class of Option, a vector<OptionBase *> can be used to
//                    collect all your instances of Option for easy iterating.
//   * Option<T>    : define an option, use for later args parsing, result and status storing.
//   * OptionParser : constructed with argv and args, parsing args by using instances of Option.
// Supported option types:
//   * bool : in the form such as:
//       * -c    : single letter/digit with prefix '-' as it's name.
//       * --str : multiple chars (starts with letter/digit) with prefix "--" as it's name.
//     default value is false, when name exists in args, it will be parsed as true.
//   * string : in the form such as:
//       * -c value    : single letter/digit with prefix '-' as it's name.
//                       flowing a space and a string as it's value.
//       * --str value : multiple chars (starts with letter/digit) with prefix "--" as it's name.
//                       flowing a space and a string as it's value.
//     note:
//       * if space exits in value, can be replacd as "\ ".
//       * if special char exists in value, the value can be wrapper with quotation marks.
//   * numeric : in the form such as:
//       * -c value    : single letter/digit with prefix '-' as it's name.
//                       flowing a space and a numeric literal string as it's value.
//       * --str value : multiple chars (starts with letter/digit) with prefix "--" as it's name.
//                       flowing a space and a numeric literal string as it's value.
//     note:
//       * supported numeric type: int32_t, uint32_t, int64_t, float and double.
// Remark:
//   * all space in args can be one or more ' '.
//   * if an option repeat in args, first one take priority and leave others unparsed.
//   * multiple bool options with single '-' prefix and single char, can be merged,
//     such as "-a -b -c" can be merged as "-abc", the fisrt char must not be a digit.
//   * option name starts without '-' is allowed, but not recommended.
//   * option name "-" and "--" is allowed, but may cause undefined behavior.

// for debug
#define LOYOPTION_VERBOSE true

class OptionParser;

class OptionBase
{
    friend class OptionParser;
protected:
    using Val = variant<bool, string, int32_t, uint32_t, int64_t, float, double>;
public:
    enum Status : uint32_t
    {
        NotParsed     = 0, // has not been tried parsing from args.
        NotFound         , // has been tried parsing from args, but not found.
        ValueInvalid     , // has been tried parsing from args, but followed with invalid value.
        ValueNotFound    , // has been tried parsing from args, but without following value.
        ClampedMax       , // has been parsed from args, and updated value with max since overflow.
        ClampedMin       , // has been parsed from args, and updated value with min since underflow.
        Parsed             // has been parsed from args, and updated value.
    };
    // is option name exist in args?
    inline bool IsNameExist()
    {
        return status != NotParsed && status != NotFound;
    }
    // is option value updated ?
    // (option name exist and option value is in range or clamped into range)
    inline bool IsValueUpdated()
    {
        return status == Parsed || status == ClampedMax || status == ClampedMin;
    }

protected:
    inline static const string sts_str[8]     // human readable status string.
    {
        "Opt Not Parsed"s, "Opt Not Found "s,
        "Value Invalid "s, "Value NotFound"s,
        "Clamped To Max"s, "Clamped To Min"s,
        "Parsed Success"s, ""s
    };
    Status status = NotParsed;                // status of this option.
    string name;                              // name of option, such as "-a", "--str"
    string alt_name;                          // alternative name, such as "-b", "--str2"
    const string *matched_name = &sts_str[7]; // last matched name, name or alt_name
    int base = 10;                            // base of numeric option's value, 2-36, normally 2, 8, 10 and 16.
    Val default_val;                          // default value when there is
    Val value;                                // value equal to default_val before parsed.
    Val min;                                  // minimal allowed value.
    Val max;                                  // maximal allowed value.
public:
    vector<string> HelpLines;  // user define help info, will be output formatted by AppendHelpLinesTo().
protected:
    template<typename U, typename T>
    inline T clamp(U val, T min, T max)
    {
        status = val < min ? ClampedMin : val > max ? ClampedMax : Parsed;
        return (T)(val < min ? min : val > max ? max : val);
    }
    template<typename T>
    void update_numeric_value_n_status(const T &val)
    {
        switch (value.index())
        {
        case 2: value = clamp(val, get<int32_t>(min), get<int32_t>(max));
            break;
        case 3: value = clamp(val, get<uint32_t>(min), get<uint32_t>(max));
            break;
        case 4: value = clamp(val, get<int64_t>(min), get<int64_t>(max));
            break;
        case 5: value = clamp(val, get<float>(min), get<float>(max));
            break;
        case 6: value = clamp(val, get<double>(min), get<double>(max));
            break;
        default: value = val; status = Parsed;
            break;
        }
    }
public:
    virtual ~OptionBase() = default;
    bool NameMatch(const string &name) 
    {
        if(this->name == name)
        {
            this->matched_name = &this->name;
            return true;
        }
        else if(this->alt_name == name)
        {
            this->matched_name = &this->alt_name;
            return true;
        }
        else
        {
            return false;
        }
    }    
    Status GetStatus() const { return status; }
    const string &GetName() const { return name; }
    const string &GetAltName() const { return alt_name; }
    const string &GetLastMatchedName() const { return *matched_name; }
    const string &GetStatusString() const { return sts_str[(uint32_t)status]; }
    int Base() const { return base; }
    virtual string GetValueString(const char *fmt = nullptr) const = 0;
    virtual string GetDefaultValueString(const char *fmt = nullptr) const = 0;
    virtual string GetMinValueString(const char *fmt = nullptr) const = 0;
    virtual string GetMaxValueString(const char *fmt = nullptr) const = 0;
    virtual void AppendHelpLinesTo(stringstream &ss) const = 0;
#if(LOYOPTION_VERBOSE)
    virtual string GetStatusNameAndValueString() const = 0;
#endif
};

// OptionDefine: define an option, will be used by OptionParser.Parse() for parsing args.
template <typename T, typename = enable_if_t< is_same_v<T, bool> || is_same_v<T, string> ||
        is_same_v<T, int32_t> || is_same_v<T, uint32_t> || is_same_v<T, int64_t> ||
        is_same_v<T, float> || is_same_v<T, double> > >
class Option : public OptionBase
{
    friend class OptionParser;
public:
    const T &Value()        const { return get<T>(value); }
    const T &DefaultValue() const { return get<T>(default_val); }
    const T &Min()          const { return get<T>(min); }
    const T &Max()          const { return get<T>(max); }
    void SetValue(T val)          { value = val; }
    virtual ~Option() = default;
    Option() = delete;
    Option(const Option<T> &) = delete;
    Option(Option<T> &&) = delete;
    
    // construct a bool type instance.
    //   * name     : name of the option, such as "-r", "--rst".
    //   * alt_name : alterative name of the option, such as "--reset".
    template<typename U = T, typename = enable_if_t<is_same_v<U, bool>>>
    Option(const string &name, const string &alt_name = "")
    {
        this->name = name;
        this->alt_name = alt_name;
        this->default_val = false;
        this->value = false;
        this->min = numeric_limits<T>::lowest();
        this->max = numeric_limits<T>::max();
    }
    // construct a non-bool type instance.
    //   * default_val : default value, used as value when
    //       * args has not been parsed for the option, or
    //       * option is not found in args, or
    //       * option value is missing or invalid.
    //   * name     : name of the option, such as "-r", "--rst".
    //   * alt_name : alterative name of the option, such as "--reset".
    template<typename U = T, typename = enable_if_t<
            is_same_v<U, string>  || is_same_v<U, int32_t> || is_same_v<U, uint32_t> ||
            is_same_v<U, int64_t> || is_same_v<U, float>   || is_same_v<U, double> >>
    Option(const T &default_val, 
            const string &name, const string &alt_name = "")
    {
        this->name = name;
        this->alt_name = alt_name;
        this->default_val = default_val;
        this->value = default_val;
        this->min = numeric_limits<T>::lowest();
        this->max = numeric_limits<T>::max();
    }
    // construct a integer(no bool) or floating type instance.
    //   * default_val : default value, used as value when
    //       * args has not been parsed for the option, or
    //       * option is not found in args, or
    //       * option value is missing or invalid.
    //   * min      : minimal value allowed, used when parsed value is less then it.
    //   * max      : maximal value allowed, used when parsed value is larger then it.
    //   * name     : name of the option, such as "-r", "--rst".
    //   * alt_name : alterative name of the option, such as "--reset".
    template<typename U = T, typename = enable_if_t<
            is_same_v<U, int32_t> || is_same_v<U, uint32_t> || is_same_v<U, int64_t> ||
            is_same_v<U, float>   || is_same_v<U, double> >>
    Option(T default_val, T min, T max, 
            const string &name, const string &alt_name = "")
    {
        this->name = name;
        this->alt_name = alt_name;
        this->default_val = default_val;
        this->value = default_val;
        this->min = min;
        this->max = max;
    }
    // construct a integer(no bool) or floating type instance.
    //   * default_val : default value, used as value when
    //       * args has not been parsed for the option, or
    //       * option is not found in args, or
    //       * option value is missing or invalid.
    //   * base     : base of the value literal in args, 2~36, commonly 2, 8, 10 and 16.
    //   * name     : name of the option, such as "-r", "--rst".
    //   * alt_name : alterative name of the option, such as "--reset".
    template<typename U = T, typename = enable_if_t<
            is_same_v<U, int32_t> || is_same_v<U, uint32_t> || is_same_v<U, int64_t> ||
            is_same_v<U, float>   || is_same_v<U, double> >>
    Option(T default_val, int base, 
            const string &name, const string &alt_name = "")
    {
        base = base <  2 ?  2 :
               base > 36 ? 36 : base;
        this->name = name;
        this->alt_name = alt_name;
        this->base = base;
        this->default_val = default_val;
        this->value = default_val;
        this->min = numeric_limits<T>::lowest();
        this->max = numeric_limits<T>::max();
    }
    // construct a integer(no bool) or floating type instance.
    //   * default_val : default value, used as value when
    //       * args has not been parsed for the option, or
    //       * option is not found in args, or
    //       * option value is missing or invalid.
    //   * base     : base of the value literal in args, 2~36, commonly 2, 8, 10 and 16.
    //   * min      : minimal value allowed, used when parsed value is less then it.
    //   * max      : maximal value allowed, used when parsed value is larger then it .
    //   * name     : name of the option, such as "-r", "--rst".
    //   * alt_name : alterative name of the option, such as "--reset".
    template<typename U = T, typename = enable_if_t<
            is_same_v<U, int32_t> || is_same_v<U, uint32_t> || is_same_v<U, int64_t> ||
            is_same_v<U, float>   || is_same_v<U, double> >>
    Option(T default_val, int base, T min, T max, 
            const string &name, const string &alt_name = "")
    {
        base = base <  2 ?  2 :
               base > 36 ? 36 : base;
        this->name = name;
        this->alt_name = alt_name;
        this->base = base;
        this->default_val = default_val;
        this->value = default_val;
        this->min = min;
        this->max = max;
    }
private:
    inline string get_val_string(const Val &v, const char *fmt = nullptr) const
    {
        if constexpr(is_same_v<T, bool>)
            return get<bool>(v) ? "True" : "False";
        else if constexpr(is_same_v<T, string>)
            return get<string>(v);
        else if(!fmt)
            return to_string(get<T>(v));
        else
        {
            char *str = new char[256];
            snprintf(str, 255, fmt, get<T>(v));
            string rtn{str};
            delete[] str;
            return rtn;
        }
    }
public:
    // return a string of value, fmt is a printf like format string
    string GetValueString       (const char *fmt = nullptr) const { return get_val_string(value      , fmt); }
    // return a string of default value, fmt is a printf like format string
    string GetDefaultValueString(const char *fmt = nullptr) const { return get_val_string(default_val, fmt); }
    // return a string of min value, fmt is a printf like format string
    string GetMinValueString    (const char *fmt = nullptr) const { return get_val_string(min        , fmt); }
    // return a string of max value, fmt is a printf like format string
    string GetMaxValueString    (const char *fmt = nullptr) const { return get_val_string(max        , fmt); }
#if(LOYOPTION_VERBOSE)
    string GetStatusNameAndValueString() const
    {
        string rtn = "[" + GetStatusString() + "] " + name;
        if(!alt_name.empty())
            rtn += ", " + alt_name;
        if constexpr(is_same_v<T, string>)
            rtn += " = \"" + GetValueString() + "\"";
        else
            rtn += " = " + GetValueString(base == 16 ? "0x%x" : nullptr);
        return rtn;
    }
#endif
    void AppendHelpLinesTo(stringstream &ss) const
    {
        if constexpr(is_same_v<T, bool>)
        {
            ss << "    " << name;
            if(!alt_name.empty())
                ss << ", " << alt_name;
        }
        else
        {
            ss << "    " << name << " <value>";
            if(!alt_name.empty())
                ss << ", " << alt_name << " <value>";
            ss << ", value is ";
            ss << setbase(base) << showbase;
            if constexpr(is_same_v<T, string>)
                ss << "a string, default = \"" << get<string>(default_val) << "\"";
            else if constexpr(is_same_v<T, int32_t>)
            {
                ss << "an integer literal";
                ss << ( base ==  2 ? " in Binary"      :
                        base ==  8 ? " in Octal"       : 
                        base == 10 ? ""                : 
                        base == 16 ? " in Hexadecimal" : " in Base-" + to_string(base));
                ss << ", default = " << get<int32_t>(default_val);
            }
            else if constexpr(is_same_v<T, int64_t>)
            {
                ss << "an integer literal";
                ss << ( base ==  2 ? " in Binary"      :
                        base ==  8 ? " in Octal"       : 
                        base == 10 ? ""                : 
                        base == 16 ? " in Hexadecimal" : " in Base-" + to_string(base));
                ss << ", default = " << get<int64_t>(default_val);
            }
            else if constexpr(is_same_v<T, uint32_t>)
            {
                ss << "an unsigned integer literal";
                ss << ( base ==  2 ? " in Binary"      :
                        base ==  8 ? " in Octal"       : 
                        base == 10 ? ""                : 
                        base == 16 ? " in Hexadecimal" : " in Base-" + to_string(base));
                ss << ", default = " << get<uint32_t>(default_val);
            }
            else if constexpr(is_same_v<T, float>)
                ss << "a floating point literal, default = " << get<float>(default_val);
            else
                ss << "a floating point literal, default = " << get<double>(default_val);
            if(get<T>(min) != numeric_limits<T>::lowest() &&
               get<T>(max) != numeric_limits<T>::max())
            {
                if constexpr(!is_same_v<T, string>)
                    ss << ", range = [" << get<T>(min) << ", " << get<T>(max) << "]";
            }
            ss << setbase(0) << noshowbase;
            ss << '.';
        }
        ss << endl;
        if(HelpLines.size())
            ss << "      : " << HelpLines[0] << endl;
        for(size_t i = 1; i < HelpLines.size(); i++)
            ss << "        " << HelpLines[i] << endl;
    }
};

class OptionParser
{
    using Arg = pair<bool, string>; // the bool: true if the arg string has been dealed;
private:
    vector<Arg> args;
    string exec_name;
    void get_exec_name()
    {
        int i;
        for(i = args[0].second.size() - 1; i >= 0; i--)
            if(args[0].second[i] == '/' || args[0].second[i] == '\\')
                break;
        exec_name = args[0].second.substr(i + 1);
    }
public:
    // construct an instance by using argc and argv from main()
    OptionParser(int argc, const char **argv)
    {
        char c[3] = {'-', ' ', '\0'};
        int arglen = 0;
        for(int i = 0; i < argc; i++)
        {
            arglen = strlen(argv[i]);
            if(argv[i][0] == '-' && 3 <= arglen &&
                    argv[i][1] != '-' && argv[i][1] != '.' &&
                    !(argv[i][1] >= '0' && argv[i][1] <= '9'))
            {   // single '-' started multi chars, starts with non digit 
                for(int j = 1; j < arglen; j++)
                {
                    c[1] = argv[i][j];
                    args.push_back(make_pair(false, string(c)));
                }
            }
            else
            {
                args.push_back(make_pair(false, string(argv[i])));
            }
        }
        args[0].first = true;   // exec name
        get_exec_name();
#if(LOYOPTION_VERBOSE)
        cout << "[debug info] OptionParser.args:" << endl;
        for(const Arg &a : args)
        {
            cout << "[debug info]     " << a.second << endl;
        }
#endif
    }
private:
    void deal_integer(OptionBase &opt, const string &val_str) const
    {
        char *s_end;
        const char *c_str = val_str.c_str();
        int64_t val = strtoll(c_str, &s_end, opt.base);
        if(s_end < c_str + strlen(c_str))
            opt.status = OptionBase::ValueInvalid;
        else
            opt.update_numeric_value_n_status(val);
    }
    void deal_floating(OptionBase &opt, const string &val_str) const
    {
        char *s_end;
        const char *c_str = val_str.c_str();
        double val = strtod(c_str, &s_end);
        if(s_end < c_str + strlen(c_str) || isnan(val))
            opt.status = OptionBase::ValueInvalid;
        else
            opt.update_numeric_value_n_status(val);
    }
public:
    // return number of args (including exe name)
    size_t NumArgs() const { return args.size(); }
    // return ref of the executable name without path
    const string &ExecName() const { return exec_name; }
    // Parse args for the opt
    //   * opt    : an instance of Option to be updated.
    //   * return : status of the opt.
    OptionBase::Status Parse(OptionBase &opt)
    {
        const size_t type = opt.value.index();
        size_t i;
        for(i = 0; i < args.size(); i++)
        {
            Arg &a = args[i];
            if(opt.NameMatch(a.second))
            {
                a.first = true;
                if(0 == type) // constexpr(is_same_v<T, bool>)
                {   // bool (no value)
                    opt.update_numeric_value_n_status(true);
                    break;
                }
                else
                {   // string and numeric
                    if(i + 1 < args.size())
                    {
                        Arg &v = args[i + 1];
                        v.first = true;
                        if(1 == type)
                        {
                            opt.value = v.second;
                            opt.status = OptionBase::Parsed;
                            break;
                        }
                        else if(2 <= type && type <= 4)
                        {
                            deal_integer(opt, v.second);
                            break;
                        }
                        else
                        {
                            deal_floating(opt, v.second);
                            break;
                        }
                    }
                    else
                    {
                        opt.status = OptionBase::ValueNotFound;
                        break;
                    }
                }
            }
        }
        if(i == args.size())
        {
            opt.status = OptionBase::NotFound;
        }
#if(LOYOPTION_VERBOSE)
        cout << "[debug info] OptionParser::Parse() : " << opt.GetStatusNameAndValueString() << endl;
#endif
        return opt.status;
    }
    // FirstUnparedArg returns the 1st unparsed arg.
    // after parsed for all options, this will be
    // the 1st unrecognised option or duplicated option.
    const string &FirstUnparsedArg() const
    {
        static string empty;
        for(const Arg &a : args)
            if(!a.first)
                return a.second;
        return empty;
    }
    // GetAllUnparsedArgs returns all unparsed args.
    // after parsed for all options, these args will be
    // all the unrecognised option or duplicated option.
    vector<string> GetAllUnparsedArgs() const
    {
        vector<string> rtn;
        for(const Arg &a : args)
            if(!a.first)
                rtn.push_back(a.second);
        return rtn;
    }
};

#endif
