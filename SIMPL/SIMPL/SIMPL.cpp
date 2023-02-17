


#include <iostream>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <vector>


using namespace std;

/*


characters:

!
"
§
$
%
&
/
(
)
=
?
²
³
{
[
]
}
\
+
#
-
.
,
<
*
'
_
:
;
>
~
|
@
€

*/


string prog = R"(

A<#55


_)";

string registers = "ABCDPS";

bool in(char ch, string test) {
	for (int i = 0; i < test.size(); i++) {
		if (ch == test[i]) return true;
	}

	return false;
}

int main() {
	

	int RAM[65536];
	int A, B, C, D, P, S;

	int ProgPointer = 0;
	int MemPointer = 0;

	

	while (true) {
		char inst = prog[ProgPointer];
		if (inst == '<') {
			if (prog[ProgPointer + 1] == '<') {
				// Reg<<Ram
			}
			else if ( in(prog[ProgPointer + 1], registers) ) {
				
			}
		}
		else if (inst == '_') { break; }

		ProgPointer++;
	}

	return 0;
}