#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <unistd.h>

using namespace std;

struct inst_R {     // R-instruction struct, only contains name and funct part
    string name;
    string funct;
};

struct inst_I {     // I-instruction struct, only contaions name and op part
    string name;
    string op;
};

struct inst_J {     // J-instruction struct, only contains name and op part
    string name;
    string op;
};

struct data {       // data struct for .data
    string name;
    vector<string> content;
    int addr;
};

// instruction lists
struct inst_R Rtype[10] = {         // R-instructions
    {"addu", "100001"}, {"and", "100100"}, {"jr", "001000"}, 
    {"nor", "100111"}, {"or", "100101"}, {"sltu", "101011"}, 
    {"sll", "000000"}, {"srl", "000010"}, {"subu", "100011"}, 
    {"", ""}};
struct inst_I Itype[10] = {         // I-instructions
    {"addiu", "001001"}, {"andi", "001100"}, {"beq", "000100"}, 
    {"bne", "000101"}, {"lui", "001111"}, {"lw", "100011"}, 
    {"ori", "001101"}, {"sltiu", "001011"}, {"sw", "101011"}, 
    {"", ""}};
struct inst_J Jtype[3] = {{"j", "000010"}, {"jal", "000011"}, {"", ""}}; // J-instructions

vector<struct data> v_data;         // vector for .data
vector<pair<int, string> > store;   // vector for .text part which needs to fill in later
vector<pair<string, int> > dict;    // vector for .text part which stores block names and addresses

// instruction name lists
string inst[21] = {"addu", "and", "jr", "nor", "or", "sltu", "sll", "srl", 
    "subu", "addiu", "andi", "beq", "bne", "lui", "lw", "ori", "sltiu", "sw", 
    "j", "jal", "la"};

/*
 * size_change : change the length of a bin string 'num' to 'size'
 */
string size_change(string num, int size){
    int old = num.length();     // old size of num
    int n = size - old;         // difference between new and old size
    int i;
    string k = "";

    if (n < 0) {                // if num need to shrink
        return num.substr(-n);
    }
    else if(n > 0) {            // if num need to extend
        for (i = 1 ; i <= n ; i++)
            k += num[0];
        return k+num;
    }
    else                        // if the size is same
        return num;
}

/*
 *  is_number : check the string and return 1 if str is number
 */
int is_number(string str)
{
    int flag = (str[0] == '-') ? 1 : 0;     // in case it is a negative number
    int i;
    for (i = flag; i < str.size(); ++i)
        if (str[i] < '0' || str[i] > '9')
            return 0;
    return 1;
}

/*
 *  ch_erase : erase character c in string
 */
string ch_erase(string line, char c)
{
    string::iterator iter;
    int find = 0;

    for (iter = line.begin(); iter != line.end(); ++iter, ++find)
        if (*iter == c)
            line.erase(find, 1);
    return line;
}

/*
 *  int2str : change int to string
 */
string int2str(int n)
{
    stringstream s;
    s << n;
    return s.str();
}

/*
 *  dec2bin : change decimal number into length n binary numbered string
 */
string dec2bin(int dec, int n)
{
    int i;
    string s;

    for (i = n - 1; i >= 0; --i)            // divide by 2 and get the binary number
        s.append(int2str((dec >> i) & 1));
    return s;
}

/*
 *  hex2bin : change hex string number into length n binary numbered string
 */
string hex2bin(string str, int n)
{
    int i;
    string s;

    for (i = 0; i < 8 - str.size(); ++i)
        s.append("0000");
    for (i = 0; i < str.size() ; ++i)
    {
        if(str[i] >= '0' && str[i] <= '9')
            s.append(dec2bin(str[i] - 48, 4));
        else
            s.append(dec2bin(str[i] - 87, 4));
    }
    return size_change(s, n);
}

/*
 *  str2bin : get string and change into length n binary string number
 */
string str2bin(string str, int n)
{
    int idx = 0;
    string s = "";

    if (str[0] == '$')
        return dec2bin(atoi(str.substr(1, str.size() - 1).c_str()), n);

    else if (str[str.size() - 1] == ')')
    {
        while (str[idx] != '(')
            ++idx;
        if (str.substr(0, 2).compare("0x") == 0)
            s.append(hex2bin(str.substr(2), 16));
        else
            s.append(dec2bin(atoi(str.substr(0, idx).c_str()), 16));
        s.append(dec2bin(
                    atoi(
                        str.substr(idx + 2, str.size() - idx - 3).c_str()), 5));
        return s;
    }

    else if (str.substr(0, 2).compare("0x") == 0)
        return hex2bin(str.substr(2), n);
    else if (is_number(str) == 1)
        return dec2bin(atoi(str.c_str()), n);
    else
        return s;
}

/*
 *  parsing : parse the line and return the parsed lines
 */
vector<string> parsing(string line){
    vector<string> par_vec;
    string token;
    stringstream strstr(ch_erase(line, ','));

    while (strstr >> token)
        par_vec.push_back(token);

    return par_vec;
}

/*
 *  data_trnaslate : translate .data part to binary numbers
 */
string data_translate(vector<string> v)
{
    struct data ret;
    string data_bin = "";

    if (v[1].compare(".word") == 0)                         // store the data's address and content
    {
        ret.name = v[0].substr(0, v[0].size() - 1);
        ret.content.push_back(v[2]);
        if (v_data.empty())                 // .data starts with 0x10000000
            ret.addr = 0x10000000;
        else                                // assign address
            ret.addr = v_data[v_data.size() - 1].addr
                + (4 * v_data[v_data.size() - 1].content.size());
        v_data.push_back(ret);
    }
    else                                                    // if a new data is declared
        v_data[v_data.size() - 1].content.push_back(v[1]);


    for (int p = 0 ; p < v_data.size() ; p++){              // translate the content to binary numbers
        for (int u = 0 ; u < v_data[p].content.size() ; u++){
            data_bin += str2bin(v_data[p].content[u], 32);
        }
    }

    return data_bin;
}

