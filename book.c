/*
MIT License

Copyright (c) 2000 Adrien M. Regimbald

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/**************************************************
 * Faile version 1.4                              *
 * Author: Adrien Regimbald                       *
 * E-mail: adrien@ee.ualberta.ca                  *
 * Web Page: http://www.ee.ualberta.ca/~adrien/   *
 *                                                *
 * File: book.c                                   *
 * Purpose: misc. functions for EPD, SAN, PGN etc *
 * as well as book creation and use in Faile      *
 **************************************************/

#include "faile.h"
#include "extvars.h"
#include "protos.h"


#define B_HASH_MB 32

typedef struct {
  d_long hash;
  unsigned short int freq;
} b_hash_s;

b_hash_s *b_hash_table;

unsigned long int b_hash_mask, collisions = 0;


void b_hash_report (void);
void b_shut_down (int status);
void init_b_hash_tables (void);
unsigned short int search_book (FILE *book, d_long cur_pos);
void update_b_hash (d_long cur_pos);
void write_book (void);


void b_hash_report (void) {

  /* print out a report on what we have stored in the book hash table: */

  unsigned long int i, counter = 0, over_100 = 0, over_50 = 0, over_10 = 0,
    under_10 = 0, num_1 = 0, f = 0;

  for (i = 0; i <= b_hash_mask; i++) {
    f = b_hash_table[i].freq;
    if (f) {
      if (counter < ULONG_MAX)
	counter++;
      if (f > 100) {
	if (over_100 < ULONG_MAX)
	  over_100++;
      }
      else if (f > 50) {
	if (over_50 < ULONG_MAX)
	  over_50++;
      }
      else if (f > 10) {
	if (over_10 < ULONG_MAX)
	  over_10++;
      }
      else if (f > 1) {
	if (under_10 < ULONG_MAX)
	  under_10++;
      }
      else {
	if (num_1 < ULONG_MAX)
	  num_1++;
      }
    }
  }

  printf ("\nBook Hash Table Report\n");
  printf ("(Counter max is %lu)\n\n", ULONG_MAX);
  printf ("Positions found: %lu\n", counter);
  printf ("Number of Hash Collisions: %lu\n", collisions);
  printf ("Positions occuring > 100 times: %lu\n", over_100);
  printf ("50 < Positions occuring <= 100 times: %lu\n", over_50);
  printf ("10 < Positions occuring <= 50 times: %lu\n", over_10);
  printf ("1 < Positions occuring <= 10 times: %lu\n", under_10);
  printf ("Positions occuring 1 time: %lu\n", num_1);

}


void b_shut_down (int status) {

  /* shut down Faile from book creation: */

  free (hash_table);
  free (b_hash_table);
  exit (status);

}


