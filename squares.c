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
#define BUFFERS_LENGTH 255
unsigned int  buffers[BUFFERS][BUFFERS_LENGTH]; // 15 is empiricaly found sufficient size
// Used when we want to only check the number of fitting pieces and throw away result
unsigned int  fake_buffer[255];

// Declarations regarding fallbacks
// How many fallbacks before giving up :)
#define FALLBACKS 100000000
unsigned int number_of_fallbacks = 0;
unsigned char fallback_flag = 0;

int order[256] = {20, 21, 36, 37, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

////////////////////////////////////////////////////////////////////////////////
/////////////////////// Functions declarations /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Fill the arguments with according colors needed at given position
void get_constraints(int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left);

// According to the given colors, store fitting pieces in buffer
// and return the number of them
unsigned char get_fitting_pieces(unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left);

// Nicely print the current state of the board
void print_board();

// Print the board and also options for the unfilled positions
void print_board_in_e2bucas_format();

int options[24][24][24][24][256];
int options_lengths[24][24][24][24];

int square_log[10];
int iterator = 0;

int main(int argc, char** argv) {

	// for return code
	int res;

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
	int force_fallback_flag;
	while (1) {

                // This would mean I had become a damn lucky guy
		if (iterator == 256) {
                    printf("%d\n", max);
                    break;
                }

		force_fallback_flag = 0;

		if (iterator == 4) {
			char translate[] = "aihgfedcbqponmlkjwvutsr";
			//print_board();
			//print_board_in_e2bucas_format();
			force_fallback_flag = 1;
			for (int i = 0; i < 4; i++) {

				if (square_log[i] & RIGHT_TOP) {
					printf("r%d", square_log[i] & WITHOUT_TAGS);
				} else if (square_log[i] & BOTTOM_TOP ) {
					printf("b%d", square_log[i] & WITHOUT_TAGS);
				} else if (square_log[i] & LEFT_TOP ) {
					printf("l%d", square_log[i] & WITHOUT_TAGS);
				} else {
					printf("n%d", square_log[i] & WITHOUT_TAGS);
				}

			}
			printf(" %c%c%c%c%c%c%c%c",
                                    translate[board[20]->a],
                                    translate[board[21]->a],
                                    translate[board[21]->b],
                                    translate[board[37]->b],
                                    translate[board[36]->c],
                                    translate[board[37]->c],
                                    translate[board[20]->d],
                                    translate[board[36]->d]);

			printf("\n");
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
		if (! fallback_flag && iterator != 42) {

			// Find the constrains for current position
			get_constraints(current, &top, &right, &bottom, &left);
			D printf("Top: %d Right: %d Bottom: %d Left: %d \n", top, right, bottom, left);

			// Find fitting pieces for current position
			buffer_counts[current_buffer] = get_fitting_pieces(buffers[current_buffer], top, right,  bottom, left);
		}

		D printf( "The length of the current buffer is  %d\n", buffer_counts[current_buffer]);

		// Check if I have still pieces for special positions
                // This makes sense only in case of the order filling the frame first and then going by lines
/*
		for (int q = 0; q < 12; q++) {
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
			}
		}
*/
		D printf("Force fallback %d \n", force_fallback_flag);
		// If not -> force fallback
		// End loop if no further option is posible
		if (buffer_counts[current_buffer] == 0 || force_fallback_flag) {


		    if (iterator == 0) {
			printf("This is the end!\n");
			    break;
		    }

                    /* OUTPUT FOR GRAPHING - every fallback, print the current position
                    fprintf(stderr, "%d %d\n", number_of_fallbacks, iterator);
                    */

		    // if (! fallback_flag) { printf("local max: %d\n", iterator); }

			// Print out the current max solution
			if (iterator > max) {
				max = iterator;
				if (max > 190) {
					printf("Max:%d\n", max);
                                        print_board_in_e2bucas_format();
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
			//printf("%d\n", max);
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

		square_log[iterator] = number;
		board[current] = &pieces[winner];
		pieces[winner].used = 1;

		D printf("Colors of the currently picked piece num. %d : %d %d %d %d\n", board[current]->number, board[current]->a, board[current]->b, board[current]->c, board[current]->d);

		// Move pointer to the next buffer
		current_buffer = current_buffer+1;

		iterator++;
	} // End of while(1) - iterating over the places on board

        //print_board();
        //print_board_in_e2bucas_format();

	return 0;
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


unsigned char get_fitting_pieces(unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left) {
	int count = 0;

	for (int i = 0; i <256; i++) {

		if (pieces[i].used == 1) continue;

		if (left != 23) {
			if (pieces_reference[i].a != left &&
				pieces_reference[i].b != left &&
				pieces_reference[i].c != left &&
				pieces_reference[i].d != left) continue;
		}
		if (top != 23) {
			if (pieces_reference[i].a != top &&
				pieces_reference[i].b != top &&
				pieces_reference[i].c != top &&
				pieces_reference[i].d != top) continue;
		}
		if (right != 23) {
			if (pieces_reference[i].a != right &&
				pieces_reference[i].b != right &&
				pieces_reference[i].c != right &&
				pieces_reference[i].d != right) continue;
		}
		if (bottom != 23) {
			if (pieces_reference[i].a != bottom &&
				pieces_reference[i].b != bottom &&
				pieces_reference[i].c != bottom &&
				pieces_reference[i].d != bottom) continue;
		}

		if ( ( (top    == 23 && pieces_reference[i].a != 0) || pieces_reference[i].a == top   ) &&
		     ( (right  == 23 && pieces_reference[i].b != 0) || pieces_reference[i].b == right ) &&
		     ( (bottom == 23 && pieces_reference[i].c != 0) || pieces_reference[i].c == bottom) &&
		     ( (left   == 23 && pieces_reference[i].d != 0) || pieces_reference[i].d == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +correct rotation\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i;
			count++;
			// the first piece only in one rotation
		}
			if (iterator == 1) continue;

		if ( ( (top    == 23 && pieces_reference[i].d != 0) || pieces_reference[i].d == top) &&
		     ( (right  == 23 && pieces_reference[i].a != 0) || pieces_reference[i].a == right) &&
		     ( (bottom == 23 && pieces_reference[i].b != 0) || pieces_reference[i].b == bottom) &&
		     ( (left   == 23 && pieces_reference[i].c != 0) || pieces_reference[i].c == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +left->top\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i | LEFT_TOP;
			count++;
		}
		if ( ( (top    == 23 && pieces_reference[i].c != 0) || pieces_reference[i].c == top) &&
		     ( (right  == 23 && pieces_reference[i].d != 0) || pieces_reference[i].d == right) &&
		     ( (bottom == 23 && pieces_reference[i].a != 0) || pieces_reference[i].a == bottom) &&
		     ( (left   == 23 && pieces_reference[i].b != 0) || pieces_reference[i].b == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +bottom->top\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i | BOTTOM_TOP;
			count++;
		}
		if ( ( (top    == 23 && pieces_reference[i].b != 0) || pieces_reference[i].b == top) &&
		     ( (right  == 23 && pieces_reference[i].c != 0) || pieces_reference[i].c == right) &&
		     ( (bottom == 23 && pieces_reference[i].d != 0) || pieces_reference[i].d == bottom) &&
		     ( (left   == 23 && pieces_reference[i].a != 0) || pieces_reference[i].a == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +right->top\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i | RIGHT_TOP;
			count++;
		}
	}
	return count;
}

void print_board_in_e2bucas_format() {

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

