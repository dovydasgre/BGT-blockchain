#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <sstream>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <omp.h>
#include <random>

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

class Transaction {
  public:
    std::string transactionId = "";
    std::string sender;
    std::string receiver;
    double sum;
};

void generateTransactions(std::vector<Transaction> &trans, std::vector<User> &users) {
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_real_distribution<double> uni(10.0,100000.0);
  std::uniform_int_distribution<int> uni1(0, 999);
  Transaction transaction;
  trans.reserve(10000);
  
  double amount;
  int a, b;
  std::string sender, receiver;

  for(int i = 0; i < 10000; i++) {
    transaction.sum = round(uni(rng));
    a = uni1(rng);
    b = uni1(rng);

    while(a == b) {
      b = uni1(rng);
    }

    transaction.sender = users.at(a).public_key;
    transaction.receiver = users.at(b).public_key;
    transaction.transactionId = customHash(transaction.sender + transaction.receiver + std::to_string(transaction.sum));
    trans.push_back(transaction);
  }

  std::ofstream wf("transactions.txt");

  for (unsigned int i = 0; i < trans.size(); i++) {
    wf << trans.at(i).sum << std::fixed << std::setprecision(2) << std::endl;
    wf << trans.at(i).sender << std::endl;
    wf << trans.at(i).receiver << std::endl;
    wf << trans.at(i).transactionId << std::endl;
    wf << std::endl;
  }
  wf.close();
}

void readUsers(std::vector<User> &con) {
  std::ifstream df("Users.txt");
  std::stringstream my_buffer;
  my_buffer.clear();
  if (df.is_open()) {
    my_buffer << df.rdbuf();
    df.close();
  }
  User u;
  while(!my_buffer.eof()) {
    my_buffer >> u.name;
    my_buffer >> u.public_key;
    my_buffer >> u.balance;
    my_buffer >> std::ws;
    con.push_back(u);
  }
}


int main() {
    std::vector<User> users;
    readUsers(users);

    std::vector<Transaction>trans;
    generateTransactions(trans, users);
    return 0;
}
