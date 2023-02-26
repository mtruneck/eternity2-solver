#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

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
int max_buffer = 0; // max reached buffer occupation

// Definition of one piece
typedef struct {
		unsigned char number;
		unsigned char used;
		unsigned char a;
		unsigned char b;
		unsigned char c;
		unsigned char d;
} piece;


// Array of all pieces
piece pieces[256];

// Referential array of pieces to keep original position of colors
piece pieces_reference[256];


//Board - Array of pointers to the pieces
piece * board[256];


// Declaration of buffers
#define BUFFERS 256
unsigned char current_buffer = 0;
unsigned char buffer_counts[BUFFERS];
unsigned int  buffers[BUFFERS][15]; // 15 is empiricaly found sufficient size


// Declarations regarding fallbacks
#define FALLBACKS 2000000
unsigned int number_of_fallbacks = 0;
unsigned char fallback_flag = 0;


int order_diagonala[256] = {0, 1, 16, 2, 17, 32, 3, 18, 33, 48, 4, 19, 34, 49, 64, 5, 20, 35, 50, 65, 80, 6, 21, 36, 51, 66, 81, 96, 7, 22, 37, 52, 67, 82, 97, 112, 8, 23, 38, 53, 68, 83, 98, 113, 128, 9, 24, 39, 54, 69, 84, 99, 114, 129, 144, 10, 25, 40, 55, 70, 85, 100, 115, 130, 145, 160, 11, 26, 41, 56, 71, 86, 101, 116, 131, 146, 161, 176, 12, 27, 42, 57, 72, 87, 102, 117, 132, 147, 162, 177, 192, 13, 28, 43, 58, 73, 88, 103, 118, 133, 148, 163, 178, 193, 208, 14, 29, 44, 59, 74, 89, 104, 119, 134, 149, 164, 179, 194, 209, 224, 15, 30, 45, 60, 75, 90, 105, 120, 135, 150, 165, 180, 195, 210, 225, 240, 31, 46, 61, 76, 91, 106, 121, 136, 151, 166, 181, 196, 211, 226, 241, 47, 62, 77, 92, 107, 122, 137, 152, 167, 182, 197, 212, 227, 242, 63, 78, 93, 108, 123, 138, 153, 168, 183, 198, 213, 228, 243, 79, 94, 109, 124, 139, 154, 169, 184, 199, 214, 229, 244, 95, 110, 125, 140, 155, 170, 185, 200, 215, 230, 245, 111, 126, 141, 156, 171, 186, 201, 216, 231, 246, 127, 142, 157, 172, 187, 202, 217, 232, 247, 143, 158, 173, 188, 203, 218, 233, 248, 159, 174, 189, 204, 219, 234, 249, 175, 190, 205, 220, 235, 250, 191, 206, 221, 236, 251, 207, 222, 237, 252, 223, 238, 253, 239, 254, 255};


//order_ram_diagonala
int order[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238};
int order_ram[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15, 31, 47, 63, 79, 95, 111, 127, 143, 159, 175, 191, 207, 223, 239, 255, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254};

int order_original[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255};

// Fill the arguments with according colors needed at given position
void get_constraints(int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left);

unsigned char get_fitting_pieces(unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left);

void print_buffers();

void print_board();
int check_board();


