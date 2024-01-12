#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
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

// Definition of one piece
typedef struct {
		unsigned char number;
		unsigned char used;
		unsigned char a;
		unsigned char b;
		unsigned char c;
		unsigned char d;
} piece;

// Referential array of pieces to keep original rotation
piece pieces_reference[256];

// Declarations regarding fallbacks
// How many fallbacks before giving up :)
#define FALLBACKS 1000000

// Different orders of going through the board:
int order_diagonala[256] = {0, 1, 16, 2, 17, 32, 3, 18, 33, 48, 4, 19, 34, 49, 64, 5, 20, 35, 50, 65, 80, 6, 21, 36, 51, 66, 81, 96, 7, 22, 37, 52, 67, 82, 97, 112, 8, 23, 38, 53, 68, 83, 98, 113, 128, 9, 24, 39, 54, 69, 84, 99, 114, 129, 144, 10, 25, 40, 55, 70, 85, 100, 115, 130, 145, 160, 11, 26, 41, 56, 71, 86, 101, 116, 131, 146, 161, 176, 12, 27, 42, 57, 72, 87, 102, 117, 132, 147, 162, 177, 192, 13, 28, 43, 58, 73, 88, 103, 118, 133, 148, 163, 178, 193, 208, 14, 29, 44, 59, 74, 89, 104, 119, 134, 149, 164, 179, 194, 209, 224, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226, 241, 47, 62, 77, 92, 107, 122, 137, 152, 167, 182, 197, 212, 227, 242, 63, 78, 93, 108, 123, 138, 153, 168, 183, 198, 213, 228, 243, 79, 94, 109, 124, 139, 154, 169, 184, 199, 214, 229, 244, 95, 110, 125, 140, 155, 170, 185, 200, 215, 230, 245, 111, 126, 141, 156, 171, 186, 201, 216, 231, 246, 127, 142, 157, 172, 187, 202, 217, 232, 247, 143, 158, 173, 188, 203, 218, 233, 248, 159, 174, 189, 204, 219, 234, 249, 175, 190, 205, 220, 235, 250, 191, 206, 221, 236, 251, 207, 222, 237, 252, 223, 238, 253, 239, 254, 255};


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
void get_constraints(piece **board, int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left);

// According to the given colors, store fitting pieces in buffer
// and return the number of them
//unsigned char get_fitting_pieces(piece *pieces, unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left);
unsigned char get_fitting_pieces(int options[24][24][24][24][256], int options_lengths[24][24][24][24], piece *pieces, unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left);

void get_random_corner_from_file(piece *pieces, char corner, unsigned int *corner_pieces);

// Debug function to print buffers, not used in prod
void print_buffers(unsigned char *buffer_counts, unsigned int **buffers);

// Nicely print the current state of the board
void print_board(piece **board);

void count_fitting_edges(piece **board);

// Print the board and also options for the unfilled positions
void print_board_with_options(int options[24][24][24][24][256], int options_lengths[24][24][24][24], piece **board, piece *pieces);
void print_board_in_e2bucas_format(piece **board);

// Check if the board has met all constraints (basically that
// there is no stupid error in the code)
int check_board(piece **board);


int solve_eternity(int *order, int toptions[24][24][24][24][256], int toptions_lengths[24][24][24][24]);


