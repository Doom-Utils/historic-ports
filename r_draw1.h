//
// DOSDoom Column/Span Drawing for 8-bit Colour Code
//
// Based on the Doom Source Code,
//
// Released by id software, (c) 1993-1996 (see DOOMLIC.TXT)
//
// -ACB- 1998/09/10 Cleaned up.
//
#ifndef __R_DRAW1__
#define __R_DRAW1__

#ifdef __GNUG__
#pragma interface
#endif

void resinit_r_draw_c8(void);

void R_DrawColumn8_CVersion(void);// C Version
void R_DrawColumn8_KM (void);      // C Version, optimised by KM for GCC
void R_DrawColumn8_id(void);      // id's original
void R_DrawColumn8_id_Erik(void); // improved id's original
void R_DrawColumn8_Pentium(void); // Optimised for Pentium
void R_DrawColumn8_NOMMX(void);   // DOSDoom original
void R_DrawColumn8_K6_MMX(void);  // MMX asm version, optimised for K6
void R_DrawColumn8_Rasem(void);   // Rasem's

void R_DrawFuzzColumn8 (void);

void R_DrawTranslucentColumn8 (void);

void R_DrawTranslatedColumn8 (void);
void R_DrawTranslucentTranslatedColumn8 (void);

void R_DrawSpan8_CVersion (void); // C Version
void R_DrawSpan8_MMX (void);      // MMX asm version
void R_DrawSpan8_KM (void);       // id's original
void R_DrawSpan8_id (void);       // id's original
void R_DrawSpan8_id_Erik (void);  // improved id's original
void R_DrawSpan8_Rasem (void);    // Rasem's

void R_VideoErase8 (unsigned ofs, int count);
void R_InitBuffer8 (int width, int height);
void R_InitTranslationTables8 (void);
void R_FillBackScreen8 (void);
void R_DrawViewBorder8 (void);

#endif

