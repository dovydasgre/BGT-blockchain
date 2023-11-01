#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>
#include <ctime>
#include <random>
#include <vector>

const uint32_t HASH_CODE[] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

class User {
public:
    User(const std::string& name) : name_(name) {
        generatePublicKey();
        generateRandomBalance();
    }

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

    void generatePublicKey() {
        uint32_t hash[8];
        memcpy(hash, HASH_CODE, sizeof(HASH_CODE));
        for (char c : name_) {
            for (int i = 0; i < 8; ++i) {
                hash[i] = Mix(hash[i], hash[(i + 1) % 8], static_cast<uint32_t>(c));
            }
        }

        char result[65];
        snprintf(result, sizeof(result),
                 "%08x%08x%08x%08x%08x%08x%08x%08x",
                 hash[0], hash[1], hash[2], hash[3],
                 hash[4], hash[5], hash[6], hash[7]);
        public_key_ = result;
    }

    void generateRandomBalance() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(100, 1000000);
        balance_ = dist(gen);
    }
};

int main() {
    std::vector<User> users;

    for (int i = 0; i < 1000; ++i) {
        users.push_back(User("User" + std::to_string(i)));
    }

    std::ofstream outfile("users.txt");

    for (const User& user : users) {
        outfile << user.getName() << " " << user.getPublicKey() << " " << user.getBalance() << "\n";
    }
    outfile.close();
    

    return 0;
}