/*
 *  translate : translate MIPS assembly code into binary numbers
 */
string translate(string old, vector<string> v)
{
    int idx = 0;
    string bin, tmp;
    vector<struct data>::iterator it;

    for (idx = 0; idx < 21; ++idx)
        if(v[0].compare(inst[idx]) == 0)
            break;

    if (idx < 9) // R-type
    {
        bin = "000000";
        if (idx == 2) // jr : only have rs
        {
            bin += str2bin(v[1], 5);
            bin += "000000000000000";
        }
        else if (idx == 6 || idx == 7) // sll or slr : shamt is present, not rs
        {
            bin += "00000";
            bin += str2bin(v[2], 5);
            bin += str2bin(v[1], 5);
            bin += str2bin(v[3], 5);
        }
        else
        {
            bin += str2bin(v[2], 5);
            bin += str2bin(v[3], 5);
            bin += str2bin(v[1], 5);
            bin += "00000";
        }
        bin += Rtype[idx].funct;
        (old) += bin;
    }
    else if (idx < 18) // I-type
    {
        bin = Itype[idx - 9].op;
        if (idx == 11 || idx == 12) // beq or bne : we think about order of rs and rt
        {
            bin += str2bin(v[1], 5);
            bin += str2bin(v[2], 5);
            bin += "0000000000000000";
            store.push_back(make_pair(old.size() + 16, v[3]));
        }

        else if (idx == 13) // lui : have not rs
        {
            bin += "00000";
            bin += str2bin(v[1], 5);
            bin += str2bin(v[2], 16);
        }

        else if (idx == 14 || idx == 17) // lw and sw : must split M[rs + SEI] -> rs/SEI 
        {
            tmp = str2bin(v[2], 21);
            bin += tmp.substr(16, 5);
            bin += str2bin(v[1], 5);
            bin += tmp.substr(0, 16);
        }

        else
        {
            bin += str2bin(v[2], 5);
            bin += str2bin(v[1], 5);
            bin += str2bin(v[3], 16);
        }
        (old) += bin;
    }

    else if (idx == 18 || idx == 19) // J-Type
    {
        bin += Jtype[idx - 18].op;
        bin += "00000000000000000000000000";
        store.push_back(make_pair((old).size() + 6, v[1]));
        (old) += bin;
    }

    else if (idx == 20) // la
    {
        for (it = v_data.begin(); it != v_data.end(); ++it)
            if (v[2].compare(it->name) == 0)
                break;

        bin += Itype[4].op;
        bin += "00000";
        bin += str2bin(v[1], 5);
        bin += dec2bin(((it->addr) / (1 << 16)), 16);

        if (((it->addr) & 0xffff) != 0x0)   // if la need to be represented into 2 lines
        {
            bin += Itype[6].op;
            bin += str2bin(v[1], 5);
            bin += str2bin(v[1], 5);
            bin += dec2bin(((it->addr) & 0xffff), 16);
        }
        (old) += bin;
    }

    else // block name
        dict.push_back(
                make_pair(v[0].substr(0, v[0].size() - 1),
                    (0x400000 + (old.size() / 8))));
    return old;
}

/*
 *  main
 */
int main(int argc, char *argv[]){
    FILE *fd;               // input file
    FILE *fd1;              // output file
    string input[200];      // list of input string
    string output = "";     // string of translated binary numbers
    string data_bin = "";   // .data part's binary numbers
    string front = "";      // front two lines which tells the .text sixe and .data size
    char buf[1024];         // buffer
    int i = 0, j = 0;
    int idx;
    int n;
    int address;
    string data[50];        // list of .data part's lines


    if (argc != 2)          // if the input is wrong
    {
        printf("Argument number wrong");
        exit(0);
    }

    fd = fopen(argv[1], "r");

    while(!feof(fd))    //get the input by lines
    {
        fgets(buf, 255, fd);
        input[i] = buf;
        i++;
    }

    while(input[j + 1].find(".text") == string::npos)   //get .data part only
    {
        data[j] = input[j + 1];
        j++;
    }

    for (idx = 0; idx < j; ++idx)                       //  parse .data and translate, and get the address
        data_bin = data_translate(parsing(data[idx]));

    for (idx = j + 2; idx < i-1; ++idx)                 // parse .text and translate
        output = translate(output, parsing(input[idx]));

    front += dec2bin((output.size()/8), 32);            // calculate the size of .text and .data
    front += dec2bin((data_bin.size()/8), 32);          // and make front two lines

    i = 0;
    j = 0;
    for (i ; i < store.size() ; i++)                    // fill in the blanks by using the vectors
    {                                                   // declared above
        n = 32 - (store[i].first % 32);
        for (j ; j < dict.size() ; j++)         // find the missing address
        {
            if (dict[j].first == store[i].second)
            {
                address = dict[j].second;
            }
        }
        j=0;

        if ( n == 16 )                          // if it is I-type
        {                                       // need to fill in ony 16 bits relatively
            address = (address - 0x400000)/4;
            output.replace(store[i].first, 16, dec2bin((address - ((store[i].first)/32)) - 1, 16));
        }
        else                                    // if it is J-type
        {                                       // need to fill in 26 bits by real address
            address /= 4;
            output.replace( store[i].first, 26, dec2bin(address, 26));
        }

    }

    output = front + output + data_bin;         // add all the parts togather into one sitrng

    fclose(fd);

    fd1 = fopen("output.o", "w");               // open the output file
    fprintf(fd1, "%s", output.c_str());         // write in the output
    fclose(fd1);

    return 0;
}
