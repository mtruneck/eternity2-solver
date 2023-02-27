#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
/*
 * This one is purely to generate the precomputed code for getting list of fitting pieces. Most of the code here is unused
 */

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

unsigned char get_fitting_pieces(const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left);

int main(int argc, char** argv) {

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

        unsigned char count;
        unsigned char top = 0;
        unsigned char right = 0;
        unsigned char bottom = 0;
        unsigned char left = 0;

        //printf("int options[24][24][24][24][256];\nint options_lengths[24][24][24][24];\n");
        for (int i = 0; i < 24; i++) {
          if (i == 23) { top = 30; } else { top = i; }
          for (int j = 0; j < 24; j++) {
            if (j == 23) { right = 30; } else { right = j; }
            for (int k = 0; k < 24; k++) {
              if (k == 23) { bottom = 30; } else { bottom = k; }
              for (int l = 0; l < 24; l++) {
                if (l == 23) { left = 30; } else { left = l; }
                //printf("I have %d %d  %d %d \n", i, j, k, l);
                //printf("gonna call %d %d  %d %d \n", top, right, bottom, left);
                count = get_fitting_pieces(top, right, bottom, left);
              }
            }
          }
        }
        return 0;

} // End of main()

unsigned char get_fitting_pieces(const unsigned char top, const unsigned char right, const unsigned char bottom, const unsigned char left) {
	int count = 0;
        //printf("zde");

        unsigned char w, x, y, z;
        if (top == 30)    { w = 23; } else { w = top; }
        if (right == 30)  { x = 23; } else { x = right; }
        if (bottom == 30) { y = 23; } else { y = bottom; }
        if (left == 30)   { z = 23; } else { z = left; }

	for (int i = 0; i <256; i++) {

		//printf("Constr: %d %d %d %d - D%d(a %d b %d c %d d %d) ", top, right, bottom, left, i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
		if (pieces[i].used == 1) continue;
                //printf("a ");

		if (left != 30) {
			if (pieces_reference[i].a != left &&
				pieces_reference[i].b != left &&
				pieces_reference[i].c != left &&
				pieces_reference[i].d != left) continue;
		}
                //printf("b ");
		if (top != 30) {
			if (pieces_reference[i].a != top &&
				pieces_reference[i].b != top &&
				pieces_reference[i].c != top &&
				pieces_reference[i].d != top) continue;
		}
                //printf("c ");
		if (right != 30) {
			if (pieces_reference[i].a != right &&
				pieces_reference[i].b != right &&
				pieces_reference[i].c != right &&
				pieces_reference[i].d != right) continue;
		}
                //printf("d ");
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
			//printf("D%d(%d %d %d %d) ", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
                        printf("options[%d][%d][%d][%d][%d] = %d;\n", w, x, y, z, count, i);
			count++;
			continue;
		}
                //printf("f ");

		if ( ( (top    == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == top) &&
		     ( (right  == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == right) &&
		     ( (bottom == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == bottom) &&
		     ( (left   == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == left) ) {
			//printf("D%d(%d %d %d %d) ", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
                        printf("options[%d][%d][%d][%d][%d] = %d;\n", w, x, y, z, count, i);
			count++;
			continue;
		}
                //printf("g ");
		if ( ( (top    == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == top) &&
		     ( (right  == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == right) &&
		     ( (bottom == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == bottom) &&
		     ( (left   == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == left) ) {
			//printf("D%d(%d %d %d %d) ", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
                        printf("options[%d][%d][%d][%d][%d] = %d;\n", w, x, y, z, count, i);
			count++;
			continue;
                //printf("h ");
		}
		if ( ( (top    == 30 && pieces_reference[i].b != 0) || pieces_reference[i].b == top) &&
		     ( (right  == 30 && pieces_reference[i].c != 0) || pieces_reference[i].c == right) &&
		     ( (bottom == 30 && pieces_reference[i].d != 0) || pieces_reference[i].d == bottom) &&
		     ( (left   == 30 && pieces_reference[i].a != 0) || pieces_reference[i].a == left) ) {
			//printf("D%d(%d %d %d %d) ", i, pieces_reference[i].a, pieces_reference[i].b, pieces_reference[i].c, pieces_reference[i].d);
                        printf("options[%d][%d][%d][%d][%d] = %d;\n", w, x, y, z, count, i);
			count++;
			continue;
		}
		//printf("\n");
	}
        printf("options_lengths[%d][%d][%d][%d] = %d;\n", w, x, y, z, count);
	return count;
}

