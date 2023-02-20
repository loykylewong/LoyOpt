#include "LoyOpt.h"

using namespace std;

// define of Option instances
Option<bool>     opt_help    ("-?", "--help");                            // bool option "-?" with alt_name "--help"
Option<bool>     opt_bool_a  ("-a");                                      // bool option "-a"
Option<bool>     opt_bool_b  ("-b", "--bool_b");                          // bool option "-b" with alt_name "--bool_b"
Option<string>   opt_string_c("./path/file_c", "-c");                     // string option "-c"
Option<string>   opt_string_d("a string for d", "-d", "--string_d");      // string option "-d" with alt_name "--string_d"
Option<int32_t>  opt_int32_e (0, "-e");                                   // int32 option "-e"
Option<int32_t>  opt_int32_f (0, "--int32_f");                            // int32 option "--int32_f"
Option<int32_t>  opt_int32_g (50, 0, 100,"-g", "--int32_g");              // int32 option "-g" with alt_name "--int32_g" and range [0,100]
Option<int32_t>  opt_int32_h (0, 16, "-h", "--int32_h");                  // int32 option "-h" with alt_name "--int32_h" and parse value as hex
Option<int32_t>  opt_int32_i (0, 16, 0, 0xffff, "--int32_i");             // int32 option "--int32_i" and parse value as hex, range [0, 0xffff]
Option<uint32_t> opt_uint32_j(150, 100, 200, "-j", "--uint32_j");         // uint32 option "-j" with alt_name "--uint32_j" and range [100, 200]
Option<int64_t>  opt_int64_k (0, -10000000000, 10000000000, "--int64_k"); // int64 option "--int64_k" with range [-1e10, 1e10]
Option<float>    opt_float_l (0.0f, "-l", "--float_l");                   // float option "-l" with alt_name "--float_l"
Option<float>    opt_float_m (0.0f, -10.0f, 100.0f, "--float_m");         // float option "--float_m", with range [-10, 100]
Option<double>   opt_double_n(0.0f, "-n", "--double_n");                  // double option "--double_n"
// a vector collects all option insts
vector<OptionBase*> options 
{
    &opt_help,
    &opt_bool_a  , &opt_bool_b  ,
    &opt_string_c, &opt_string_d,
    &opt_int32_e , &opt_int32_f ,
    &opt_int32_g , &opt_int32_h ,
    &opt_int32_i , &opt_uint32_j,
    &opt_int64_k , &opt_float_l ,
    &opt_float_m , &opt_double_n
};
// if needed, add help info lines to each option.
void init_opt_helps()
{
    opt_help    .HelpLines.push_back("Print this help info.");
    opt_bool_a  .HelpLines.push_back("Test option a, option value will be true if exists in args.");
    opt_bool_b  .HelpLines.push_back("Test option b, help info line 1,");
    opt_bool_b  .HelpLines.push_back("    help info line 2.");
    opt_bool_b  .HelpLines.push_back("    help info line 3.");
    opt_string_c.HelpLines.push_back("Test option c, string option can be used for paths.");
    opt_string_c.HelpLines.push_back("e.g.: -c ~/abc/def.txt, -c \"abc def/g h i.txt\", -c abc\\ def/g\\ h\\ i.txt.");
    opt_string_d.HelpLines.push_back("Test option d, or can be used for special foramtted data.");
    opt_string_d.HelpLines.push_back("e.g.: AC2022-12-01, and it's user's responsibility to parse the value string.");
    opt_int32_e .HelpLines.push_back("Test option e, an integer option.");
    opt_int32_f .HelpLines.push_back("Test option f, an integer option.");
    opt_int32_g .HelpLines.push_back("Test option g, an integer option.");
    opt_int32_h .HelpLines.push_back("Test option h, an integer option.");
    opt_int32_i .HelpLines.push_back("Test option i, value string must be in hex,");
    opt_int32_i .HelpLines.push_back("    can be with or without prefix\"0x\",");
    opt_int32_i .HelpLines.push_back("    can be upper or lower case.");
    opt_uint32_j.HelpLines.push_back("Test option j, an unsigned integer.");
    opt_int64_k .HelpLines.push_back("Test option k, an 64bit integer option.");
    opt_float_l .HelpLines.push_back("Test option l, an float option.");
    opt_float_m .HelpLines.push_back("Test option m, an float option.");
    opt_double_n.HelpLines.push_back("Test option n, an double option.");
}

