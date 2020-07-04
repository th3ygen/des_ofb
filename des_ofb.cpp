#include <iostream>
#include <string>
#include <math.h>
#include <vector>
#include <bitset>

#include "des_data_table.h"
#include "des_key_table.h"

using namespace std;

#define TOTAL_ROUND 16

int XOR(int a, int b) {
    if (a == b) {
        return 0;
    }
    return 1;
}

vector<int> charToBit(char c, int size, bool pad, bool cap) {
    int n = int(c);

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
        res += char(d);
        d = 0;
        p = 7;
    }

    return res;
}
/* 

void keygen(string keyString, int subKey[16][48]) {
    // PC1 and PC2, 56bits
    int pc1[56];

    int k[64];
    ascii2Bit64(keyString, k);

    int kCount = 0;
    for (int x : k) {
        cout << x;
        kCount++;
    }
    cout << " -> " << kCount << "bits (K)";

    cout << "\n";

    // Permutation choice 1
    int col = 0;
    for (int x : PC1) {
        pc1[col++] = k[x - 1];
    }

    for (int x : pc1) {
        cout << x;
    }
    cout << " -> " << col << "bits (K+) \n";

    // split to left(c) [28] and right(d) [28]
    int c[TOTAL_ROUND + 1][28];
    int d[TOTAL_ROUND + 1][28];

    int pc1x = 0;
    for (int x = 0; x < 28; x++) c[0][x] = pc1[pc1x++];
    for (int x = 0; x < 28; x++) d[0][x] = pc1[pc1x++];

    for (int b : c[0]) {
        cout << b;
    }
    cout << " -> c0 \n";
    for (int b : d[0]) {
        cout << b;
    }
    cout << " -> d0 \n";

    // iteration shift table
    // n=0 is already defined
    for (int n = 1; n <= TOTAL_ROUND; n++) {
        for (int x = 0; x < 28 - ITERATION_SHIFT[n - 1]; x++) {
            c[n][x] = c[n - 1][x + ITERATION_SHIFT[n - 1]];
            d[n][x] = d[n - 1][x + ITERATION_SHIFT[n - 1]];
        }
        for (int x = 28 - ITERATION_SHIFT[n - 1]; x < 28; x++) {
            // x % (28 - shiftCount) = 0 or = 0,1
            int s = x % (28 - ITERATION_SHIFT[n - 1]);
            c[n][x] = c[n - 1][s];
            d[n][x] = d[n - 1][s];
        }
    }

    for (int n = 1; n <= TOTAL_ROUND; n++) {
        for (int b : c[n]) {
            cout << b;
        }
        cout << " -> c" << n << "\n";
        for (int b : d[n]) {
            cout << b;
        }
        cout << " -> d" << n << "\n";
    }

    // key is the pc1 of left(c) and right(d) concatenation
    int cd[16][56];
    for (int n = 0; n < TOTAL_ROUND; n++) {
        // c, [0 - 27]
        for (int x = 0; x < 28; x++) {
            cd[n][x] = c[n + 1][x];
        }
        // d, [28, 55];
        for (int x = 0; x < 28; x++) {
            cd[n][x + 28] = d[n + 1][x];
        }

        // Permute with PC2 table
        int col = 0;
        for (int b : PC2) {
            subKey[n][col++] = cd[n][b - 1];
        }
    }
}

void desEncrypt(string txt, string keyString, int pc1[64]) {
    // DES subkeys
    int subKey[16][48];
    keygen(keyString, subKey);

    int m[64];
    ascii2Bit64(txt, m);

    // permute with IP table
    int ip[64];

    int col = 0;
    for (int b : IP) {
        ip[col++] = m[b - 1];
    }

    // divide into two parts [32], left and right
    int left[TOTAL_ROUND + 1][32];
    int right[TOTAL_ROUND + 1][32];

    int ipx = 0;
    for (int x = 0; x < 32; x++) left[0][x] = ip[ipx++];
    for (int x = 0; x < 32; x++) right[0][x] = ip[ipx++];

    // data block and key operation iteration
    for (int n = 1; n < TOTAL_ROUND + 1; n++) {
        // left[n] = right[n - 1]
        copy(right[n - 1], end(right[n - 1]), left[n]);

        // right[n] = left[n - 1] XOR f(right[n - 1], subKey[n])

        // first, calc f(right[n - 1], subKey[n])
        // f, expand right[n - 1] from [32] to [48] using E bit-selection
        int rPrev[48];
        col = 0;
        for (int b : EBITSELECT) {
            rPrev[col++] = right[n - 1][b];
        }

        // right[n - 1] XOR key, rxk
        int rxk[48];
        for (int x = 0; x < 48; x++) {
            // right[n - 1] XOR k[n]
            rxk[x] = XOR(rPrev[x], subKey[n - 1][x]);
        }

        // seperate into 8 groups of 6 bits Bi
        int B[8][6];
        col = 0;
        for (int x = 0; x < 8; x++) {
            for (int y = x * 6; y < (x * 6) + 6; y++) {
                B[x][y] = rxk[col++];
            }
        }

        // S-box
        int s[8][4];
        // iterate through the groups
        for (int x = 0; x < 8; x++) {
            // determine i and j

            // i is the first and last bit as decimal number, ranged 0 - 3
            int i = (B[x][0] * 1) + (B[x][5] * 2);

            // j is the middle 4 bits as decimal number, ranged 0 - 15
            int j = 0;
            for (int y = 0; y < 4; y++) {
                j += B[x][y] * pow(2, y);
            }

            // get decimal number from S-box[n] using i as row, j as column
            int d = SBOX[x][i * 16 + j];

            // convert to binary
            for (int l = 3; l > -1; l--) {
                s[x][l] = d % 2;
                d /= 2;
            }
        }

        // concatenate the s, pc1 in 32bit
        int scon[32];

        col = 0;
        for (int x = 0; x < 8; x++) {
            for (int y : s[x]) {
                scon[col++] = s[x][y];
            }
        }

        // final stage for the function, permute with P table
        int fRes[32];
        col = 0;
        for (int b : PBOX) {
            fRes[col++] = scon[b - 1];
        }

        // right[n] = left[n - 1] XOR f(right[n - 1], subKey[n])

        // left[n - 1] XOR f, lxf
        int lxf[32];
        for (int x = 0; x < 32; x++) {
            lxf[x] = XOR(left[n - 1][x], fRes[x]);
        }
        copy(lxf, end(lxf), right[n]);
    }

    // at the last index [16]
    // reverse left and right, rl
    int rl[64];
    col = 0 ;
    for (int x = 0; x < 32; x++) rl[col++] = right[15][x];
    for (int x = 0; x < 32; x++) rl[col++] = left[15][x];

    // permute with inverse IP table
    col = 0;
    for (int b : IIP) {
        pc1[col++] = rl[b - 1];
    }
}

void ofb(int o[64], int m[64]) {

}

void decrypt(string cipher, string key, string text) {

} */
void print(std::vector<int> const &input)
{
	for (int i = 0; i < input.size(); i++) {
		std::cout << input.at(i);
	}
    cout << "\n";
}
vector<vector<int>> desKeygen(vector<int> key) {
    vector<vector<int>> result;
    vector<int> pc1, pc2;

    // permute with PC1 table
    for (int x : PC1) {
        pc1.push_back(key.at(x - 1));
    }

    // split into to parts, (c, b)
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
        concat.insert(concat.end(), c[n].begin(), c[n].end());

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

vector<int> desEncrypt(vector<int> key, vector<int> data) {
    vector<vector<int>> subkeys = desKeygen(key);
    
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
        // left[n] = right[n - 1]
        left.push_back(right.back());

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

        // S-box
        int i, j;
        int sbIndex = 0;
        for (vector<int> grp : grps) {
            // determine i and j

            // i is the first and last bit as decimal number, ranged 0 - 3
            i = (grp.at(0) * 1) + (grp.at(5) * 2);

            // j is the middle 4 bits as decimal number, ranged 0 - 15
            j = 0;
            for (int y = 0; y < 4; y++) {
                j += grp.at(y) * pow(2, y);
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

void encrypt(string iv, string text, string key) {
    vector<int> k = strToBit(key, 64, true, true);

    int size = text.size();
    int l = 0;

    int totalBlocks = ceil(float(size) / 8);
    // split plain text into blocks of 64 bits
    vector<vector<int>> textBlocks;
    for (int x = 0; x < totalBlocks; x++) {
        int s = 8;
        if (x == totalBlocks - 1) {
            s = size % 8;
        }

        textBlocks.push_back(strToBit(text.substr(x * 8, s), 64, true, false));
    }

    cout << "Text blocks\n";
    for (int x = 0; x < totalBlocks; x++) {
        int s = 8;
        if (x == totalBlocks - 1) {
            s = size % 8;
        }

        cout << text.substr(x * 8, s) << " -> ";
        print(textBlocks.at(x));
    }

    cout << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n";

    // get output for each round
    // then XOR with plain text
    vector<vector<int>> output;

    // initial vector
    output.push_back(desEncrypt(k, strToBit(iv, 64, true, true)));

    for (int x = 0; x < totalBlocks - 1; x++) {
        output.push_back(desEncrypt(k, output.back()));
    }

    // XOR with plaintext
    vector<vector<int>> cipherBlocks;
    vector<int> c;
    for (int x = 0; x < totalBlocks; x++) {
        for (int y = 0; y < 64; y++) {
            c.push_back(XOR(textBlocks.at(x).at(y), output.at(x).at(y)));
        }
        cipherBlocks.push_back(c);

        c.clear();
    }

    cout << "Cipher blocks\n";
    for (int x = 0; x < totalBlocks; x++) {
        cout << bit64ToStr(cipherBlocks.at(x)) << " -> ";
        print(cipherBlocks.at(x));
    }
    /* vector<int> output = ;
    cout << "\n";
    for (int x : output) {
        cout << x;
    } */
}

int main() {
    string message = "Aslam world!";
    string key = "del mode";

    cout << "Plaintext: " << message << "\n";
    cout << "Key: " << key << "\n";

    string iv = "crypto69";
    cout << "Initial vector: " << iv << "\n";

    cout << "\n";

    encrypt(iv, message, key);

    return 0;
}