move_s book_move (int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]) {

  /* select a move from the book.  Try to favour the move that occurs the
     most, but also add some randomness to it so that we don't play the same
     book moves over and over. */

  move_s move = dummy, moves[MOVE_BUFF], ign_me;
  int num_moves, i, ep_temp;
  d_long temp_hash;
  unsigned long int scores[MOVE_BUFF] = {0}, best_score = 0;
  FILE *book_f;
  static bool use_book = TRUE;
  char str_move[6];

  /* don't bother doing anything if we are past the max book ply: */
  if (game_ply > MAX_B_PLY || !use_book) {
    return (move);
  }

  if ((book_f = fopen ("faile.obk", "rb")) == NULL) {
    fprintf (stderr, "Couldn't open book file faile.obk!\n");
    fprintf (stderr, "Proceeding without book.\n");
    use_book = FALSE;
    return (move);
  }

  num_moves = 0;
  ep_temp = ep_square;
  temp_hash = cur_pos;

  srand ((int) time (0));

  /* generate moves: */
  gen (&moves[0], &num_moves, white_to_move, ep_square, captures, board, moved, pieces, num_pieces);

  /* loop through the moves: */
  for (i = 0; i < num_moves; i++) {
    make (&moves[0], i, &white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, &ep_square, board, moved, pieces, &piece_count, rep_history, &game_ply, &fifty, fifty_move, squares, ply, &cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);
    assert (cur_pos.x1 == compute_hash (board, moved).x1 &&
	    cur_pos.x2 == compute_hash (board, moved).x2);
    ply++;

    /* check in the book if this is a legal move: */
    if (check_legal (&moves[0], i, white_to_move, wking_loc, bking_loc, board)) {
      scores[i] = search_book (book_f, cur_pos);
    }

    ply--;
    unmake (&moves[0], i, &white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, board, moved, pieces, &piece_count, &game_ply, &fifty, fifty_move, squares, ply);
    ep_square = ep_temp;
    cur_pos = temp_hash;
  }

  /* find the top frequency: */
  for (i = 0; i < num_moves; i++) {
    if (scores[i] > best_score) {
      best_score = scores[i];
    }
  }

  /* add some randomness to each frequency: */
  for (i = 0; i < num_moves; i++) {
    if (scores[i]) {
      scores[i] += rand () % (best_score+15);
    }
  }

  /* now pick our best move: */
  best_score = 0;
  for (i = 0; i < num_moves; i++) {
    if (scores[i] > best_score) {
      best_score = scores[i];
      move = moves[i];
    }
  }

  fclose (book_f);

  /* make sure our book move is legal in the current position: */
  comp_to_coord (move, str_move);
  if (verify_coord (str_move, &ign_me, white_to_move, white_castled, black_castled, wking_loc, bking_loc, ep_square, captures, board, moved, pieces, num_pieces, piece_count, rep_history, game_ply, fifty, fifty_move, squares, ply, cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values)) {
    return (move);
  }
  else {
    return (dummy);
  }

}


void init_b_hash_tables (void) {

  /* this function allocates space for the book hash tables, as well as setting
     some necessary values for storing hashes */

  int i = 1;
  unsigned long int b_max_hash = 1, b_element_size = sizeof (b_hash_s);

  /* problems to solve...
   * 1/ this is non-portable
   * 2/ I'm quite certain there's a better way to calculate that power of 2...
   * 3/ that power of 2 might be greater than SIZE_MAX, so passing to `malloc`
   *    is bad! Lecture me again on array processing, daddeh; I love it ;)
   */

  /* compute the maximum book hash element, based upon size desired: */
  while (b_max_hash * b_element_size <= B_HASH_MB<<19) {
    assert(INT_MAX > (1UL << i));  // You really do need that UL-suffix, right, daddeh?
    b_max_hash = 1/*UL*/ << i++;   // ... I'll leave you to make that change... baws man.
  }

  /* compute the book hash mask, for computing book hash table indices: */
  b_hash_mask = b_max_hash-1;

  // I'll also leave you to test this...
# ifdef ALTERNATIVE
# define MB * (1UL << 19)
  unsigned long my_max_hash = (32 MB) / b_element_size;
  while (my_max_hash & -~my_max_hash) // why is this a better loop?
    my_max_hash |= my_max_hash >> i++;
  my_max_hash++;
  assert(b_max_hash == my_max_hash); 
# endif  

  /* allocate our book hash table's memory, and report on the allocation: */
  assert(SIZE_MAX / b_element_size >= b_max_hash); // but we don't want an implicit conversion to cause non-portable lossy truncation, right?
  if ((b_hash_table = malloc (b_max_hash*b_element_size)) == NULL) {
    fprintf (stderr, "Couldn't allocate memory for book hash tables!\n");
    b_shut_down (EXIT_FAILURE);
  }
  printf ("\nBook Creation Memory Allocation:\n");
  printf ("%lu book hash entries * %lu bytes/entry = %lu kb of RAM\n",
	  b_max_hash, b_element_size, (b_max_hash*b_element_size)>>10);

  /* clear our hash tables of possible errant values: */
  memset (b_hash_table, 0, b_max_hash*b_element_size);

}


