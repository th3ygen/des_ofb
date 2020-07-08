#include <iostream>
#include <string>
#include <math.h>
#include <vector>
#include <bitset>
#include <bits/stdc++.h>
#include <conio.h>

#include "des_data_table.h"
#include "des_key_table.h"

using namespace std;

#define TOTAL_ROUND 16

// helper functions
int XOR(int a, int b) {
    if (a == b) {
        return 0;
    }
    return 1;
}

vector<int> blockXOR(vector<int> a, vector<int> b) {
    vector<int> res;
    for (int x = 0; x < a.size(); x++) {
        res.push_back(XOR(a.at(x), b.at(x)));
    }
    return res;
}

char intToHex(int x) {
    return ("0123456789ABCDEF")[x];
}

int hexToInt(char h) {
    return string("0123456789ABCDEF").find(h);
}

string byteToHex(vector<int> byte) {
    string res = "";
    vector<vector<int>> d;

    d.push_back(vector<int>(byte.begin(), byte.begin() + 4));
    d.push_back(vector<int>(byte.begin() + 4, byte.end()));

    int p = 4;
    int j = 0;
    for (vector<int> x : d) {
        for (int y : x) {
            j += pow(2, --p) * y;
        }

        res += intToHex(j);
        p = 4;
        j = 0;
    }

    return res;
}

vector<int> charToBit(char c, int size, bool pad, bool cap) {
    int n = int(c);

    if (n < 0) {
        n = 256 + n;
    }

    vector<int> bits;
    
    while (n > 0) {
        bits.insert(bits.begin(), n % 2);
        n /= 2;
    }

    int pads = size - bits.size();
    if (pad && pads > 0) {
        for (int x = 0; x < pads; x++) {
            bits.insert(bits.begin(), 0);
        }
    } else if (cap) {
        for (int x = pads; x < 0; x++) {
            bits.erase(bits.begin());
        }
    }

    return bits;
}

vector<int> numToBit(int n, int size, bool pad) {
    vector<int> bits;

    while (n > 0) {
        bits.insert(bits.begin(), n % 2);
        n /= 2;
    }

    int pads = size - bits.size();
    if (pad && pads > 0) {
        for (int x = 0; x < pads; x++) {
            bits.insert(bits.begin(), 0);
        }
    }

    return bits;
}

vector<char> strToVecASCII(string text) {
    vector<char> res;

    int x = 0;
    for (char c : text) {
        res.push_back(char(++x));
    }

    return res;
}

vector<int> strToBit(string str, int size, bool pad, bool cap) {
    vector<int> bits;
    for (char c : str) {
        vector<int> tmpBits;
        for (int bit : charToBit(c, 8, true, false)) {
            bits.push_back(bit);
        }
    }
    
    int pads = size - bits.size();
    if (pad && pads > 0) {
        for (int x = 0; x < pads; x++) {
            bits.push_back(0);
        }
    } else if (cap) {
        for (int x = pads; x < 0; x++) {
            bits.erase(bits.begin());
        }
    }

    return bits;
}

vector<vector<int>> strToBit64Blocks(string str, bool pad, bool cap) {
    vector<vector<int>> blocks;

    int n = str.size();
    int totalBlocks = ceil(float(n) / 8);

    int s;
    for (int x = 0; x < totalBlocks; x++) {
        s = 8;
        if (x == totalBlocks - 1 && (n % 8) != 0) {
            s = n % 8;
        }

        blocks.push_back(strToBit(str.substr(x * 8, s), 64, pad, cap));
    }

    return blocks;
}

char byteToChar(vector<int> bits) {
    int p = 8;
    int d = 0;
    for (int x : bits) {
        d += pow(2, --p) * x;
    }

    return char(d);
}

string bit64ToStr(vector<int> bits) {
    string res = "";

    vector<vector<int>> grps;
    for (int x = 0; x < 8; x++) {
        grps.push_back(vector<int>(bits.begin() + (x * 8), bits.begin() + ((x * 8) + 8)));
    }

    int d = 0;
    int p = 7;
    for (vector<int> x : grps) {
        for (int y : x) {
            d += pow(2, p--) * y;
        }
        /* d %= 128; */
        res += (char)d;
        d = 0;
        p = 7;
    }

    return res;
}

string bit64BlocksToStr(vector<vector<int>> blocks) {
    string res = "";

    for (vector<int> block : blocks) {
        res += bit64ToStr(block);
    }

    return res;
}

void print(std::vector<int> const &input)
{
	for (int i = 0; i < input.size(); i++) {
		std::cout << input.at(i);
	}
}

/******************************************************************************************
 * DES
 * 
 ******************************************************************************************/
