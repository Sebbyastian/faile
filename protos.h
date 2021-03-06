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
 * File: protos.h                                 *
 * Purpose: holds prototypes for functions        *
 **************************************************/

#ifndef PROTOS_H
#define PROTOS_H

long int allocate_time (void);
int bioskey (void);
move_s book_move (const int white_to_move, const int white_castled, const int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
bool check_legal (move_s moves[], int m, const int white_to_move, const int wking_loc, const int bking_loc, int board[]);
long int chk_hash (int alpha, int beta, int depth, int *type, move_s *move, int ply, d_long cur_pos);
void comp_to_coord (move_s move, char str[]);
d_long compute_hash (const int white_to_move, const int ep_square, int board[], int moved[], int pieces[], const int num_pieces, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void display_board (FILE *stream, int color, int board[]);
void hash_to_pv (int depth, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
long int end_eval (const int white_to_move, int board[], int pieces[], const int num_pieces, const long piece_count, const long start_piece_count);
long int eval (const int white_to_move, const int white_castled, const int black_castled, const int wking_loc, const int bking_loc, int board[], int moved[], int pieces[], const int num_pieces, const long piece_count, const long start_piece_count);
void gen (move_s moves[], int *num_moves, const int white_to_move, const int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces);
void ics_game_end (void);
bool in_check (const int white_to_move, const int wking_loc, const int bking_loc, int board[]);
void init_book (void);
void init_game (int *white_to_move, int *white_castled, int *black_castled, int *wking_loc, int *bking_loc, int *ep_square, bool *captures, int board[], int moved[], int pieces[], int *num_pieces, long *piece_count, d_long rep_history[], int *game_ply, int *fifty, int squares[], d_long *cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void init_hash_tables (void);
void init_hash_values (d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
bool is_attacked (int square, int color, int board[]);
bool is_castle (char c);
bool is_column (char c);
bool is_comment (char *input);
bool is_draw (d_long rep_history[], int game_ply, int fifty, int ply, d_long cur_pos);
bool is_move (char str[]);
bool is_rank (char c);
bool is_seperator (char c);
bool is_trailing (char c);
bool is_valid_comp (move_s in_move);
void make (move_s moves[], int i, int *white_to_move, int *white_castled, int *black_castled, int *wking_loc, int *bking_loc, int *ep_square, int board[], int moved[], int pieces[], long *piece_count, d_long rep_history[], int *game_ply, int *fifty, int fifty_move[], int squares[], int ply, d_long *cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void make_book (char *file_name, int max_ply);
long int mid_eval (const int white_to_move, const int white_castled, const int black_castled, const int wking_loc, const int bking_loc, int board[], int moved[], int pieces[], const int num_pieces, const long piece_count, const long start_piece_count);
long int opn_eval (const int white_to_move, const int white_castled, const int black_castled, const int wking_loc, const int bking_loc, int board[], int moved[], int pieces[], const int num_pieces);
void order_moves (move_s moves[], long int move_ordering[], int num_moves, move_s *h_move, int board[], int ply);
void perft (int depth, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, long int *raw_nodes, int board[], int moved[], int perft[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void parse_cmdline (int argc, char *argv[]);
void perft_debug (void);
move_s pgn_to_comp (const char *input, int white_to_move, int white_castled, int black_castled, const int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
bool possible_move (char *input);
void post_thinking (long int score);
void print_move (move_s moves[], int m, FILE *stream);
void push_king (move_s moves[], int *num_moves, int from, int target, int castle_type, const bool captures, int board[]);
void push_knight (move_s moves[], int *num_moves, int from, int target, const bool captures, int board[]);
void push_pawn (move_s moves[], int *num_moves, int from, int target, bool is_ep, int board[]);
void push_slide (move_s moves[], int *num_moves, int from, int target, const bool captures, int board[]);
long int qsearch (int alpha, int beta, int depth, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, const long start_piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void rdelay (int time_in_s);
long int rdifftime (rtime_t end, rtime_t start);
void refresh_hash (void);
bool remove_one (int *marker, long int move_ordering[], int num_moves);
void reset_piece_square (int board[], int pieces[], int *num_pieces, int squares[]);
void rinput (char str[], int n, FILE *stream);
rtime_t rtime (void);
long int search (int alpha, int beta, int depth, bool do_null, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, const long start_piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
move_s search_root (int alpha, int beta, int depth, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void shut_down (int status);
void show_counter (long int game_count);
void start_up (void);
void store_hash (int depth, int score, int flag, move_s move, int ply, d_long cur_pos);
move_s think (const int white_to_move, const int white_castled, const int black_castled, const int wking_loc, const int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, const long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void toggle_bool (bool *var);
void tree (int depth, int indent, FILE *output, char *disp_b, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
void tree_debug (void);
void unmake (move_s moves[], int i, int *white_to_move, int *white_castled, int *black_castled, int *wking_loc, int *bking_loc, int board[], int moved[], int pieces[], long *piece_count, int *game_ply, int *fifty, int fifty_move[], int squares[], int ply);
void u_killers (move_s move, long int score, int ply);
bool verify_coord (char input[], move_s *move, int white_to_move, int white_castled, int black_castled, int wking_loc, int bking_loc, int ep_square, const bool captures, int board[], int moved[], int pieces[], const int num_pieces, long piece_count, d_long rep_history[], int game_ply, int fifty, int fifty_move[], int squares[], int ply, d_long cur_pos, d_long wck_h_values[], d_long wcq_h_values[], d_long bck_h_values[], d_long bcq_h_values[], d_long h_values[][144], d_long ep_h_values[], d_long color_h_values[]);
piece_t which_piece (char c);
void xor (d_long *a, d_long b);

#endif
