#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <random>
#include <algorithm>

const uint32_t HASH_CODE[] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

class User {
public:
    User(const std::string& name, const std::string& public_key, int balance)
        : name_(name), public_key_(public_key), balance_(balance) {}

    const std::string& getName() const {
        return name_;
    }

    const std::string& getPublicKey() const {
        return public_key_;
    }

    int getBalance() const {
        return balance_;
    }

private:
    std::string name_;
    std::string public_key_;
    int balance_;
};

class Transaction {
public:
    Transaction(const std::string& sender, const std::string& receiver, int amount)
        : sender_(sender), receiver_(receiver), amount_(amount) {
        generateTransactionID();
    }

    const std::string& getTransactionID() const {
        return transaction_id_;
    }

    const std::string& getSender() const {
        return sender_;
    }

    const std::string& getReceiver() const {
        return receiver_;
    }

    int getAmount() const {
        return amount_;
    }

private:
    std::string transaction_id_;
    std::string sender_;
    std::string receiver_;
    int amount_;

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

    void generateTransactionID() {
        uint32_t hash[8];
        memcpy(hash, HASH_CODE, sizeof(HASH_CODE));
        std::string combined_info = sender_ + receiver_ + std::to_string(amount_);
        for (char c : combined_info) {
            for (int i = 0; i < 8; ++i) {
                hash[i] = Mix(hash[i], hash[(i + 1) % 8], static_cast<uint32_t>(c));
            }
        }

        char result[65];
        snprintf(result, sizeof(result),
                 "%08x%08x%08x%08x%08x%08x%08x%08x",
                 hash[0], hash[1], hash[2], hash[3],
                 hash[4], hash[5], hash[6], hash[7]);
        transaction_id_ = result;
    }
};

int main() {
    std::vector<User> users;
    std::ifstream userFile("users.txt");
    std::string name, public_key;
    int balance;

    if (userFile.is_open()) {
        while (userFile >> name >> public_key >> balance) {
            users.emplace_back(name, public_key, balance);
        }
        userFile.close();
    } else {
        std::cerr << "Unable to open the user file." << std::endl;
        return 1;
    }

    std::vector<Transaction> transactions;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> amount_distribution(1, 1000);

    for (int i = 0; i < 10000; ++i) {
        int sender_idx = std::uniform_int_distribution<int>(0, users.size() - 1)(gen);
        int receiver_idx = std::uniform_int_distribution<int>(0, users.size() - 1)(gen);
        int amount = amount_distribution(gen);
        if (users[sender_idx].getBalance() >= amount) {
            transactions.emplace_back(users[sender_idx].getPublicKey(), users[receiver_idx].getPublicKey(), amount);
        }
    }

    std::ofstream outfile("transactions.txt");

    for (const Transaction& transaction : transactions) {
        outfile << transaction.getTransactionID()
                  << " " << transaction.getSender()
                  << " " << transaction.getReceiver()
                  << " " << transaction.getAmount() << std::endl;
    }

    return 0;
}
