#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <bitset>
#include <iomanip>
using namespace std;
/*

16 bit ram + registers

op:
    
    instr, arg0, arg1
 

*/

//template< typename T >
//std::string int_to_hex(T i)
//{
//    std::stringstream stream;
//    stream << "0x"
//        << std::setfill('0') << std::setw(sizeof(T) * 2)
//        << std::hex << i;
//    return stream.str();
//}

bool inArray(int value, int* arr) {

    int arrayLength = sizeof(*arr) / sizeof(int);

    for (int i = 0; i < arrayLength; i++) {
        if (value == arr[i])
            break; return true;
    }

    return false;
}

string IntHexBinary(int n) {
    std::string binary = std::bitset<32>(n).to_string(); //to binary
    return binary;
}

unsigned int BinaryInt(string binary) {
    unsigned int decimal = bitset<32>(binary).to_ulong();
    return decimal;
}

float IEEE754_to_float(int number) {
    

    bool negativ = !!(number & 0x80000000);
    int exponent =   (number & 0x7f800000) >> 23;
    int sign = negativ ? -1 : 1;

    string Binary = IntHexBinary(number);
    string newBinary = "";

    cout << Binary << endl;
    exponent -= 127;
    int power = -1;
    float total = 0.0;

    for (int i = 0; i < 23; i++) {
        int c = Binary[i + 9] - '0';
        total += (float)c * (float)pow(2.0, power);
        power--;
    }
    total += 1.0;
    float value = sign * (float)pow(2.0, exponent) * total;

    return value;
}

unsigned int float_to_IEEE754(float value) {
    union 
    {
        float input;
        int output;
    }data;

    data.input = value;

    bitset<sizeof(float)* CHAR_BIT> bits(data.output);
    
    string res = bits.to_string<char,
        char_traits<char>,
        allocator<char> >();
    
    return BinaryInt(res);

}


unsigned int fadd(unsigned int a, unsigned int b) {
    if (a == 0) {
        return b;
    }
    if (b == 0) {
        return a;
    }

    //Extracting information from a and b
    unsigned int a_sign = (a & 0x80000000) >> 31;
    unsigned int a_enc_exp = (a & 0x7f800000) >> 23;
    unsigned int a_mantissa = (a & 0x7fffff);


    unsigned int b_sign = (b & 0x80000000) >> 31;
    unsigned int b_enc_exp = (b & 0x7f800000) >> 23;
    unsigned int b_mantissa = (b & 0x7fffff);


    unsigned int a_significand = (a_enc_exp >= 1) ? (a_mantissa | (1 << 23)) : a_mantissa;
    unsigned int b_significand = (b_enc_exp >= 1) ? (b_mantissa | (1 << 23)) : b_mantissa;


    //Initially shifting a and b 7 bits left to increase precison for rounding
    unsigned int a_shift_significand = (a_significand << 7);
    unsigned int b_shift_significand = (b_significand << 7);

    //Taking care of denormal numbers
    unsigned int a_exp = ((a_enc_exp == 0) ? 1 : a_enc_exp);
    unsigned int b_exp = ((b_enc_exp == 0) ? 1 : b_enc_exp);
    unsigned int ans_exp;
    unsigned int ans_significand;
    unsigned int ans_sign;
    bool ans_denormal = false;

    /* Special Cases */

    //Case when a is NaN
    if (a_exp == 255 && a_mantissa != 0) {
        return 0x7fffffff;
    }

    //Case when b is NaN
    if (b_exp == 255 && b_mantissa != 0) {
        return 0x7fffffff;
    }

    //Case when Infinity - Infinity
    if (a_exp == 255 && a_mantissa == 0 && b_exp == 255 && b_mantissa == 0 && a_sign != b_sign) {
        return 0x7fffffff;
    }

    //Case when a is Infinity
    if (a_exp == 255 && a_mantissa == 0) {
        return a;
    }

    //Case when b is Infinty
    if (b_exp == 255 && b_mantissa == 0) {
        return b;
    }

    /* Making Exponent Same */


    if (a_exp >= b_exp) {
        unsigned int shift = a_exp - b_exp;

        b_shift_significand = (b_shift_significand >> ((shift > 31) ? 31 : shift));
        ans_exp = a_exp;



    }
    else {
        unsigned int shift = b_exp - a_exp;
        a_shift_significand = (a_shift_significand >> ((shift > 31) ? 31 : shift));

        ans_exp = b_exp;
    }

    /* Adding Significands */
    if (a_sign == b_sign) {
        ans_significand = a_shift_significand + b_shift_significand;
        ans_sign = a_sign;

    }
    else {
        if (a_shift_significand > b_shift_significand) {
            ans_sign = a_sign;
            ans_significand = a_shift_significand - b_shift_significand;
        }
        else if (a_shift_significand < b_shift_significand) {
            ans_sign = b_sign;
            ans_significand = b_shift_significand - a_shift_significand;
        }
        else if ((a_shift_significand == b_shift_significand)) {
            ans_sign = 0;
            ans_significand = a_shift_significand - b_shift_significand;

        }
    }

    /* Normalization */
    int i;
    for (i = 31; i > 0 && ((ans_significand >> i) == 0); i--) { ; }

    if (i > 23) {

        //Rounding
        unsigned int residual = ((ans_significand & (1 << (i - 23 - 1))) >> (i - 23 - 1));

        unsigned int sticky = 0;
        for (int j = 0; j < i - 23 - 1; j++) {
            sticky = sticky | ((ans_significand & (1 << j)) >> j);
        }

        if ((int(ans_exp) + (i - 23) - 7) > 0 && (int(ans_exp) + (i - 23) - 7) < 255) {

            ans_significand = (ans_significand >> (i - 23));

            ans_exp = ans_exp + (i - 23) - 7;

            if (residual == 1 && sticky == 1) {
                ans_significand += 1;

            }
            else if ((ans_significand & 1) == 1 && residual == 1 && sticky == 0) {
                ans_significand += 1;

            }

            if ((ans_significand >> 24) == 1) {
                ans_significand = (ans_significand >> 1);
                ans_exp += 1;

            }
        }

        //Denormal number
        else if (int(ans_exp) + (i - 23) - 7 <= 0) {
            ans_denormal = true;
            ans_significand = ans_significand >> 7;
            ans_significand = ans_significand << (ans_exp - 1);
            ans_exp = 0;
        }

        //Overflow
        else if (int(ans_exp) + (i - 23) - 7 >= 255) {
            ans_significand = (1 << 23);
            ans_exp = 255;
        }

    }
    else if (i <= 23 && i != 0) {
        if ((int(ans_exp) - (23 - i) - 7) > 0 && (int(ans_exp) - (23 - i) - 7) < 255) {
            ans_significand = (ans_significand << (23 - i));
            ans_exp = ans_exp - (23 - i) - 7;
        }

        //Denormal Number
        else if (int(ans_exp) - (23 - i) - 7 <= 0) {
            ans_denormal = true;
            ans_significand = ans_significand >> 7;
            ans_significand = ans_significand << (ans_exp - 1);
            ans_exp = 0;
        }

        //Overflow
        else if ((int(ans_exp) - (23 - i) - 7) >= 255) {
            ans_significand = (1 << 23);
            ans_exp = 255;
        }


    }

    //When answer is zero
    else if (i == 0 && ans_exp < 255) {
        ans_exp = 0;
    }

    /* Constructing floating point number from sign, exponent and significand */

    unsigned int ans = (ans_sign << 31) | (ans_exp << 23) | (ans_significand & (0x7FFFFF));
    return ans;
}

