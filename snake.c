#include "snake.h"
#include "glyphs.c"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define WINDOW_X 500
#define WINDOW_Y 50

#define GRID_SIZE 14
#define TREE_SIZE (GRID_SIZE / 2)
#define V (TREE_SIZE * TREE_SIZE)
#define V_M (GRID_SIZE * GRID_SIZE)
#define MAX_SNAKE_SIZE (GRID_SIZE * GRID_SIZE)
#define GRID_DIM 800

#define DELAY 0

typedef enum {
  UP,    // 0
  DOWN,  // 1
  LEFT,  // 2
  RIGHT, // 3
} Dir;

typedef struct {
  int x, y;
  Dir d;
} Point;

typedef struct {
  Point p;
  int score, high_score;
} Fruit;

typedef struct {
  Point queue[V];
  int front;
  int rear;
} Frontier;

void enqueue(Frontier *f, Point p) {
  f->queue[f->rear] = p;
  f->rear = (f->rear + 1) % V;
}

Point dequeue(Frontier *f) {
  Point p = f->queue[f->front];
  f->front = (f->front + 1) % V;
  return p;
}

Fruit fruit;
Point snake[MAX_SNAKE_SIZE];
int snake_begin_index = 0;
// IMPORANT RESTRICTIONS FOR MAZE TRAVERSAL
int snake_size = 1;
Dir snake_dir = UP;

int mod(int a, int b) {
  int res = a % b;
  if (res < 0) {
    return res + b;
  }
  return res;
}

void init_snake(int size) {
  int x = 0;
  int y = 0;
  snake[snake_begin_index].x = x;
  snake[snake_begin_index].y = y;
  for (int i = 1; i < size; i++) {
    snake[snake_begin_index + i].x = snake[snake_begin_index + i - 1].x;
    snake[snake_begin_index + i].y =
        mod(snake[snake_begin_index + i - 1].y + 1, GRID_SIZE);
    snake_size++;
  }
}

void reset_game() {
  snake_begin_index = 0;
  snake_dir = UP;
  snake_size = 1;
  init_snake(5);
  if (fruit.score > fruit.high_score) {
    fruit.high_score = fruit.score;
  }
  fruit.score = 0;
}

void add_snake(int x, int y) {
  // add to the head
  snake_begin_index = mod(snake_begin_index - 1, MAX_SNAKE_SIZE);
  snake[snake_begin_index].x = x;
  snake[snake_begin_index].y = y;
  snake_size++;
}

void delete_snake() {
  // to maintain a sliding window containing points that belong
  // to the snake body
  snake_size--;
}

void render_grid(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0x28, 0x28, 0x28, 255);
  // set 1 for rendering grid
#if 1

  int cell_size = GRID_DIM / GRID_SIZE;

  SDL_Rect cell;
  cell.w = cell_size;
  cell.h = cell_size;

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      cell.x = x + i * cell_size;
      cell.y = y + j * cell_size;
      SDL_RenderDrawRect(renderer, &cell); // border only
    }
  }
#else

  SDL_Rect outline;
  outline.x = x;
  outline.y = y;
  outline.w = GRID_DIM;
  outline.h = GRID_DIM;

  SDL_RenderDrawRect(renderer, &outline);

#endif
}

void render_fruit(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0xfb, 0x49, 0x34, 255);

  int fruit_size = GRID_DIM / GRID_SIZE;
  SDL_Rect fruit_seg;
  fruit_seg.w = fruit_size;
  fruit_seg.h = fruit_size;
  fruit_seg.x = x + fruit.p.x * fruit_size;
  fruit_seg.y = y + fruit.p.y * fruit_size;

  SDL_RenderFillRect(renderer, &fruit_seg);
}

