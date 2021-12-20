
//{{BLOCK(desert)

//======================================================================
//
//	desert, 256x256@4, 
//	+ palette 16 entries, not compressed
//	+ 1025 tiles not compressed
//	+ regular map (flat), fake compressed, 32x32 
//	Total size: 32 + 32800 + 2052 = 34884
//
//	Time-stamp: 2020-06-03, 04:41:41
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.6
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_DESERT_H
#define GRIT_DESERT_H

#define desertTilesLen 32800
extern const unsigned short desertTiles[16400];

#define desertMapLen 2052
extern const unsigned short desertMap[1026];

#define desertPalLen 32
extern const unsigned short desertPal[16];

#endif // GRIT_DESERT_H

//}}BLOCK(desert)