unsigned int fmul(unsigned int a, unsigned int b) {
    if (a == 0) {
        return b;
    }
    if (b == 0) {
        return a;
    }

    //Extracting information from a and b
    unsigned int a_sign = (a & 0x80000000) >> 31;
    unsigned int a_enc_exp = (a & 0x7f800000) >> 23;
    unsigned int a_mantissa = (a & 0x7fffff);


    unsigned int b_sign = (b & 0x80000000) >> 31;
    unsigned int b_enc_exp = (b & 0x7f800000) >> 23;
    unsigned int b_mantissa = (b & 0x7fffff);


    cout << a_mantissa << " " << b_mantissa << endl;
}


class CPU {
     
public:
    // RAM
    unsigned int RAM[65536] = { 
        8, 12, 34, 56, 78
    };


    
    // BUS
    unsigned int bus = 0;
    unsigned int tmpBus = 0;

    unsigned int src = 0;
    unsigned int dst = 0;

    string tmpString = "";

    // general purpose registers
    unsigned int ax = 0;
    unsigned int bx = 0;
    unsigned int cx = 0;
    unsigned int dx = 0;

    unsigned int r1 = 0;
    unsigned int r2 = 0;
    unsigned int r3 = 0;
    unsigned int r4 = 0;

    // stack registers
    unsigned int sp = 65535; // stack pointer (point to top inc when push dec when pop)
    unsigned int bp = 65535; // base pointer

    unsigned int pc = 0; // programm counter

    unsigned int ir = 0; // instruction register
    unsigned int a0 = 0; // arg0 register
    unsigned int a1 = 0; // arg1 register
    unsigned int a2 = 0; // arg2 register
    unsigned int a3 = 0; // arg3 register

    unsigned int si = 0; // source register
    unsigned int di = 0; // destination register

    unsigned int cs = 256; // code segment register
    unsigned int ds = 21845; // data segment register
    unsigned int ss = 43690; // stack segment register


    bool z = false; // zero
    bool c = false; // carry
    bool s = false;
    bool eq = false; // equal
    bool gr = false; // greater
    bool le = false; // lesser

    bool h = false;

    void RamRead(int address) {
        bus = RAM[address];
    }

    void RamWrite(int address) {
        RAM[address] = bus;
    }