void gen_fruit() {
  if (snake_size == (GRID_SIZE * GRID_SIZE)) {
    printf("ERROR: No Fruit Can be generated");
    return;
  }
  int x, y;
  bool overlap;
  do {
    // reset state each iteration
    overlap = false;
    x = rand() % GRID_SIZE;
    y = rand() % GRID_SIZE;
    for (int i = 0; i < snake_size; i++) {
      int snake_index = mod(snake_begin_index + i, MAX_SNAKE_SIZE);
      if (x == snake[snake_index].x && y == snake[snake_index].y) {
        overlap = true;
        break;
      }
    }
  } while (overlap);
  fruit.p.x = x;
  fruit.p.y = y;
  fruit.score++;
}

bool check_collision_fruit() {
  int head_x = snake[snake_begin_index].x;
  int head_y = snake[snake_begin_index].y;
  if (fruit.p.x == head_x && fruit.p.y == head_y) {
    gen_fruit();
    return true;
  }
  return false;
}

bool check_collision_snake() {
  int head_x = snake[snake_begin_index].x;
  int head_y = snake[snake_begin_index].y;
  for (int i = 1; i < snake_size; i++) {
    int snake_index = mod(snake_begin_index + i, MAX_SNAKE_SIZE);
    if (head_x == snake[snake_index].x && head_y == snake[snake_index].y) {
      return true;
    }
  }
  return false;
}

void move_snake() {
  int head_x = snake[snake_begin_index].x;
  int head_y = snake[snake_begin_index].y;
  Dir head_dir = snake_dir;

  bool fruit_eaten = false;
  bool snake_col = false;

  switch (head_dir) {
  case UP:
    head_y = mod(head_y - 1, GRID_SIZE);
    break;
  case DOWN:
    head_y = mod(head_y + 1, GRID_SIZE);
    break;
  case LEFT:
    head_x = mod(head_x - 1, GRID_SIZE);
    break;
  case RIGHT:
    head_x = mod(head_x + 1, GRID_SIZE);
    break;
  }
  add_snake(head_x, head_y);
  fruit_eaten = check_collision_fruit();
  snake_col = check_collision_snake();
  if (!fruit_eaten) {
    delete_snake();
  }
  if (snake_col) {
    reset_game();
  }
}

void render_snake(SDL_Renderer *renderer, int x, int y) {

  int seg_size = GRID_DIM / GRID_SIZE;

  SDL_Rect seg;
  seg.w = seg_size - 2;
  seg.h = seg_size - 2;

  SDL_Rect seg_out;
  seg_out.w = seg_size;
  seg_out.h = seg_size;

  SDL_Rect dir_dot_1, dir_dot_2;
  dir_dot_1.w = seg_size / 4;
  dir_dot_1.h = seg_size / 4;
  dir_dot_2.w = seg_size / 4;
  dir_dot_2.h = seg_size / 4;

  int bright = 255;
  int b_dir = 0;

  for (int i = 0; i < snake_size; i++) {
    int snake_index = mod(snake_begin_index + i, MAX_SNAKE_SIZE);
    SDL_SetRenderDrawColor(renderer, 0x92, 0x83, bright, 255);
    seg_out.x = x + snake[snake_index].x * seg_size;
    seg_out.y = y + snake[snake_index].y * seg_size;
    SDL_RenderFillRect(renderer, &seg_out);

    SDL_SetRenderDrawColor(renderer, 0xb8, bright, 0x26, 255);
    seg.x = x + snake[snake_index].x * seg_size;
    seg.y = y + snake[snake_index].y * seg_size;
    SDL_RenderFillRect(renderer, &seg);
    // dot pointing in the direction of snake head
    SDL_SetRenderDrawColor(renderer, 0x00, 0x26, 0xab, 255);
    dir_dot_1.x = x + snake[snake_begin_index].x * seg_size;
    dir_dot_1.y = y + snake[snake_begin_index].y * seg_size;
    dir_dot_2 = dir_dot_1;
    switch (snake_dir) {
    case UP:
      dir_dot_1.x = dir_dot_1.x + seg.w / 5;
      dir_dot_2.x = dir_dot_1.x + 2 * seg.w / 5;
      break;
    case LEFT:
      dir_dot_1.y = dir_dot_1.y + seg.h / 5;
      dir_dot_2.y = dir_dot_1.y + 2 * seg.h / 5;
      break;
    case DOWN:
      dir_dot_1.x = dir_dot_1.x + seg.w / 5;
      dir_dot_2.x = dir_dot_1.x + 2 * seg.w / 5;
      dir_dot_1.y = dir_dot_1.y + 4 * seg.h / 5;
      dir_dot_2.y = dir_dot_1.y;
      break;
    case RIGHT:
      dir_dot_1.x = dir_dot_1.x + 4 * seg.w / 5;
      dir_dot_2.x = dir_dot_1.x;
      dir_dot_1.y = dir_dot_1.y + seg.h / 5;
      dir_dot_2.y = dir_dot_1.y + 2 * seg.h / 5;
      break;
    }
    SDL_RenderFillRect(renderer, &dir_dot_1);
    SDL_RenderFillRect(renderer, &dir_dot_2);

    if (b_dir == 0) {
      bright -= 5;
      if (bright < 150) {
        b_dir = 1;
      }
    }
    if (b_dir == 1) {
      bright += 5;
      if (bright > 250) {
        b_dir = 0;
      }
    }
  }
}