int main(int argc, char** argv) {

	// for return code
	int res;

	// General initialization of arrays and random generator
	srand ( getpid() + time(NULL) );
	for (int i = 0; i <256; i++) {
		board[i] = NULL;
	}
	for (int i = 0; i < BUFFERS; i++) {
		buffer_counts[i] = 0;
	}
	for (int i = 0; i < BUFFERS; i++) {
		for (int j = 0; j < 200; j++) {
			buffers[i][j] = 300;
		}
	}
	
	// Load pieces from file
	FILE* input;
	input = fopen("pieces.txt", "r");
	long int a, b, c, d;
	int counter = 0;
	while (1) {
		if (fscanf(input, "%d%d%d%d", &a, &b, &c, &d) != EOF) {
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

	// Print all the loaded pieces
	//for (int i = 0; i <256; i++) {
			//fprintf(stderr, "%04d %02d %02d %02d %02d\n", pieces[i].number, pieces[i].a, pieces[i].b, pieces[i].c, pieces[i].d);
	//}

	// Put start piece and hint pieces to the right places
	// Keep in mind indexes start with zero!
	// Proper rotation was prepared in the pieces source file
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
	while (1) {

		if (iterator == 256) break;
		int current = order[iterator];

		// Skip positions with start and hint pieces
		if (current == 34 || current == 45 || current == 135 || current == 210 || current == 221) {
				iterator++;
				continue;
		}

		D printf("------Position %d-------Buffer: %d------Iterator: %d---\n", current, current_buffer, iterator);

		// If it is fallback, we don't need to search for the posibilities again
		if (! fallback_flag) {

			// Find the constrains for current position
			get_constraints(current, &top, &right, &bottom, &left);
			D printf("Top: %d Right: %d Bottom: %d Left: %d \n", top, right, bottom, left);

			// Find fitting pieces for current position
			unsigned char count;
			count = get_fitting_pieces(buffers[current_buffer], top, right,  bottom, left);
			buffer_counts[current_buffer] = count;
			if (count > max_buffer) { max_buffer = count;}
		}

		D printf( "Delka aktualniho bufferu je %d\n", buffer_counts[current_buffer]);

		// End loop if no further option is posible
		if (buffer_counts[current_buffer] == 0) {

		    // if (! fallback_flag) { printf("local max: %d\n", iterator); }

			// Print out the current max solution
			if (iterator > max) {
				max = iterator;
				if (max > 190) {
					fprintf(stderr, "Max:%d\n", max);
					print_board();
				}

			}

			if (number_of_fallbacks < FALLBACKS) {
				
				// Move to the previous buffer and position
				current_buffer = (current_buffer+BUFFERS-1)%BUFFERS;
				iterator--;
				current = order[iterator];
				if (current == 34 || current==45 || current==135 || current == 210 || current == 221) {
					iterator--;
				}
				current = order[iterator];

				// Remove current piece from buffer not to be chosen again
				int to_remove = 1000;
				D printf("hledame %d v bufferu cislo %d\n", board[current]->number, current_buffer);
				for (int l = 0; l < buffer_counts[current_buffer]; l++) {
					//D printf(" %d ", buffers[current_buffer][l] & WITHOUT_TAGS);
					//D printf(" %d ", board[current]->number);
					int x = buffers[current_buffer][l] & WITHOUT_TAGS;
					int w = board[current]->number;
					if ( x == w ) { to_remove = l; }
				}
				if (to_remove == 1000) printf("Neco je spatne, %d \n", current);
				if (to_remove == (buffer_counts[current_buffer]-1)) {
					buffer_counts[current_buffer] = buffer_counts[current_buffer]-1;
				} else {
					buffers[current_buffer][to_remove] = buffers[current_buffer][buffer_counts[current_buffer]-1];
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

			// Print maximum reached solution and die
			printf("%d\n", max);
			break;
		}

		//if (fallback_flag) { printf("new: %d\n", iterator); }
		
		// reset fallback_flag
		fallback_flag = 0;

		// Select random piece from buffer
		unsigned char chosen = rand() % buffer_counts[current_buffer];
		unsigned int  number = buffers[current_buffer][chosen];
		unsigned char winner = buffers[current_buffer][chosen & WITHOUT_TAGS];
		D printf("A vitezem se stava %d\n", winner);

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

		D printf("aktualni barvy vybraneho dilku c. %d : %d %d %d %d\n", board[current]->number, board[current]->a, board[current]->b, board[current]->c, board[current]->d);

		// Move pointer to the next buffer
		current_buffer = (current_buffer+1)%BUFFERS;

		iterator++;
	} // End of for - iterating over the places on board

        print_board();
        res = check_board();

	return res;
} // End of main()


void get_constraints(int position, unsigned char *top, unsigned char *right, unsigned char *bottom, unsigned char *left) {

		if (position < 16) {
			*top = 0;
		} else if (board[position -16] == NULL) {
			*top = 30;
		} else {
			*top = board[position-16]->c;
		}

		if (position%16 == 0) {
			*left = 0;
		} else if (board[position -1] == NULL) {
			*left = 30;
		} else {
			*left = board[position-1]->b;
		}

		if (position%16 == 15) {
			*right = 0;
		} else if (board[position +1] == NULL) {
			*right = 30;
		} else {
			*right = board[position+1]->d;
		}

		if (position > 239) {
			*bottom = 0;
		} else if (board[position +16] == NULL) {
			*bottom = 30;
		} else {
			*bottom = board[position+16]->a;
		}

}


unsigned char get_fitting_pieces(unsigned int *buffer, const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left) {
	int count = 0;

	for (int i = 0; i <256; i++) {

		if (pieces[i].used == 1) continue;

		if (left != 30) {
			if (pieces_reference[i].a != left &&
				pieces_reference[i].b != left &&
				pieces_reference[i].c != left &&
				pieces_reference[i].d != left) continue;
		}
		if (top != 30) {
			if (pieces_reference[i].a != top &&
				pieces_reference[i].b != top &&
				pieces_reference[i].c != top &&
				pieces_reference[i].d != top) continue;
		}
		if (right != 30) {
			if (pieces_reference[i].a != right &&
				pieces_reference[i].b != right &&
				pieces_reference[i].c != right &&
				pieces_reference[i].d != right) continue;
		}
		if (bottom != 30) {
			if (pieces_reference[i].a != bottom &&
				pieces_reference[i].b != bottom &&
				pieces_reference[i].c != bottom &&
				pieces_reference[i].d != bottom) continue;
		}

		if ( ( (top    == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == top   ) &&
		     ( (right  == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == right ) &&
		     ( (bottom == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == bottom) &&
		     ( (left   == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +correct rotation\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i;
			count++;
			continue;
		}

		if ( ( (top    == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == top) &&
		     ( (right  == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == right) &&
		     ( (bottom == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == bottom) &&
		     ( (left   == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +left->top\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i | LEFT_TOP;
			count++;
			continue;
		}
		if ( ( (top    == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == top) &&
		     ( (right  == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == right) &&
		     ( (bottom == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == bottom) &&
		     ( (left   == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +bottom->top\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i | BOTTOM_TOP;
			count++;
			continue;
		}
		if ( ( (top    == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == top) &&
		     ( (right  == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == right) &&
		     ( (bottom == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == bottom) &&
		     ( (left   == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == left) ) {
			D printf("Vyhral dilek cislo %d - %d %d %d %d +right->top\n", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
			buffer[count] = i | RIGHT_TOP;
			count++;
			continue;
		}
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
		//if (i%16 == 0) fprintf(stderr, "\n");
		if (board[i] != NULL) {
			get_constraints(i, &top, &right, &bottom, &left);
			if ((top != 30 && top != board[i]->a) || 
				(top != 30 && top != board[i]->a) ||
				(top != 30 && top != board[i]->a) ||
				(top != 30 && top != board[i]->a)) {

					res = 1;
					fprintf(stderr, "%d %4d %02d %02d %02d %02d\n", i, board[i]->number, top, right, bottom, left);
					fprintf(stderr, "%d here %02d %02d %02d %02d\n", i, board[i]->a, board[i]->b, board[i]->c, board[i]->d);
			}
		}
	}
	fprintf(stderr, "\n");
	return res;
}

void print_board() {

	for (int i = 0; i < 256; i++) {
		if (i%16 == 0) fprintf(stderr, "\n");
		if (board[i] != NULL) {
			fprintf(stderr, "%4d", board[i]->number);
		} else {
			fprintf(stderr, "  --");
		}
	}
	fprintf(stderr, "\n");

}