    void idxRegRead(int idx) {
        if (idx == 0) { bus = ax; }
        if (idx == 1) { bus = bx; }
        if (idx == 2) { bus = cx; }
        if (idx == 3) { bus = dx; }
        if (idx == 4) { bus = r1; }
        if (idx == 5) { bus = r2; }
        if (idx == 6) { bus = r3; }
        if (idx == 7) { bus = r4; }
        if (idx == 8) { bus = sp; }
        if (idx == 9) { bus = bp; }
        if (idx == 10) { bus = pc; }
        if (idx == 11) { bus = si; }
        if (idx == 12) { bus = di; }
        if (idx == 13) { bus = cs; }
        if (idx == 14) { bus = ds; }
        if (idx == 15) { bus = ss; }
    }

    void idxRegWrite(int idx) {
        if (idx == 0) {  ax = bus; }
        if (idx == 1) {  bx = bus; }
        if (idx == 2) {  cx = bus; }
        if (idx == 3) {  dx = bus; }
        if (idx == 4) {  r1 = bus; }
        if (idx == 5) {  r2 = bus; }
        if (idx == 6) {  r3 = bus; }
        if (idx == 7) {  r4 = bus; }
        if (idx == 8) {  sp = bus; }
        if (idx == 9) {  bp = bus; }
        if (idx == 10) {  pc = bus; }
        if (idx == 11) { si = bus; }
        if (idx == 12) { di = bus; }
        if (idx == 13) { cs = bus; }
        if (idx == 14) { ds = bus; }
        if (idx == 15) { ss = bus; }
    }


    /*
    * 
    * arithemtic rr
        
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = bus + tmpBus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;

    arithmetic ra
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(a1);
            bus = bus + tmpBus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false; 
            s = (bus < 0) ? true : false;
    
    arithmetic rc
            idxRegRead(a0);
            bus = bus + a1;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false; 
            s = (bus < 0) ? true : false;

    arthmetic ar
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(a0);
            bus = bus + tmpBus;
            RamWrite(a0);

    arithmetic ac
            RamRead(a0);
            bus = bus + a1;
            RamWrite(a0);

    arithmetic ptr r
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus + bus;
            RamWrite(tmpBus);

    arithmetic ptr c
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus + a1;
            RamWrite(tmpBus);

    mov ptr r
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            RamWrite(tmpBus);

    mov ptr c
            idxRegRead(a0);
            tmpBus = bus;
            bus = a1;
            RamWrite(tmpBus); 

    mov r ptr
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            idxRegWrite(a0);

    cmp rr
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            eq = (tmpBus == bus) ? true : false;
            gr = (tmpBus >  bus) ? true : false;
            le = (tmpBus <  bus) ? true : false;

    
    word = byte+byte
    pc = stoi(to_string(a0) + to_string(a1));


    push r (pre decrement (base at 65536 sp can go up to 43690) )
            sp--;
            idxRegRead(a0);
            RamWrite(sp);

    pop r (post increment)
            RamRead(sp);
            idxRegWrite(a0); 
            sp++; 

    read from ptr
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);

    write to ptr
            idxRegRead(a0);
            tmpBus = bus;
            // other thing here to get somehting else on thbe bus
            RamWrite(tmpBus);
    */


