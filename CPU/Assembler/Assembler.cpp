#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include <regex>
#include <algorithm>
#include <list>

using namespace std;

/*

i need ptr %

so i can do:

[ax] where ax is pointing to memloc

*/



/*

example:

"jmp .main",
"",
"add:",
"   push bp",
"   mov bp sp",
"   mov ax $5",
"   mov bx $6",
"   add ax bx",
"   mov cx ax",
"   pop bp",
"   ret",
"",
"main:",
"   call .add",
"   out cx"


use $ for constants
use [x] for addresses

label:

jmp .label

*/


vector<string> registers = { "ax", "bx", "cx", "dx", "sp", "bp", "pc", "ir", "a0", "a1", "si", "di" };

vector<string> labels; //  name address
vector<string> vars;   //  name size address

template< template<typename, typename> class Container, typename Separator >
Container<std::string, std::allocator<std::string> > split(const std::string& line, Separator sep) {
    std::size_t pos = 0;
    std::size_t next = 0;
    Container<std::string, std::allocator<std::string> > fields;
    while (next != std::string::npos) {
        next = line.find_first_of(sep, pos);
        std::string field = next == std::string::npos ? line.substr(pos) : line.substr(pos, next - pos);
        fields.push_back(field);
        pos = next + 1;
    }
    return fields;
}

string getIndex(vector<string> v, string K)
{
    auto it = find(v.begin(), v.end(), K);

    // If element was found
    if (it != v.end())
    {

        // calculating the index
        // of K
        int index = (it - v.begin());
        return to_string(index);
    }
    else {
        // If the element is not
        // present in the vector
        return to_string(-1);
    }
}

bool inArray(string value, vector<string> arr) {

    int arrayLength = sizeof(arr) / sizeof(int);

    for (int i = 0; i < arrayLength; i++) {
        if (value == arr[i])
            break; return true;
    }

    return false;
}

bool inVector(string value, vector<string> v) {
    return ( find(v.begin(), v.end(), value) != v.end() );
}

string getAddressFromLabel(string arg0) {
    arg0.erase(remove(arg0.begin(), arg0.end(), *"."), arg0.end());
    vector<string> tokens;

    for (int i = 0; i < labels.size(); i++) {
        tokens = split<vector>(labels[i], " :");
        string name = tokens[0];
        string address = tokens[1];
        if (arg0 == name) {
            return address;
        }
        
    }
}



string RegisterRegister(string instr, string arg0, string arg1) {
    string dst = getIndex(registers, arg0);
    string src = getIndex(registers, arg1);
    cout << "RegisterRegister " << dst << " " << src << endl;
    if(instr == "mov"){}
    return "";
}

string RegisterAddress(string instr, string arg0, string arg1) {
    string dst = getIndex(registers, arg0);
    string src = arg1;
    src.erase(remove(src.begin(), src.end(), *"["), src.end());
    src.erase(remove(src.begin(), src.end(), *"]"), src.end());
    cout << "RegisterAddress " << dst << " " << src << endl;
    return "";
}

string RegisterConstant(string instr, string arg0, string arg1) {
    string dst = getIndex(registers, arg0);
    string src = arg1;
    src.erase(remove(src.begin(), src.end(), *"$"), src.end());
    cout << "RegisterConstant " << dst << " " << src << endl;
    return "";

}

string AddressRegister(string instr, string arg0, string arg1) {
    string dst = arg0;
    string src = getIndex(registers, arg1);
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    cout << "AddressRegister " << dst << " " << src << endl;
    return "";
}

string AddressConstant(string instr, string arg0, string arg1) {
    string dst = arg0;
    string src = arg1;
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    src.erase(remove(src.begin(), src.end(), *"$"), src.end());
    cout << "AddressConstant " << dst << " " << src << endl;
    return "";
}

string PointerRegister(string instr, string arg0, string arg1) {
    string dst = arg0;
    string src = getIndex(registers, arg0);
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    dst = getIndex(registers, dst);
    cout << "PointerRegister " << dst << " " << src << endl;
    return "";

}

string PointerConstant(string instr, string arg0, string arg1) {
    string dst = arg0;
    string src = arg1;
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    src.erase(remove(src.begin(), src.end(), *"$"), src.end());
    dst = getIndex(registers, dst);
    cout << "PointerConstant " << dst << " " << src << endl;
    return "";
}

string RegisterPointer(string instr, string arg0, string arg1) {
    string dst = arg0;
    string src = arg1;
    src.erase(remove(src.begin(), src.end(), *"["), src.end());
    src.erase(remove(src.begin(), src.end(), *"]"), src.end());
    dst = getIndex(registers, arg0);
    src = getIndex(registers, src);
    cout << "RegisterPointer " << dst << " " << src << endl;
    return "";

}



string AddressAddress(string instr, string arg0, string arg1) {
    string dst = arg0;
    string src = arg1;
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    src.erase(remove(src.begin(), src.end(), *"["), src.end());
    src.erase(remove(src.begin(), src.end(), *"]"), src.end());
    cout << "AddressAddress " << dst << " " << src << endl;
    return "";
}

string Register(string instr, string arg0, string arg1) {
    string dst = getIndex(registers, arg0);
    cout << "Register " << dst << endl;
    return "";
}

string Constant(string instr, string arg0, string arg1) {
    string dst = arg0;
    dst.erase(remove(dst.begin(), dst.end(), *"$"), dst.end());
    cout << "Constant " << dst << endl;
    return "";
}

