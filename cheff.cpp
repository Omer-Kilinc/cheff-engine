// CHEFF chess engine by Omer Kilinc v1.0b
// 2022
// ----------------------------------------------------------------

// System Headers
#include <stdio.h>
#include <iostream>

// Define Bitboard data type
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
enum { P, N, B, R, Q, K, p, n, b, r, q, k };
enum { white, black, both };
enum { rook, bishop };

// Castling
enum { wk = 1, wq = 2, bk = 4, bq = 8 };
char ascii_pieces[12] = {'P','N','B','R','Q','K','p','n','b','r','q','k'};
char *unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};


const char *square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};


/**********************************\
 ==================================
 
           Random numbers
 
 ==================================
\**********************************/

// pseudo random number state
unsigned int random_state = 1804289383;

// generate 32-bit pseudo legal numbers
unsigned int get_random_U32_number()
{
    
    unsigned int number = random_state;
    
    // XOR shift algorithm
    number ^= number << 13;
    number ^= number >> 17;
    number ^= number << 5;
    
   
    random_state = number;
    
    
    return number;
}


U64 get_random_U64_number() {

    U64 n1, n2, n3, n4;
    
    // init random numbers slicing 16 bits from MS1B side
    n1 = (U64)(get_random_U32_number()) & 0xFFFF;
    n2 = (U64)(get_random_U32_number()) & 0xFFFF;
    n3 = (U64)(get_random_U32_number()) & 0xFFFF;
    n4 = (U64)(get_random_U32_number()) & 0xFFFF;
    
   
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}



/**********************************\
 ==================================
 
          Bit manipulations
 
 ==================================
\**********************************/

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) ((get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0))
#define count_bits(bitboard) __builtin_popcountll(bitboard)

static inline int get_ls1b_index(U64 bitboard) {
    if (bitboard) {
        return count_bits((bitboard & -bitboard) - 1);
    }
    else {
        return -1;
    }
}




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
/**********************************\
 ==================================
 
          Attacks
 
 ==================================
\**********************************/

const U64 not_a_file = 18374403900871474942ULL;
const U64 not_h_file = 9187201950435737471ULL;
const U64 not_hg_file = 4557430888798830399ULL;
const U64 not_ab_file = 18229723555195321596ULL;

const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6
};

const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12
};

U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

// bishop magic numbers
U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};


// pawn attacks table [side][square]
U64 pawn_attacks[2][64];

// knight attacks table [square]
U64 knight_attacks[64];

// king attacks table [square]
U64 king_attacks[64];

// bishop attack masks
U64 bishop_masks[64];

// rook attack masks
U64 rook_masks[64];

// bishop attacks table [square][occupancies]
U64 bishop_attacks[64][512];

// rook attacks rable [square][occupancies]
U64 rook_attacks[64][4096];


// Generate Attacks
U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    set_bit(bitboard, square);

    if(!side) {
        if (bitboard & not_h_file) attacks |= bitboard >> 7;
        if (bitboard & not_a_file) attacks |= bitboard >> 9;
    } else {
        if (bitboard & not_h_file) attacks |= bitboard << 9;
        if (bitboard & not_a_file) attacks |= bitboard << 7;
    }

    return attacks;
}

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    set_bit(bitboard, square);

    if ((bitboard >> 17) & not_h_file) attacks |= (bitboard >> 17);
    if ((bitboard >> 15) & not_a_file) attacks |= (bitboard >> 15);
    if ((bitboard >> 10) & not_hg_file) attacks |= (bitboard >> 10);
    if ((bitboard >> 6) & not_ab_file) attacks |= (bitboard >> 6);
    if ((bitboard << 17) & not_a_file) attacks |= (bitboard << 17);
    if ((bitboard << 15) & not_h_file) attacks |= (bitboard << 15);
    if ((bitboard << 10) & not_ab_file) attacks |= (bitboard << 10);
    if ((bitboard << 6) & not_hg_file) attacks |= (bitboard << 6);

    return attacks;
}

U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;

    set_bit(bitboard, square);
    
    // generate king attacks
    if (bitboard >> 8) attacks |= (bitboard >> 8);
    if ((bitboard >> 9) & not_h_file) attacks |= (bitboard >> 9);
    if ((bitboard >> 7) & not_a_file) attacks |= (bitboard >> 7);
    if ((bitboard >> 1) & not_h_file) attacks |= (bitboard >> 1);
    if (bitboard << 8) attacks |= (bitboard << 8);
    if ((bitboard << 9) & not_a_file) attacks |= (bitboard << 9);
    if ((bitboard << 7) & not_h_file) attacks |= (bitboard << 7);
    if ((bitboard << 1) & not_a_file) attacks |= (bitboard << 1);


    return attacks;
}

U64 mask_bishop_attacks(int square) {
    U64 attacks = 0ULL;

    int r, f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++) attacks |= (1ULL << (r * 8 + f));
    for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--) attacks |= (1ULL << (r * 8 + f));
    for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--) attacks |= (1ULL << (r * 8 + f));

    return attacks;
}

U64 mask_rook_attacks(int square) {
    U64 attacks = 0ULL;

    int r, f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 6; r++) attacks |= (1ULL << (r * 8 + tf));
    for (r = tr - 1; r >= 1; r--) attacks |= (1ULL << (r * 8 + tf));
    for (f = tf + 1; f <= 6; f++) attacks |= (1ULL << (tr * 8 + f));
    for (f = tf - 1; f >= 1; f--) attacks |= (1ULL << (tr * 8 + f));

    return attacks;
}


U64 bishop_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;

    int r, f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }
    for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= (1ULL << (r * 8 + f));
        if ((1ULL << (r * 8 + f)) & block) break;
    }

    return attacks;
}

