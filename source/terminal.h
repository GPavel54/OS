#ifndef TERMINAL_H
#define TERMINAL_H

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

enum colors {
	cl_black,
	cl_red,
	cl_green,
	cl_brown,
	cl_blue,
	cl_magenta,
	cl_cyan,
	cl_light_blue,
	cl_default
};

void mt_clrscr ();								// производит очистку и перемещение курсора в левый верхний угол экрана;
int mt_gotoXY (int x, int y);					// перемещает курсор в указанную позицию. Первый параметр номер строки, второй - номер столбца;
int mt_getscreensize (int *rows, int *cols);	// определяет размер экрана терминала (количество строк и столбцов);
int mt_setfgcolor (enum colors color);			// устанавливает цвет последующих выводимых символов. В качестве параметра передаѐтся константа из созданного Вами перечислимого типа colors , описывающего цвета терминала;
int mt_setbgcolor (enum colors color);			// устанавливает цвет фона последующих выводимых символов. В качестве параметра передаѐтся константа из созданного Вами перечислимого типа colors , описывающего цвета терминала.

#endif
