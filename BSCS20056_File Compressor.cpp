#include <iostream>
#include <string>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <bitset>
using namespace std;

struct Node
{
    char ch;
    int freq;
    Node* left, * right;
};

Node* getNode(char ch, int freq, Node* left, Node* right)
{
    Node* node = new Node();

    node->ch = ch;
    node->freq = freq;
    node->left = left;
    node->right = right;

    return node;
}

struct comp
{
    bool operator()(const Node* l, const Node* r) const
    {
        return l->freq > r->freq;
    }
};

bool isLeaf(Node* root) {
    return root->left == nullptr && root->right == nullptr;
}

void encode(Node* root, vector<bool> str, unordered_map<char, vector<bool>>& huffmanCode)
{
    if (root == nullptr) {
        return;
    }

    if (isLeaf(root)) {
        huffmanCode[root->ch] = (!str.empty()) ? str : vector<bool>(true);
    }
    str.push_back(false);
    encode(root->left, str, huffmanCode);
    str.pop_back();
    str.push_back(true);
    encode(root->right, str, huffmanCode);
}

template<std::size_t N>
void reverse(std::bitset<N>& b, int si = 0, int ei = -1)
{
    for (std::size_t i = 0; i < ((ei != -1) ? ei : N) / 2; ++i)
    {
        bool t = b[i];
        b[i] = b[((ei != -1) ? ei : N) - i - 1];
        b[((ei != -1) ? ei : N) - i - 1] = t;
    }
}

void decode(Node* root, int& index, string str)
{
    if (root == nullptr) {
        return;
    }

    if (isLeaf(root))
    {
        cout << root->ch;
        return;
    }

    index++;

    if (str[index] == '0') {
        decode(root->left, index, str);
    }
    else {
        decode(root->right, index, str);
    }
}

void compressfael(fstream& rdr)
{
    if (!rdr) {
        return;
    }

    unordered_map<char, int> freq;
    while (rdr.good())
    {
        char ch = rdr.get();
        if (ch < 0)
            break;
        freq[ch]++;
    }

    priority_queue<Node*, vector<Node*>, comp> pq;
    for (auto pair : freq) 
    {
        pq.push(getNode(pair.first, pair.second, nullptr, nullptr));
    }

    while (pq.size() > 1)
    {
        Node* left = pq.top();
        pq.pop();
        Node* right = pq.top();
        pq.pop();

        int sum = left->freq + right->freq;
        pq.push(getNode('\0', sum, left, right));
    }

    Node* root = pq.top();

    unordered_map<char, vector<bool>> huffmanCode;
    vector<bool> str;
    encode(root, str, huffmanCode);

    ofstream wtr("codes.txt");
    for (auto pair : huffmanCode) 
    {
        wtr << pair.first << " ";
        for (int i = 0; i < pair.second.size(); i++)
        {
            wtr << (pair.second[i] ? "1" : "0");
        }
        wtr << endl;
    }
    wtr.close();
    fstream bina("truecompressed.bin", ios::out | ios::binary);
    rdr.clear();
    rdr.seekg(0, ios::beg);
    char c;
    bitset<8> s;
    int i=0;
    while (rdr.good())
    {
        c = rdr.get();
        if (c > 0)
        {
            for (int j = 0; j < huffmanCode[c].size(); j++)
            {
                s[i++] = huffmanCode[c].at(j);
                if (i == 8)
                {
                    reverse(s);
                    char n = s.to_ulong();
                    bina.write(&n,sizeof(n));
                    i = 0;
                    s.reset();
                }
            }
        }
    }
    if (i > 0)
    {
        reverse(s,0,i);
        reverse(s);
        char n = s.to_ulong();
        bina.write(&n, sizeof(n));
    }    
}

void decodefael(string filename)
{
    string str;
    bitset<8> first;
    std::ifstream input("truecompressed.bin", std::ios::binary);

    char n;
    int count;
    input.seekg(0, ios::end);
    count = input.tellg();
    input.seekg(0, ios::beg);

    int k = 0;
    while (k++ < count)
    {
        input.read(&n, sizeof(n));
        if (n == 26)
        {
            break;
        }
        first = n;
        str += first.to_string();
    }

    unordered_map<string, char> codes;
    ifstream rdr("codes.txt");
    char s0;
    char s1[20];
    while (rdr)
    {
        s0 = rdr.get();
        if (s0 < 0)
        {
            break;
        }
        rdr.get();
        rdr.getline(s1,20);
        codes[s1] = s0;
    }
    rdr.close();

    ofstream wtr("uncompressed.txt");

    for (int p = 0; p < str.size(); p++)
    {
        string st = str.substr(0, p + 1);
        if (codes.find(st)!=codes.end())
        {
            cout << codes[st];
            wtr << codes[st];
            str.erase(0, p + 1);
            p = -1;
        }
    }
}

int main()
{
    fstream rdr("file.txt", ios::in);
    cout << "1. To Compress" << endl;
    cout << "2. To Uncompress Last file" << endl;
    cout << "> ";
    int i;
    cin >> i;
    switch (i)
    {
    case 1:
        compressfael(rdr);
        cout << "Successfully Compressed";
        break;
    case 2:
        decodefael("compressed.bin");
        break;
    default:
        break;
    }

    return 0;
}