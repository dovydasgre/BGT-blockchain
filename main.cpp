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

using namespace std;

#define THREAD_NUM 5

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

string customHash(const string& input) {
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

string unixTimeToHumanReadable(time_t timestamp) {
    struct tm* localTime = gmtime(&timestamp);

    if (localTime != nullptr) {
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
        return string(buffer);
    }

    return "Invalid Time";
}

class User {
public:
    string name;
    string public_key;
    double balance;

    User() : name(""), public_key(""), balance(0.0) {}

    User(const string& name, const string& public_key, double balance)
        : name(name), public_key(public_key), balance(balance) {}
};

class Transaction {
public:
    string transactionId = "";
    string sender;
    string receiver;
    double sum;

    Transaction() : transactionId(""), sender(""), receiver(""), sum(0.0) {}

    Transaction(const string& transactionId, const string& sender, const string& receiver, double sum)
        : transactionId(transactionId), sender(sender), receiver(receiver), sum(sum) {}
};

class Block {
public:
    string hash;
    vector<Transaction> transactions;

    Block() : hash("") {}

    void addTransactions(const vector<Transaction>& newTransactions) {
        transactions.insert(transactions.end(), newTransactions.begin(), newTransactions.end());
    }
};

class Blockchain {
public:
    string prevHash;
    int timestamp;
    string version;
    string merkelRoot;
    int nonce;
    string diff;
    Block block;

    void initialize(const string& prevHash, int timestamp, const string& version, const string& merkelRoot, int nonce, const string& diff) {
        this->prevHash = prevHash;
        this->timestamp = timestamp;
        this->version = version;
        this->merkelRoot = merkelRoot;
        this->nonce = nonce;
        this->diff = diff;
    }
};

void readUsers(vector<User> &con) {
    ifstream df("Users.txt");
    if (df.is_open()) {
        User u;
        while (df >> u.name >> u.public_key >> u.balance) {
            con.push_back(u);
        }
        df.close();
    }
}

void readTrans(vector<Transaction> &con) {
    ifstream df("transactions.txt");
    if (df.is_open()) {
        Transaction t;
        while (df >> t.sum >> t.sender >> t.receiver >> t.transactionId) {
            con.push_back(t);
        }
        df.close();
    }
}

bool verifyTransaction(const Transaction& trans, const vector<User>& users) {
    User sender, receiver;
    for (const User& user : users) {
        if (user.public_key == trans.sender) {
            sender = user;
        }
        if (user.public_key == trans.receiver) {
            receiver = user;
        }
    }

    if (sender.balance >= trans.sum) {
        return true;
    }

    return false;
}

void verifyTransactions(vector<Transaction>& trans, const vector<User>& users) {
    vector<Transaction> validTrans;
    for (const Transaction& t : trans) {
        if (verifyTransaction(t, users)) {
            validTrans.push_back(t);
        }
    }
    trans = validTrans;
}

string generateMerkleRoot(vector<Transaction> trans) {
  vector<string> merkel, merkel2;
  string word, hash;

  for(unsigned int i = 0; i < trans.size(); i++) {
    merkel.push_back(trans.at(i).transactionId);
  }

  while (merkel.size() > 1) {
    if (merkel.size() % 2 == 0) {
      for (unsigned i = 0; i < merkel.size(); i+=2) {
        word = merkel.at(i) + merkel.at(i+1);
        hash = customHash(word);
        merkel2.push_back(hash);
      }
    } else {
      for (unsigned i = 0; i < merkel.size() - 1; i+=2) {
        word = merkel.at(i) + merkel.at(i+1);
        hash = customHash(word);
        merkel2.push_back(hash);
      } 

      word = merkel.at(merkel.size()-1);
      hash = customHash(word);
      merkel2.push_back(hash);
    }

    merkel.clear();
    merkel = merkel2;
    merkel2.clear();
  }

  return merkel.at(0);
}

string mineBlock(Blockchain &bc, string prevHash, int b) {
    string newHash;
    int threadCount = omp_get_num_threads();
    int threadNum = omp_get_thread_num();
    int nonce = threadNum;

    if (b == 0) {
        bc.prevHash = customHash("");
    } else {
        bc.prevHash = prevHash;
    }


    string target = bc.diff;

    while (newHash.substr(0, 3) != target) { 
        newHash = customHash(bc.diff + bc.merkelRoot + bc.prevHash + to_string(bc.timestamp) + bc.version + to_string(nonce));

        nonce += threadCount;
    }

    bc.block.hash = newHash;
    bc.timestamp = time(nullptr);
    bc.nonce = nonce;
    return newHash;
}

void printBlock(Blockchain bc, int nr) {
  cout << "------------------------- #" << nr+1 << " BLOCK -------------------------" << endl;
  cout << "Block hash: " << bc.block.hash << endl;
  cout << "Previous block hash: " << bc.prevHash << endl;
  cout << "Version: " << bc.version << endl;
  cout << "Merkle Root: " << bc.merkelRoot.substr(0, 64) << endl;
  cout << "Nonce: " << bc.nonce << endl;
  cout << "Mined on: " << unixTimeToHumanReadable(bc.timestamp) << " (GMT+2)" << endl;
  cout << "Difficulty: " << bc.diff << endl;
  cout << "----------------------------------------------------------" << endl;
}

void printTrans(Transaction t) {
  cout << "---------------------- TRANSACTION ----------------------" << endl;
  cout << "ID: " << t.transactionId << endl;
  cout << "Sender: " << t.sender << endl;
  cout << "Receiver: " << t.receiver << endl;
  cout << "Amount: " << t.sum << endl;
  cout << "---------------------------------------------------------" << endl;
}

void usersData(const vector<User>& users) {
    ofstream wf("UsersRez.txt");
    for (const User& user : users) {
        wf << user.name << endl;
        wf << user.public_key << endl;
        wf << fixed << setprecision(2) << user.balance << endl;
        wf << endl;
    }
    wf.close();
}

int main() {
  bool stop = 0;
  string temp;
  vector<User> users;
  vector<Transaction> trans;
  vector<Blockchain> bc;
  bc.reserve(100);
  random_device rd;
  mt19937 gen(rd());

  omp_set_num_threads(THREAD_NUM);

  readUsers(users);
  readTrans(trans);

  for (int i = 0; i < 5; i++) {
        stop = 0;
        Blockchain newBC;
        while (newBC.block.transactions.size() != 100 && !trans.empty()) {
            uniform_int_distribution<> distr(0, trans.size() - 1);
            int random = distr(gen);

            if (verifyTransaction(trans[random], users)) {
                newBC.block.transactions.push_back(trans[random]);
            }

            trans.erase(trans.begin() + random);
        }
        bc.push_back(newBC);
    }

    int limit = 50000, mined = 0;
    int who = -1;

while (mined != 5) {
    stop = false;
    #pragma omp parallel num_threads(THREAD_NUM) shared(mined, stop, bc, who)
    {
        int nonce;
        string temp;
        bc[mined].version = "v" + to_string(mined + 1) + ".0";
        bc[mined].diff = "000";
        bc[mined].merkelRoot = generateMerkleRoot(bc[mined].block.transactions);

        nonce = limit * omp_get_thread_num();

        temp = mineBlock(bc[mined], (mined == 0) ? "" : bc[mined - 1].block.hash, mined);
        if (temp.size() > 2) {
            #pragma omp critical
            {
                if (who == -1) {
                    who = omp_get_thread_num(); 
                    bc[mined].block.hash = temp;
                    stop = true;
                    bc[mined].nonce = nonce;
                }
            }
        }
    }

    if (bc[mined].block.hash.length() > 1 && who != -1) {
          cout << "MINED BY " << who << " THREAD" << endl;
            for (auto tran : bc[mined].block.transactions) {
                int send = 0, get = 0;
                for (int i = 0; i < 1000; i++) {
                    if (users[i].public_key == tran.sender)
                        send = i;
                    else if (users[i].public_key == tran.receiver)
                        get = i;
                    if (send != 0 && get != 0)
                        break;
                }

                if (users[send].balance >= tran.sum) {
                    users[send].balance -= tran.sum;
                    users[get].balance += tran.sum;
                }
            }
            printBlock(bc[mined], mined);
            mined++;
            who = -1;
        }
    }

    usersData(users);

  return 0;
}