void init_book (void) {

  int i = 1;
  unsigned long int b_max_hash = 1, b_element_size = sizeof (b_hash_s);

  /* compute the maximum book hash element, based upon size desired: */
  while (b_max_hash * b_element_size <= B_HASH_MB<<19) {
    b_max_hash = 1 << i++;
  }

  /* compute the book hash mask, for computing book hash table indices: */
  b_hash_mask = b_max_hash-1;

}


bool is_castle (char c) {

  /* determine if the character is a castling character */

  if (c == '0' || c == 'O' || c == 'o')
    return TRUE;
  else
    return FALSE;

}


bool is_column (char c) {

  /* determine if the character is a column specifier */

  if (c >= 'a' && c <= 'h')
    return TRUE;
  else
    return FALSE;

}


bool is_comment (char *input) {

  /* determine if input is the start of a comment string */

  char *ptr = input;

  while (isspace ((int) *ptr) && *ptr != '\0') {
    ptr++;
  }

  if (*ptr == '[') {
    return TRUE;
  }
  else {
    return FALSE;
  }

}


bool is_rank (char c) {

  /* determine if the character is a rank specifier */

  if (c >= '1' && c <= '8')
    return TRUE;
  else
    return FALSE;

}


bool is_seperator (char c) {

  /* determine if the character is a seperator for the two parts of a move */

  if (c == 'x' || c == 'X' || c == '-' || c == ':')
    return TRUE;
  else
    return FALSE;

}


bool is_trailing (char c) {

  /* determine if the character is a trailing one (check, checkmate, etc) */

  if (c == '+' || c == '#')
    return TRUE;
  else
    return FALSE;

}


void make_book (char *file_name, int max_ply) {

  /* Make our book file faile.obk from the input file named file_name,
     using a maximum ply depth of max_ply */

  FILE *pgn_in;
  enum {s_none, s_comment, s_moves} book_state = s_none;
  char input[STR_BUFF];
  float ign_me;
  unsigned long int game_count = 0;
  bool legal = TRUE;
  move_s move = dummy;
  int white_to_move, white_castled, black_castled, wking_loc, bking_loc, ep_square, board[144], moved[144], pieces[33], num_pieces, game_ply, fifty, fifty_move[PV_BUFF], squares[144], ply = 0;
  bool captures;
  long piece_count;
  d_long rep_history[PV_BUFF], cur_pos, wck_h_values[2], wcq_h_values[2], bck_h_values[2], bcq_h_values[2], h_values[14][144], ep_h_values[144], color_h_values[2];

  if ((pgn_in = fopen (file_name, "r")) == NULL) {
    fprintf (stderr, "Couldn't open file %s!\n", file_name);
    shut_down (EXIT_FAILURE);
  }

  init_hash_values (wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);
  init_b_hash_tables ();
  init_game (&white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, &ep_square, &captures, board, moved, pieces, &num_pieces, &piece_count, rep_history, &game_ply, &fifty, squares, &cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);

  printf ("\nMaking a new book from input file %s.\n", file_name);
  printf ("(Max book ply of %d)\n", max_ply);
  printf ("(Each \".\" represents 100 games processed)\n\n");

  /* read in the PGN file: */
  while (!feof (pgn_in) && !ferror (pgn_in)) {
    if (game_count >= ULONG_MAX) {
      fprintf (stderr, "Game count has reached %lu, exiting..\n", ULONG_MAX);
      b_shut_down (EXIT_FAILURE);
    }
    sprintf (input, "-");
    if (book_state == s_comment) {
      fscanf (pgn_in, "%s", input);
      if (is_comment (input)) {
	fscanf (pgn_in, "%*[^\n]");
      }
      else {
	book_state = s_moves;
      }
    }
    else if (book_state == s_moves) {
      fscanf (pgn_in, "%s", input);
      if (is_comment (input)) {
	fscanf (pgn_in, "%*[^\n]");
	book_state = s_comment;
	show_counter (++game_count);
	init_game (&white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, &ep_square, &captures, board, moved, pieces, &num_pieces, &piece_count, rep_history, &game_ply, &fifty, squares, &cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);
	legal = TRUE;
      }
    }
    else {
      fscanf (pgn_in, "%s", input);
      if (is_comment (input)) {
	fscanf (pgn_in, "%*[^\n]");
	book_state = s_comment;
	show_counter (++game_count);
      }
      else {
	book_state = s_moves;
      }
    }

    /* if we have a move, take a look at it: */
    if (book_state == s_moves) {
      if (possible_move (input)) {
	if (legal) {
	  if (is_valid_comp (pgn_to_comp (input, white_to_move, white_castled, black_castled, wking_loc, bking_loc, ep_square, captures, board, moved, pieces, num_pieces, piece_count, rep_history, game_ply, fifty, fifty_move, squares, ply, cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values))) {
	    /* we have a legal move: */
	    move = pgn_to_comp (input, white_to_move, white_castled, black_castled, wking_loc, bking_loc, ep_square, captures, board, moved, pieces, num_pieces, piece_count, rep_history, game_ply, fifty, fifty_move, squares, ply, cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);
	    make (&move, 0, &white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, &ep_square, board, moved, pieces, &piece_count, rep_history, &game_ply, &fifty, fifty_move, squares, ply, &cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);
	    reset_piece_square (board, pieces, &num_pieces, squares);
	    /* add to our book if it's less than max_ply: */
	    if (game_ply <= max_ply) {
	      update_b_hash (cur_pos);
	    }
	    /* no point in checking for legality if we're not going to store
	       the move anyways: */
	    else {
	      legal = FALSE;
	    }
	  }
	  else {
	    /* we've received an illegal move, so read over the rest of this
	       game, but don't look at the moves: */
	    legal = FALSE;
	  }
	}
      }
      else {
	/* we have unexpected input: */
	if (!sscanf (input, "%e", &ign_me) && strcmp (input, "ep") &&
	    strcmp (input, "e.p.") && strcmp (input, "1-0") &&
	    strcmp (input, "0-1") && strcmp (input, "1/2-1/2") &&
	    strcmp (input, "*")) {
	  fprintf (stdout, "Unexpected input: %s\n", input);
	}
      }
    }
  }

  printf ("\n\nTotal games found: %ld\n", game_count);

  b_hash_report ();

  write_book ();

  fclose (pgn_in);

  b_shut_down (EXIT_SUCCESS);

}