U64 rook_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;

    int r, f;

    int tr = square / 8;
    int tf = square % 8;

    for (r = tr + 1; r <= 7; r++) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    for (r = tr - 1; r >= 0; r--) {
        attacks |= (1ULL << (r * 8 + tf));
        if ((1ULL << (r * 8 + tf)) & block) break;
    }
    for (f = tf + 1; f <= 7; f++) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    for (f = tf - 1; f >= 0; f--) {
        attacks |= (1ULL << (tr * 8 + f));
        if ((1ULL << (tr * 8 + f)) & block) break;
    }
    
    return attacks;
}


void init_leaper_attacks() {
    for(int square = 0; square < 64; square++) {
        // Initialize pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white , square);
        pawn_attacks[black][square] = mask_pawn_attacks(black , square);
        knight_attacks[square] = mask_knight_attacks(square);
        king_attacks[square] = mask_king_attacks(square);
    }
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask ) {
    U64 occupancy = 0ULL;

    for(int i = 0; i < bits_in_mask; i++) {
        int square = get_ls1b_index(attack_mask);
        pop_bit(attack_mask, square);
        if(index & (1 << i))
            occupancy |= (1ULL << square);
    }

    return occupancy;
}

U64 find_magic_number(int square, int relevant_bits, int bishop)
{
    // init occupancies
    U64 occupancies[4096];
    
    // init attack tables
    U64 attacks[4096];
    
    // init used attacks
    U64 used_attacks[4096];
    
    // init attack mask for a current piece
    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);
    
    // init occupancy indicies
    int occupancy_indicies = 1 << relevant_bits;
    
    // loop over occupancy indicies
    for (int index = 0; index < occupancy_indicies; index++)
    {
        // init occupancies
        occupancies[index] = set_occupancy(index, relevant_bits, attack_mask);
        
        // init attacks
        attacks[index] = bishop ? bishop_attacks_on_the_fly(square, occupancies[index]) :
                                    rook_attacks_on_the_fly(square, occupancies[index]);
    }
    
    // test magic numbers loop
    for (int random_count = 0; random_count < 100000000; random_count++)
    {
        // generate magic number candidate
        U64 magic_number = generate_magic_number();
        
        // skip inappropriate magic numbers
        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;
        
        // init used attacks
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        
        // init index & fail flag
        int index, fail;
        
        // test magic index loop
        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++)
        {
            // init magic index
            int magic_index = (int)((occupancies[index] * magic_number) >> (64 - relevant_bits));
            
            // if magic index works
            if (used_attacks[magic_index] == 0ULL)
                // init used attacks
                used_attacks[magic_index] = attacks[index];
            
            // otherwise
            else if (used_attacks[magic_index] != attacks[index])
                // magic index doesn't work
                fail = 1;
        }
        
        // if magic number works
        if (!fail)
            // return it
            return magic_number;
    }
    
    // if magic number doesn't work
    printf("  Magic number fails!\n");
    return 0ULL;
}

// init magic numbers
void init_magic_numbers()
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        // init rook magic numbers
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook);

    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
        // init bishop magic numbers
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop);
}

// init slider piece's attack tables
void init_sliders_attacks(int bishop)
{
    // loop over 64 board squares
    for (int square = 0; square < 64; square++)
    {
        // init bishop & rook masks
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);
        
        // init current mask
        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        
        // init relevant occupancy bit count
        int relevant_bits_count = count_bits(attack_mask);
        
        // init occupancy indicies
        int occupancy_indicies = (1 << relevant_bits_count);
        
        // loop over occupancy indicies
        for (int index = 0; index < occupancy_indicies; index++)
        {
            // bishop
            if (bishop)
            {
                // init current occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                
                // init magic index
                int magic_index = (occupancy * bishop_magic_numbers[square]) >> (64 - bishop_relevant_bits[square]);
                
                // init bishop attacks
                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            }
            
            // rook
            else
            {
                // init current occupancy variation
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                
                // init magic index
                int magic_index = (occupancy * rook_magic_numbers[square]) >> (64 - rook_relevant_bits[square]);
                
                // init bishop attacks
                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            
            }
        }
    }
}

// get bishop attacks
static inline U64 get_bishop_attacks(int square, U64 occupancy)
{
    // get bishop attacks assuming current board occupancy
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];
    
    // return bishop attacks
    return bishop_attacks[square][occupancy];
}

// get rook attacks
static inline U64 get_rook_attacks(int square, U64 occupancy)
{
    // get bishop attacks assuming current board occupancy
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];
    
    // return rook attacks
    return rook_attacks[square][occupancy];
}


/**********************************\
 ==================================
 
              Init all
 
 ==================================
\**********************************/

// init all variables
void init_all()
{
    // init leaper pieces attacks
    init_leaper_attacks();
    
    // init slider pieces attacks
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    
    // init magic numbers
    //init_magic_numbers();
}

/**********************************\
 ==================================
 
             Main driver
 
 ==================================
\**********************************/


int main() {

    // init all
    init_all();
    
    // define test bitboard
    U64 occupancy = 0ULL;
    
    // set blocker pieces on board
    set_bit(occupancy, c5);
    set_bit(occupancy, f2);
    set_bit(occupancy, g7);
    set_bit(occupancy, b2);
    set_bit(occupancy, g5);
    set_bit(occupancy, e2);
    set_bit(occupancy, e7);
    
    // print occupancies
    print_bitboard(occupancy);
    
    // print rook attacks
    print_bitboard(get_rook_attacks(e5, occupancy));
    
    // print bishop attacks
    print_bitboard(get_bishop_attacks(d4, occupancy));
    
    return 0;
}