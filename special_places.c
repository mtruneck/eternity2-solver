#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#ifdef DEBUG
 #define D if(1)
#else
 #define D if(0)
#endif

// Rotation flags (to be able to pass them with the piece number)
#define TOP_TOP 0
#define RIGHT_TOP 1024
#define BOTTOM_TOP 2048
#define LEFT_TOP 4096
#define WITHOUT_TAGS 0b11111111

// Maximum reached position
int max = 0;

// Definition of one piece
typedef struct {
		unsigned char number;
		unsigned char used;
		unsigned char a;
		unsigned char b;
		unsigned char c;
		unsigned char d;
} piece;

// Array of all pieces rotated according the the current solution
piece pieces[256];

// Referential array of pieces to keep original rotation
piece pieces_reference[256];

//Board - Array of pointers to the pieces
piece * board[256];

// Declaration of buffers (arrays of fitting pieces for the particular position)
// Each piece is identified by its number and rotation tag/mask (TOP_TOP/RIGHT_TOP,etc.)
#define BUFFERS 256
unsigned char current_buffer = 0;
// Lengths of each buffer
unsigned char buffer_counts[BUFFERS];
// Store the position of the used piece in a buffer
unsigned char position_of_used_piece_in_buffer[BUFFERS];
// If there is at least one constraint, 15 is enough
// Actually, 12 would also be enough, but only in case we alway have 2 sides with constraints.
// If we wanted to find pieces with only 1 constraint, we'd need 56 (for edges) or 49 for the rest
#define BUFFERS_LENGTH 15
unsigned int  buffers[BUFFERS][BUFFERS_LENGTH]; // 15 is empiricaly found sufficient size
// Used when we want to only check the number of fitting pieces and throw away result
unsigned int  fake_buffer[255];

// Declarations regarding fallbacks
// How many fallbacks before giving up :)
#define FALLBACKS 100000
unsigned int number_of_fallbacks = 0;
unsigned char fallback_flag = 0;

// Different orders of going through the board:
int order_diagonala[256] = {0, 1, 16, 2, 17, 32, 3, 18, 33, 48, 4, 19, 34, 49, 64, 5, 20, 35, 50, 65, 80, 6, 21, 36, 51, 66, 81, 96, 7, 22, 37, 52, 67, 82, 97, 112, 8, 23, 38, 53, 68, 83, 98, 113, 128, 9, 24, 39, 54, 69, 84, 99, 114, 129, 144, 10, 25, 40, 55, 70, 85, 100, 115, 130, 145, 160, 11, 26, 41, 56, 71, 86, 101, 116, 131, 146, 161, 176, 12, 27, 42, 57, 72, 87, 102, 117, 132, 147, 162, 177, 192, 13, 28, 43, 58, 73, 88, 103, 118, 133, 148, 163, 178, 193, 208, 14, 29, 44, 59, 74, 89, 104, 119, 134, 149, 164, 179, 194, 209, 224, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226, 241, 47, 62, 77, 92, 107, 122, 137, 152, 167, 182, 197, 212, 227, 242, 63, 78, 93, 108, 123, 138, 153, 168, 183, 198, 213, 228, 243, 79, 94, 109, 124, 139, 154, 169, 184, 199, 214, 229, 244, 95, 110, 125, 140, 155, 170, 185, 200, 215, 230, 245, 111, 126, 141, 156, 171, 186, 201, 216, 231, 246, 127, 142, 157, 172, 187, 202, 217, 232, 247, 143, 158, 173, 188, 203, 218, 233, 248, 159, 174, 189, 204, 219, 234, 249, 175, 190, 205, 220, 235, 250, 191, 206, 221, 236, 251, 207, 222, 237, 252, 223, 238, 253, 239, 254, 255};

// Corners first
int order[256] = {0, 1, 16, 17, 2, 18, 32, 33,  15, 14, 31, 30, 13, 29, 47, 46,  240, 241, 224, 225, 208, 209, 242, 226,   255, 254, 239, 238, 253, 237, 223, 222,    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 192, 176, 160, 144, 128, 112, 96, 80, 64, 48, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 62, 78, 94, 110, 126, 142, 158, 174, 190, 206, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 193, 177, 161, 145, 129, 113, 97, 81, 65, 49, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 61, 77, 93, 109, 125, 141, 157, 173, 189, 205, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 194, 178, 162, 146, 130, 114, 98, 82, 66, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 76, 92, 108, 124, 140, 156, 172, 188, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 179, 163, 147, 131, 115, 99, 83, 67, 68, 69, 70, 71, 72, 73, 74, 75, 91, 107, 123, 139, 155, 171, 187, 186, 185, 184, 183, 182, 181, 180, 164, 148, 132, 116, 100, 84, 85, 86, 87, 88, 89, 90, 106, 122, 138, 154, 170, 169, 168, 167, 166, 165, 149, 133, 117, 101, 102, 103, 104, 105, 121, 137, 153, 152, 151, 150, 134, 118, 119, 120, 136, 135 };