vector<vector<int>> desKeygen(vector<int> key) {
    vector<vector<int>> result;
    vector<int> pc1, pc2;

    // permute with PC1 table
    for (int x : PC1) {
        pc1.push_back(key.at(x - 1));
    }

    // split into to parts, (c, d)
    vector<vector<int>> c;
    vector<vector<int>> d;

    c.push_back(vector<int>(pc1.begin(), pc1.begin() + 28));
    d.push_back(vector<int>(pc1.begin() + 28, pc1.begin() + 56));

    // iteration table
    vector<int> shiftbits;
    vector<int> otherbits;
    for (int count : ITERATION_SHIFT) {
        shiftbits = vector<int>(c.back().begin(), c.back().begin() + count);
        otherbits = vector<int>(c.back().begin() + count, c.back().end());

        otherbits.insert(otherbits.end(), shiftbits.begin(), shiftbits.end());
        c.push_back(otherbits);

        shiftbits = vector<int>(d.back().begin(), d.back().begin() + count);
        otherbits = vector<int>(d.back().begin() + count, d.back().end());

        otherbits.insert(otherbits.end(), shiftbits.begin(), shiftbits.end());
        d.push_back(otherbits);
    }

    // remove n=0
    c.erase(c.begin());
    d.erase(d.begin());

    vector<int> concat;
    for (int n = 0; n < 16; n++) {
        concat.insert(concat.begin(), c[n].begin(), c[n].end());
        concat.insert(concat.end(), d[n].begin(), d[n].end());

        // permute with PC2 table
        for (int x : PC2) {
            pc2.push_back(concat.at(x - 1));
        }
        result.push_back(pc2);
        pc2.clear();

        concat.clear();
    }

    return result;
}

vector<int> desRun(vector<vector<int>> subkeys, vector<int> data) {
    vector<int> ip;

    // permute with IP table, [64]
    for (int x : IP) {
        ip.push_back(data.at(x - 1));
    }

    // split into two parts, left[32] and right[32]
    vector<vector<int>> left;
    vector<vector<int>> right;
    
    left.push_back(vector<int>(ip.begin(), ip.begin() + 32));
    right.push_back(vector<int>(ip.begin() + 32, ip.begin() + 64));

    vector<vector<int>> grps;
    vector<int> f, fxk, sbox, sbCur, fRes, lxf;
    for (int n = 0; n < 16; n++) {
        // right[n] = left[n - 1] XOR f(right[n - 1], subkey[n])
        // f(right[n - 1], subkey[n])

        // expand right[n - 1] from [32] to [48] using E bit-selection
        for (int x : EBITSELECT) {
            f.push_back(right.back().at(x - 1));
        }

        // XOR with subkey, fxk
        for (int x = 0; x < 48; x++) {
            fxk.push_back(XOR(f.at(x), subkeys.at(n).at(x)));
        }

        // seperate into 8 groups of 6 bits
        for (int x = 0; x < 8; x++) {
            grps.push_back(vector<int>(fxk.begin() + (x * 6), fxk.begin() + ((x * 6) + 6)));
        }

        // S-box, into [32]
        int i, j;
        int sbIndex = 0;
        for (vector<int> grp : grps) {
            // determine i and j

            // i is the first and last bit as decimal number, ranged 0 - 3
            i = (grp.at(0) * 2) + (grp.at(5) * 1);

            // j is the middle 4 bits as decimal number, ranged 0 - 15
            j = 0;

            for (int y = 1; y <= 4; y++) {
                j += grp.at(y) * pow(2, 4 - y);
            }

            // get decimal number from S-box[n] using i as row, j as column
            int d = SBOX[sbIndex][i * 16 + j];

            // convert to binary
            sbCur = numToBit(d, 4, true);

            // concatenate into sbox result
            sbox.insert(sbox.end(), sbCur.begin(), sbCur.end());

            sbIndex++;
        }

        // final in f(), permute with P table
        for (int x : PBOX) {
            fRes.push_back(sbox.at(x - 1));
        }

        // right[n] = left[n - 1] XOR f(right[n - 1], subkey[n])
        // XOR with left[n - 1] to get right[n]
        for (int x = 0; x < 32; x++) {
            lxf.push_back(XOR(left.back().at(x), fRes.at(x)));
        }

        // left[n] = right[n - 1]
        left.push_back(right.back());
        // right[n] = ^ (lxf)
        right.push_back(lxf);

        grps.clear();
        f.clear();
        fxk.clear();
        sbox.clear();
        sbCur.clear();
        fRes.clear();
        lxf.clear();
    }
    vector<int> rl;

    // at the last index [16]
    // reverse left and right to right and left
    // concatenate
    rl.insert(rl.end(), right.back().begin(), right.back().end());
    rl.insert(rl.end(), left.back().begin(), left.back().end());

    // permute with inverse IP table, IIP
    vector<int> result;
    for (int x : IIP) {
        result.push_back(rl.at(x - 1));
    }

    return result;
}

vector<int> desEncrypt(vector<int> key, vector<int> data) {
    vector<vector<int>> subkeys = desKeygen(key);
    
    return desRun(subkeys, data);
}