void render_score(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0x00, 0x9f, 0xf9, 255);

  int cell_size = 11;

  SDL_Rect cell;
  cell.w = cell_size;
  cell.h = cell_size;

  char buff[10];
  snprintf(buff, sizeof(buff), "%4d", fruit.score);

  for (int k = 0; k < 4; k++) {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        if (glyphs[buff[k]][j][i]) {
          cell.x = x + cell_size * i + (cell_size * 9 * k);
          cell.y = y + cell_size * j;
          SDL_RenderFillRect(renderer, &cell);
        }
      }
    }
  }

  SDL_RenderDrawRect(renderer, &cell);
}

void render_high_score(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0x00, 0xf9, 0x9f, 255);

  int cell_size = 11;

  SDL_Rect cell;
  cell.w = cell_size;
  cell.h = cell_size;

  char buff[10];
  snprintf(buff, sizeof(buff), "%4d", fruit.high_score);

  for (int k = 0; k < 4; k++) {
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        if (glyphs[buff[k]][j][i]) {
          cell.x = x + cell_size * i + (cell_size * 9 * k);
          cell.y = y + cell_size * j;
          SDL_RenderFillRect(renderer, &cell);
        }
      }
    }
  }

  SDL_RenderDrawRect(renderer, &cell);
}

// Spanning Tree adjacency matrix
bool spanning_tree[V][V] = {false};
// To represent the presence of walls between tiles
bool maze[V_M][V_M] = {false};

void get_adjacent_nodes(int x, int y, Point *adjacent_nodes,
                        int *no_of_adjacent_nodes, int x_lim, int y_lim) {
  int i = 0;
  if (y > 0) {
    // up neighbor
    adjacent_nodes[i].x = x;
    adjacent_nodes[i].y = y - 1;
    adjacent_nodes[i].d = UP;
    i++;
  }
  if (y < y_lim) {
    // down neighbor
    adjacent_nodes[i].x = x;
    adjacent_nodes[i].y = y + 1;
    adjacent_nodes[i].d = DOWN;
    i++;
  }
  if (x > 0) {
    // left neighbor
    adjacent_nodes[i].x = x - 1;
    adjacent_nodes[i].y = y;
    adjacent_nodes[i].d = LEFT;
    i++;
  }
  if (x < x_lim) {
    // right neighbor
    adjacent_nodes[i].x = x + 1;
    adjacent_nodes[i].y = y;
    adjacent_nodes[i].d = RIGHT;
    i++;
  }
  *no_of_adjacent_nodes = i;
}

