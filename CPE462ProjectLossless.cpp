#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <vector>
#include <string>
#include <functional>

using namespace std;

//Make huffman tree node
struct Node {
    unsigned char ch; int freq;
    Node *left, *right;
    Node(unsigned char c, int f, Node* l = nullptr, Node* r = nullptr): 
    ch(c), freq(f), left(l), right(r) {}
};

//Comparator for min-heap (priority queue)
struct Compare {
    bool operator()(Node* l, Node* r) {
        return (l->freq) > (r->freq);
    }
};

//Build Huffman tree
Node* buildTree(const map<unsigned char, int>& freq) {
    priority_queue<Node*, vector<Node*>, Compare> prioqueue;

    for (const auto& pair : freq) {
        prioqueue.push(new Node(pair.first, pair.second));
    }

    while (prioqueue.size() > 1) {
        Node* left = prioqueue.top(); 
        prioqueue.pop();
        Node* right = prioqueue.top(); 
        prioqueue.pop();
        Node* parent = new Node(0, (left->freq) + (right->freq), left, right);
        prioqueue.push(parent);
    }

    return prioqueue.top();
}

//Generate Huffman codes
void generateCodes(Node* root, string code, map<unsigned char, string>& codes) {
    if (!root){
        return;
    } 

    if ((!root->left) && (!root->right)) {
        codes[root->ch] = code;
        return;
    }

    generateCodes(root->left, code + "0", codes);
    generateCodes(root->right, code + "1", codes);
}

//Compress the image file
void compress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);

    if (!in) {
        cout << "Can't open input file!" << inputFile << endl;
        return;
    }

    //Calc frequencies
    map<unsigned char, int> freq;
    unsigned char ch;

    while (in.read(reinterpret_cast<char*>(&ch), 1)) {
        freq[ch]++;
    }

    in.close();

    if (freq.empty()) {
        cout << "Empty file" << endl;
        return;
    }

    //Build tree and codes
    Node* root = buildTree(freq);
    map<unsigned char, string> codes;
    generateCodes(root, "", codes);

    //Write to output file
    ofstream out(outputFile, ios::binary);

    if (!out) {
        cout << "Cannot open output file: " << outputFile << endl;
        return;
    }

    //Write header: # of unique characters, char & freq pairs
    int unique = freq.size();
    out.write((char*)&unique, sizeof(int));

    for (const auto& pair : freq) {
        out.write(reinterpret_cast<const char*>(&pair.first), 1);
        out.write((char*)&pair.second, sizeof(int));
    }

    //Encode & write data
    ifstream in2(inputFile, ios::binary);
    string b;

    while (in2.read(reinterpret_cast<char*>(&ch), 1)) {
        b += codes[ch];

        while (b.size() >= 8) {
            char byte = 0;

            for (int i = 0; i < 8; ++i) {
                byte = (byte << 1) | (b[i] - '0');
            }

            out.write(&byte, 1);
            b = b.substr(8);
        }
    }

    
    int pad = 0; //Handle padding for last bits
    if (!b.empty()) {
        pad = 8 - b.size();
        b += string(pad, '0');
        char byte = 0;

        for (int i = 0; i < 8; ++i) {
            byte = (byte << 1) | (b[i] - '0');
        }

        out.write(&byte, 1);
    }

    unsigned char padChar = static_cast<unsigned char>(pad);
    out.write(reinterpret_cast<char*>(&padChar), 1);

    in2.close();
    out.close();

    function<void(Node*)> deleteTree = [&](Node* node) { //Recursive delete to clean up memory
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    };

    deleteTree(root);
}

//Decompress algorithm
void decompress(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    if (!in) {
        cout << "Cannot open input file: " << inputFile << endl;
        exit(1);
    }

    //Read header
    int unique;
    in.read((char*)&unique, sizeof(int));
    map<unsigned char, int> freq;
    for (int i = 0; i < unique; ++i) {
        unsigned char ch;
        int f;
        in.read(reinterpret_cast<char*>(&ch), 1);
        in.read((char*)&f, sizeof(int));
        freq[ch] = f;
    }

    
    Node* root = buildTree(freq); //Rebuilds tree

    //Read padding
    in.seekg(-1, ios::end);
    unsigned char padChar;
    in.read(reinterpret_cast<char*>(&padChar), 1);
    int pad = static_cast<int>(padChar);

    //Calculate positions as long long for arithmetic
    long long headerSize = sizeof(int) + unique * (1 + sizeof(int));
    in.seekg(0, ios::end);
    long long fileSize = static_cast<long long>(in.tellg());
    long long dataEnd = fileSize - 1;  // If I don't do this, it breaks I think it's cause of the padding byte?
    long long dataStart = headerSize;

    
    in.seekg(dataStart); //Reset to after header

    //Decoding
    ofstream out(outputFile, ios::binary);
    Node* current = root;
    unsigned char byte;
    while (in.good()) {
        long long currentpos = static_cast<long long>(in.tellg());
        if (currentpos >= dataEnd) break;

        in.read(reinterpret_cast<char*>(&byte), 1);
        if (!in.good()) break;

        int processbits = 8;
        if (currentpos + 1 >= dataEnd && pad > 0) {
            processbits = 8 - pad;
        }

        for (int i = 7; i >= (8 - processbits); --i) {
            char bit = ((byte >> i) & 1) ? '1' : '0';
            current = (bit == '0') ? current->left : current->right;
            if (!current->left && !current->right) {
                out.write(reinterpret_cast<const char*>(&current->ch), 1);
                current = root;
            }
        }
    }

    in.close();
    out.close();

    //Clean up tree
    function<void(Node*)> deleteTree = [&](Node* node) {
        if (!node) return;
        deleteTree(node->left);
        deleteTree(node->right);
        delete node;
    };
    deleteTree(root);

    cout << "Done!" << outputFile << endl;
}

int main() {
    int choice;
    string image;  
    string compressed = "compressed.huf";
    string compfile;
    string decompressed = "original.jpg";

    cout << "Would you like to 1. Compress or 2. Decompress?" << "\n";
    cin >> choice;

    switch(choice){
        case 1:
        cout << "Put the path to your image that you want compressed:" << "\n";
        cin >> image;
        compress(image, compressed); 
        break;
        case 2:
        cout << "Put the path to your compressed image that you want decompressed:" << "\n";
        cin >> compfile;
        decompress(compfile, decompressed);
        break;
        default:
        cout << "Invalid Choice! Exiting program." << "\n";
        exit(0);
    }

    return 0;
}