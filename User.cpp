#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <iomanip>

const double MIN_VALUE = 100.0;
const double MAX_VALUE = 1000000.0;

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
};

void generateUsers(std::vector<User>& users) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_real_distribution<double> uni(MIN_VALUE, MAX_VALUE);
    users.reserve(1000);

    for (int i = 0; i < 1000; i++) {
        User user;
        user.name = "Vardas" + std::to_string(i);
        user.public_key = customHash(user.name);
        user.balance = uni(rng);
        users.push_back(user);
    }

    std::ofstream wf("Users.txt");

    for (unsigned int i = 0; i < users.size(); i++) {
        wf << users.at(i).name << std::endl;
        wf << users.at(i).public_key << std::endl;
        wf << std::fixed << std::setprecision(2) << users.at(i).balance << std::endl;
        wf << std::endl;
    }
    wf.close();
}

int main() {
    std::vector<User> users;
    generateUsers(users);
    return 0;
}