void gen_tree() {

  Frontier frontier;
  frontier.front = 0;
  frontier.rear = 0;

  // To track if a node has been visited or not
  bool visited[V] = {false};
  // To track if a node is present in the frontier or not
  bool frontier_mask[V] = {false};
  int visited_size = 0;

  Point adjacent_nodes[4];
  int no_of_adjacent_nodes = 0;

  Point start = {rand() % TREE_SIZE, rand() % TREE_SIZE};

  // Add nodes adjacent to start to the frontier
  get_adjacent_nodes(start.x, start.y, adjacent_nodes, &no_of_adjacent_nodes,
                     TREE_SIZE - 1, TREE_SIZE - 1);
  for (int i = 0; i < no_of_adjacent_nodes; i++) {
    Point adj = adjacent_nodes[i];
    enqueue(&frontier, adj);
    frontier_mask[adj.x + TREE_SIZE * adj.y] = true;
  }

  // Mark start as visited
  visited[start.x + TREE_SIZE * start.y] = true;
  visited_size++;

  // Main loop for finding the Spanning Tree
  while (visited_size < (V)) {
    Point current = dequeue(&frontier);

    Point available[V];
    int available_size = 0;

    // Iterate through the nodes adjacent to current node
    no_of_adjacent_nodes = 0;
    get_adjacent_nodes(current.x, current.y, adjacent_nodes,
                       &no_of_adjacent_nodes, TREE_SIZE - 1, TREE_SIZE - 1);
    for (int i = 0; i < no_of_adjacent_nodes; i++) {
      int idx = adjacent_nodes[i].x + TREE_SIZE * adjacent_nodes[i].y;
      if (visited[idx]) {
        available[available_size++] = adjacent_nodes[i];
      } else if (!frontier_mask[idx]) {
        Point p = adjacent_nodes[i];
        enqueue(&frontier, p);
        frontier_mask[idx] = true;
      }
    }

    // Pick a random node from available nodes for connections
    Point next = available[rand() % available_size];
    int a = current.x + TREE_SIZE * current.y;
    int b = next.x + TREE_SIZE * next.y;
    spanning_tree[a][b] = true;
    spanning_tree[b][a] = true;
    // WALLS
    // these points are scaled and tranlated to map onto the grid
    // - 1 in for loops because we don't want to include bottom most or leftmost
    // tile in the connection
    int ax = current.x * 2;
    int ay = current.y * 2;
    int bx = next.x * 2;
    int by = next.y * 2;
    switch (next.d) {
    case UP:
      for (int i = ay - 1; i >= by; i--) {
        int x = ax + 1;
        int y = i + 1;
        // x, y and x-1, y have a wall between them
        int a = x + GRID_SIZE * y;
        int b = (x - 1) + GRID_SIZE * y;
        maze[a][b] = true;
        maze[b][a] = true;
      }
      break;
    case DOWN:
      for (int i = ay; i <= by - 1; i++) {
        int x = ax + 1;
        int y = i + 1;
        // x, y and x-1, y have a wall between them
        int a = x + GRID_SIZE * y;
        int b = (x - 1) + GRID_SIZE * y;
        maze[a][b] = true;
        maze[b][a] = true;
      }
      break;
    case LEFT:
      for (int i = ax - 1; i >= bx; i--) {
        int x = i + 1;
        int y = ay + 1;
        // x, y and x, y-1 have a wall between them
        int a = x + GRID_SIZE * y;
        int b = x + GRID_SIZE * (y - 1);
        maze[a][b] = true;
        maze[b][a] = true;
      }
      break;
    case RIGHT:
      for (int i = ax; i <= bx - 1; i++) {
        int x = i + 1;
        int y = ay + 1;
        // x, y and x, y-1 have a wall between them
        int a = x + GRID_SIZE * y;
        int b = x + GRID_SIZE * (y - 1);
        maze[a][b] = true;
        maze[b][a] = true;
      }
      break;
    }
    // WALLS

    // Mark current node as visited
    visited[a] = true;
    visited_size++;
    // Remove current node from the frontier
    frontier_mask[a] = false;
  }

  // complete the maze
  // border of the grid is also a part of the maze
  int x_last = GRID_SIZE - 1;
  int y_last = x_last * GRID_SIZE;
  // wall between topmost and bottommost tiles
  for (int i = 0; i <= x_last; i++) {
    maze[i][i + y_last] = true;
    maze[i + y_last][i] = true;
  }
  // wall between leftmost and rightmost tiles
  for (int i = 0; i <= y_last; i += GRID_SIZE) {
    maze[i][i + x_last] = true;
    maze[i + x_last][i] = true;
  }
}