    void execute() {
        switch (ir) {
        case  0:break; // 0x00 nop
        case  1:
            idxRegRead(a1);
            idxRegWrite(a0);
            break; // 0x01 mov rr
        case  2:
            src = stoi(to_string(a1) + to_string(a2));
            RamRead(src);
            cout << a1 << " " << a2 << endl;
            cout << src << endl;
            idxRegWrite(a0);
            break; // 0x02 mov ra
        case  3:
            bus = a1;
            idxRegWrite(a0);
            break; // 0x03 mov rc 
        case  4:
            dst = stoi(to_string(a0) + to_string(a1));
            idxRegRead(a2);
            RamWrite(dst);
            break; // 0x04 mov ar
        case  5:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            RamWrite(tmpBus);
            break; // 0x05 mov ptr r
        case  6:
            idxRegRead(a0);
            tmpBus = bus;
            bus = a1;
            RamWrite(tmpBus);
            break; // 0x06 mov ptr c
        case  7:
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            idxRegWrite(a0);
            break; // 0x07 mov r ptr
        case  8:
            dst = stoi(to_string(a0) + to_string(a1));
            bus = stoi(to_string(a2) + to_string(a3));
            RamWrite(dst);
            break; // 0x08 mov ac
        case  9:
            sp--;
            idxRegRead(a0);
            RamWrite(sp);
            break; // 0x09 push r
        case 10:
            sp--;
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            RamWrite(sp);
            break; // 0x0a push ptr
        case 11:
            sp--;
            bus = a0;
            RamWrite(sp);
            break; // 0x0b push c
        case 12:
            RamRead(sp);
            idxRegWrite(a0);
            sp++;
            break; // 0x0c pop r
        case 13:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(sp);
            RamWrite(tmpBus);
            sp++;
            break; // 0x0d pop ptr
        case 14:
            sp--;
            bus = pc;
            RamWrite(sp);
            break; // 0x0e call
        case 15:
            RamRead(sp);
            pc = bus;
            sp++;
            break; // 0x0f ret


        case 16:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus + bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x10 add rr
        case 17:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(a1);
            bus = tmpBus + bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x11 add ra 
        case 18:
            idxRegRead(a0);
            bus = bus + a1;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x12 add rc
        case 19:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus + bus;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x13 add ptr r
        case 20:
            idxRegRead(a0);
            tmpBus = bus;
            bus = tmpBus + a1;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x14 add ptr c
        case 21:
            idxRegRead(a1);
            src = bus;
            RamRead(src);
            idxRegRead(a0);
            dst = bus;
            bus = dst + src;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x15 add r ptr
        case 22:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus - bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x16 sub rr
        case 23:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(a1);
            bus = tmpBus - bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x17 sub ra 
        case 24:
            idxRegRead(a0);
            bus = bus + a1;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x18 sub rc
        case 25:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus - bus;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x19 sub ptr r
        case 26:
            idxRegRead(a0);
            tmpBus = bus;
            bus = tmpBus + a1;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x1a sub ptr c
        case 27:
            idxRegRead(a1);
            src = bus;
            RamRead(src);
            idxRegRead(a0);
            dst = bus;
            bus = dst - src;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x1b sub r ptr
        case 28:
            idxRegRead(a1);
            src = bus;
            RamRead(a0);
            bus = bus + src;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x1c add ar
        case 29:
            RamRead(a0);
            bus = bus + a1;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x1d add ac
        case 30:
            idxRegRead(a1);
            src = bus;
            RamRead(a0);
            bus = bus - src;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x1e sub ar
        case 31:
            RamRead(a0);
            bus = bus - a1;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x1f sub ac

        case 32:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus * bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x20 mul rr
        case 33:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(a1);
            bus = tmpBus * bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x21 mul ra 
        case 34:
            idxRegRead(a0);
            bus = bus * a1;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x22 mul rc
        case 35:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus * bus;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x23 mul ptr r
        case 36:
            idxRegRead(a0);
            tmpBus = bus;
            bus = tmpBus * a1;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x24 mul ptr c
        case 37:
            idxRegRead(a1);
            src = bus;
            RamRead(src);
            idxRegRead(a0);
            dst = bus;
            bus = dst * src;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x25 mul r ptr
        case 38:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus / bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x26 div rr
        case 39:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(a1);
            bus = tmpBus / bus;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x27 div ra 
        case 40:
            idxRegRead(a0);
            bus = bus / a1;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x28 div rc
        case 41:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            bus = tmpBus / bus;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x29 div ptr r
        case 42:
            idxRegRead(a0);
            tmpBus = bus;
            bus = tmpBus / a1;
            RamWrite(tmpBus);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x2a div ptr c
        case 43:
            idxRegRead(a1);
            src = bus;
            RamRead(src);
            idxRegRead(a0);
            dst = bus;
            bus = dst / src;
            idxRegWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x2b div r ptr
        case 44:
            idxRegRead(a1);
            src = bus;
            RamRead(a0);
            bus = bus / src;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x2c mul ar
        case 45:
            RamRead(a0);
            bus = bus / a1;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x2d mul ac
        case 46:
            idxRegRead(a1);
            src = bus;
            RamRead(a0);
            bus = bus / src;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x2e div ar
        case 47:
            RamRead(a0);
            bus = bus / a1;
            RamWrite(a0);
            c = (bus > 65536) ? true : false;
            z = (bus == 0) ? true : false;
            s = (bus < 0) ? true : false;
            break; // 0x2f div ac

        case 48:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus & bus;
            idxRegWrite(a0);
            break; // 0x30 and rr
        case 49:
            idxRegRead(a0);
            dst = bus;
            bus = dst & a1;
            idxRegWrite(a0);
            break; // 0x31 and rc
        case 50:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus | bus;
            idxRegWrite(a0);
            break; // 0x32 or  rr
        case 51:
            idxRegRead(a0);
            dst = bus;
            bus = dst | a1;
            idxRegWrite(a0);
            break; // 0x33 or  rc
        case 52:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus ^ bus;
            idxRegWrite(a0);
            break; // 0x34 xor rr
        case 53:
            idxRegRead(a0);
            dst = bus;
            bus = dst ^ a1;
            idxRegWrite(a0);
            break; // 0x35 xor rc
        case 54:
            idxRegRead(a0);
            bus = ~bus;
            idxRegWrite(a0);
            break; // 0x36 not r
        case 55:
            RamRead(a0);
            bus = ~bus;
            RamWrite(a0);
            break; // 0x37 not a
        case 56:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            bus = ~bus;
            RamWrite(tmpBus);
            break; // 0x38 not ptr
        case 57:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus << bus;
            idxRegWrite(a0);
            break; // 0x39 shl rr
        case 58:
            idxRegRead(a0);
            dst = bus;
            bus = dst << a1;
            idxRegWrite(a0);
            break; // 0x3a shl rc
        case 59:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            src = bus;
            RamRead(src);
            bus = dst << bus;
            idxRegWrite(a0);
            break; // 0x3b shl r ptr
        case 60:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus >> bus;
            idxRegWrite(a0);
            break; // 0x3c shr rr
        case 61:
            idxRegRead(a0);
            dst = bus;
            bus = dst >> a1;
            idxRegWrite(a0);
            break; // 0x3d shr rc
        case 62:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus >> bus;
            idxRegWrite(a0);
            break; // 0x3e shr r ptr
        case 63:break; // 0x3f 

        case 64:
            idxRegRead(a0);
            tmpBus = bus;
            idxRegRead(a1);
            eq = (tmpBus == bus) ? true : false;
            gr = (tmpBus > bus) ? true : false;
            le = (tmpBus < bus) ? true : false;
            break; // 0x40 cmp rr
        case 65:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(a1);
            eq = (tmpBus == bus) ? true : false;
            gr = (tmpBus > bus) ? true : false;
            le = (tmpBus < bus) ? true : false;
            break; // 0x41 cmp ra
        case 66:
            idxRegRead(a0);
            tmpBus = bus;
            bus = a1;
            eq = (tmpBus == bus) ? true : false;
            gr = (tmpBus > bus) ? true : false;
            le = (tmpBus < bus) ? true : false;
            break; // 0x42 cmp rc
        case 67:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            idxRegRead(a1);
            eq = (dst == bus) ? true : false;
            gr = (dst > bus) ? true : false;
            le = (dst < bus) ? true : false; 
            break; // 0x43 cmp ptr r
        case 68:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            bus = a1;
            eq = (dst == bus) ? true : false;
            gr = (dst > bus) ? true : false;
            le = (dst < bus) ? true : false;
            break; // 0x44 cmp ptr c
        case 69:
            idxRegRead(a0);
            bus = ~bus;
            bus++;
            idxRegWrite(a0);
            break; // 0x45 neg r
        case 70:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            bus = ~bus;
            bus++;
            RamWrite(tmpBus);
            break; // 0x46 neg ptr
        case 71:
            RamRead(a0);
            bus = ~bus;
            bus++;
            RamWrite(a0);
            break; // 0x47 neg a
        case 72:break; // 0x48
        case 73:break; // 0x49
        case 74:break; // 0x4a
        case 75:break; // 0x4b
        case 76:break; // 0x4c
        case 77:break; // 0x4d
        case 78:break; // 0x4e
        case 79:break; // 0x4f

        case 80:pc = stoi(to_string(a0) + to_string(a1)); break;// 0x50 jmp
        case 81:if (z)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x51 jz
        case 82:if (!z)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x52 jnz
        case 83:if (c)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x53 jc
        case 84:if (!c)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x54 jnc
        case 85:if (eq)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x55 jeq
        case 86:if (!eq)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x56 jne
        case 87:if (gr)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x57 jg
        case 88:if (le)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x58 jl
        case 89:if (s)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x59 js
        case 90:if (!s)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x5a jns
        case 91:z = true; break; // 0x5b stz
        case 92:z = false; break; // 0x5c clz
        case 93:c = true; break; // 0x5d stc
        case 94:c = false; break; // 0x5e clc
        case 95:s = true; break; // 0x5f sts

        case 96:
            idxRegRead(a0);
            bus = (unsigned int)IEEE754_to_float(bus);
            idxRegWrite(a0);
            break; // 0x60 fint r
        case 97:
            idxRegRead(a0);
            bus = float_to_IEEE754((unsigned int)bus);
            idxRegWrite(a0);
            break; // 0x61 ifloat r
        case 98:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            if (IEEE754_to_float(dst) == IEEE754_to_float(bus)) eq = true;
            if (IEEE754_to_float(dst) >  IEEE754_to_float(bus)) gr = true;
            if (IEEE754_to_float(dst) <  IEEE754_to_float(bus)) le = true;
            break; // 0x62 fcmp rr
        case 99:
            idxRegRead(a0);
            if (IEEE754_to_float(bus) == IEEE754_to_float(a1)) eq = true;
            if (IEEE754_to_float(bus) > IEEE754_to_float(a1)) gr = true;
            if (IEEE754_to_float(bus) < IEEE754_to_float(a1)) le = true;
            break; // 0x63 fcmp rc
        case 100:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            if (IEEE754_to_float(dst) == IEEE754_to_float(bus)) eq = true;
            if (IEEE754_to_float(dst) > IEEE754_to_float(bus)) gr = true;
            if (IEEE754_to_float(dst) < IEEE754_to_float(bus)) le = true;
            break; // 0x64 fcmp r ptr
        case 101:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            if (IEEE754_to_float(bus) == IEEE754_to_float(a1)) eq = true;
            if (IEEE754_to_float(bus) > IEEE754_to_float(a1)) gr = true;
            if (IEEE754_to_float(bus) < IEEE754_to_float(a1)) le = true;
            break; // 0x65 fcmp ptr c
        case 102:break; // 0x66 
        case 103:break; // 0x67
        case 104:break; // 0x68
        case 105:break; // 0x69
        case 106:break; // 0x6a
        case 107:break; // 0x6b
        case 108:break; // 0x6c
        case 109:if (le || eq)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x6d jle
        case 110:if (gr || eq)pc = stoi(to_string(a0) + to_string(a1)); break; // 0x6e jge
        case 111:s = false; break; // 0x6f cls

        case 112:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = fadd(dst, bus);
            idxRegWrite(a0);
            break; // 0x70 fadd rr
        case 113:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            bus = fadd(dst, bus);
            idxRegWrite(a0);
            break; // 0x71 fadd r ptr
        case 114:
            idxRegRead(a0);
            dst = bus;
            bus = fadd(dst, a1);
            idxRegWrite(a0);
            break; // 0x72 fadd rc
        case 115:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            idxRegRead(a1);
            bus = fadd(dst, bus);
            RamWrite(tmpBus);
            break; // 0x73 fadd ptr r
        case 116:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            bus = fadd(dst, a1);
            RamWrite(tmpBus);
            break; // 0x74 fadd ptr c
        case 117:
            idxRegRead(a0);
            dst = bus;
            RamRead(a1);
            bus = fadd(dst, bus);
            idxRegWrite(a0);
            break; // 0x75 fadd ra
        case 118:
            RamRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = fadd(dst, bus);
            RamWrite(a0);
            break; // 0x76 fadd ar
        case 119:
            RamRead(a0);
            dst = bus;
            bus = fadd(dst, a1);
            RamWrite(a0);
            break; // 0x77 fadd ac
        case 120:break; // 0x78
        case 121:break; // 0x79
        case 122:break; // 0x7a
        case 123:break; // 0x7b
        case 124:break; // 0x7c
        case 125:break; // 0x7d
        case 126:break; // 0x7e
        case 127:break; // 0x7f

        case 128:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus | 0x80000000;
            bus = fadd(dst, bus);
            idxRegWrite(a0);
            break; // 0x80 fsub rr
        case 129:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            bus = bus | 0x80000000;
            bus = fadd(dst, bus);
            idxRegWrite(a0);
            break; // 0x81 fsub r ptr
        case 130:
            idxRegRead(a0);
            dst = bus;
            bus = fadd(dst, a1 | 0x80000000);
            idxRegWrite(a0);
            break; // 0x82 fsub rc
        case 131:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            idxRegRead(a1);
            bus = bus | 0x80000000;
            bus = fadd(dst, bus);
            RamWrite(tmpBus);
            break; // 0x83 fsub ptr r
        case 132:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            bus = fadd(dst, a1 | 0x80000000);
            RamWrite(tmpBus);
            break; // 0x84 fsub ptr c
        case 133:
            idxRegRead(a0);
            dst = bus;
            RamRead(a1);
            bus = bus | 0x80000000;
            bus = fadd(dst, bus);
            idxRegWrite(a0);
            break; // 0x85 fsub ra
        case 134:
            RamRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = bus | 0x80000000;
            bus = fadd(dst, bus);
            RamWrite(a0);
            break; // 0x86 fsub ar
        case 135:
            RamRead(a0);
            dst = bus;
            bus = fadd(dst, a1 | 0x80000000);
            RamWrite(a0);
            break; // 0x87 fsub ac
        case 136:break; // 0x88
        case 137:break; // 0x89
        case 138:break; // 0x8a
        case 139:break; // 0x8b
        case 140:break; // 0x8c
        case 141:break; // 0x8d
        case 142:break; // 0x8e
        case 143:break; // 0x8f

        case 144:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(bus));
            idxRegWrite(a0);
            break; // 0x90 fmul rr
        case 145:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(bus));
            idxRegWrite(a0);
            break; // 0x91 fmul r ptr
        case 146:
            idxRegRead(a0);
            bus = float_to_IEEE754(IEEE754_to_float(bus) * IEEE754_to_float(a1));
            idxRegWrite(a0);
            break; // 0x92 fmul rc
        case 147:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            idxRegRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(bus));
            RamWrite(tmpBus);
            break; // 0x93 fmul ptr r
        case 148:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(a1));
            RamWrite(tmpBus);
            break; // 0x94 fmul ptr c
        case 149:
            idxRegRead(a0);
            dst = bus;
            RamRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(bus));
            idxRegWrite(a0);
            break; // 0x95 fmul ra
        case 150:
            RamRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(bus));
            RamWrite(a0);
            break; // 0x96 fmul ar
        case 151:
            RamRead(a0);
            dst = bus;
            bus = float_to_IEEE754(IEEE754_to_float(dst) * IEEE754_to_float(a1));
            RamWrite(a0);
            break; // 0x97 fmul ac
        case 152:break; // 0x98
        case 153:break; // 0x99
        case 154:break; // 0x9a
        case 155:break; // 0x9b
        case 156:break; // 0x9c
        case 157:break; // 0x9d
        case 158:break; // 0x9e
        case 159:break; // 0x9f

        case 160:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(bus));
            idxRegWrite(a0);
            break; // 0xa0 fdiv rr
        case 161:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            tmpBus = bus;
            RamRead(tmpBus);
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(bus));
            idxRegWrite(a0);
            break; // 0xa1 fdiv r ptr
        case 162:
            idxRegRead(a0);
            bus = float_to_IEEE754(IEEE754_to_float(bus) / IEEE754_to_float(a1));
            idxRegWrite(a0);
            break; // 0xa2 fdiv rc
        case 163:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            idxRegRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(bus));
            RamWrite(tmpBus);
            break; // 0xa3 fdiv ptr r
        case 164:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            dst = bus;
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(a1));
            RamWrite(tmpBus);
            break; // 0xa4 fdiv ptr c
        case 165:
            idxRegRead(a0);
            dst = bus;
            RamRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(bus));
            idxRegWrite(a0);
            break; // 0xa5 fdiv ra
        case 166:
            RamRead(a0);
            dst = bus;
            idxRegRead(a1);
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(bus));
            RamWrite(a0);
            break; // 0xa6 fdiv ar
        case 167:
            RamRead(a0);
            dst = bus;
            bus = float_to_IEEE754(IEEE754_to_float(dst) / IEEE754_to_float(a1));
            RamWrite(a0);
            break; // 0xa7 fdiv ac
        case 168:break; // 0xa8
        case 169:break; // 0xa9
        case 170:break; // 0xaa
        case 171:break; // 0xab
        case 172:break; // 0xac
        case 173:break; // 0xad
        case 174:break; // 0xae
        case 175:break; // 0xaf

        case 176:
            idxRegRead(a0);
            cout << bus;
            break; // 0xb0 oint r
        case 177:
            idxRegRead(a0);
            cout << IEEE754_to_float(bus);
            break; // 0xb1 ofloat r
        case 178:
            idxRegRead(a0);
            cout << static_cast<char>(bus);
            break; // 0xb2 ochar r
        case 179:
            RamRead(a0);
            cout << bus;
            break; // 0xb3 oint a
        case 180:
            RamRead(a0);
            cout << IEEE754_to_float(bus);
            break; // 0xb4 ofloat a
        case 181:
            RamRead(a0);
            cout << static_cast<char>(bus);
            break; // 0xb5 ochar a
        case 182:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            cout << bus;
            break; // 0xb6 oint ptr
        case 183:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            cout << IEEE754_to_float(bus);
            break; // 0xb7 ofloat ptr
        case 184:
            idxRegRead(a0);
            tmpBus = bus;
            RamRead(tmpBus);
            cout << static_cast<char>(bus);
            break; // 0xb8 ochar ptr
        case 185:
            cin >> bus;
            idxRegWrite(a0);
            break; // 0xb9 iint r
        case 186:
            cin >> bus;
            bus = float_to_IEEE754(bus);
            idxRegWrite(a0);
            break; // 0xba ifloat r
        case 187:
            getline(cin, tmpString);
            bus = (unsigned int)tmpString[0];
            idxRegWrite(a0);
            break; // 0xbb ichar r
        case 188:
            idxRegRead(a0);
            tmpBus = bus;
            cin >> bus;
            RamWrite(tmpBus);
            break; // 0xbc iint ptr
        case 189:
            idxRegRead(a0);
            tmpBus = bus;
            cin >> bus;
            bus = float_to_IEEE754(bus);
            RamWrite(tmpBus);
            break; // 0xbd ifloat ptr
        case 190:
            idxRegRead(a0);
            tmpBus = bus;
            getline(cin, tmpString);
            bus = (unsigned int)tmpString[0];
            bus = float_to_IEEE754(bus);
            RamWrite(tmpBus);
            break; // 0xbe ichar ptr
        case 191:break; // 0xbf

        case 192:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            si = dst - bus;
            break; // 0xc0 soff rr
        case 193:
            idxRegRead(a0);
            si = bus - a1;
            break; // 0xc1 soff rc
        case 194:
            idxRegRead(a0);
            dst = bus;
            idxRegRead(a1);
            si = dst + bus;
            break; // 0xc2 aoff rr 
        case 195:
            idxRegRead(a0);
            si = bus + a1;
            break; // 0xc3 aoff rc
        case 196:break; // 0xc4
        case 197:break; // 0xc5
        case 198:break; // 0xc6
        case 199:break; // 0xc7
        case 200:break; // 0xc8
        case 201:break; // 0xc9
        case 202:break; // 0xca
        case 203:break; // 0xcb
        case 204:break; // 0xcc
        case 205:break; // 0xcd
        case 206:break; // 0xce
        case 207:break; // 0xcf

        case 208:
            sp--;
            RamRead(a0);
            RamWrite(sp);
            break; // 0xd0 push a
        case 209:
            RamRead(sp);
            RamWrite(a0);
            sp++;
            break; // 0xd1 pop a
        case 210:
            dst = sp;
            sp--; idxRegRead(0);  RamWrite(sp);
            sp--; idxRegRead(1);  RamWrite(sp);
            sp--; idxRegRead(2);  RamWrite(sp);
            sp--; idxRegRead(3);  RamWrite(sp);
            sp--; bus = dst;      RamWrite(sp);
            sp--; idxRegRead(5);  RamWrite(sp);
            sp--; idxRegRead(10); RamWrite(sp);
            sp--; idxRegRead(11); RamWrite(sp);
            break; // 0xd2 pusha
        case 211:
            RamRead(sp); idxRegWrite(11); sp++;
            RamRead(sp); idxRegWrite(10); sp++;
            RamRead(sp); idxRegWrite(5); sp++; // bp
            sp++; // sp is ignored
            RamRead(sp); idxRegWrite(3); sp++;
            RamRead(sp); idxRegWrite(2); sp++;
            RamRead(sp); idxRegWrite(1); sp++;
            RamRead(sp); idxRegWrite(0); sp++;
            break; // 0xd3 popa
        case 212:break; // 0xd4
        case 213:break; // 0xd5
        case 214:break; // 0xd6
        case 215:break; // 0xd7
        case 216:break; // 0xd8
        case 217:break; // 0xd9
        case 218:break; // 0xda
        case 219:break; // 0xdb
        case 220:break; // 0xdc
        case 221:break; // 0xdd
        case 222:break; // 0xde
        case 223:break; // 0xdf

        case 224:break; // 0xe0
        case 225:break; // 0xe1
        case 226:break; // 0xe2
        case 227:break; // 0xe3
        case 228:break; // 0xe4
        case 229:break; // 0xe5
        case 230:break; // 0xe6
        case 231:break; // 0xe7
        case 232:break; // 0xe8
        case 233:break; // 0xe9
        case 234:break; // 0xea
        case 235:break; // 0xeb
        case 236:break; // 0xec
        case 237:break; // 0xed
        case 238:break; // 0xee
        case 239:break; // 0xef

        case 240:break; // 0xf0
        case 241:break; // 0xf1
        case 242:break; // 0xf2
        case 243:break; // 0xf3
        case 244:break; // 0xf4
        case 245:break; // 0xf5
        case 246:break; // 0xf6
        case 247:break; // 0xf7
        case 248:break; // 0xf8
        case 249:break; // 0xf9
        case 250:break; // 0xfa
        case 251:break; // 0xfb
        case 252:break; // 0xfc
        case 253:break; // 0xfd
        case 254:break; // 0xfe
        case 255:
            h = true;
            break; // 0xff hlt

            
        

        default:
            break;
        }
        



        

    }

    void cycle() {

        if (!h) {

            int args = 0;

            // fetch
            RamRead(pc);
            ir = bus;
            pc++;

            switch (ir)
            {
            case 2:args = 3; break;
            case 3:args = 4; break;
            case 4:args = 3; break;
            case 8:args = 4; break;
            case 17:args = 3; break;
            case 23:args = 3; break;
            case 28:args = 3; break;
            case 29:args = 3; break;
            case 30:args = 3; break;
            case 31:args = 3; break;
            case 33:args = 3; break;
            case 39:args = 3; break;
            case 44:args = 3; break;
            case 45:args = 3; break;
            case 46:args = 3; break;
            case 47:args = 3; break;
            case 117:args = 3; break;
            case 118:args = 3; break;
            case 119:args = 3; break;
            case 133:args = 3; break;
            case 134:args = 3; break;
            case 135:args = 3; break;
            case 149:args = 3; break;
            case 150:args = 3; break;
            case 151:args = 3; break;
            case 165:args = 3; break;
            case 166:args = 3; break;
            case 167:args = 3; break;


            default:
                break;
            }

            RamRead(pc);
            a0 = bus;
            pc++;
            
            RamRead(pc);
            a1 = bus;
            pc++;
            
            if (args == 3) {
                RamRead(pc);
                a2 = bus;
                pc++;

                std::cout << "instruction: " << ir << " " << a0 << " " << a1 << " " << a2 << std::endl;
                execute();
            }

            else if (args == 4) {
                RamRead(pc);
                a2 = bus;
                pc++;

                RamRead(pc);
                a3 = bus;
                pc++;

                std::cout << "instruction: " << ir << " " << a0 << " " << a1 << " " << a2 << " " << a3 << std::endl;
                execute();
            }

            else {
                cout << "here:(" << endl;
                cout << args;
                std::cout << "instruction: " << ir << " " << a0 << " " << a1 << " " << std::endl;
                execute();
            }
            

            // execute

            
        }
            



    }

    void init() {
        
    }

    void repr(int start, int end) {
        std::cout << "ax: " << ax << " bx: " << bx << " cx: " << cx << " dx: " << dx << std::endl;
        std::cout << "si: " << si << " di: " << di << std::endl;
        std::cout << "bus: " << bus << std::endl;
        std::cout << "pc: " << pc << std::endl;
        std::cout << "sp+bp: " << sp << " " << bp << std::endl;
        std::cout << "stack: " << std::endl;
        for (int i = start; i < end; i++) {
            if (i % 16 == 0) {
                std::cout << std::endl;
            }
            std::cout << RAM[i] << " ";
            
        }

        std::cout << std::endl;
    }


};



int main()
{
    CPU cpu;

    /*int intructions = 11;

    for (int i = 0; i < intructions; i++) {
        cpu.cycle();
        cpu.repr();
    }*/
    int c;
    while ((c = getchar()) != EOF) {
        cpu.cycle();
        cpu.repr(1200, 1328);
    }



    return 0;
}

