# Star Defender - Graphics Edition

Ein grafischer Space-Shooter in C mit SDL2. Keine Engine – alles selbst gebaut.

## Features

- 🚀 **Eigene Sprite-Engine** - Texture loading, Animation, Rotation
- 💥 **Partikel-System** - Explosionen, Thruster-Effekte
- 🎮 **3 Gegnertypen** - Normal, Fast (Zickzack), Tank (3 Treffer)
- 🔫 **Waffen-Upgrades** - Single → Dual → Triple Shot (alle 500 Punkte)
- 🌌 **Parallax-Sterne** - Tiefe durch verschiedene Geschwindigkeiten
- ⚡ **Fixed Timestep** - Konstante 60 FPS Game Logic

## Steuerung

| Taste | Aktion |
|-------|--------|
| WASD / Pfeiltasten | Bewegen |
| Leertaste | Schießen |
| Enter | Start / Neustart |
| ESC | Pause |
| Q | Beenden |

## Bauen

```bash
make          # Release build
make debug    # Debug build
make clean    # Aufräumen
make run      # Bauen & starten
```

## Projektstruktur

```
src/
├── math.h/c       - Vektor-Mathematik
├── sprite.h/c     - Sprite loading & rendering
├── particle.h/c   - Partikel-System
├── entity.h/c     - Spieler, Gegner, Projektile
├── game.h/c       - Game State, Loop, Kollisionen
└── main.c         - Entry point
```

## Technische Details

- **Rendering:** SDL2 Hardware-Accelerated
- **Sprites:** SDL_image für PNG/JPG
- **Partikel:** 1000 Partikel max, Alpha-Blending
- **Kollision:** AABB (Axis-Aligned Bounding Box)
- **Game Loop:** Fixed timestep mit Accumulator

## Abhängigkeiten

```bash
brew install sdl2 sdl2_image
```

---

*Ursprünglich ein Terminal-Spiel, komplett neu als Grafik-Version gebaut.*
