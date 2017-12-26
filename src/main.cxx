
#include <iostream>
#include <fstream>
#include <sstream>
#include "scm2js.h"

int main(int argc, char *argv[]) {
    
    using std::stringstream;
    using std::cin;
    using std::cout;    

    stringstream aString;

    if (argc > 1) {
        // Read from a file
        std::string fname(argv[1]);
        std::ifstream myfile;
        myfile.open ("example.scm");
        std::string line;
        if (myfile.is_open()) {
            while ( getline (myfile, line) ) {
            aString << line << '\n';
            }
            myfile.close();
        }
    }

    if (aString.str().empty()) {
        char tempChar;
        while (!cin.eof())
        {
            cin.get(tempChar);
            aString << tempChar;
        }
    }

    scheme_to_javascript translator;

    translator.translate(aString);

    cout << translator.get_string();
    cout.flush();

    return 0;
    }