int main(int argc, const char *argv[])
{
    cout << "Hello from LoyOpt Test." << endl;

    cout << "Defining a OptionParser," << endl;
    cout << "with LOYOPTION_VERBOSE = true, arg list will be shown below." << endl;
    OptionParser op(argc, argv);

    cout << endl << "Parsing for all Options," << endl;
    cout << "with LOYOPTION_VERBOSE = true, status and value of each option will be shown below." << endl;
    for(OptionBase *o : options)
    {
        op.Parse(*o);
        switch (o->GetStatus())
        {
        case OptionBase::ClampedMin :
        case OptionBase::ClampedMax : 
            cout << "    option value for \"" << o->GetLastMatchedName() << "\" is out of range, has been clampped to " << o->GetValueString() << "." << endl;
            break;
        case OptionBase::ValueInvalid :
            cout << "    option value for \"" << o->GetLastMatchedName() << "\" is invalid." << endl;
            break;
        // case ... other exception promt if you need.
        default:
            break;
        }
    }

    cout << endl << "See if any arg has not been parsed." << endl;
    const string &first_uparsed_arg = op.FirstUnparsedArg();
    if(!first_uparsed_arg.empty())
        cout << "Unrecognised option \"" << first_uparsed_arg << "\" found, please chek your command line." << endl;
    vector<string> unparsed_args = op.GetAllUnparsedArgs();
    if(!unparsed_args.empty())
    {
        cout << "There is/are " << unparsed_args.size() << " unparsed args." << endl;
    }
    for(const string &s : unparsed_args)
    {
        cout << "     " << s << endl;
    }

    cout << endl << "Now we can use/check some option." << endl;
    if(opt_help.Value())
    {
        stringstream ss;
        ss << "\"" << opt_help.GetLastMatchedName() << "\" found, show help infos:" << endl;
        ss << op.ExecName() << " build in " << __DATE__ << endl;
        ss << "Usage: " << op.ExecName() << " [option1] [option2] ..." << endl;
        ss << "options:" << endl;
        init_opt_helps();
        for(const OptionBase *opt : options)
            opt->AppendHelpLinesTo(ss);
        cout << ss.str();
    }
    if(opt_bool_a.Value())
    {
        cout << endl << opt_bool_a.GetName() << " exists, do something..." << endl;
    }
    if(opt_float_l.GetStatus() == OptionBase::Parsed)
    {
        cout << endl << "square of float_l = " << opt_float_l.Value() * opt_float_l.Value() << endl;
    }

    return 0;
}

// ====================================================================================
// -------- a typical test case --------
// ./test_loyopt -? -ab -c "~/Documents/Work Files/foo.txt" --string_d_typo --int32_f 123456 -g 1000 -h 0xa5A5 --int32_i 0x5x5x -j -10 --float_l 3.1416 opps -n
// -------- output --------
// Hello from LoyOpt Test.
// Defining a OptionParser,
// with LOYOPTION_VERBOSE = true, arg list will be shown below.
// [debug info] OptionParser.args:
// [debug info]     ./test_loyopt
// [debug info]     -?
// [debug info]     -a
// [debug info]     -b
// [debug info]     -c
// [debug info]     ~/Documents/Work Files/foo.txt
// [debug info]     --string_d_typo
// [debug info]     --int32_f
// [debug info]     123456
// [debug info]     -g
// [debug info]     1000
// [debug info]     -h
// [debug info]     0xa5A5
// [debug info]     --int32_i
// [debug info]     0x5x5x
// [debug info]     -j
// [debug info]     -10
// [debug info]     --float_l
// [debug info]     3.1416
// [debug info]     opps
// [debug info]     -n

