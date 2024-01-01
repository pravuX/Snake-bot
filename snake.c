#include "snake.h"
#include "glyphs.c"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000
#define WINDOW_X 500
#define WINDOW_Y 50

#define GRID_SIZE 20
#define TREE_DIM (GRID_SIZE / 2) // 4
#define V (TREE_DIM * TREE_DIM)
#define MAX_SNAKE_SIZE (GRID_SIZE * GRID_SIZE)
#define GRID_DIM 800

#define DELAY 100

typedef enum {
  SNAKE_UP,
  SNAKE_DOWN,
  SNAKE_LEFT,
  SNAKE_RIGHT,
} Dir;

typedef struct {
  int x, y;
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
int snake_size = 1;
Dir snake_dir = SNAKE_UP;

int mod(int a, int b) {
  int res = a % b;
  if (res < 0) {
    return res + b;
  }
  return res;
}

void init_snake(int size) {
  int x = rand() % GRID_SIZE;
  int y = rand() % GRID_SIZE;
  snake[snake_begin_index].x = x;
  snake[snake_begin_index].y = y;
  for (int i = 1; i < size; i++) {
    snake[snake_begin_index + i].x = snake[snake_begin_index + i - 1].x;
    snake[snake_begin_index + i].y =
        mod(snake[snake_begin_index + i - 1].y + 1, GRID_SIZE);
    snake_size++;
  }
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

void gen_fruit() {
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

void render_grid(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0x28, 0x28, 0x28, 255);
#if 1

  int cell_size = GRID_DIM / GRID_SIZE;

  SDL_Rect cell;
  cell.w = cell_size;
  cell.h = cell_size;

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      cell.x = x + i * cell_size;
      cell.y = y + j * cell_size;
      // SDL_RenderFillRect(renderer, &cell); // no fill
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

void reset_game() {
  snake_begin_index = 0;
  snake_dir = SNAKE_UP;
  snake_size = 1;
  init_snake(5);
  if (fruit.score > fruit.high_score) {
    fruit.high_score = fruit.score;
  }
  fruit.score = 0;
}

void move_snake() {
  int head_x = snake[snake_begin_index].x;
  int head_y = snake[snake_begin_index].y;
  Dir head_dir = snake_dir;

  bool fruit_eaten = false;
  bool snake_col = false;

  switch (head_dir) {
  case SNAKE_UP:
    head_y = mod(head_y - 1, GRID_SIZE);
    break;
  case SNAKE_LEFT:
    head_x = mod(head_x - 1, GRID_SIZE);
    break;
  case SNAKE_DOWN:
    head_y = mod(head_y + 1, GRID_SIZE);
    break;
  case SNAKE_RIGHT:
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

void flash_snake(SDL_Renderer *renderer, int x, int y) {

  int seg_size = GRID_DIM / GRID_SIZE;

  SDL_Rect seg;
  seg.w = seg_size - 2;
  seg.h = seg_size - 2;

  SDL_Rect seg_out;
  seg_out.w = seg_size;
  seg_out.h = seg_size;

  int bright = 255;
  int b_dir = 0;

  int r = rand() % 255;
  int g = rand() % 255;
  int b = rand() % 255;

  for (int i = 0; i < snake_size; i++) {
    int snake_index = mod(snake_begin_index + i, MAX_SNAKE_SIZE);

    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, bright, 255);
    seg_out.x = x + snake[snake_index].x * seg_size;
    seg_out.y = y + snake[snake_index].y * seg_size;
    SDL_RenderFillRect(renderer, &seg_out);

    SDL_SetRenderDrawColor(renderer, r, g, b, 255);
    seg.x = x + snake[snake_index].x * seg_size + 1;
    seg.y = y + snake[snake_index].y * seg_size + 1;

    SDL_RenderFillRect(renderer, &seg);

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
bool spanning_tree[V][V];

void get_adjacent_nodes(int x, int y, Point *adjacent_nodes,
                        int *no_of_adjacent_nodes) {
  int x_lim = TREE_DIM - 1;
  int y_lim = TREE_DIM - 1;
  int i = 0;
  if (y > 0) {
    // up neighbor
    adjacent_nodes[i].x = x;
    adjacent_nodes[i].y = y - 1;
    i++;
  }
  if (x > 0) {
    // left neighbor
    adjacent_nodes[i].x = x - 1;
    adjacent_nodes[i].y = y;
    i++;
  }
  if (y < y_lim) {
    // down neighbor
    adjacent_nodes[i].x = x;
    adjacent_nodes[i].y = y + 1;
    i++;
  }
  if (x < x_lim) {
    // right neighbor
    adjacent_nodes[i].x = x + 1;
    adjacent_nodes[i].y = y;
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

  // Spanning Tree adjacency matrix
  for (int i = 0; i < V; i++) {
    for (int j = 0; j < V; j++) {
      spanning_tree[j][i] = false;
    }
  }

  Point adjacent_nodes[4];
  int no_of_adjacent_nodes = 0;

  Point start;
  /* start.x = 2;
  start.y = 0; */
  start.x = rand() % TREE_DIM;
  start.y = rand() % TREE_DIM;

  // Add nodes adjacent to start to the frontier
  get_adjacent_nodes(start.x, start.y, adjacent_nodes, &no_of_adjacent_nodes);
  for (int i = 0; i < no_of_adjacent_nodes; i++) {
    Point adj;
    adj.x = adjacent_nodes[i].x;
    adj.y = adjacent_nodes[i].y;
    enqueue(&frontier, adj);
    frontier_mask[adj.x + TREE_DIM * adj.y] = true;
  }

  // Mark start as visited
  visited[start.x + TREE_DIM * start.y] = true;
  visited_size++;
  // print_adjacent_nodes(start.x, start.y, adjacent_nodes,
  // &no_of_adjacent_nodes);

  // Main loop for finding the Spanning Tree
  while (visited_size < (V)) {
    Point current = dequeue(&frontier);

    Point available[V];
    int available_size = 0;

    // Iterate through the nodes adjacent to current node
    no_of_adjacent_nodes = 0;
    get_adjacent_nodes(current.x, current.y, adjacent_nodes,
                       &no_of_adjacent_nodes);
    for (int i = 0; i < no_of_adjacent_nodes; i++) {
      int idx = adjacent_nodes[i].x + TREE_DIM * adjacent_nodes[i].y;
      if (visited[idx]) {
        available[available_size].x = adjacent_nodes[i].x;
        available[available_size].y = adjacent_nodes[i].y;
        available_size++;
      } else if (!frontier_mask[idx]) {
        Point p;
        p.x = adjacent_nodes[i].x;
        p.y = adjacent_nodes[i].y;
        enqueue(&frontier, p);
        frontier_mask[idx] = true;
      }
    }

    // Pick a random node from available nodes for connections
    Point next = available[rand() % available_size];
    int a = current.x + TREE_DIM * current.y;
    int b = next.x + TREE_DIM * next.y;
    spanning_tree[a][b] = true;
    spanning_tree[b][a] = true;
    // Mark current node as visited
    visited[a] = true;
    visited_size++;
    // Remove current node from the frontier
    frontier_mask[a] = false;
  }
}

void render_tree(SDL_Renderer *renderer, int x, int y) {
  SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x28, 255);

  int node_size = (GRID_DIM / GRID_SIZE);

  SDL_Rect node;
  node.w = node_size / 4;
  node.h = node_size / 4;

  bool one_d_tree[V * V] = {false};
  for (int i = 0; i < V; i++) {
    for (int j = 0; j < V; j++) {
      one_d_tree[j + V * i] = spanning_tree[i][j];
    }
  }

  // array of nodes to draw the graph
  SDL_Rect nodes[V];

  // calculate node positions
  int k = 0;
  for (int i = 0; i < TREE_DIM; i++) {
    for (int j = 0; j < TREE_DIM; j++, k++) {
      node.x = x + i * node_size * 2 + 3 * node_size / 4;
      node.y = y + j * node_size * 2 + 3 * node_size / 4;
      nodes[k] = node;
    }
  }

  // draw nodes
  /* for (int i = 0; i < V; i++) {
    SDL_Rect temp = nodes[i];
    temp.x = temp.x + node_size/8;
    temp.y = temp.y + node_size/8;
    SDL_RenderFillRect(renderer, &temp); // no fill
  } */

  // draw lines
  for (int i = 0; i < V; i++) {
    for (int j = 0; j < V; j++) {
      if (spanning_tree[j][i] == true && spanning_tree[i][j] == true) {
        SDL_RenderDrawLine(renderer, nodes[i].x + node_size / 4, nodes[i].y + node_size / 4,
                           nodes[j].x + node_size / 4, nodes[j].y + node_size / 4);
      }
    }
  }
}

int main() {
  srand(time(NULL));

  init_snake(5);
  gen_fruit();
  gen_tree();

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
  bool pause = false;
  SDL_Event event;

  int flash_dur = 0;

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
        case SDLK_w:
          if (snake_dir != SNAKE_DOWN)
            snake_dir = SNAKE_UP;
          break;
        case SDLK_a:
          if (snake_dir != SNAKE_RIGHT)
            snake_dir = SNAKE_LEFT;
          break;
        case SDLK_s:
          if (snake_dir != SNAKE_UP)
            snake_dir = SNAKE_DOWN;
          break;
        case SDLK_d:
          if (snake_dir != SNAKE_LEFT)
            snake_dir = SNAKE_RIGHT;
          break;
        }
        break;
      }
    }
    SDL_RenderClear(renderer);
    // Render Loop Start
    if (!pause) {
      move_snake();
    }
    render_grid(renderer, grid_x, grid_y);
    render_tree(renderer, grid_x, grid_y);
    render_snake(renderer, grid_x, grid_y);
    render_fruit(renderer, grid_x, grid_y);

    if (fruit.score % 10 == 0 && fruit.score != 0) {
      flash_dur = 10;
    }

    if (flash_dur > 0) {
      flash_snake(renderer, grid_x, grid_y);
      flash_dur--;
    }

    render_score(renderer, -25, grid_y + GRID_DIM);
    render_high_score(renderer, GRID_DIM - 3 * grid_x + 25, grid_y + GRID_DIM);

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
