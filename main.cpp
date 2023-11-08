#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <algorithm>
#include <iomanip>
#include <bitset>
#include <sstream>
#include <omp.h>
#include <cstdlib>
#include <ctime>

const uint32_t HASH_CODE[] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

uint32_t Mix(uint32_t a, uint32_t b, uint32_t c) {
    uint32_t temp1 = b >> 6 | b << (32 - 6);
    uint32_t temp2 = b >> 11 | b << (32 - 11);
    uint32_t temp3 = b >> 25 | b << (32 - 25);
    uint32_t s1 = temp3 ^ temp2 ^ b;

    temp1 = a >> 2 | a << (32 - 2);
    temp2 = a >> 13 | a << (32 - 13);
    temp3 = a >> 22 | a << (32 - 22);
    uint32_t s0 = temp3 ^ temp2 ^ temp1;

    return a + s0 + b + s1 + c;
}

std::string customHash(const std::string& input) {
    uint32_t hash[8];

    memcpy(hash, HASH_CODE, sizeof(HASH_CODE));

    for (char c : input) {
        for (int i = 0; i < 8; ++i) {
            hash[i] = Mix(hash[i], hash[(i + 1) % 8], static_cast<uint32_t>(c));
        }
    }

    char result[65];
    snprintf(result, sizeof(result),
             "%08x%08x%08x%08x%08x%08x%08x%08x",
             hash[0], hash[1], hash[2], hash[3],
             hash[4], hash[5], hash[6], hash[7]);

    return result;
}

class User {
public:
    std::string name;
    std::string public_key;
    double balance;

    User(const std::string& name, const std::string& public_key, double balance)
        : name(name), public_key(public_key), balance(balance) {}
};

class Transaction {
public:
    std::string id;
    std::string sender;
    std::string receiver;
    double amount;

    Transaction(std::string id, const std::string& sender, const std::string& receiver, double amount)
        : id(id), sender(sender), receiver(receiver), amount(amount) {}
};

std::vector<User> readUsersFromFile(const std::string& filename) {
    std::vector<User> users;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string name, public_key;
            double balance;
            iss >> name >> public_key >> balance;
            users.emplace_back(name, public_key, balance);
        }
        file.close();
    } else {
        std::cerr << "Failed to open " << filename << std::endl;
    }

    return users;
}

std::vector<Transaction> readTransactionsFromFile(const std::string& filename) {
    std::vector<Transaction> transactions;
    std::ifstream file(filename);

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string id;
            std::string sender, receiver;
            double amount;
            iss >> id >> sender >> receiver >> amount;
            transactions.emplace_back(id, sender, receiver, amount);
        }
        file.close();
    } else {
        std::cerr << "Failed to open " << filename << std::endl;
    }

    return transactions;
}

class Block {
public:
    int index;
    std::string previous_hash;
    time_t timestamp;
    int nonce;
    std::vector<Transaction> transactions;
    std::string hash;

    Block(int index, const std::string& previous_hash, const std::vector<Transaction>& transactions)
        : index(index), previous_hash(previous_hash), transactions(transactions) {
        timestamp = static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
        nonce = 0;
        hash = mineBlock();
    }

private:
    std::string mineBlock() {
        std::string target(2, '0');
        std::string data = std::to_string(index) + previous_hash + std::to_string(timestamp) + std::to_string(nonce);

        for (const Transaction& transaction : transactions) {
            data += transaction.id + transaction.sender + transaction.receiver + std::to_string(transaction.amount);
        }

        std::string hash;
        do {
            nonce++;
            hash = customHash(data + std::to_string(nonce));
        } while (hash.substr(0, 2) != target);

        return hash;
    }
};

std::vector<Transaction> selectRandomTransactions(const std::vector<Transaction>& allTransactions, int count) {
    if (count >= allTransactions.size()) {
        return allTransactions; 
    }

    std::vector<Transaction> randomTransactions;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, allTransactions.size() - 1);

    for (int i = 0; i < count; ++i) {
        int randomIndex = dist(gen);
        randomTransactions.push_back(allTransactions[randomIndex]);
    }

    return randomTransactions;
}

class Blockchain {
public:
    Blockchain() {
        chain_.push_back(createGenesisBlock());
    }

    void addBlock(const std::vector<Transaction>& transactions) {
        int index = chain_.size();
        const std::string& previous_hash = chain_.back().hash;
        chain_.push_back(Block(index, previous_hash, transactions));
    }

    void printChain() {
        for (const Block& block : chain_) {
            std::cout << "Block Index: " << block.index << std::endl;
            std::cout << "Previous Hash: " << block.previous_hash << std::endl;
            std::cout << "Timestamp: " << ctime(&block.timestamp) << std::endl;
            std::cout << "Nonce: " << block.nonce << std::endl;
            std::cout << "Hash: " << block.hash << std::endl;
            std::cout << "Transactions:" << std::endl;
        }
    }

private:
    std::vector<Block> chain_;

    Block createGenesisBlock() {
        return Block(0, "0", std::vector<Transaction>());
    }
};

int main() {
    Blockchain blockchain;

    std::vector<User> users = readUsersFromFile("users.txt");

    std::vector<Transaction> allTransactions = readTransactionsFromFile("transactions.txt");
     std::vector<Transaction> randomTransactions = selectRandomTransactions(allTransactions, 100);
     
    blockchain.addBlock(randomTransactions);

    blockchain.printChain();

    return 0;
}
