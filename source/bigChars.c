#include "bigChars.h"

int bc_printA(char *str)
{
	printf("\E(0%s\E(B", str);
	return 0;
}

int bc_box(int x1, int y1, int x2, int y2)
{
	int tmp;
	int maxx, maxy;
	int i;
	
	if (x1 > x2)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y1 > y2)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	mt_getscreensize(&maxy, &maxx);
	if ((x1 < 0) || (y1 < 0) || (x2 > maxx) || (y2 > maxy) || (x2 - x1 < 2) || (y2 - y1 < 2))
		return -1;
	mt_gotoXY(x1, y1);
	bc_printA(BOXCHAR_TL);
	for (i = x1 + 1; i < x2; i++)
		bc_printA(BOXCHAR_HOR);
	bc_printA(BOXCHAR_TR);
	for (i = y1 +1; i < y2; i++)
	{
		mt_gotoXY(x1, i);
		bc_printA(BOXCHAR_VERT);
		mt_gotoXY(x2, i);
		bc_printA(BOXCHAR_VERT);
	}
	mt_gotoXY(x1, y2);
	bc_printA(BOXCHAR_BL);
	for (i = x1 + 1; i < x2; i++)
		bc_printA(BOXCHAR_HOR);
	bc_printA(BOXCHAR_BR);
	return 0;
}

int bc_printbigchar(int *big, int x, int y, enum colors fg, enum colors bg)
{
	int maxx, maxy;
	int pos, bit;
	int i, j;
	char row[9];
	
	mt_getscreensize(&maxy, &maxx);
	if ((x < 0) || (y < 0) || (x + 8 > maxx) || (y + 8 > maxy))
		return -1;
	row[8] = '\0';
	mt_setfgcolor(fg);
	mt_setbgcolor(bg);
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			pos = i >> 2;
			bit = (big[pos] >> ((i % 4) * 8 + j)) & 1;
			if (bit == 0)
				row[j] = ' ';
			else
				row[j] = BOXCHAR_REC;
		}
		mt_gotoXY(x, y + i);
		bc_printA(row);
	}
	mt_setfgcolor(cl_default);
	mt_setbgcolor(cl_default);
	return 0;
}

int bc_printcourse(int *big, int pos, enum colors fg, enum colors bg)
{
  int o[9][12] = {{ 5,  4,  5,  7,  4,  5,  4,  6,  7,  5,  7,  6},
		  {14,  4, 14,  7, 13,  5, 13,  6, 16,  5, 16,  6},
		  {23,  4, 23,  7, 22,  5, 22,  6, 25,  5, 25,  6},
		  { 5, 13,  5, 16,  4, 14,  4, 15,  7, 14,  7, 15},
		  {14, 13, 14, 16, 13, 14, 13, 15, 16, 14, 16, 15},
		  {23, 13, 23, 16, 22, 14, 22, 15, 25, 14, 25, 15},
		  { 5, 22,  5, 25,  4, 23,  4, 24,  7, 23,  7, 24},
		  {14, 22, 14, 25, 13, 23, 13, 24, 16, 23, 16, 24},
		  {23, 22, 23, 25, 22, 23, 22, 24, 25, 23, 25, 24}};
    mt_gotoXY(o[pos][0], o[pos][1]);
    mt_setfgcolor(fg);
    char row[3] = {BOXCHAR_REC, BOXCHAR_REC, '\0'};
    char row2[2] = {BOXCHAR_REC, '\0'};
    bc_printA(row);
    mt_setfgcolor(cl_default);
    mt_gotoXY(o[pos][2], o[pos][3]);
    mt_setfgcolor(fg);
    bc_printA(row);
    mt_setfgcolor(cl_default);
    mt_gotoXY(o[pos][4], o[pos][5]);
    mt_setfgcolor(fg);
    bc_printA(row2);
    mt_setfgcolor(cl_default);
    mt_gotoXY(o[pos][6], o[pos][7]);
    mt_setfgcolor(fg);
    bc_printA(row2);
    mt_setfgcolor(cl_default);
    mt_gotoXY(o[pos][8], o[pos][9]);
    mt_setfgcolor(fg);
    bc_printA(row2);
    mt_setfgcolor(cl_default);
    mt_gotoXY(o[pos][10], o[pos][11]);
    mt_setfgcolor(fg);
    bc_printA(row2);
    mt_setfgcolor(cl_default);
    return 0;
}

int bc_setbigcharpos(int *big, int x, int y, int value)
{
	int pos;

	if ((x < 0) || (y < 0)  || (x > 7) || (y > 7) || (value < 0) || (value > 1))
		return -1;
	if (y <3)
		pos = 0;
	else
		pos = 1;
	y = y % 4;
	if (value == 0)
		big[pos] &= ~(1 << (8 * y + x));
	else
		big[pos] |= 1 << (8 * y + x);
	
	return 0;
}

int bc_bigcharwrite(int fd, int *big, int count)
{
	int err = write(fd, &count, sizeof(count));
	if (err == -1)
		return -1;
	err = write(fd, big, count * (sizeof(int)) * 2);
	if (err == -1)
		return -1;
	
	return 0;
}

int bc_bigcharread(int fd, int *big, int need_count, int *count)
{
	int n, cnt, err;
	
	err = read(fd, &n, sizeof(n));
	if (err == -1 || (err != sizeof(n)))
		return -1;
	cnt = read(fd, big, need_count * sizeof(int) * 2);
	if (cnt == -1)
		return -1;
	*count = cnt / (sizeof(int) * 2);
	
	return 0;
}


