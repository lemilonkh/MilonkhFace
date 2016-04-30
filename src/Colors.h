#ifndef COLORS_H
#define COLORS_H

typedef struct {
	int background;
	int text;
	int detail;
} color_triple;

const color_triple colors[] = {
	{0xFF5500, 0xAAFF00, 0xFFAAFF},
	{0x005555, 0xFF5500, 0xFFAA55},
	{0xFFAAAA, 0x55FFFF, 0x00AA55},
	{0xAA5500, 0x00AA00, 0xFFAA00}
	
	/*{GColorMidnightGreen, GColorOrange, GColorRajah},
	{GColorKellyGreen, GColorJazzberryJam, GColorJazzberryJam},
	{GColorMelon, GColorElectricBlue, GColorJaegerGreen},
	{GColorSpringBud, GColorIndigo, GColorOxfordBlue},
	{GColorLavenderIndigo, GColorIcterine, GColorMayGreen},
	{GColorBulgarianRose, GColorLimerick, GColorBrass},
	{GColorMediumSpringGreen, GColorBrilliantRose, GColorRichBrilliantLavender},
	{GColorSunsetOrange, GColorElectricUltramarine, GColorBabyBlueEyes},
	{GColorDarkGreen, GColorScreaminGreen, GColorInchworm},
	{GColorWindsorTan, GColorIslamicGreen, GColorOrange},
	{GColorDarkCandyAppleRed, GColorTiffanyBlue, GColorMidnightGreen},
	{GColorOrange, GColorSpringBud, GColorRichBrilliantLavender}*/
};

#endif