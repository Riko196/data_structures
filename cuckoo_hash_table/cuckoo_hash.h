#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#include "tabulation_hash.h"

using namespace std;

// If the condition is not true, report an error and halt.
#define EXPECT(condition, message)                \
    do {                                          \
        if (!(condition)) expect_failed(message); \
    } while (0)

void expect_failed(const string &message);

class CuckooTable {
    /*
     * Hash table with Cuckoo hashing.
     *
     * We have two hash functions, which map 32-bit keys to buckets of a common
     * hash table. Unused buckets contain 0xffffffff.
     */

    const uint32_t UNUSED = 0xffffffff;

    // The array of buckets
    vector<uint32_t> table;
    uint32_t num_buckets;
    uint32_t max_attempts;

    // Hash functions and the random generator used to create them
    TabulationHash *hashes[2];
    RandomGen *random_gen;

   private:
    uint32_t log(uint32_t number) {
        uint32_t log = 0;
        while (number > 0) {
            log++;
            number /= 2;
        }
        return log;
    }

    void refresh_function() {
        delete this->random_gen;
        this->random_gen = new RandomGen(rand());
        for (int i = 0; i < 2; i++) {
            delete this->hashes[i];
            this->hashes[i] = new TabulationHash(this->num_buckets, this->random_gen);
        }
    }

    void rehash_table() {
        vector<uint32_t> table_copy(this->table);
        bool rehashed = false;
        while (!rehashed) {
            rehashed = true;
            for (int i = 0; i < this->table.size(); i++)
                this->table[i] = this->UNUSED;

            this->refresh_function();

            for (int i = 0; i < table_copy.size(); i++) {
                uint32_t key = table_copy[i];

                if ((key != this->UNUSED) && (this->try_insert(key) != this->UNUSED)) {
                    rehashed = false;
                    break;
                }
            }
        }
    }

    uint32_t try_insert(uint32_t key) {
        EXPECT(key != this->UNUSED, "Keys must differ from UNUSED.");
        uint32_t attempt = 0;
        uint32_t previous_hash = 0;
        while (attempt < this->max_attempts) {
            attempt++;
            uint32_t h0 = this->hashes[0]->hash(key);
            uint32_t h1 = this->hashes[1]->hash(key);
            if (this->table[h0] == this->UNUSED) {
                this->table[h0] = key;
                return this->UNUSED;  // insert succeeded so it returns UNUSED value because there is no key without bucket
            } else if (this->table[h1] == this->UNUSED) {
                this->table[h1] = key;
                return this->UNUSED;  // insert succeeded so it returns UNUSED value because there is no key without bucket
            } else {
                uint32_t next_hash = previous_hash == h0 ? h1 : h0;
                uint32_t temp = this->table[next_hash];
                this->table[next_hash] = key;
                key = temp;
                previous_hash = next_hash;
            }
        }
        return key;  // insert failed so it returns last key which didnt find its bucket
    }

   public:
    CuckooTable(unsigned num_buckets) {
        // Initialize the table with the given number of buckets.
        // The number of buckets is expected to stay constant.

        this->num_buckets = num_buckets;
        this->max_attempts = 6*this->log(num_buckets);
        this->table.resize(num_buckets, this->UNUSED);

        // Obtain two fresh hash functions.
        this->random_gen = new RandomGen(rand());
        for (int i = 0; i < 2; i++)
            this->hashes[i] = new TabulationHash(this->num_buckets, this->random_gen);
    }

    ~CuckooTable() {
        for (int i = 0; i < 2; i++)
            delete this->hashes[i];
        delete this->random_gen;
    }

    bool lookup(uint32_t key) {
        // Check if the table contains the given key. Returns True or False.
        unsigned h0 = this->hashes[0]->hash(key);
        unsigned h1 = this->hashes[1]->hash(key);
        return (this->table[h0] == key || this->table[h1] == key);
    }

    void insert(uint32_t key) {
        // Insert a new key to the table.
        EXPECT(key != this->UNUSED, "Keys must differ from UNUSED.");

        if (this->lookup(key)) // If there is already key then do nothing
            return;

        uint32_t last_key = this->try_insert(key);
        if (last_key != this->UNUSED) {
            for (int i = 0; i < this->table.size(); i++)
                if (this->table[i] == this->UNUSED) {
                    this->table[i] = last_key;
                    break;
                }
            this->rehash_table();
        }
    }

    void remove(uint32_t key) {
        // Delete a key from the table.
        EXPECT(key != this->UNUSED, "Keys must differ from UNUSED.");

        uint32_t h0 = this->hashes[0]->hash(key);
        uint32_t h1 = this->hashes[1]->hash(key);

        if (this->table[h0] == key)
            this->table[h0] = this->UNUSED;
        else if (this->table[h1] == key)
            this->table[h1] = this->UNUSED;
        else
            return; // The key is not in the table so do nothing
    }
};