void render_tree(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 255);

  int node_size = (GRID_DIM / GRID_SIZE);

  SDL_Rect node;
  node.w = node_size / 4;
  node.h = node_size / 4;

  // array of nodes to draw the graph
  SDL_Rect nodes[V];

  // calculate node positions
  int k = 0;
  for (int i = 0; i < TREE_SIZE; i++) {
    for (int j = 0; j < TREE_SIZE; j++, k++) {
      node.x = x + j * node_size * 2 + node_size;
      node.y = y + i * node_size * 2 + node_size;
      nodes[k] = node;
    }
  }

  // draw nodes
  for (int i = 0; i < V; i++) {
    SDL_Rect temp = nodes[i];
    temp.x = temp.x - node_size / 8;
    temp.y = temp.y - node_size / 8;
    SDL_RenderFillRect(renderer, &temp);
  }

  // draw lines
  for (int i = 0; i < V; i++) {
    for (int j = 0; j < V; j++) {
      if (spanning_tree[j][i] == true && spanning_tree[i][j] == true) {
        SDL_RenderDrawLine(renderer, nodes[i].x, nodes[i].y, nodes[j].x,
                           nodes[j].y);
      }
    }
  }
}

void turn_left() {
  switch (snake_dir) {
  case UP:
    snake_dir = LEFT;
    break;
  case DOWN:
    snake_dir = RIGHT;
    break;
  case LEFT:
    snake_dir = DOWN;
    break;
  case RIGHT:
    snake_dir = UP;
    break;
  }
}

void turn_right() {
  switch (snake_dir) {
  case UP:
    snake_dir = RIGHT;
    break;
  case DOWN:
    snake_dir = LEFT;
    break;
  case LEFT:
    snake_dir = UP;
    break;
  case RIGHT:
    snake_dir = DOWN;
    break;
  }
}

Point try_right(Point try, Dir d) {
  switch (d) {
  case UP:
    try.x = mod(try.x + 1, GRID_SIZE);
    break;
  case DOWN:
    try.x = mod(try.x - 1, GRID_SIZE);
    break;
  case LEFT:
    try.y = mod(try.y - 1, GRID_SIZE);
    break;
  case RIGHT:
    try.y = mod(try.y + 1, GRID_SIZE);
    break;
  }
  return try;
}

Point try_forward(Point try, Dir d) {
  switch (d) {
  case UP:
    try.y = mod(try.y - 1, GRID_SIZE);
    break;
  case DOWN:
    try.y = mod(try.y + 1, GRID_SIZE);
    break;
  case LEFT:
    try.x = mod(try.x - 1, GRID_SIZE);
    break;
  case RIGHT:
    try.x = mod(try.x + 1, GRID_SIZE);
    break;
  }
  return try;
}

Point try_left(Point try, Dir d) {
  switch (d) {
  case UP:
    try.x = mod(try.x - 1, GRID_SIZE);
    break;
  case DOWN:
    try.x = mod(try.x + 1, GRID_SIZE);
    break;
  case LEFT:
    try.y = mod(try.y + 1, GRID_SIZE);
    break;
  case RIGHT:
    try.y = mod(try.y - 1, GRID_SIZE);
    break;
  }
  return try;
}
// each int represents a point in the tile
int maze_path[V_M];
int maze_size = 0;

Dir get_left(Dir try_dir) {
  switch (try_dir) {
  case UP:
    return LEFT;
    break;
  case DOWN:
    return RIGHT;
    break;
  case LEFT:
    return DOWN;
    break;
  case RIGHT:
    return UP;
    break;
  }
}

Dir get_right(Dir try_dir) {
  switch (try_dir) {
  case UP:
    return RIGHT;
    break;
  case DOWN:
    return LEFT;
    break;
  case LEFT:
    return UP;
    break;
  case RIGHT:
    return DOWN;
    break;
  }
}