bool possible_move (char *input) {

  /* check to see if the input could possibly be a move.  Basically, this
     function is like pgn_to_comp, but it only determines if the input
     could be a move or not, and doesn't decode the move at all */

  bool legal = TRUE;
  const char *move = input;

  /* try a pawn move first: */
  if (is_column (*move)) {
    move++;
    if (is_rank (*move)) {
      move++;
      if (is_seperator (*move)) {
	move++;
      }
      if (is_column (*move)) {
	/* full e2e4 style */
	move++;
	if (is_rank (*move)) {
	  move++;
	}
      }
      else {
	/* shorter e4 style */
      }
    }
    else {
      /* bc style of pawn capture */
      if (is_seperator (*move)) {
	move++;
      }
      if (is_column (*move)) {
	move++;
	if (is_rank (*move)) {
	  move++;
	}
      }
      else {
	legal = FALSE;
      }
    }
    /* check for promotions: */
    if (legal) {
      if (*move == '=') {
	move++;
      }
      if (which_piece ((char) toupper (*move)) != p_none) {
	move++;
      }
    }
  }
  else if (which_piece (*move) != p_none) {
    move++;
    if (is_rank (*move)) {
      /* using rank to remove ambiguity */
      move++;
      if (is_seperator (*move)) {
	move++;
      }
      if (is_column (*move)) {
	move++;
	if (is_rank (*move)) {
	  move++;
	}
	else {
	  legal = FALSE;
	}
      }
      else {
	legal = FALSE;
      }
    }
    else {
      if (is_seperator (*move)) {
	move++;
	if (is_column (*move)) {
	  move++;
	  if (is_rank (*move)) {
	    move++;
	  }
	  else {
	    legal = FALSE;
	  }
	}
	else {
	  legal = FALSE;
	}
      }
      else if (is_column (*move)) {
	move++;
	if (is_seperator (*move)) {
	  move++;
	}
	if (is_rank (*move)) {
	  move++;
	  if (is_seperator (*move)) {
	    move++;
	  }
	  if (is_column (*move)) {
	    /* Nb1d2 */
	    move++;
	    if (is_rank (*move)) {
	      move++;
	    }
	    else {
	      legal = FALSE;
	    }
	  }
	  else {
	  }
	}
	else if (is_column (*move)) {
	  /* Nbd2 */
	  move++;
	  if (is_rank (*move)) {
	    move++;
	  }
	  else {
	    legal = FALSE;
	  }
	}
	else {
	  legal = FALSE;
	}
      }
      else {
	legal = FALSE;
      }
    }
  }
  else if (is_castle (*move)) {
    /* a castling move */
    move++;
    if (*move == '-') {
      move++;
    }
    if (is_castle (*move)) {
      move++;
      if (*move == '-') {
	move++;
      }
      if (is_castle (*move)) {
	move++;
      }
      else {
      }
    }
    else {
      legal = FALSE;
    }
  }
  else {
    legal = FALSE;
  }

  if (!legal) {
    return (FALSE);
  }

  /* deal with trailing crap after the move: */
  while (is_trailing (*move)) {
    move++;
  }
  if (*move != '\0' && strcmp (move, "ep") && strcmp (move, "e.p.")) {
    legal = FALSE;
  }


  return (legal);
}


