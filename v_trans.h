
//
// This structure holds the translation information for DrawPatchTrans,
// and DrawPatchInDirectTrans. Probably a good idea to reserve the first
// 32 for the player backdrops and after that do whatever you want.
//

struct {
          int ploc;  // Palette offset
          int numc;  // Number of colors used. Valid no's 1/3/7/15.
       }  pdecode[MAXTRANSLATIONS] = {
                  {112,15}, // Player Backdrops (Green)	- 0
                  {109,3 }, // Indigo			- 1
                  {76 ,3 }, // Brown			- 2
                  {36 ,7 }, // Red			- 3
                  {162,3 }, // Gold			- 4
                  {202,3 }, // Blue			- 5
                  {240,3 }, // Dark Blue		- 6
                  {16 ,15}, // Pink			- 7
                  { 0 ,0 }, // -- Begin Backdrop reserve
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 },
                  { 0 ,0 }, // -- End of reserve
		  {176,15}, // Bright Red		- 32
		  {112,15}, // Green			- 33
		  {192,15}, // Blue			- 34
		  {208,15}, // Orange			- 35
		  {64 ,15}, // Brown			- 36
		  {80 ,15}, // Light Grey		- 37
		  {96 ,15}, // Indigo			- 38
		  {48 ,15}, // Dull Orange		- 39
                  {16 ,15}, // Pink			- 40
                  {32 ,15}, // Dark Red			- 41
		  {128,15}  // Washed out brown		- 42

};