vector<int> desDecrypt(vector<int> key, vector<int> data) {
    vector<vector<int>> subkeys = desKeygen(key);
    reverse(subkeys.begin(), subkeys.end());

    return desRun(subkeys, data);
}

/******************************************************************************************
 * OFB
 * 
 ******************************************************************************************/

// OFB using 2D vector
vector<vector<int>> ofb(vector<vector<int>> data, vector<int> iv, vector<int> key) {
    vector<vector<int>> o; // output
    vector<vector<int>> c; // ciphertext

    // p, plaintext
    for (vector<int> p : data) {
        o.push_back(desEncrypt(key, iv));

        // XOR with plaintext
        c.push_back(blockXOR(p, o.back()));

        // next output
        iv = o.back();
    }

    return c;
}

vector<vector<int>> hexToBin(string hex) {
    vector<vector<int>> p;

    // parse hex
    vector<int> byte;
    vector<int> block;
    int byteCount = 0;
    for (char h : hex) {
        if (h != ' ') {
            for (int x : numToBit(hexToInt(h), 4, true)) {
                byte.push_back(x);
            }
        } else {
            for (int x : byte) {
                block.push_back(x);
            }
            byte.clear();

            byteCount++;
        }
        if (byteCount >= 8) {
            p.push_back(block);
            block.clear();

            byteCount = 0;
        }
        
    }
    for (int x : byte) {
        block.push_back(x);
    }
    p.push_back(block);

    return p;
}

// OFB using string
string ofb(string text, string iv, string key) {
    vector<vector<int>> textblocks, cipherblocks;
    vector<int> i, k;

    k = strToBit(key, 64, true, true);
    i = strToBit(iv, 64, true, true);

    textblocks = strToBit64Blocks(text, true, true);

    cipherblocks = ofb(textblocks, i, k);

    return bit64BlocksToStr(cipherblocks);
}

void printline(int count, char line) {
    for (int x = 0; x < count; x++) {
        cout << line;
    }
    cout << "\n";
}

int main() {
    string key;
    string iv;

    char mode = 0;
    cout << "Select mode (0 - encryption, 1 - decryption): ";
    cin >> mode;
    
    if (mode != '0' && mode != '1') {
        return 0;
    }

    cout << "Insert key: ";
    cin >> key;

    cout << "Insert IV (any): ";
    cin >> iv;

    string ciphertext;
    string plaintext;

    cout << "Insert text: ";
    cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n'); 
    getline(cin, plaintext);

    cout << "\n\n";
    printline(100, '=');

    cout << "Plaintext: " << plaintext << "\n";
    cout << "Key: " << key << "\n";
    cout << "IV: " << iv << "\n";
    cout << "\n";

    vector<vector<int>> textblocks, cipherblocks;
    vector<int> i, k;

    k = strToBit(key, 64, true, true);
    i = strToBit(iv, 64, true, true);

    if (mode == '0') {
        textblocks = strToBit64Blocks(plaintext, true, true);
    } else {
        textblocks = hexToBin(plaintext);
    }

    cipherblocks = ofb(textblocks, i, k);

    ciphertext = bit64BlocksToStr(cipherblocks);

    int bitCount = 0;
    cout << "Result (by block)\n";

    printline(100, '=');
    cout << "Binary:\n\n";
    for (vector<int> b : cipherblocks) {
        for (int x : b) {
            cout << x;

            bitCount++;
            if (bitCount >= 4) {
                cout << " ";
                bitCount = 0;
            }
        }
        cout << "\n";
    }
    printline(100, '=');

    cout << "Hex:\n\n";
    for (vector<int> b : cipherblocks) {
        for (int x = 0; x < 8; x++) {
            cout << byteToHex(vector<int>(b.begin() + (x * 8), b.begin() + (x * 8 + 8))) << " ";
        }
        cout << "\n";
    }
    printline(100, '=');

    cout << "ASCII:\n\n";
    for (vector<int> b : cipherblocks) {
        for (int x = 0; x < 8; x++) {
            cout << byteToChar(vector<int>(b.begin() + (x * 8), b.begin() + (x * 8 + 8))) << " ";
        }
        cout << "\n";
    }
    printline(100, '=');

    cout << "\n\n";

    if (mode == '0') {
        cout << "\n\nCopy the following hex as plaintext for decryption\n";
        string hex = "";
        string h;
        for (vector<int> b : cipherblocks) {
            for (int x = 0; x < 8; x++) {
                h = byteToHex(vector<int>(b.begin() + (x * 8), b.begin() + (x * 8 + 8)));
                hex += h;
                cout << h << " ";
            }
        }
        cout << "\n\nYour key is: " << key;
        cout << "\nYour initial vector is: " << iv << "\n\n";
    }

    if (mode == '1') {
        cout << "Decrypted text: " << bit64BlocksToStr(cipherblocks) << "\n\n";
    }

    char exit;
    cout << "Enter 0 to exit or enter any key to continue\n";
    cin >> exit;

    if (exit != '0') {
        main();
    }

    return 0;
}