move_s pgn_to_comp (const char *input, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]) {

  /* try to translate a "reasonable" PGN/SAN move into Faile's internal
     move format.  The algorithm for this function is based upon the
     source from extract by David Barnes. */

  move_s ret_move = dummy, moves[MOVE_BUFF];
  bool pawn_move = FALSE, legal = TRUE,
    prom_move = FALSE;
  int tmp_rank = 0, tmp_col = 0, from_rank = 0, from_col = 0, to_rank = 0,
    to_col = 0, piece_to_move = p_none, prom_piece = npiece, num_matches = 0,
    num_moves, i, ep_temp;
  piece_t piece_type = npiece, prom_type = npiece;
  const char *move = input;
  d_long temp_hash;

  /* try a pawn move first: */
  if (is_column (*move)) {
    pawn_move = TRUE;
    piece_to_move = (white_to_move ? wpawn : bpawn);
    tmp_col = (*move)-'a'+1;
    move++;
    if (is_rank (*move)) {
      tmp_rank = (*move)-'1'+1;
      move++;
      if (is_seperator (*move)) {
	move++;
      }
      if (is_column (*move)) {
	/* full e2e4 style */
	from_col = tmp_col;
	from_rank = tmp_rank;
	to_col = (*move)-'a'+1;
	move++;
	if (is_rank (*move)) {
	  to_rank = (*move)-'1'+1;
	  move++;
	}
      }
      else {
	/* shorter e4 style */
	to_col = tmp_col;
	to_rank = tmp_rank;
      }
    }
    else {
      /* bc style of pawn capture */
      if (is_seperator (*move)) {
	move++;
      }
      if (is_column (*move)) {
	from_col = tmp_col;
	to_col = (*move)-'a'+1;
	move++;
	if (is_rank (*move)) {
	  to_rank = (*move)-'1'+1;
	  move++;
	}
      }
      else {
	legal = FALSE;
      }
    }
    /* check for promotions: */
    if (legal) {
      if (*move == '=') {
	move++;
      }
      prom_type = which_piece ((char) toupper (*move));
      switch (prom_type) {
        case (p_Q):
	  prom_piece = (white_to_move ? wqueen : bqueen);
	  prom_move = TRUE;
	  move++;
	  break;
        case (p_R):
	  prom_piece = (white_to_move ? wrook : brook);
	  prom_move = TRUE;
	  move++;
	  break;
        case (p_B):
	  prom_piece = (white_to_move ? wbishop : bbishop);
	  prom_move = TRUE;
	  move++;
	  break;
        case (p_N):
	  prom_piece = (white_to_move ? wknight : bknight);
	  prom_move = TRUE;
	  move++;
	  break;
        default:
	  break;
      }
    }
  }
  else if ((piece_type = which_piece (*move)) != p_none) {
    switch (piece_type) {
      case (p_Q):
	piece_to_move = (white_to_move ? wqueen : bqueen);
	break;
      case (p_R):
	piece_to_move = (white_to_move ? wrook : brook);
	break;
      case (p_B):
	piece_to_move = (white_to_move ? wbishop : bbishop);
	break;
      case (p_N):
	piece_to_move = (white_to_move ? wknight : bknight);
	break;
      case (p_K):
	piece_to_move = (white_to_move ? wking : bking);
	break;
      default:
	legal = FALSE;
	break;
    }
    move++;
    if (is_rank (*move)) {
      /* using rank to remove ambiguity */
      from_rank = (*move)-'1'+1;
      move++;
      if (is_seperator (*move)) {
	move++;
      }
      if (is_column (*move)) {
	to_col = (*move)-'a'+1;
	move++;
	if (is_rank (*move)) {
	  to_rank = (*move)-'1'+1;
	  move++;
	}
	else {
	  legal = FALSE;
	}
      }
      else {
	legal = FALSE;
      }
    }
    else {
      if (is_seperator (*move)) {
	move++;
	if (is_column (*move)) {
	  to_col = (*move)-'a'+1;
	  move++;
	  if (is_rank (*move)) {
	    to_rank = (*move)-'1'+1;
	    move++;
	  }
	  else {
	    legal = FALSE;
	  }
	}
	else {
	  legal = FALSE;
	}
      }
      else if (is_column (*move)) {
	tmp_col = (*move)-'a'+1;
	move++;
	if (is_seperator (*move)) {
	  move++;
	}
	if (is_rank (*move)) {
	  tmp_rank = (*move)-'1'+1;
	  move++;
	  if (is_seperator (*move)) {
	    move++;
	  }
	  if (is_column (*move)) {
	    /* Nb1d2 */
	    from_col = tmp_col;
	    from_rank = tmp_rank;
	    to_col = (*move)-'a'+1;
	    move++;
	    if (is_rank (*move)) {
	      to_rank = (*move)-'1'+1;
	      move++;
	    }
	    else {
	      legal = FALSE;
	    }
	  }
	  else {
	    to_col = tmp_col;
	    to_rank = tmp_rank;
	  }
	}
	else if (is_column (*move)) {
	  /* Nbd2 */
	  from_col = tmp_col;
	  to_col = (*move)-'a'+1;
	  move++;
	  if (is_rank (*move)) {
	    to_rank = (*move)-'1'+1;
	    move++;
	  }
	  else {
	    legal = FALSE;
	  }
	}
	else {
	  legal = FALSE;
	}
      }
      else {
	legal = FALSE;
      }
    }
  }
  else if (is_castle (*move)) {
    /* a castling move */
    move++;
    from_col = 5;
    from_rank = (white_to_move ? 1 : 8);
    piece_to_move = (white_to_move ? wking : bking);
    if (*move == '-') {
      move++;
    }
    if (is_castle (*move)) {
      move++;
      if (*move == '-') {
	move++;
      }
      if (is_castle (*move)) {
	move++;
	to_col = 3;
	to_rank = (white_to_move ? 1 : 8);
      }
      else {
	to_col = 7;
	to_rank = (white_to_move ? 1 : 8);
      }
    }
    else {
      legal = FALSE;
    }
  }
  else {
    legal = FALSE;
  }

  if (!legal) {
    return (ret_move);
  }

  /* deal with trailing crap after the move: */
  while (is_trailing (*move)) {
    move++;
  }
  if (*move != '\0' && (pawn_move && (strcmp (move, "ep") &&
				      strcmp (move, "e.p.")))) {
    legal = FALSE;
  }

  /* try to match up our details with a move: */
  num_moves = 0;
  gen (&moves[0], &num_moves, white_to_move, ep_square, captures, board, moved, pieces, num_pieces);

  /* compare the info we have now to what is generated: */
  for (i = 0; i < num_moves; i++) {
    if (board[moves[i].from] == piece_to_move) {
      legal = TRUE;
      if (from_col && file (moves[i].from) != from_col) {
	legal = FALSE;
      }
      if (from_rank && rank (moves[i].from) != from_rank) {
	legal = FALSE;
      }
      if (to_col && file (moves[i].target) != to_col) {
	legal = FALSE;
      }
      if (to_rank && rank (moves[i].target) != to_rank) {
	legal = FALSE;
      }
      if (prom_move && moves[i].promoted != prom_piece) {
	legal = FALSE;
      }
      if (legal) {
	temp_hash = cur_pos;
	ep_temp = ep_square;
	make (&moves[0], i, &white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, &ep_square, board, moved, pieces, &piece_count, rep_history, &game_ply, &fifty, fifty_move, squares, ply, &cur_pos, wck_h_values, wcq_h_values, bck_h_values, bcq_h_values, h_values, ep_h_values, color_h_values);
	ply++;
	if (check_legal (&moves[0], i, white_to_move, wking_loc, bking_loc, board)) {
	  ret_move = moves[i];
	  num_matches++;
	}
	ply--;
	unmake (&moves[0], i, &white_to_move, &white_castled, &black_castled, &wking_loc, &bking_loc, board, moved, pieces, &piece_count, &game_ply, &fifty, fifty_move, squares, ply);
	ep_square = ep_temp;
	cur_pos = temp_hash;
      }
    }
  }

  /* check for ambiguity in the move entered: */
  if (num_matches > 1) {
    return (dummy);
  }

  /* if we've made it here, we have a match on a legal move (finally :P) */
  return (ret_move);

}


