#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "snake_utils.h"
#include "state.h"

/* Helper function definitions */
static void set_board_at(game_state_t* state, unsigned int x, unsigned int y, char ch);
static bool is_tail(char c);
static bool is_head(char c);
static bool is_snake(char c);
static char body_to_tail(char c);
static char head_to_body(char c);
static unsigned int get_next_x(unsigned int cur_x, char c);
static unsigned int get_next_y(unsigned int cur_y, char c);
static void find_head(game_state_t* state, unsigned int snum);
static char next_square(game_state_t* state, unsigned int snum);
static void update_tail(game_state_t* state, unsigned int snum);
static void update_head(game_state_t* state, unsigned int snum);


/* Task 1 */
game_state_t* create_default_state() {

  game_state_t* def = (game_state_t*)malloc(sizeof(game_state_t));
  def->board = (char**)malloc(18*sizeof(char*));

  char* defboard = 
  "####################\n"
  "#                  #\n"
  "# d>D    *         #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "#                  #\n"
  "####################\n";
  for (int i = 0; i < 18; i++) {
    def->board[i] = (char*)malloc(sizeof(char)*22);
    def->board[i][21] = '\0';
    strncpy(def->board[i], &defboard[21*i], 21);
  }

  def->num_rows = 18;
  def->num_snakes = 1;

  def->snakes = (snake_t*)malloc(sizeof(snake_t));
  def->snakes[0].head_x = 4;
  def->snakes[0].head_y = 2;
  def->snakes[0].tail_x = 2;
  def->snakes[0].tail_y = 2;
  def->snakes[0].live = true;

  return def;
}


/* Task 2 */
void free_state(game_state_t* state) {
  for (int i = 0; i < state->num_rows; i++) {
    free(state->board[i]);
  }
  free(state->board);
  free(state->snakes);
  free(state);
  return;
}

/* Task 3 */
void print_board(game_state_t* state, FILE* fp) {
  for (int i = 0; i < state->num_rows; i++) {
    fprintf(fp, "%s", state->board[i]);
  }
  return;
}

/*
  Saves the current state into filename. Does not modify the state object.
  (already implemented for you).
*/
void save_board(game_state_t* state, char* filename) {
  FILE* f = fopen(filename, "w");
  print_board(state, f);
  fclose(f);
}


