#outtahere

its gonna work on macos only just for now but im gonna make it comppatible with windows and linux too when i have the time

## Tiled collectibles

Add a boolean custom property named `collectible` to a tile in your TSX tileset.
Tiles with `collectible = true` will not collide and will disappear when the player touches them.

## Doors / next level

Add a boolean custom property named `door` to a tile in your TSX tileset.
Tiles with `door = true` will not collide.
When the player overlaps the door tile and presses `E`, the game loads the next map in `src/game.c`.

## Dynamic spawn point

For each level, add an object layer named `spawn` in Tiled and place a point object where the player should start.
The game will use that point as the spawn position for that level.
If you use a rectangle object instead, it will still work as a fallback.
The `spawn` layer is ignored for collision, so it acts only as a marker.
If no spawn layer exists, it falls back to the default spawn.