// Corners first - special_places
int special_places_lengths[256];
int special_places[256][50] = {
    { }, //position 0
    { }, //position 1
    { }, //position 2
    { }, //position 3
    { 19 }, //position 4
    { }, //position 5
    { }, //position 6
    { }, //position 7
    { }, //position 8
    { }, //position 9
    { }, //position 10
    { }, //position 11
    { 19 }, //position 12
    { }, //position 13
    { }, //position 14
    { }, //position 15
    { }, //position 16
    { }, //position 17
    { }, //position 18
    { }, //position 19
    { 35 }, //position 20
    { }, //position 21
    { }, //position 22
    { }, //position 23
    { }, //position 24
    { }, //position 25
    { }, //position 26
    { }, //position 27
    { 35 }, //position 28
    { }, //position 29
    { }, //position 30
    { }, //position 31
    { }, //position 32
    { }, //position 33
    { }, //position 34
    { }, //position 35
    { }, //position 36
    { }, //position 37
    { }, //position 38
    { }, //position 39
    { }, //position 40
    { }, //position 41
    { }, //position 42
    { }, //position 43
    { }, //position 44
    { }, //position 45
    { }, //position 46
    { }, //position 47
    { }, //position 48
    { }, //position 49
    { }, //position 50
    { }, //position 51
    { }, //position 52
    { }, //position 53
    { }, //position 54
    { }, //position 55
    { }, //position 56
    { }, //position 57
    { }, //position 58
    { }, //position 59
    { }, //position 60
    { }, //position 61
    { 35, 44, 206, 236, 227, 193, 49 }, //position 62
    { 28 }, //position 63
    { }, //position 64
    { }, //position 65
    { }, //position 66
    { }, //position 67
    { }, //position 68
    { }, //position 69
    { }, //position 70
    { }, //position 71
    { }, //position 72
    { }, //position 73
    { }, //position 74
    { }, //position 75
    { }, //position 76
    { }, //position 77
    { 61 }, //position 78
    { 62 }, //position 79
    { }, //position 80
    { }, //position 81
    { }, //position 82
    { }, //position 83
    { }, //position 84
    { }, //position 85
    { }, //position 86
    { }, //position 87
    { }, //position 88
    { }, //position 89
    { }, //position 90
    { }, //position 91
    { }, //position 92
    { }, //position 93
    { }, //position 94
    { }, //position 95
    { }, //position 96
    { }, //position 97
    { }, //position 98
    { }, //position 99
    { }, //position 100
    { }, //position 101
    { }, //position 102
    { }, //position 103
    { }, //position 104
    { }, //position 105
    { }, //position 106
    { }, //position 107
    { }, //position 108
    { }, //position 109
    { }, //position 110
    { }, //position 111
    { }, //position 112
    { }, //position 113
    { }, //position 114
    { }, //position 115
    { }, //position 116
    { }, //position 117
    { }, //position 118
    { }, //position 119
    { }, //position 120
    { }, //position 121
    { }, //position 122
    { }, //position 123
    { }, //position 124
    { }, //position 125
    { }, //position 126
    { }, //position 127
    { }, //position 128
    { }, //position 129
    { }, //position 130
    { }, //position 131
    { }, //position 132
    { }, //position 133
    { }, //position 134
    { }, //position 135
    { }, //position 136
    { }, //position 137
    { }, //position 138
    { }, //position 139
    { }, //position 140
    { }, //position 141
    { }, //position 142
    { }, //position 143
    { }, //position 144
    { }, //position 145
    { }, //position 146
    { }, //position 147
    { }, //position 148
    { }, //position 149
    { }, //position 150
    { }, //position 151
    { }, //position 152
    { }, //position 153
    { }, //position 154
    { }, //position 155
    { }, //position 156
    { }, //position 157
    { }, //position 158
    { }, //position 159
    { }, //position 160
    { }, //position 161
    { }, //position 162
    { }, //position 163
    { }, //position 164
    { }, //position 165
    { }, //position 166
    { }, //position 167
    { }, //position 168
    { }, //position 169
    { }, //position 170
    { }, //position 171
    { }, //position 172
    { }, //position 173
    { }, //position 174
    { }, //position 175
    { }, //position 176
    { }, //position 177
    { }, //position 178
    { }, //position 179
    { }, //position 180
    { }, //position 181
    { }, //position 182
    { }, //position 183
    { }, //position 184
    { }, //position 185
    { }, //position 186
    { }, //position 187
    { }, //position 188
    { }, //position 189
    { }, //position 190
    { }, //position 191
    { }, //position 192
    { }, //position 193
    { }, //position 194
    { }, //position 195
    { }, //position 196
    { }, //position 197
    { }, //position 198
    { }, //position 199
    { }, //position 200
    { }, //position 201
    { }, //position 202
    { }, //position 203
    { }, //position 204
    { }, //position 205
    { }, //position 206
    { }, //position 207
    { }, //position 208
    { }, //position 209
    { }, //position 210
    { }, //position 211
    { }, //position 212
    { }, //position 213
    { }, //position 214
    { }, //position 215
    { }, //position 216
    { }, //position 217
    { }, //position 218
    { }, //position 219
    { }, //position 220
    { }, //position 221
    { }, //position 222
    { }, //position 223
    { }, //position 224
    { }, //position 225
    { }, //position 226
    { }, //position 227
    { }, //position 228
    { }, //position 229
    { }, //position 230
    { }, //position 231
    { }, //position 232
    { }, //position 233
    { }, //position 234
    { }, //position 235
    { }, //position 236
    { }, //position 237
    { }, //position 238
    { }, //position 239
    { }, //position 240
    { }, //position 241
    { }, //position 242
    { }, //position 243
    { }, //position 244
    { }, //position 245
    { }, //position 246
    { }, //position 247
    { }, //position 248
    { }, //position 249
    { }, //position 250
    { 236 }, //position 251
    { 206 }, //position 252
    { }, //position 253
    { }, //position 254
    { } //position 255
};

