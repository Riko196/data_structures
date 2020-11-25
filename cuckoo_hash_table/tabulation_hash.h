#include <cstdint>

#include "random_gen.h"

class TabulationHash {
    /*
     * Hash function for hashing by tabulation.
     *
     * The 32-bit key is split to four 8-bit parts. Each part indexes
     * a separate table of 256 randomly generated values. Obtained values
     * are XORed together.
     */

    uint32_t num_buckets;
    uint32_t tables[4][256];

   public:
    TabulationHash(uint32_t num_buckets, RandomGen *random_gen) {
        this->num_buckets = num_buckets;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 256; j++)
                this->tables[i][j] = random_gen->next_u32();
    }

    uint32_t hash(uint32_t key) {
        uint32_t h0 = key & 0xff;
        uint32_t h1 = (key >> 8) & 0xff;
        uint32_t h2 = (key >> 16) & 0xff;
        uint32_t h3 = (key >> 24) & 0xff;
        return (this->tables[0][h0] ^ this->tables[1][h1] ^ this->tables[2][h2] ^ this->tables[3][h3]) % this->num_buckets;
    }
};