/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_state_t* state, unsigned int x, unsigned int y) {
  return state->board[y][x];
}

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_state_t* state, unsigned int x, unsigned int y, char ch) {
  state->board[y][x] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
  return (strchr("wasd", c) != NULL);
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
  return (strchr("WASDx", c) != NULL);
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<>vWASDx"
*/
static bool is_snake(char c) {
  return (strchr("wasd^<>vWASDx", c) != NULL);
}

/*
  Converts a character in the snake's body ("^<>v")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
  if (c == '^') return 'w';
  if (c == '<') return 'a';
  if (c == 'v') return 's';
  if (c == '>') return 'd';
  return '?';
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<>v").
*/
static char head_to_body(char c) {
  if (c == 'W') return '^';
  if (c == 'A') return '<';
  if (c == 'S') return 'v';
  if (c == 'D') return '>';
  return '?';

}

/*
  Returns cur_x + 1 if c is '>' or 'd' or 'D'.
  Returns cur_x - 1 if c is '<' or 'a' or 'A'.
  Returns cur_x otherwise.
*/
static unsigned int get_next_x(unsigned int cur_x, char c) {
  if (strchr(">dD", c) != NULL) return cur_x + 1;
  if (strchr("<aA", c) != NULL) return cur_x - 1;
  return cur_x;
}

/*
  Returns cur_y + 1 if c is '^' or 'w' or 'W'.
  Returns cur_y - 1 if c is 'v' or 's' or 'S'.
  Returns cur_y otherwise.
*/
static unsigned int get_next_y(unsigned int cur_y, char c) {
  if (strchr("^wW", c) != NULL) return cur_y - 1;
  if (strchr("vsS", c) != NULL) return cur_y + 1;
  return cur_y;
}

/*
  Task 4.2

  Helper function for update_state. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_state_t* state, unsigned int snum) {
  const snake_t* snek = state->snakes + snum;
  const char head = get_board_at(state, snek->head_x, snek->head_y);
  return get_board_at(state, 
  get_next_x(snek->head_x, head),
  get_next_y(snek->head_y, head));
}


/*
  Task 4.3

  Helper function for update_state. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the x and y coordinates of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_state_t* state, unsigned int snum) {
  snake_t* snek = state->snakes + snum;
  const char head = get_board_at(state, snek->head_x, snek->head_y);
  set_board_at(state, snek->head_x, snek->head_y, head_to_body(head));
  snek->head_x = get_next_x(snek->head_x, head);
  snek->head_y = get_next_y(snek->head_y, head);
  set_board_at(state, snek->head_x, snek->head_y, head);
}


/*
  Task 4.4

  Helper function for update_state. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^v<>) into a tail character (wasd)

  ...in the snake struct: update the x and y coordinates of the tail
*/
static void update_tail(game_state_t* state, unsigned int snum) {
  snake_t* snek = state->snakes + snum;
  const char tail = get_board_at(state, snek->tail_x, snek->tail_y);
  set_board_at(state, snek->tail_x, snek->tail_y, ' ');
  snek->tail_x = get_next_x(snek->tail_x, tail);
  snek->tail_y = get_next_y(snek->tail_y, tail);
  const char temp = get_board_at(state, snek->tail_x, snek->tail_y);
  set_board_at(state, snek->tail_x, snek->tail_y, body_to_tail(temp));
}


/* Task 4.5 */
void update_state(game_state_t* state, int (*add_food)(game_state_t* state)) {
  for (unsigned int snum = 0; snum < state->num_snakes; snum++) {
    snake_t* snek = state->snakes + snum;
    if (!snek->live) continue;
    switch (next_square(state, snum)) {
      case '*':
        update_head(state, snum);
        add_food(state);
        continue;
      case ' ':
        update_head(state, snum);
        update_tail(state, snum);
        continue;
      default:
        snek->live = false;
        set_board_at(state, snek->head_x, snek->head_y, 'x');
    }
  }
  return;
}


/* Task 5 */
game_state_t* load_board(char* filename) {
  FILE* fp = fopen(filename, "r");
  fseek(fp, 0, SEEK_END);
  unsigned int end_index = (unsigned int) ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *contents = malloc((end_index + 1) * sizeof(char));
  fread(contents, sizeof(char), end_index + 1, fp);
  unsigned int lines = 0;
  for (int i = 0; i < end_index; i++) {
    if (contents[i] == '\n') lines++;
  }
  game_state_t* retval = malloc(sizeof(game_state_t));
  retval->num_rows = lines;
  retval->board = malloc(lines * sizeof(char*));
  char *ptr = contents;
  char *prev = contents - 1;
  int count = 0;
  while (count < lines) {
    if (*ptr == '\n') {
      unsigned int len = ptr - prev;
      retval->board[count] = (char*)malloc(sizeof(char)*(len + 1));
      strncpy(retval->board[count], prev+1, len+1);
      retval->board[count][len] = '\0';
      count += 1;
      prev = ptr;
    }
    ptr += 1;
  }
  return retval;
}


/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail coordinates filled in,
  trace through the board to find the head coordinates, and
  fill in the head coordinates in the struct.
*/
static void find_head(game_state_t* state, unsigned int snum) {
  snake_t *snek = state->snakes + snum;
  int tail_x = snek->tail_x;
  int tail_y = snek->tail_y;
  const char *heads = "WASDx";
  char search = get_board_at(state, tail_x, tail_y);
  while (strchr(heads, search) == NULL) {
    int temp_x = tail_x;
    tail_x = get_next_x(temp_x, search);
    tail_y = get_next_y(tail_y, search);
    search = get_board_at(state, tail_x, tail_y);
  }
  snek->head_x = tail_x;
  snek->head_y = tail_y;
  return;
}


/* Task 6.2 */
game_state_t* initialize_snakes(game_state_t* state) {
  char *row;
  const char *tails = "wasd";
  int i;
  for (i = 0; i < state->num_rows; i++) {
    row = state->board[i]; 
    while (*row) {
      if (strchr(tails, *row) != NULL) {
        state->num_snakes++;
      }
      row++;
    }
  }
  state->snakes = malloc(sizeof(snake_t) * state->num_snakes);

  int snum = 0;
  for (int i = 0; i < state->num_rows; i++) {
    snake_t *snek = &(state->snakes[snum]);
    char *row = state->board[i];
    for (int j = 0; j > -1; j+=1) {
      if (*row == '\n') break;
      if (strchr(tails, row) != NULL) {
        snek->tail_x = j;
        snek->tail_y = i;
        snum+=1;
        if (++snum == state->num_snakes) break;
        snek = &(state->snakes[snum]);
      }
      row++;
    }
  }

  /*
  bool check = false;
  for (i = 0; i < state->num_rows; i++) {
    int snum = 0;
    snake_t *snek = &(state->snakes[snum]);
    int k;
    for (k = 0; get_board_at(state, i, k) != '\n'; k++) {
      if (strchr(tails, get_board_at(state, i, k)) != NULL) {
        snek->tail_x = k;
        snek->tail_y = i;
        if (snum == state->num_snakes) {
          check = true;
          break;
        }
        snum++;
        snek = &(state->snakes[snum]);
      }
      
    }
    if (check) {
      break;
    }
  }*/

  for (i = 0; i < state->num_snakes; i++) {
    snake_t *temp = &(state->snakes[i]);
    temp->live = true;
    find_head(state, i);
  }

  return state;
}
