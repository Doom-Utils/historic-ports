//
// DOSDoom Text Menu (Header)
//
// By The DOSDoom Team
//

#ifndef _optmenu_h_
#define _optmenu_h_

extern int optionsmenuon;

void M_InitOptmenu(void);
void M_OptDrawer(void);
void M_OptTicker(void);
boolean M_OptResponder (event_t *ev, int ch);


#endif