// Spiral
int order_spiral[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 254, 253, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 242, 241, 240, 224, 208, 192, 176, 160, 144, 128, 112, 96, 80, 64, 48, 32, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 46, 62, 78, 94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 237, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 226, 225, 209, 193, 177, 161, 145, 129, 113, 97, 81, 65, 49, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 61, 77, 93, 109, 125, 141, 157, 173, 189, 205, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 194, 178, 162, 146, 130, 114, 98, 82, 66, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 76, 92, 108, 124, 140, 156, 172, 188, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 179, 163, 147, 131, 115, 99, 83, 67, 68, 69, 70, 71, 72, 73, 74, 75, 91, 107, 123, 139, 155, 171, 187, 186, 185, 184, 183, 182, 181, 180, 164, 148, 132, 116, 100, 84, 85, 86, 87, 88, 89, 90, 106, 122, 138, 154, 170, 169, 168, 167, 166, 165, 149, 133, 117, 101, 102, 103, 104, 105, 121, 137, 153, 152, 151, 150, 134, 118, 119, 120, 136, 135 };

//order_ram_diagonala
int order_fram_and_lines[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238};
int order_ram[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254};

int order_original[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

int places_to_check[12] = {17, 18, 33, 29, 30, 46, 209, 225, 226, 222, 237, 238};

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Functions declarations /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Fill the arguments with according colors needed at given position
void get_constraints(int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left);

// According to the given colors, store fitting pieces in buffer
// and return the number of them
unsigned char get_fitting_pieces(unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left);

// Debug function to print buffers, not used in prod
void print_buffers();

// Nicely print the current state of the board
void print_board();

void count_fitting_edges();

// Print the board and also options for the unfilled positions
void print_board_with_options();

// Check if the board has met all constraints (basically that
// there is no stupid error in the code)
int check_board();

int options[24][24][24][24][256];
int options_lengths[24][24][24][24];

int main(int argc, char** argv) {

special_places_lengths[0] = 0;
special_places_lengths[1] = 0;
special_places_lengths[2] = 0;
special_places_lengths[3] = 0;
special_places_lengths[4] = 1;
special_places_lengths[5] = 0;
special_places_lengths[6] = 0;
special_places_lengths[7] = 0;
special_places_lengths[8] = 0;
special_places_lengths[9] = 0;
special_places_lengths[10] = 0;
special_places_lengths[11] = 0;
special_places_lengths[12] = 1;
special_places_lengths[13] = 0;
special_places_lengths[14] = 0;
special_places_lengths[15] = 0;
special_places_lengths[16] = 0;
special_places_lengths[17] = 0;
special_places_lengths[18] = 0;
special_places_lengths[19] = 0;
special_places_lengths[20] = 1;
special_places_lengths[21] = 0;
special_places_lengths[22] = 0;
special_places_lengths[23] = 0;
special_places_lengths[24] = 0;
special_places_lengths[25] = 0;
special_places_lengths[26] = 0;
special_places_lengths[27] = 0;
special_places_lengths[28] = 1;
special_places_lengths[29] = 0;
special_places_lengths[30] = 0;
special_places_lengths[31] = 0;
special_places_lengths[32] = 0;
special_places_lengths[33] = 0;
special_places_lengths[34] = 0;
special_places_lengths[35] = 0;
special_places_lengths[36] = 0;
special_places_lengths[37] = 0;
special_places_lengths[38] = 0;
special_places_lengths[39] = 0;
special_places_lengths[40] = 0;
special_places_lengths[41] = 0;
special_places_lengths[42] = 0;
special_places_lengths[43] = 0;
special_places_lengths[44] = 0;
special_places_lengths[45] = 0;
special_places_lengths[46] = 0;
special_places_lengths[47] = 0;
special_places_lengths[48] = 0;
special_places_lengths[49] = 0;
special_places_lengths[50] = 0;
special_places_lengths[51] = 0;
special_places_lengths[52] = 0;
special_places_lengths[53] = 0;
special_places_lengths[54] = 0;
special_places_lengths[55] = 0;
special_places_lengths[56] = 0;
special_places_lengths[57] = 0;
special_places_lengths[58] = 0;
special_places_lengths[59] = 0;
special_places_lengths[60] = 0;
special_places_lengths[61] = 0;
special_places_lengths[62] = 7;
special_places_lengths[63] = 1;
special_places_lengths[64] = 0;
special_places_lengths[65] = 0;
special_places_lengths[66] = 0;
special_places_lengths[67] = 0;
special_places_lengths[68] = 0;
special_places_lengths[69] = 0;
special_places_lengths[70] = 0;
special_places_lengths[71] = 0;
special_places_lengths[72] = 0;
special_places_lengths[73] = 0;
special_places_lengths[74] = 0;
special_places_lengths[75] = 0;
special_places_lengths[76] = 0;
special_places_lengths[77] = 0;
special_places_lengths[78] = 1;
special_places_lengths[79] = 2;
special_places_lengths[80] = 0;
special_places_lengths[81] = 0;
special_places_lengths[82] = 0;
special_places_lengths[83] = 0;
special_places_lengths[84] = 0;
special_places_lengths[85] = 0;
special_places_lengths[86] = 0;
special_places_lengths[87] = 0;
special_places_lengths[88] = 0;
special_places_lengths[89] = 0;
special_places_lengths[90] = 0;
special_places_lengths[91] = 0;
special_places_lengths[92] = 0;
special_places_lengths[93] = 0;
special_places_lengths[94] = 0;
special_places_lengths[95] = 0;
special_places_lengths[96] = 0;
special_places_lengths[97] = 0;
special_places_lengths[98] = 0;
special_places_lengths[99] = 0;
special_places_lengths[100] = 0;
special_places_lengths[101] = 0;
special_places_lengths[102] = 0;
special_places_lengths[103] = 0;
special_places_lengths[104] = 0;
special_places_lengths[105] = 0;
special_places_lengths[106] = 0;
special_places_lengths[107] = 0;
special_places_lengths[108] = 0;
special_places_lengths[109] = 0;
special_places_lengths[110] = 0;
special_places_lengths[111] = 0;
special_places_lengths[112] = 0;
special_places_lengths[113] = 0;
special_places_lengths[114] = 0;
special_places_lengths[115] = 0;
special_places_lengths[116] = 0;
special_places_lengths[117] = 0;
special_places_lengths[118] = 0;
special_places_lengths[119] = 0;
special_places_lengths[120] = 0;
special_places_lengths[121] = 0;
special_places_lengths[122] = 0;
special_places_lengths[123] = 0;
special_places_lengths[124] = 0;
special_places_lengths[125] = 0;
special_places_lengths[126] = 0;
special_places_lengths[127] = 0;
special_places_lengths[128] = 0;
special_places_lengths[129] = 0;
special_places_lengths[130] = 0;
special_places_lengths[131] = 0;
special_places_lengths[132] = 0;
special_places_lengths[133] = 0;
special_places_lengths[134] = 0;
special_places_lengths[135] = 0;
special_places_lengths[136] = 0;
special_places_lengths[137] = 0;
special_places_lengths[138] = 0;
special_places_lengths[139] = 0;
special_places_lengths[140] = 0;
special_places_lengths[141] = 0;
special_places_lengths[142] = 0;
special_places_lengths[143] = 0;
special_places_lengths[144] = 0;
special_places_lengths[145] = 0;
special_places_lengths[146] = 0;
special_places_lengths[147] = 0;
special_places_lengths[148] = 0;
special_places_lengths[149] = 0;
special_places_lengths[150] = 0;
special_places_lengths[151] = 0;
special_places_lengths[152] = 0;
special_places_lengths[153] = 0;
special_places_lengths[154] = 0;
special_places_lengths[155] = 0;
special_places_lengths[156] = 0;
special_places_lengths[157] = 0;
special_places_lengths[158] = 0;
special_places_lengths[159] = 0;
special_places_lengths[160] = 0;
special_places_lengths[161] = 0;
special_places_lengths[162] = 0;
special_places_lengths[163] = 0;
special_places_lengths[164] = 0;
special_places_lengths[165] = 0;
special_places_lengths[166] = 0;
special_places_lengths[167] = 0;
special_places_lengths[168] = 0;
special_places_lengths[169] = 0;
special_places_lengths[170] = 0;
special_places_lengths[171] = 0;
special_places_lengths[172] = 0;
special_places_lengths[173] = 0;
special_places_lengths[174] = 0;
special_places_lengths[175] = 0;
special_places_lengths[176] = 0;
special_places_lengths[177] = 0;
special_places_lengths[178] = 0;
special_places_lengths[179] = 0;
special_places_lengths[180] = 0;
special_places_lengths[181] = 0;
special_places_lengths[182] = 0;
special_places_lengths[183] = 0;
special_places_lengths[184] = 0;
special_places_lengths[185] = 0;
special_places_lengths[186] = 0;
special_places_lengths[187] = 0;
special_places_lengths[188] = 0;
special_places_lengths[189] = 0;
special_places_lengths[190] = 0;
special_places_lengths[191] = 0;
special_places_lengths[192] = 0;
special_places_lengths[193] = 0;
special_places_lengths[194] = 0;
special_places_lengths[195] = 0;
special_places_lengths[196] = 0;
special_places_lengths[197] = 0;
special_places_lengths[198] = 0;
special_places_lengths[199] = 0;
special_places_lengths[200] = 0;
special_places_lengths[201] = 0;
special_places_lengths[202] = 0;
special_places_lengths[203] = 0;
special_places_lengths[204] = 0;
special_places_lengths[205] = 0;
special_places_lengths[206] = 0;
special_places_lengths[207] = 0;
special_places_lengths[208] = 0;
special_places_lengths[209] = 0;
special_places_lengths[210] = 0;
special_places_lengths[211] = 0;
special_places_lengths[212] = 0;
special_places_lengths[213] = 0;
special_places_lengths[214] = 0;
special_places_lengths[215] = 0;
special_places_lengths[216] = 0;
special_places_lengths[217] = 0;
special_places_lengths[218] = 0;
special_places_lengths[219] = 0;
special_places_lengths[220] = 0;
special_places_lengths[221] = 0;
special_places_lengths[222] = 0;
special_places_lengths[223] = 0;
special_places_lengths[224] = 0;
special_places_lengths[225] = 0;
special_places_lengths[226] = 0;
special_places_lengths[227] = 0;
special_places_lengths[228] = 0;
special_places_lengths[229] = 0;
special_places_lengths[230] = 0;
special_places_lengths[231] = 0;
special_places_lengths[232] = 0;
special_places_lengths[233] = 0;
special_places_lengths[234] = 0;
special_places_lengths[235] = 0;
special_places_lengths[236] = 0;
special_places_lengths[237] = 0;
special_places_lengths[238] = 0;
special_places_lengths[239] = 0;
special_places_lengths[240] = 0;
special_places_lengths[241] = 0;
special_places_lengths[242] = 0;
special_places_lengths[243] = 0;
special_places_lengths[244] = 0;
special_places_lengths[245] = 0;
special_places_lengths[246] = 0;
special_places_lengths[247] = 0;
special_places_lengths[248] = 0;
special_places_lengths[249] = 0;
special_places_lengths[250] = 0;
special_places_lengths[251] = 1;
special_places_lengths[252] = 1;
special_places_lengths[253] = 0;
special_places_lengths[254] = 0;
special_places_lengths[255] = 0;



	// for return code
	int res;

#include "precomputed-options-array.c"

        // Initialize the random number generator
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	srand (spec.tv_sec + spec.tv_nsec);

	// General initialization of arrays
	for (int i = 0; i <256; i++) {
		board[i] = NULL;
	}
	for (int i = 0; i < BUFFERS; i++) {
		buffer_counts[i] = 0;
	}
	for (int i = 0; i < BUFFERS; i++) {
		for (int j = 0; j < BUFFERS_LENGTH; j++) {
			buffers[i][j] = 300;
		}
	}

	// Load pieces from file
	FILE* input;
	input = fopen("pieces.txt", "r");
	long int a, b, c, d;
	int counter = 0;
	while (1) {
		if (fscanf(input, "%lu%lu%lu%lu", &a, &b, &c, &d) != EOF) {
			pieces[counter].a=a;
			pieces[counter].b=b;
			pieces[counter].c=c;
			pieces[counter].d=d;
			pieces[counter].number=counter;
			pieces[counter].used=0;
			pieces_reference[counter].a=a;
			pieces_reference[counter].b=b;
			pieces_reference[counter].c=c;
			pieces_reference[counter].d=d;
			pieces_reference[counter].number=counter;
			pieces_reference[counter].used=0;
		} else {
			break;
		}
		counter++;
	}
	fclose(input);

	// Put start piece and hint pieces to the right places
	// Keep in mind indexes start with zero!
	// Proper rotation was prepared in the pieces source file not to bother with it here
	board[45] = &pieces[254];
	pieces[254].used = 1;
	board[135] = &pieces[138];
	pieces[138].used = 1;
	board[210] = &pieces[180];
	pieces[180].used = 1;
	board[34] = &pieces[207];
	pieces[207].used = 1;
	board[221] = &pieces[248];
	pieces[248].used = 1;

	// Parameters for appropriate piece
	unsigned char top, left, right, bottom;

	//  Loop over current position on board to be solved
	int iterator = 0;
	int force_fallback_flag;
	while (1) {

                // This would mean I had become a damn lucky guy
		if (iterator == 256) {
                    printf("%d\n", max);
                    break;
                }

                // Get the next position according to used order
		int current = order[iterator];

		// Skip positions with start and hint pieces
		if (current == 34 || current == 45 || current == 135 || current == 210 || current == 221) {
				iterator++;
				continue;
		}

		D printf("------Position %d-------Buffer: %d------Iterator: %d---\n", current, current_buffer, iterator);

                // If it is not fallback, get list of fitting pieces (otherwise it's already filled)
		if (! fallback_flag) {

			// Find the constrains for current position
			get_constraints(current, &top, &right, &bottom, &left);
			D printf("Top: %d Right: %d Bottom: %d Left: %d \n", top, right, bottom, left);

			// Find fitting pieces for current position
			buffer_counts[current_buffer] = get_fitting_pieces(buffers[current_buffer], top, right,  bottom, left);
		}

		D printf( "The length of the current buffer is  %d\n", buffer_counts[current_buffer]);

		force_fallback_flag = 0;
		// Check if I have still pieces for special positions
                // This makes sense only in case of the order filling the frame first and then going by lines
/*
		for (int q = 0; q < special_places_lengths[current]; q++) {
                        D printf("On position %d, checking place num %d\n", current, special_places[current][q]);
			int count;
			//if it's not placed already
			if (board[places_to_check[q]] != NULL) {
				continue;
			}
			get_constraints(places_to_check[q], &top, &right, &bottom, &left);
			// if it's anything to check there
			if (top != 23 || right != 23 || bottom != 23 || left != 23){
				D printf("something to check %d \n", places_to_check[q]);
				count = get_fitting_pieces(fake_buffer, top, right,  bottom, left);
			} else {
				continue;
			}
			D printf("Top: %d Right: %d Bottom: %d Left: %d \n", top, right, bottom, left);
			D printf("count: %d \n", count);
			if (count == 0) {
				force_fallback_flag = 1;
				D printf("Setting the fallback flag \n");
                                D printf("Test failed.\n");
			}
		}
*/

		D printf("Force fallback %d \n", force_fallback_flag);
		// If not -> force fallback
		// End loop if no further option is posible
		if (buffer_counts[current_buffer] == 0 || force_fallback_flag) {

                    /* OUTPUT FOR GRAPHING - every fallback, print the current position
                    fprintf(stderr, "%d %d\n", number_of_fallbacks, iterator);
                    print_board_with_options();
                    */

		    // if (! fallback_flag) { printf("local max: %d\n", iterator); }

			// Print out the current max solution
			if (iterator > max) {
				max = iterator;
				if (max > 190) {
					printf("Max:%d\n", max);
					print_board_with_options();
				}
			}

                        // Increasing the fallbacks number when high max positions achieved
                        // max < 60 => FALLBACKS
                        // max < 120 => 2*FALLBACKS
                        // max < 180 => 4*FALLBACKS
			if (number_of_fallbacks < FALLBACKS*((max/60)*(max/60)+1)) {

				// Move to the previous buffer and position
				current_buffer--;
				iterator--;
				current = order[iterator];

                                // Skip the pre-defined pieces on the way back, too
				if (current == 34 || current==45 || current==135 || current == 210 || current == 221) {
					iterator--;
				}
				current = order[iterator];

				// Remove current piece from buffer not to be chosen again
				if (position_of_used_piece_in_buffer[current_buffer] == (buffer_counts[current_buffer]-1)) {
                                        // If we want to remove the last one, only decrease the size by 1
					buffer_counts[current_buffer] = buffer_counts[current_buffer]-1;
				} else {
                                        // Remove the element by replacing it with the last one and decrese the size by 1
					buffers[current_buffer][position_of_used_piece_in_buffer[current_buffer]] = buffers[current_buffer][buffer_counts[current_buffer]-1];
					buffer_counts[current_buffer] = buffer_counts[current_buffer]-1;
				}

				// Remove piece from board
				board[current]->used = 0;
				board[current] = NULL;

				// Set fallback mode and run..
				number_of_fallbacks++;
				fallback_flag = 1;
				continue;
			}

			// We're done here, print maximum reached solution and die
			printf("%d\n", max);
			break;
		}

		// reset fallback_flag
		fallback_flag = 0;

		// Select random piece from buffer
		unsigned char chosen = rand() % buffer_counts[current_buffer];
		unsigned int  number = buffers[current_buffer][chosen];
		unsigned char winner = buffers[current_buffer][chosen & WITHOUT_TAGS];
                position_of_used_piece_in_buffer[current_buffer] = chosen;
		D printf("Aaaand, the winner is %d\n", winner);

		// Get piece from referential array with correct rotation
		unsigned char temp;
		if (number & RIGHT_TOP) {
			pieces[winner].a = pieces_reference[winner].b;
			pieces[winner].b = pieces_reference[winner].c;
			pieces[winner].c = pieces_reference[winner].d;
			pieces[winner].d = pieces_reference[winner].a;
			D printf("right - top\n");
		} else if (number & BOTTOM_TOP ) {
			pieces[winner].a = pieces_reference[winner].c;
			pieces[winner].c = pieces_reference[winner].a;
			pieces[winner].b = pieces_reference[winner].d;
			pieces[winner].d = pieces_reference[winner].b;
			D printf("bottom - top\n");
		} else if (number & LEFT_TOP ) {
			pieces[winner].a = pieces_reference[winner].d;
			pieces[winner].d = pieces_reference[winner].c;
			pieces[winner].c = pieces_reference[winner].b;
			pieces[winner].b = pieces_reference[winner].a;
			D printf("left - top\n");
		} else {
			pieces[winner].a = pieces_reference[winner].a;
			pieces[winner].b = pieces_reference[winner].b;
			pieces[winner].c = pieces_reference[winner].c;
			pieces[winner].d = pieces_reference[winner].d;
			D printf("top - top\n");
		}

		board[current] = &pieces[winner];
		pieces[winner].used = 1;

		D printf("Colors of the currently picked piece num. %d : %d %d %d %d\n", board[current]->number, board[current]->a, board[current]->b, board[current]->c, board[current]->d);

		// Move pointer to the next buffer
		current_buffer = (current_buffer+1)%BUFFERS;

		iterator++;
	} // End of while(1) - iterating over the places on board

        print_board();
        //count_fitting_edges();
        res = check_board();

	return res;
} // End of main()

// Fill the arguments with according colors needed at given position
void get_constraints(int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left) {

		if (position < 16) {
			*top = 0;
		} else if (board[position -16] == NULL) {
			*top = 23;
		} else {
			*top = board[position-16]->c;
		}

		if (position%16 == 0) {
			*left = 0;
		} else if (board[position -1] == NULL) {
			*left = 23;
		} else {
			*left = board[position-1]->b;
		}

		if (position%16 == 15) {
			*right = 0;
		} else if (board[position +1] == NULL) {
			*right = 23;
		} else {
			*right = board[position+1]->d;
		}

		if (position > 239) {
			*bottom = 0;
		} else if (board[position +16] == NULL) {
			*bottom = 23;
		} else {
			*bottom = board[position+16]->a;
		}

}

// According to the given colors, store fitting pieces in buffer and return the number of them
unsigned char get_fitting_pieces(unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left) {
	int count = 0;

        D printf("Going to get fitting pieces for %d %d %d %d - length of options: %d\n", top, right, bottom, left, options_lengths[top][right][bottom][left]);
        for (int p = 0; p < options_lengths[top][right][bottom][left]; p++) {
                int i = options[top][right][bottom][left][p];

		if (pieces[i & WITHOUT_TAGS].used == 1) continue;
		D printf("The winner is %d - %d %d %d %d +correct rotation\n", i, pieces_reference[i & WITHOUT_TAGS].a, pieces_reference[i & WITHOUT_TAGS].b, pieces_reference[i & WITHOUT_TAGS].c, pieces_reference[i & WITHOUT_TAGS].d);
		buffer[count] = i;
		count++;
	}
	return count;
}

void print_buffers() {
	for (int i = 0; i < BUFFERS; i++) {
		if (buffer_counts[i] != 0) {
			D printf("bufer %d ", i);
			for (int j = 0; j < buffer_counts[i]; j++) {
				D printf(" %d ", buffers[i][j]);
			}
			D printf("\n");
		}
	}
}

int check_board() {

	unsigned char top, left, right, bottom;
	int res = 0;

	for (int i = 0; i < 256; i++) {
		//if (i%16 == 0) printf("\n");
		if (board[i] != NULL) {
			get_constraints(i, &top, &right, &bottom, &left);
			if ((top != 23 && top != board[i]->a) ||
				(top != 23 && top != board[i]->a) ||
				(top != 23 && top != board[i]->a) ||
				(top != 23 && top != board[i]->a)) {

					res = 1;
					printf("%d %4d %02d %02d %02d %02d\n", i, board[i]->number, top, right, bottom, left);
					printf("%d here %02d %02d %02d %02d\n", i, board[i]->a, board[i]->b, board[i]->c, board[i]->d);
			}
		}
	}
	printf("\n");
	return res;
}

void count_fitting_edges() {

        int edges = 0;
	for (int i = 0; i < 256; i++) {
		if (board[i] != NULL && i%16 != 15 && board[i+1] != NULL ) {
			if (board[i] -> b == board[i+1] -> d) edges++;
		}
		if (board[i] != NULL && i < 240 && board[i+16] != NULL ) {
			if (board[i] -> c == board[i+16] -> a) edges++;
		}
	}
	printf("Edges: %d \n", edges);

}

void print_board() {

	for (int i = 0; i < 256; i++) {
		if (i%16 == 0) printf("\n");
		if (board[i] != NULL) {
			printf("%4d", board[i]->number);
		} else {
			printf("  --");
		}
	}
	printf("\n");

}

void print_board_with_options() {

	for (int i = 0; i < 256; i++) {
		if (i%16 == 0) printf("\n");
		if (board[i] != NULL) {
			printf("%4d", board[i]->number);
		} else {
			printf("  --");
		}
	}
	printf("\n");

	unsigned char top, left, right, bottom;
        unsigned int buffer[250];
        unsigned char count;

	for (int i = 0; i < 256; i++) {
            if (board[i] == NULL) {
               get_constraints(i, &top, &right, &bottom, &left);
               if (top != 23 || right !=23 || bottom != 23 || left != 23) {
                   count = get_fitting_pieces(buffer, top, right,  bottom, left);
                   printf(" Pos %d - %d:", i, count);
                   for (int j = 0; j < count; j++) {
                       printf("%d ", buffer[j] & WITHOUT_TAGS );
                   }
               }
               printf("\n");
            }
	}

}