void gen_maze_path() {
  // the hamiltonian cycle traced by the snake as it travrses the maze
  Point head = snake[snake_begin_index];
  Point try_head = head;
  Dir try_dir = snake_dir;
  while (maze_size < V_M) {
    Point try_r = try_right(try_head, try_dir);
    Point try_f = try_forward(try_head, try_dir);
    Point try_l = try_left(try_head, try_dir);
    int a = try_head.x + GRID_SIZE * try_head.y;
    maze_path[maze_size++] = a;
    int b_r = try_r.x + GRID_SIZE * try_r.y;
    int b_f = try_f.x + GRID_SIZE * try_f.y;
    int b_l = try_l.x + GRID_SIZE * try_l.y;
    // the right hand maze traversing algorithm
    if (maze[a][b_r] == false) {
      // no wall between head and right tile
      // turn right skip the rest
      try_head = try_r;
      try_dir = get_right(try_dir);
    } else if (maze[a][b_f] == false) {
      // no wall between head and forward tile
      // move forward skip the rest
      try_head = try_f;
    } else if (maze[a][b_l] == false) {
      // no wall between head and forward tile
      // turn left skip the rest
      try_head = try_l;
      // also change try_dir
      try_dir = get_left(try_dir);
    }
  }
}

int distance_to_fruit(int try_head) {
  int f = fruit.p.x + GRID_SIZE * fruit.p.y;
  int try_head_pos_in_maze, fruit_pos_in_maze;

  for (int i = 0; i < V_M; i++) {
    if (try_head == maze_path[i]) {
      try_head_pos_in_maze = i;
    }
    if (f == maze_path[i]) {
      fruit_pos_in_maze = i;
    }
  }

  int distance = 0;
  for (int i = try_head_pos_in_maze; i != fruit_pos_in_maze;
       i = (i + 1) % V_M) {
    distance++;
  }
  return abs(distance);
}

bool is_ordered(int head, int try_head, int tail) {
  int try_head_pos_in_maze;
  int tail_pos_in_maze;
  int head_pos_in_maze;
  for (int i = 0; i < V_M; i++) {
    if (try_head == maze_path[i]) {
      try_head_pos_in_maze = i;
    }
    if (tail == maze_path[i]) {
      tail_pos_in_maze = i;
    }
    if (head == maze_path[i]) {
      head_pos_in_maze = i;
    }
  }
  // if try head comes between current tail and head return false
  // else return true;
  for (int i = tail_pos_in_maze; i != head_pos_in_maze;
      i = (i + 1) % V_M) {
    if (try_head_pos_in_maze == i) {
      return false;
    }
  }
  return true;
}

