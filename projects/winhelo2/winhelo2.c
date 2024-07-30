#include "apilib.h"

void HariMain(void)
{
	int win;
	char buf[170 * 70];
	
	win = api_openwin(buf, 170, 70, -1, "help");
	/* api_putstrwin(win, 32, 32, 0 /*黑色*//* , 138, "a bball deepdown calc chklang color gview hello invader iroha lines mmlplay noodle notrec picdata sosu star1 stars tview type walk winhelo"); */
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break; /*按下回车键则break; */
		}
	}
	api_end();
}
