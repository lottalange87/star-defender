# Star Defender

A fast-paced terminal-based space shooter written in pure C. No external dependencies—just standard library and ANSI escape codes.

## Features

- 🚀 Real-time smooth gameplay (~30 FPS)
- 👾 3 enemy types with different behaviors:
  - **Normal** (red V) - Standard speed
  - **Fast** (magenta v) - Quick zigzag movement
  - **Tank** (yellow W) - Takes 3 hits, wider hitbox
- 💥 Particle explosion effects
- 🔫 Weapon upgrades (every 500 points):
  - Level 1: Single shot
  - Level 2: Dual shot
  - Level 3: Triple shot
- 🌊 Progressive difficulty (waves get harder)
- 🎮 Keyboard controls:
  - **WASD** or **Arrow keys** - Move ship
  - **SPACE** - Shoot
  - **Q** - Quit
  - **R** - Restart (after game over)

## Build & Run

```bash
# Compile
make

# Or manually:
gcc -Wall -Wextra -O2 -o star_defender main.c

# Run
./star_defender
```

## How to Play

1. Your ship starts at the bottom
2. Enemies spawn from the top and move down
3. Shoot them before they reach the bottom
4. Avoid collisions—3 lives only!
5. Survive as long as you can!

## Score System

| Enemy Type | Points |
|------------|--------|
| Normal     | 10     |
| Fast       | 20     |
| Tank       | 50     |

## Technical Details

- **Rendering:** ANSI escape codes (no ncurses needed)
- **Input:** Raw terminal mode with non-blocking reads
- **Target FPS:** ~30 frames per second
- **Terminal Size:** Optimized for 60×24 characters

## Requirements

- Unix-like system (Linux/macOS)
- Terminal with ANSI color support
- C compiler (gcc/clang)

---

*Written from scratch in ~450 lines of C*