string Address(string instr, string arg0, string arg1) {
    string dst = arg0;
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    cout << "Address " << dst << endl;
    return "";
}

string Pointer(string instr, string arg0, string arg1) {
    string dst = arg0;
    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
    dst = getIndex(registers, dst);
    cout << "Pointer " << dst << endl;
    return "";
}

string NoArgs(string instr, string arg0, string arg1) {
    cout << "NoArgs " << endl;
    return "";
}



vector<string> input = {
    "mov ax bx",
    "mov bx [89]",
    "mov cx $56",
    "mov [77] dx",
    "mov [cx] ax",
    "mov [bx] $34",
    "mov dx [ax]",
    "mov [80] $45"
};




int main(int argc, char** argv)
{
    string instr;
    string arg0;
    string arg1;
    int pc = 0;
    for (int line = 0; line < input.size(); line++) {
        if (input[line][0] == *" ") {
            input[line] = input[line].substr(3);
        }
    }

    /*if (arg0.length() > 0) {
        arg0.erase(remove(arg0.begin(), arg0.end(), *","), arg0.end());
    }*/


    for (int line = 0; line < input.size(); line++) {
        vector<string> tokens;
        tokens = split<vector>(input[line], " :");

        if (input[line].size() > 0) {
            if (input[line].back() == *":") {
                arg0 = input[line];
                arg0.erase(remove(arg0.begin(), arg0.end(), *":"), arg0.end());
                labels.push_back(arg0 + " " + to_string(pc));
                //lines.push_back("0, 0, 0");
            }
            pc += 3;
        }
    }

    vector<string> output;


    for (int line = 0; line < input.size(); line++) {
        vector<string> tokens;
        tokens = split<vector>(input[line], " :");

        vector<string> lines;
        if (input[line].size() > 0) {
            if (input[line].back() == *":") {
                output.push_back("0, 0, 0,");
            }
        }


        if (tokens.size() == 1) {
            instr = tokens[0];
            arg0 = "";
            arg1 = "";
            NoArgs(instr, arg0, arg1);
            continue;
        }
        if (tokens.size() == 2) {
            instr = tokens[0];
            arg0 = tokens[1];
            arg1 = "";

            if (inVector(arg0, registers)) {
                output.push_back(Register(instr, arg0, arg1));
                continue;
            }

            if (arg0[0] == *"[") {
                string dst = arg0;
                dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
                dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
                if (inVector(dst, registers)) {
                    output.push_back(Pointer(instr, arg0, arg1));
                    continue;
                }
                else {
                    output.push_back(Address(instr, arg0, arg1));
                    continue;
                }
            }

            if (arg0[0] == *"$") {
                output.push_back(Constant(instr, arg0, arg1));
                continue;
            }
        }
        if (tokens.size() == 3) {
            instr = tokens[0];
            arg0 = tokens[1];
            arg1 = tokens[2];

            if (inVector(arg0, registers)) {
                if (inVector(arg1, registers)) {
                    output.push_back(RegisterRegister(instr, arg0, arg1));
                    continue; 
                }
                if (arg1[0] == *"[") {
                    string dst = arg1;
                    dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
                    dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
                    if (inVector(dst, registers)) {
                        output.push_back(RegisterPointer(instr, arg0, arg1));
                        continue;
                    }
                    else {
                        output.push_back(RegisterAddress(instr, arg0, arg1));
                        continue;
                    }
                }
                if (arg1[0] == *"$") {
                    output.push_back(RegisterConstant(instr, arg0, arg1));
                    continue;
                }
                if (arg1 == "") {
                    output.push_back(Register(instr, arg0, arg1));
                    continue;
                }
            }

            

            string dst = arg0;
            dst.erase(remove(dst.begin(), dst.end(), *"["), dst.end());
            dst.erase(remove(dst.begin(), dst.end(), *"]"), dst.end());
            if (inVector(dst, registers)) {
                if (inVector(arg0, registers)) {
                    output.push_back(PointerRegister(instr, dst, arg1));
                    continue;
                }
                if (arg1[0] == *"[") {
                    // PoinerAddress(instr, arg0, arg1);
                }
                if (arg0[0] == *"$") {
                    output.push_back(PointerConstant(instr, arg0, arg1));
                    continue;
                }
                if (arg1 == "") {
                    output.push_back(Pointer(instr, arg0, arg1));
                    continue;
                }

            }

            if (arg0[0] == *"[") {
                if (inVector(arg1, registers)) {
                    output.push_back(AddressRegister(instr, arg0, arg1));
                    continue;
                }
                if (arg1[0] == *"[") {
                    output.push_back(AddressAddress(instr, arg0, arg1));
                    continue;
                }
                if (arg1[0] == *"$") {
                    output.push_back(AddressConstant(instr, arg0, arg1));
                    continue;
                }
                if (arg1 == "") {
                    output.push_back(Address(instr, arg0, arg1));
                    continue;
                }
            }

            else {
                string name = instr;
                string type = arg0;
                string value = arg1;

                if (type == "db") {
                    


                    // mov ac .datasegment + current_i 1
                    
                }
                if (type == "dw") {


                    // mov ac .datasegment + current_i 1
                    // mov ac .datasegment + current_i 
                }
            }

        }



        


        


    }


    for (int i = 0; i < output.size(); i++) {
        cout << output[i] << endl;
    }

    for (int i = 0; i < labels.size(); i++) {
        cout << labels[i] << endl;
    }
}