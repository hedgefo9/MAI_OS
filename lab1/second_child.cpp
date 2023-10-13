#include <iostream>
#include <sstream>
#include <string>

using namespace std;

int main(int argc, char *argv[]) {

    if (argc < 2) {
        cerr << "Too few arguments — please provide input line" << endl;
        return 1;
    }

    string word = argv[1];

    for (char &c: word) {
        if (c == ' ') {
            c = '_';
        }
    }
    word += '\n';
    cout << word;

    return 0;
}