unsigned short int search_book (FILE *book, d_long cur_pos) {

  /* search through the opening book, and see if we can find the position
     in question, and return the frequency for the position */

  unsigned long int target, start = 0, end = 0, index_mask = 0;
  long index = 0;
  b_hash_s b_hash;

  /* calculate our target: */
  target = b_hash_mask & cur_pos.x1;

  /* get the number of book hash entries: */
  fseek (book, -sizeof (unsigned long int), SEEK_END);
  fread (&end, sizeof (unsigned long int), 1, book);
  if (!end) {
    fprintf (stderr, "Error reading number of book entries from book!\n");
    fclose (book);
    shut_down (EXIT_FAILURE);
  }
  end -= 1; /* 0 indexed */

  /* start off from the beginning of the book file: */
  rewind (book);

  /* perform binary search to find a match: */
  while (start <= end) {
    index = (start+end)/2;
    fseek (book, index * sizeof (b_hash_s), SEEK_SET);
    fread (&b_hash, sizeof (b_hash_s), 1, book);
    index_mask = b_hash_mask & b_hash.hash.x1;
    if (index_mask == target) {
      break;
    }
    else if (target < index_mask) {
      end = index-1;
    }
    else {
      start = index+1;
    }
  }

  if (b_hash.hash.x1 == cur_pos.x1 && b_hash.hash.x2 == cur_pos.x2) {
    return (b_hash.freq);
  }
  else {
    /* try to look to the "left" and "right" of the entry for entries that
       were possibly there due to linear probing: */
    start = index;
    /* try left: */
    index--;
    while (index >= 0 && index_mask == target) {
      fseek (book, index * sizeof (b_hash_s), SEEK_SET);
      fread (&b_hash, sizeof (b_hash_s), 1, book);
      index_mask = b_hash_mask & b_hash.hash.x1;
      if (b_hash.hash.x1 == cur_pos.x1 && b_hash.hash.x2 == cur_pos.x2) {
	return (b_hash.freq);
      }
      index--;
    }
    /* try right: */
    index = start + 1;
    index_mask = target;
    while (index <= (long) b_hash_mask && index_mask == target) {
      fseek (book, index * sizeof (b_hash_s), SEEK_SET);
      fread (&b_hash, sizeof (b_hash_s), 1, book);
      index_mask = b_hash_mask & b_hash.hash.x1;
      if (b_hash.hash.x1 == cur_pos.x1 && b_hash.hash.x2 == cur_pos.x2) {
	return (b_hash.freq);
      }
      index++;
    }
  }

  return (0);

}


