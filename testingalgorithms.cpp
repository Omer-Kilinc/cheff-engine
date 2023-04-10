#include <iostream>

#define U64 unsigned long long 

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1
};

// Color to move
enum {white, black};


#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) ((get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : e4))
#define count_bits(bitboard) __builtin_popcountll(bitboard)
#define get_ls1b_index(bitboard) (bitboard & -bitboard)

void print_bitboard(U64 bitboard) {
    std::cout << std::endl;
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            if(!file) {
                std::cout << " " << 8-rank << " ";
            }
            std::cout << " " << (get_bit(bitboard, square) ? 1 : 0);
        }
        std::cout << std::endl;
    }
    std::cout << "\n    a b c d e f g h\n\n";
    std::cout << "    Bitboard:" << bitboard << std::endl << std::endl;
}

static inline int get_ls1b_index2(U64 bitboard)
{
    // make sure bitboard is not 0
    if (bitboard)
    {
        // count trailing bits before LS1B
        return count_bits((bitboard & -bitboard) - 1);
    }
    
    //otherwise
    else
        // return illegal index
        return -1;
}

int main() {
    U64 testing;

    set_bit(testing, e2);
    set_bit(testing, e4);
    set_bit(testing, e6);
    set_bit(testing, d2);
    set_bit(testing, g7);
    print_bitboard(testing);
    print_bitboard(get_ls1b_index(testing));
    print_bitboard(get_ls1b_index2(testing));
    print_bitboard(-testing);

    return 0;
}