//
// DOSDoom Column/Span Drawing for 16-bit Colour Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// Note: The best place for optimisation!
//
// -ACB- 1998/09/10 Cleaned up.
//

#ifndef __R_DRAW2__
#define __R_DRAW2__

#ifdef __GNUG__
#pragma interface
#endif

void resinit_r_draw_c16(void);

void R_DrawColumn16_KM(void);          // Smooth version
void R_DrawColumn16_CVersion (void);   // C Version
void R_DrawColumn16_Rasem(void);       // Rasem's
void R_DrawColumn16_Old(void);         // Chi's Original

void R_DrawFuzzColumn16 (void);

void R_DrawTranslucentColumn16 (void);

void R_DrawTranslatedColumn16 (void);
void R_DrawTranslucentTranslatedColumn16 (void);

void R_DrawSpan16_KM (void);          // Smooth version
void R_DrawSpan16_CVersion (void);    // C Version
void R_DrawSpan16_Rasem (void);       // Rasem's

void R_VideoErase16 (unsigned ofs, int count);
void R_InitBuffer16 (int width, int height);
void R_InitTranslationTables16 (void);
void R_FillBackScreen16 (void);
void R_DrawViewBorder16 (void);

#endif

