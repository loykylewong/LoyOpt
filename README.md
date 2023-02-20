## LoyOption
A single .h, easy to use, command line options/args parser, 
with option status checking, default value, clamping and kinds of value type.

Please compile with "-std=c++17".

### Features:
  * support bool, string, and numeric options.
  * support option default value.
  * support option max, min value and clamping.
  * support status check for:
      * option(s) not parsed (unsupported option(s)).
      * invalid or missing option value (not parsalbe).

### Class:
  * OptionBase   : base class of Option, a vector<OptionBase *> can be used to
                   collect all your instances of Option for easy iterating.
  * Option<T>    : define an option, use for later args parsing, result and status storing.
  * OptionParser : constructed with argv and args, parsing args by using instances of Option.

### Supported option types:
  * bool : in the form such as:
      * -c    : single letter/digit with prefix '-' as it's name.
      * --str : multiple chars (starts with letter/digit) with prefix "--" as it's name.
    default value is false, when name exists in args, it will be parsed as true.
  * string : in the form such as:
      * -c value    : single letter/digit with prefix '-' as it's name.
                      flowing a space and a string as it's value.
      * --str value : multiple chars (starts with letter/digit) with prefix "--" as it's name.
                      flowing a space and a string as it's value.
                      
    note:
      * if space exits in value, can be replacd as "\ ".
      * if special char exists in value, the value can be wrapper with quotation marks.
  * numeric : in the form such as:
      * -c value    : single letter/digit with prefix '-' as it's name.
                      flowing a space and a numeric literal string as it's value.
      * --str value : multiple chars (starts with letter/digit) with prefix "--" as it's name.
                      flowing a space and a numeric literal string as it's value.
    note:
      * supported numeric type: int32_t, uint32_t, int64_t, float and double.

### Remark:
  * all space in args can be one or more ' '.
  * if an option repeat in args, first one take priority and leave others unparsed.
  * multiple bool options with single '-' prefix and single char, can be merged,
    such as "-a -b -c" can be merged as "-abc", the fisrt char must not be a digit.
  * option name starts without '-' is allowed, but not recommended.
  * option name "-" and "--" is allowed, but may cause undefined behavior.