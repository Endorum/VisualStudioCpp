
/*


Brainfuck + 
    + inc cell
    - dec cell
    > move right
    < move left
    [ while cell != 0
    ] jmp to [ if not zero else next
    . out in ascii
    , in in ascii
    ~ jump to idx in cell
    : push current cell to stack
    ; pop current cell to stack
    ? idx to cell (get current head loc)


*/


#include <iostream>
#include <string>
#include <vector>

using namespace std;



int main()
{
    /*int n1 = 0, n2 = 1, n3, i, number=15;

    i = 2;
    while (i < number) {
        n3 = n1 + n2;

        n1 = n2;
        n2 = n3;
        i++;
    }*/



    int tape[65536]  = { 0 };
    
    int memPointer  = 0;
    int ProgPointer = 0;

    int braceCount = 0;

    vector<int> stack;

    vector<int> functions;

    bool debug = true;
    bool hlt = false;

    string program = R"(


    {:>:>:|>>~:|>~;|>>;<;<;|}
    
    {:>:>:|>>>>:|;>>>:|>;|+:>>>>>;|>>;<;<;|}
    {:>:>:|>>>>:|;>>>:|>;|-:>>>>>;|>>;<;<;|}
    {:>:>:|>>>>:|;>>>:|>;|*:>>>>>;|>>;<;<;|}
    {:>:>:|>>>>:|;>>>:|>;|/:>>>>>;|>>;<;<;|}
    
    {:>>>;|>:>>>;|>>:>>>;|>>>>>>;|

    i[d , >>i<< >%:< - ]|
    >>o_

    |>>>:|;>>>>:|>;>>>>>:|>>;|>>>>>>:|>>>>>:|>>;|}
    
    
    
    %A>%2>%3|
    C5|
    
    here
    
   
)";
   
    /*
    * 
    proluge
    :>:>:|

    epilouge
    >>;<;<;|
    
    >>>%0>%1>%0>%2>%>|{>>>:|;>>>>:|>;|
    +:>>>>>;|>>>>>o|%P.|>>>>:<;|>>>>>:
    <;|>>>>>>i:|;>>>>>>>:|>;|-$0|}C0


    >>>%0>%1>%0>%2>%>|


    
    {
    >>>:|
    ;>>>>:|
    >;|
    +:>>>>>;|
    >>>>>o|
    %P.|
    >>>>:<;|
    >>>>>:<;|
    >>>>>>i:|
    ;>>>>>>>:|
    >;|
    -
    $0|
    }
    
    C0

    
    init: n1,n2=1,n3,i=2,number=exmpl-15

    loop


    hlt


    */
    int i = 2;
    int number = 15;
    while (i < number) {

        i++;
    }

    for (int i = 0; i < program.size(); i++) {
        if (program[i] == '{') {
            functions.push_back(i);
        }
    }
    
    while (ProgPointer < program.length() && not hlt) {

        char inst = program[ProgPointer];
        if (inst == '#') {
            tape[memPointer] = program[ProgPointer + 1]; // ascii value of next char in progm (0-127)
            ProgPointer++;
        }
        if (inst == '%') {
            tape[memPointer] = program[ProgPointer + 1] - 48;
            ProgPointer++;
        }
        if (inst == 'i') tape[memPointer]++; // increase
        if (inst == 'd') tape[memPointer]--; // decrease
        if (inst == '+') tape[memPointer] = tape[memPointer] + tape[memPointer + 1];        // tape[i] = tape[i] + tape[i+1];
        if (inst == '-') tape[memPointer] = tape[memPointer] - tape[memPointer + 1];        // tape[i] = tape[i] - tape[i+1];
        if (inst == '*') tape[memPointer] = tape[memPointer] * tape[memPointer + 1];        // tape[i] = tape[i] * tape[i+1];
        if (inst == '/') tape[memPointer] = (int)(tape[memPointer] / tape[memPointer + 1]); // tape[i] = tape[i] / tape[i+1];

        if (inst == '>') memPointer++; // move left
        if (inst == '<') memPointer--; // move right

        if (inst == '[') { 
            if (tape[memPointer] == 0) {
                braceCount++;
                while (program[ProgPointer] != ']' || braceCount != 0) {
                    ProgPointer++;
                    if (program[ProgPointer] == '[') braceCount++;
                    else if (program[ProgPointer] == ']') braceCount--;
                }
            }
        } 
        // while (cell != 0)

        if (inst == ']') {
            if (tape[memPointer] != 0) {
                braceCount++;
                while (program[ProgPointer] != '[' || braceCount != 0) {
                    ProgPointer--;
                    if (program[ProgPointer] == ']') braceCount++;
                    else if (program[ProgPointer] == '[') braceCount--;
                }
            }
        }
        // close of while loop

        if (inst == '.')  putchar(tape[memPointer]) ; // ascii out
        if (inst == 'o') {
            if (debug) {
                std::cout << (tape[memPointer]) << endl;
            }
            else {
                std::cout << (tape[memPointer]);
            }
        } // number out
        if (inst == ',') tape[memPointer] = getchar(); // ascii in
        if (inst == '~') memPointer = tape[memPointer]; // jmp in memory to where cell points to
        if (inst == ':') stack.push_back(tape[memPointer]); // push to stack
        if (inst == ';') {
            if (stack.size() > 0) {
                tape[memPointer] = stack.back();
                stack.pop_back();
            }
                
        }
        // pop from stack
        if (inst == '?') tape[memPointer] = memPointer;
        // get current location
        if (inst == '&') tape[memPointer+1] = tape[tape[memPointer]];

        
        if (inst == '$') {
            if (tape[memPointer] != 0) { // jmp to programm loc in i+1 if i == 0
                int idx = program[ProgPointer + 1] - 48;
                ProgPointer = functions[idx];
            }

            //ProgPointer = tape[memPointer + 1];
        }

        if (inst == 'C') { // goto pos of { from prog+1
            int idx = program[ProgPointer + 1] - 48;
            stack.push_back(ProgPointer);
            ProgPointer = functions[idx];
        }

        if (inst == '}') { // return (pop and jmp to poped)
            if (stack.size() > 0) {
                int tmp = stack.back();
                stack.pop_back();
                ProgPointer = tmp;
                ProgPointer = tmp;
            }
                
        }

        if (inst == '{') { // jmp to } if not called
            for (int i = ProgPointer; i < program.size(); i++) {
                if (program[i] == '}') {
                    ProgPointer = i;
                    break;
                }
            }
        }

        if (inst == '|') {
            memPointer = 0;
        }

        if (inst == '_') {
            std::cout << endl << "exit status 0" << endl;
            hlt = true;
        }


        if (memPointer > 65535) {
            memPointer = 65535 - memPointer;
        }
        if (memPointer < 0) {
            memPointer = 65535;
        }
        if (tape[memPointer] > 65535) {

            while (tape[memPointer] > 65535) {
                tape[memPointer] -= 65535;
            }
                
        }
        if (tape[memPointer] < 0) {
            while (tape[memPointer] < 0) {
                tape[memPointer] += 65535;
            }
        }

        //std::cout << tape[memPointer-1] << tape[memPointer] << tape[memPointer + 1] << endl;

        if (debug && inst != ' ' && inst != '\n') {
            for (int i = 0; i < 25; i++) {
                std::cout << tape[i] << " ";
            }
            /*for (int i = 0; i < functions.size(); i++) {
                cout << "functions: " << i << ": " << functions[i] << " ";
            }*/
            cout << "stack: ";
            for (int i = 0; i < stack.size(); i++) {
                cout << stack[i] << " ";
            }
            std::cout << "memory: " << memPointer << " program: " << ProgPointer << " instruction: " << inst;
            std::cout << endl;
        }


        
            
        ProgPointer++;
    }


    std::cout << "\n" << "-----------------------" << endl;
    
    for (int i = 0; i < 20; i++) {
        std::cout << tape[i] << " ";
    }
    std::cout << "memory: " << memPointer << " program: " << ProgPointer;
    std::cout << endl;
    ProgPointer++;
}
