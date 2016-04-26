#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <cmath>
using namespace std;

//Return the digit place spaces from the right.
int extract(int n, int place) {
    return (n % (int) pow(10,place) / (int) pow(10,place-1));
}

//You'll want to expand this if you want to hash more numbers.
const int HASHTABLE_SIZE = 65001;
int search_exponent = 1;

template <class valT>
class HashTable {
private:

    //Proxy class returned by operator[] so that assigning and getting value of things operator[]
    //returns is done corrrectly.
    class proxy {
        HashTable<valT> &origin;
        int key;
    };

    struct record {
        int key;
        valT value;
    };

    record ** backing;

    //I based this one on the diagram withotu realizing that the function defined in the text was different.
    /*
    //Returns a number with every other digit from the given number. i.e.
    //34567 becomes  357
    int hash(int n) {
        int placecount;
        int result = 0;
        placecount = ceil(log10(n));

        //For every other place in the number, starting from the 2nd (100ths) and ending at the very last one)
        for (int t = 2; t <= placecount; t+=2) {
            //cout << extract(n,t) << endl;
            //Get the number and put it in the t/2th place
            result += extract(n, t) * pow(10, (t / 2) - 1);
        }
        return result % HASHTABLE_SIZE;
    }*/

    //Extraction hash; extracts tens, hundreds, ten-thousands, and millions places and returns a 4 digit integer.
    //i.e. 123456789 becomes 3578.
    int hash(int n) {
        int result = 0;

        result += extract(n, 2);
        result += extract(n, 3) * 10;
        result += extract(n, 5) * 100;
        result += extract(n, 7) * 1000;

        return result;
    }

    //Given a key, return a pointer to its record.
    //Detects collisions and implements polynomial search to resolve them.
    //If the appropriate record does not exist:
        //If create_if_not_found is true, creates a new record.
        //otherwise returns NULL.
    record * getrecord(int key, bool create_if_not_found=false) {
        int idx;
        int offset = search_exponent;
        record * addr;
        idx = hash(key);

        //Iterate until we find an appropriate location
        while (true) {
            addr = backing[idx];

            //If there's no record there, make one and return it
            if (addr == NULL) {
                if (create_if_not_found) {
                    backing[idx] = new record;
                    backing[idx]->key = key;
                }
                return backing[idx];
            //Otherwise, there is a record, so let's check if it's the one we want...
            } else {
                if (addr -> key == key) {
                    //It IS the one we want, return it.
                    return addr;
                }
                //Otherwise we need to find a new addr.
                //If our offset value is the size of our memory or greater, the memory must be full.
                if (offset >= HASHTABLE_SIZE) {
                    cout << "ERROR: Hashtable out of memory. Aborting." << endl;
                    abort();
                }
                offset++;
                idx = (idx + (offset * offset)) % HASHTABLE_SIZE;
            }
        }
    }

public:
    HashTable() {
        backing = new record* [HASHTABLE_SIZE];
        for (int t=0; t < HASHTABLE_SIZE; t++) { backing[t] = NULL; }
    }

    ~HashTable() {
        for (int t=0; t < HASHTABLE_SIZE; t++) {
            if (backing[t] != NULL) {
                delete backing[t];
            }
        }
        delete[] backing;
    }

    //Retrieve value at key from the table
    valT get(int key) {
        record * therecord;
        therecord = getrecord(key);
        if (therecord == NULL) {
            cout << "ERROR: Key " << key << " does not exist in this table. Aborting." << endl;
            abort();
        }
        return therecord -> value;
    }

    //Set value at key to given value
    void set(int key, valT val) {
        record * therecord;
        therecord = getrecord(key, true);
        therecord -> value = val;
    }

    //Write the set of used indexes in this table to the given stream in CSV format.
    void writekeys(string fname) {
        ofstream out;
        vector<int> keys;

        //Stick all our used indexes in a vector...
        for (int t=0; t < HASHTABLE_SIZE; t++) {
            if (backing[t] != NULL) {
                keys.push_back(hash(backing[t]->key)); // I seriously don't understand what I'm supposed to be writing out here.
            }
        }

        //Then write them to a file.
        out.open(fname.c_str());
        for (int t=0; t < keys.size(); t++) {
            out << keys[t];
            //All but the last get a comma after them
            if (t != keys.size() - 1) {
                out << ',';
            }
        }
        out.close();
    }
};

//Given a pointer to a vector of ints, fil it with integers read from of the file at the given path
void read_file_into_vector(const string & filename, vector<int> & target) {
	ifstream myfile;
	myfile.open(filename.c_str(), ifstream::in);
	int x;
    string line;
	while (!myfile.eof()) {
        getline(myfile, line, ',');
        x = stoi(line, NULL);
		target.push_back(x);
	}
    myfile.close();
}

int main(int argc, char* argv[]) {
    HashTable<int> table;
    vector<int> input;

    read_file_into_vector("everybodys_socials.txt",input);

    cout << "Input search exponent: ";
    cin >> search_exponent;

    for (int t=0; t < input.size(); t++) {
        table.set(input[t], input[t]);
    }


    int ok = 0;
    for (int t=0; t < input.size(); t++) {
        int result;
        result = table.get(input[t]);
        if ( result != input[t]) {
            cout << "ASSERTION FAILED: Expected " << input[t] << ", table contained " << result << "." << endl;
        } else {
            ok++;
        }
    }
    cout << ok << " entries succesfully stored and checked." << endl;


    table.writekeys("hashed_socials.txt");

	return 0;
}
