# Snake Bot
A complete but inefficient bot for playing Snake.

## Method
It works by generating a Spanning Tree half the dimensions of the grid. By scaling the spanning tree and translating it onto the grid, a Maze is formed. The bot follows this Maze using the right hand rule and the path forms a Hamiltonian Cycle.

## Usage
You'll need the SDL2 library to run this code.
```shell
$ make && ./snake
```
## Controls
| Key | Function    |
|-----|-------------|
| `w` | Move Up     |
| `a` | Move Left   |
| `s` | Move Down   |
| `d` | Move Right  |
| `i` | Toggle AI   |
| `p` | Toggle Pause|
| `m` | Toggle Maze |
| `q` | Quit Game   |

## References
https://johnflux.com/2015/05/02/nokia-6110-part-3-algorithms/