// Parsing for all Options,
// with LOYOPTION_VERBOSE = true, status and value of each option will be shown below.
// [debug info] OptionParser::Parse() : [Parsed Success] -?, --help = True
// [debug info] OptionParser::Parse() : [Parsed Success] -a = True
// [debug info] OptionParser::Parse() : [Parsed Success] -b, --bool_b = True
// [debug info] OptionParser::Parse() : [Parsed Success] -c = "~/Documents/Work Files/foo.txt"
// [debug info] OptionParser::Parse() : [Opt Not Found ] -d, --string_d = "a string for d"
// [debug info] OptionParser::Parse() : [Opt Not Found ] -e = 0
// [debug info] OptionParser::Parse() : [Parsed Success] --int32_f = 123456
// [debug info] OptionParser::Parse() : [Clamped To Max] -g, --int32_g = 100
//     option value for "-g" is out of range, has been clampped to 100.
// [debug info] OptionParser::Parse() : [Parsed Success] -h, --int32_h = 0xa5a5
// [debug info] OptionParser::Parse() : [Value Invalid ] --int32_i = 0x0
//     option value for "--int32_i" is invalid.
// [debug info] OptionParser::Parse() : [Clamped To Min] -j, --uint32_j = 100
//     option value for "-j" is out of range, has been clampped to 100.
// [debug info] OptionParser::Parse() : [Opt Not Found ] --int64_k = 0
// [debug info] OptionParser::Parse() : [Parsed Success] -l, --float_l = 3.141600
// [debug info] OptionParser::Parse() : [Opt Not Found ] --float_m = 0.000000
// [debug info] OptionParser::Parse() : [Value NotFound] -n, --double_n = 0.000000

// See if any arg has not been parsed.
// Unrecognised option "--string_d_typo" found, please chek your command line.
// There is/are 2 unparsed args.
//      --string_d_typo
//      opps

// Now we can use/check some option.
// "-?" found, show help infos:
// test_loyopt build in Dec  7 2022
// Usage: test_loyopt [option1] [option2] ...
// options:
//     -?, --help
//       : Print this help info.
//     -a
//       : Test option a, option value will be true if exists in args.
//     -b, --bool_b
//       : Test option b, help info line 1,
//             help info line 2.
//             help info line 3.
//     -c <value>, value is a string, default = "./path/file_c".
//       : Test option c, string option can be used for paths.
//         e.g.: -c ~/abc/def.txt, -c "abc def/g h i.txt", -c abc\ def/g\ h\ i.txt.
//     -d <value>, --string_d <value>, value is a string, default = "a string for d".
//       : Test option d, or can be used for special foramtted data.
//         e.g.: AC2022-12-01, and it's user's responsibility to parse the value string.
//     -e <value>, value is an integer literal, default = 0.
//       : Test option e, an integer option.
//     --int32_f <value>, value is an integer literal, default = 0.
//       : Test option f, an integer option.
//     -g <value>, --int32_g <value>, value is an integer literal, default = 50, range = [0, 100].
//       : Test option g, an integer option.
//     -h <value>, --int32_h <value>, value is an integer literal in Hexadecimal, default = 0.
//       : Test option h, an integer option.
//     --int32_i <value>, value is an integer literal in Hexadecimal, default = 0, range = [0, 0xffff].
//       : Test option i, value string must be in hex,
//             can be with or without prefix"0x",
//             can be upper or lower case.
//     -j <value>, --uint32_j <value>, value is an unsigned integer literal, default = 150, range = [100, 200].
//       : Test option j, an unsigned integer.
//     --int64_k <value>, value is an integer literal, default = 0, range = [-10000000000, 10000000000].
//       : Test option k, an 64bit integer option.
//     -l <value>, --float_l <value>, value is a floating point literal, default = 0.
//       : Test option l, an float option.
//     --float_m <value>, value is a floating point literal, default = 0, range = [-10, 100].
//       : Test option m, an float option.
//     -n <value>, --double_n <value>, value is a floating point literal, default = 0.
//       : Test option n, an double option.

// -a exists, do something...

// square of float_l = 9.86965
// ====================================================================================