int main(int argc, char** argv) {

    int (*host_options)[24][24][24][256] = malloc(sizeof(int[24][24][24][24][256]));
    int (*host_options_lengths)[24][24][24] = malloc(sizeof(int[24][24][24][24]));


	// Corners first
	int order[256] = {0, 1, 16, 17, 2, 18, 32, 33,  15, 14, 31, 30, 13, 29, 47, 46,  240, 241, 224, 225, 208, 209, 242, 226,   255, 254, 239, 238, 253, 237, 223, 222,    3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 252, 251, 250, 249, 248, 247, 246, 245, 244, 243, 192, 176, 160, 144, 128, 112, 96, 80, 64, 48, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 62, 78, 94, 110, 126, 142, 158, 174, 190, 206, 236, 235, 234, 233, 232, 231, 230, 229, 228, 227, 193, 177, 161, 145, 129, 113, 97, 81, 65, 49, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 61, 77, 93, 109, 125, 141, 157, 173, 189, 205, 221, 220, 219, 218, 217, 216, 215, 214, 213, 212, 211, 210, 194, 178, 162, 146, 130, 114, 98, 82, 66, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 76, 92, 108, 124, 140, 156, 172, 188, 204, 203, 202, 201, 200, 199, 198, 197, 196, 195, 179, 163, 147, 131, 115, 99, 83, 67, 68, 69, 70, 71, 72, 73, 74, 75, 91, 107, 123, 139, 155, 171, 187, 186, 185, 184, 183, 182, 181, 180, 164, 148, 132, 116, 100, 84, 85, 86, 87, 88, 89, 90, 106, 122, 138, 154, 170, 169, 168, 167, 166, 165, 149, 133, 117, 101, 102, 103, 104, 105, 121, 137, 153, 152, 151, 150, 134, 118, 119, 120, 136, 135 };

	// Load pieces from file
	FILE* input;
	input = fopen("pieces.txt", "r");
	long int a, b, c, d;
	int counter = 0;
	while (1) {
		if (fscanf(input, "%lu%lu%lu%lu", &a, &b, &c, &d) != EOF) {
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

#include "precomputed-options-array.c"

	// for return code
	int res = 0;

	res = solve_eternity(order, host_options, host_options_lengths);

	return res;
}



int solve_eternity(int *order, int options[24][24][24][24][256], int options_lengths[24][24][24][24]){

// Declaration of buffers (arrays of fitting pieces for the particular position)
// Each piece is identified by its number and rotation tag/mask (TOP_TOP/RIGHT_TOP,etc.)
#define BUFFERS 256
unsigned char current_buffer = 0;
// Lengths of each buffer
unsigned char buffer_counts[BUFFERS];
// Store the position of the used piece in a buffer
unsigned char position_of_used_piece_in_buffer[BUFFERS];
// If there is at lfeast one constraint, 15 is enough
// Actually, 12 would also be enough, but only in case we alway have 2 sides with constraints.
// If we wanted to find pieces with only 1 constraint, we'd need 56 (for edges) or 49 for the rest
#define BUFFERS_LENGTH 15
unsigned int  buffers[BUFFERS][BUFFERS_LENGTH]; // 15 is empiricaly found sufficient size
// Used when we want to only check the number of fitting pieces and throw away result
unsigned int  fake_buffer[255];


unsigned int number_of_fallbacks = 0;
unsigned char fallback_flag = 0;

unsigned int corner_pieces[8];



	// Array of all pieces rotated according the the current solution
	piece pieces[256];
        for (int i = 0; i < 256; i++) {
			pieces[i].a=pieces_reference[i].a;
			pieces[i].b=pieces_reference[i].b;
			pieces[i].c=pieces_reference[i].c;
			pieces[i].d=pieces_reference[i].d;
			pieces[i].number=pieces_reference[i].number;
			pieces[i].used=pieces_reference[i].used;
	}

        // Initialize the random number generator
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	srand (spec.tv_sec + spec.tv_nsec);

	//Board - Array of pointers to the pieces
	piece * board[256];

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

	// Maximum reached position
	int max = 0;

	//  Loop over current position on board to be solved
	int iterator = 0;
	int force_fallback_flag;
	while (1) {

                // This would mean I had become a damn lucky guy
		if (iterator == 256) {
                    printf("%d\n", max);
                    break;
                }

		if (iterator > 220) {
                    print_board(board);
                }

                // Get the next position according to used order
		int current = order[iterator];

		// Skip positions with start and hint pieces
		if (current == 34 || current == 45 || current == 135 || current == 210 || current == 221) {
				iterator++;
				continue;
		}

		// Beware - this is highly dependant on the right order!!
		if (current == 0 || current == 15 || current == 240 || current == 255) {
			char corner;
			if (current == 0) {
				corner = 0;
			} else if (current == 15) {
				corner = 1;
			} else if (current == 240) {
				corner = 2;
			} else if (current == 255) {
				corner = 3;
			}
			D printf("Going to fill corner %d!\n", corner);

        		get_random_corner_from_file(pieces, corner, corner_pieces);
			for (int c = 0; c < 8; c++) {
				//print_board_in_e2bucas_format(board);

				unsigned int  number = corner_pieces[c];
				unsigned char winner = number & WITHOUT_TAGS;
				D printf("Number: %d, piece: %d for position %d\n", number, winner, current);

				// Get piece from referential array with correct rotation
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
				current_buffer = current_buffer+1;

				iterator++;
		                current = order[iterator];

			}
			D print_board_in_e2bucas_format(board);
			continue;
		}

		D printf("------Position %d-------Buffer: %d------Iterator: %d---\n", current, current_buffer, iterator);

                // If it is not fallback, get list of fitting pieces (otherwise it's already filled)
		if (! fallback_flag) {

			// Find the constrains for current position
			get_constraints(board, current, &top, &right, &bottom, &left);
			D printf("Top: %d Right: %d Bottom: %d Left: %d \n", top, right, bottom, left);

			// Find fitting pieces for current position
			buffer_counts[current_buffer] = get_fitting_pieces(options, options_lengths, pieces, buffers[current_buffer], top, right,  bottom, left);
		}

		D printf( "The length of the current buffer is  %d\n", buffer_counts[current_buffer]);

		force_fallback_flag = 0;
		// Check if I have still pieces for special positions
                // This makes sense only in case of the order filling the frame first and then going by lines
/*
		for (int q = 0; q < 12; q++) {
			int count;
			//if it's not placed already
			if (board[places_to_check[q]] != NULL) {
				continue;
			}
			get_constraints(board, places_to_check[q], &top, &right, &bottom, &left);
			// if it's anything to check there
			if (top != 23 || right != 23 || bottom != 23 || left != 23){
				D printf("something to check %d \n", places_to_check[q]);
				count = get_fitting_pieces(options, options_lengths, pieces, fake_buffer, top, right,  bottom, left);
			} else {
				continue;
			}
			D printf("Top: %d Right: %d Bottom: %d Left: %d \n", top, right, bottom, left);
			D printf("count: %d \n", count);
			if (count == 0) {
				force_fallback_flag = 1;
				D printf("Setting the fallback flag \n");
			}
		}
*/
		D printf("Force fallback %d \n", force_fallback_flag);
		// If not -> force fallback
		// End loop if no further option is posible
		if (buffer_counts[current_buffer] == 0 || force_fallback_flag) {

                    /* OUTPUT FOR GRAPHING - every fallback, print the current position
                    fprintf(stderr, "%d %d\n", number_of_fallbacks, iterator);
                    print_board_with_options(options, options_lengths, board, pieces);
                    */

		    // if (! fallback_flag) { printf("local max: %d\n", iterator); }

			// Print out the current max solution
			if (iterator > max) {
				max = iterator;
				if (max > 190) {
					printf("Max:%d\n", max);
                                        count_fitting_edges(board);
                                        print_board_in_e2bucas_format(board);
					print_board_with_options(options, options_lengths, board, pieces);
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

        print_board(board);
        count_fitting_edges(board);
        print_board_in_e2bucas_format(board);

        //count_fitting_edges();
        int res = check_board(board);

	return res;
} // End of main()

// Fill the arguments with according colors needed at given position
void get_constraints(piece **board, int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left) {

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
unsigned char get_fitting_pieces(int options[24][24][24][24][256], int options_lengths[24][24][24][24], piece *pieces, unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left) {
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

void print_buffers(unsigned char *buffer_counts, unsigned int **buffers) {
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

int check_board(piece **board) {

	unsigned char top, left, right, bottom;
	int res = 0;

	for (int i = 0; i < 256; i++) {
		//if (i%16 == 0) printf("\n");
		if (board[i] != NULL) {
			get_constraints(board, i, &top, &right, &bottom, &left);
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

void count_fitting_edges(piece **board) {

        int edges = 0;
	for (int i = 0; i < 256; i++) {
		if (board[i] != NULL && i%16 != 15 && board[i+1] != NULL ) {
			if (board[i] -> b == board[i+1] -> d) edges++;
		}
		if (board[i] != NULL && i < 240 && board[i+16] != NULL ) {
			if (board[i] -> c == board[i+16] -> a) edges++;
		}
	}
	printf("Score: %d \n", edges);

}

void get_random_corner_from_file(piece *pieces, char corner, unsigned int *corner_pieces) {
	// Load corner pieces combinations from files
	// corner - 0=top-left 1=top-right 2=bottom-left 3=bottom-right
	FILE* input;
	int length;
	if (corner == 0 ){
		input = fopen("top-left-corners", "r");
		length = 3084;
	} else if (corner == 1) {
		input = fopen("top-right-corners", "r");
		length = 3782;
	} else if (corner == 2) {
		input = fopen("bottom-left-corners", "r");
		length = 3195;
	} else {
		input = fopen("bottom-right-corners", "r");
		length = 2897;
	}
	long int a, b, c, d, e, f, g, h;

        // Initialize the random number generator
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	srand (spec.tv_sec + spec.tv_nsec);

	unsigned int chosen;
	char success = 0;
	int counter;
	char is_used;
	while (1) {
		chosen = rand() % length;
		D printf("%d - length: %d chosen: %d\n", corner, length, chosen);
		counter = 0;
		while (1) {
			is_used = 0;
			if (fscanf(input, "%lu%lu%lu%lu%lu%lu%lu%lu", &a, &b, &c, &d, &e, &f, &g, &h) != EOF) {
				D printf(".");
				if (counter == chosen) {
					D printf("Found the chosen one!");
					corner_pieces[0] = a;
					corner_pieces[1] = b;
					corner_pieces[2] = c;
					corner_pieces[3] = d;
					corner_pieces[4] = e;
					corner_pieces[5] = f;
					corner_pieces[6] = g;
					corner_pieces[7] = h;
					for (int q = 0; q < 8; q++){
						D printf("checking piece %d is used - %d\n", corner_pieces[q] & WITHOUT_TAGS, pieces[corner_pieces[q] & WITHOUT_TAGS].used);
						if (pieces[corner_pieces[q] & WITHOUT_TAGS].used == 1) is_used++;
						D printf("Current state of is_used: %d", is_used);
					}
					if (is_used > 0) {
						D printf("Used - going again!\n");
						break;
					} else {
						success = 1;
						D printf("Success!\n");
						break;
					}
				}
			} else {
				D printf("Hit EOF!\n");
				break;
			}
			counter++;
		}
		if (success) {
			fclose(input);
			D printf("Closing file!\n");
			break;
		} else {
			int position;
			position = ftell(input);
			D printf("Rewinding the input - position: %d\n", position);
			rewind(input);
			position = ftell(input);
			D printf("Rewinding the input - position: %d\n", position);
		}
	}
}

void print_board_in_e2bucas_format(piece **board) {

char translate[] = "aihgfedcbqponmlkjwvutsr";

        printf("https://e2.bucas.name/#custom&board_w=16&board_h=16&board_edges=");
        for (int i = 0; i < 256; i++) {
		if (board[i] != NULL) {
			printf("%c%c%c%c",
                                    translate[board[i]->a],
                                    translate[board[i]->b],
                                    translate[board[i]->c],
                                    translate[board[i]->d]);
		} else {
			printf("aaaa");
		}
	}
        printf("&board_pieces=");
	for (int i = 0; i < 256; i++) {
		if (board[i] != NULL) {
			printf("%03d", board[i]->number+1);
		} else {
			printf("000");
		}
	}
	printf("&motifs_order=jef\n");

}

void print_board(piece **board) {

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

void print_board_with_options(int options[24][24][24][24][256], int options_lengths[24][24][24][24], piece **board, piece *pieces) {

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
               get_constraints(board, i, &top, &right, &bottom, &left);
               if (top != 23 || right !=23 || bottom != 23 || left != 23) {
                   count = get_fitting_pieces(options, options_lengths, pieces, buffer, top, right,  bottom, left);
                   printf(" Pos %d - %d:\n", i, count);
                   for (int j = 0; j < count; j++) {
                       printf("%d ", buffer[j] & WITHOUT_TAGS );
                   }
               }
            }
	}

}