void show_counter (long int game_count) {

  /* Show a progress counter: */

  if (!(game_count % 100) && game_count != 0) {
    printf (".");
  }
  if (!(game_count % 5000) && game_count != 0) {
    printf (" (%ld)\n", game_count);
  }

}


void update_b_hash (d_long cur_pos) {

  /* update a position in the book hash table: */

  b_hash_s *b_hash_p;
  d_long hash_tmp;
  unsigned long int index;

  /* look for the correct place to put the hash: */
  b_hash_p = b_hash_table + (b_hash_mask & cur_pos.x1);

  /* if this position hasn't been seen before, store our key in the book
     hash table: */
  if (!(b_hash_p->freq)) {
    b_hash_p->hash = cur_pos;
    b_hash_p->freq = 1;
    return;
  }

  /* look for the right slot (I'm using the linear probing scheme): */
  hash_tmp = b_hash_p->hash;
  /* keep track of the end of the book hash table: */
  index = b_hash_mask & cur_pos.x1;
  while ((hash_tmp.x1 != cur_pos.x1 || hash_tmp.x2 != cur_pos.x2) &&
	 (b_hash_mask & cur_pos.x1) == (b_hash_mask & hash_tmp.x1) &&
	 (index < b_hash_mask)) {
    b_hash_p++;
    hash_tmp = b_hash_p->hash;
    index++;
  }

  /* if the slot we've arrived at isn't empty, and the hash stored in it
     isn't the right one, we have a collision: */
  if ((b_hash_p->freq) &&
      (hash_tmp.x1 != cur_pos.x1 || hash_tmp.x2 != cur_pos.x2)) {
    if (collisions < ULONG_MAX)
      collisions++;
    return;
  }

  /* if the number of occurances isn't larger than an unsigned short can
     handle, then increment the frequency counter: */
  if (b_hash_p->freq < USHRT_MAX) {
    (b_hash_p->freq)++;
  }

}