void traverse_maze() {
  Point head = snake[snake_begin_index];
  Point try_r = try_right(head, snake_dir);
  Point try_f = try_forward(head, snake_dir);
  Point try_l = try_left(head, snake_dir);
  int a = head.x + GRID_SIZE * head.y;
  int snake_tail_index = mod(snake_begin_index + snake_size - 1, MAX_SNAKE_SIZE);
  int tail = snake[snake_tail_index].x + GRID_SIZE * snake[snake_tail_index].y;
  int b_r = try_r.x + GRID_SIZE * try_r.y;
  int b_f = try_f.x + GRID_SIZE * try_f.y;
  int b_l = try_l.x + GRID_SIZE * try_l.y;

  if (snake_size <= V_M / 2) {

  // turn in the distance that minimizes path cost to fruit
  // while making sure order of the snake in the maze path is
  // not broken and it doesnot hit a wall

  int distance_to_fruit_from_r = distance_to_fruit(b_r);
  int distance_to_fruit_from_f = distance_to_fruit(b_f);
  int distance_to_fruit_from_l = distance_to_fruit(b_l);
  bool is_ordered_at_r = is_ordered(a, b_r, tail);
  bool is_ordered_at_f = is_ordered(a, b_f, tail);
  bool is_ordered_at_l = is_ordered(a, b_l, tail);

  // there's probably a better way of doing this
  // inflate the distances of those direction where there is a wall
  // or moving to which will break the snake order
  if (!is_ordered_at_r) {
    distance_to_fruit_from_r += V_M;
  }
  if (!is_ordered_at_f) {
    distance_to_fruit_from_f += V_M;
  }
  if (!is_ordered_at_l) {
    distance_to_fruit_from_l += V_M;
  }
  if (maze[a][b_r]) {
    distance_to_fruit_from_r += V_M;
  }
  if (maze[a][b_f]) {
    distance_to_fruit_from_f += V_M;
  }
  if (maze[a][b_l]) {
    distance_to_fruit_from_l += V_M;
  }


  if (distance_to_fruit_from_r <= distance_to_fruit_from_f &&
      distance_to_fruit_from_r <= distance_to_fruit_from_l) {
    turn_right();
  } else {
    if (distance_to_fruit_from_f < distance_to_fruit_from_l) {
      // continue forwared
    } else {
      turn_left();
    }
  }
  } else {

  // traverse maze without taking shortcuts

  if (maze[a][b_r] == false) {
    // no wall between head and right tile
    // turn right skip the rest
    turn_right();
  } else if (maze[a][b_f] == false) {
    // no wall between head and forward tile
    // move forward skip the rest
  } else if (maze[a][b_l] == false) {
    // no wall between head and forward tile
    // turn left skip the rest
    turn_left();
  }
  }
}

int main() {
  srand(time(NULL));

  init_snake(5);
  gen_fruit();
  gen_tree();
  gen_maze_path();

  fruit.score = 0;
  fruit.high_score = 0;

  SDL_Window *window;
  SDL_Renderer *renderer;

  if (SDL_INIT_VIDEO < 0) {
    fprintf(stderr, "ERROR: SDL_INIT_VIDEO");
  }

  window = SDL_CreateWindow("Snake", WINDOW_X, WINDOW_Y, WINDOW_WIDTH,
                            WINDOW_HEIGHT, SDL_WINDOW_BORDERLESS);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  int grid_x = (WINDOW_WIDTH / 2) - (GRID_DIM / 2);
  int grid_y = (WINDOW_HEIGHT / 2) - (GRID_DIM / 2);

  bool quit = false;
  bool pause = true;
  bool ai = false;
  bool show_tree = true;
  SDL_Event event;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
        quit = true;
        break;
      case SDL_KEYUP:
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
        case SDLK_q:
          quit = true;
          break;
        case SDLK_p:
          pause = !pause;
          break;
        case SDLK_i:
          ai = !ai;
          break;
        case SDLK_m:
          show_tree = !show_tree;
          break;
        case SDLK_w:
          if (snake_dir != DOWN)
            snake_dir = UP;
          break;
        case SDLK_a:
          if (snake_dir != RIGHT)
            snake_dir = LEFT;
          break;
        case SDLK_s:
          if (snake_dir != UP)
            snake_dir = DOWN;
          break;
        case SDLK_d:
          if (snake_dir != LEFT)
            snake_dir = RIGHT;
          break;
        }
        break;
      }
    }
    SDL_RenderClear(renderer);
    // Render Loop Start
    if (!pause) {
      if (!ai) {
        traverse_maze();
      }
      move_snake();
    }
    render_grid(renderer, grid_x, grid_y);
    render_snake(renderer, grid_x, grid_y);
    render_fruit(renderer, grid_x, grid_y);
    if (!show_tree) {
      render_tree(renderer, grid_x, grid_y);
    }
    render_score(renderer, -25, grid_y + GRID_DIM);
    render_high_score(renderer, GRID_DIM - 3 * grid_x + 25, grid_y + GRID_DIM);

    if (snake_size >= (GRID_SIZE * GRID_SIZE) - 1) {
      pause = true;
    }
    SDL_Delay(DELAY);
    // Render Loop End
    SDL_SetRenderDrawColor(renderer, 0x11, 0x11, 0x11, 255);
    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