piece_t which_piece (char c) {

  /* determine which kind of piece (if any) this character represents */

  piece_t piece;

  switch (c) {
    case 'k': /* fall through */
    case 'K':
      piece = p_K;
      break;
    case 'q': /* fall through */
    case 'Q':
      piece = p_Q;
      break;
    case 'r': /* fall through */
    case 'R':
      piece = p_R;
      break;
    case 'n': /* fall through */
    case 'N':
      piece = p_N;
      break;
    case 'B':
      piece = p_B;
      break;
    default:
      piece = p_none;
      break;
  }

  return piece;

}


void write_book (void) {

  /* write our book to the binary file faile.obk */

  FILE *book_out;
  b_hash_s b_hash;
  unsigned long int counter = 0, i;

  if ((book_out = fopen ("faile.obk", "wb")) == NULL) {
    fprintf (stderr, "Couldn't open file faile.obk for writing!\n");
    b_shut_down (EXIT_FAILURE);
  }

  /* make sure we are at the start of the book: */
  rewind (book_out);

  /* write our book: */
  for (i = 0; i <= b_hash_mask; i++) {
    b_hash = b_hash_table[i];
    if (b_hash.freq > 1) {
      counter++;
      fwrite (&b_hash, sizeof (b_hash_s), 1, book_out);
    }
  }

  /* put the total number of lines at the end: */
  fwrite (&counter, sizeof (unsigned long int), 1, book_out);

  /* output some stats: */
  printf ("\nBook File Creation:\n");
  printf ("%lu book entries * %lu bytes/entry ~= %lu kb book size\n",
	  counter, sizeof (b_hash_s), ((counter*(sizeof (b_hash_s)))>>10));

  fclose (book_out);

}
