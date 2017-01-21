#pragma once

#include <iostream>

#include "stdafx.h"

namespace {

	static const WORD bgMask(BACKGROUND_BLUE | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_INTENSITY);
	static const WORD fgMask(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);

	static const WORD flBlack(0);
	static const WORD flDarkRed(FOREGROUND_RED);
	static const WORD flDarkGreen(FOREGROUND_GREEN);
	static const WORD flDarkBlue(FOREGROUND_BLUE);
	static const WORD flDarkCyan(flDarkGreen | flDarkBlue);
	static const WORD flDarkMagenta(flDarkRed | flDarkBlue);
	static const WORD flDarkYellow(flDarkRed | flDarkGreen);

	static const WORD flGray(flDarkRed | flDarkGreen | flDarkBlue);
	static const WORD flDarkGray(flBlack | FOREGROUND_INTENSITY);

	static const WORD flWhite(flGray | FOREGROUND_INTENSITY);
	static const WORD flBlue(flDarkBlue | FOREGROUND_INTENSITY);
	static const WORD flGreen(flDarkGreen | FOREGROUND_INTENSITY);
	static const WORD flRed(flDarkRed | FOREGROUND_INTENSITY);
	static const WORD flCyan(flDarkCyan | FOREGROUND_INTENSITY);
	static const WORD flMagenta(flDarkMagenta | FOREGROUND_INTENSITY);
	static const WORD flYellow(flDarkYellow | FOREGROUND_INTENSITY);

	static class con_util
	{
	private:
		DWORD                       cCharsWritten;
		CONSOLE_SCREEN_BUFFER_INFO  csbi;
		DWORD                       dwConSize;

	public:
		HANDLE                      hCon;
		con_util()
		{
			hCon = GetStdHandle(STD_OUTPUT_HANDLE);
		}
	private:
		void GetInfo()
		{
			GetConsoleScreenBufferInfo(hCon, &csbi);
			dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
		}
	public:
		void Clear()
		{
			COORD coordScreen = { 0, 0 };

			GetInfo();
			FillConsoleOutputCharacter(hCon, ' ',
				dwConSize,
				coordScreen,
				&cCharsWritten);
			GetInfo();
			FillConsoleOutputAttribute(hCon,
				csbi.wAttributes,
				dwConSize,
				coordScreen,
				&cCharsWritten);
			SetConsoleCursorPosition(hCon, coordScreen);
		}
		void SetColor(WORD wFlags, WORD wMask)
		{
			GetInfo();
			csbi.wAttributes &= wMask;
			csbi.wAttributes |= wFlags;
			SetConsoleTextAttribute(hCon, csbi.wAttributes);
		}
	} console;


}

namespace con {

	typedef WORD CONSOLE_COLOR;

	static const CONSOLE_COLOR Black(flBlack);
	static const CONSOLE_COLOR DarkRed(flDarkRed);
	static const CONSOLE_COLOR DarkGreen(flDarkGreen);
	static const CONSOLE_COLOR DarkBlue(flDarkBlue);
	static const CONSOLE_COLOR DarkCyan(flDarkCyan);
	static const CONSOLE_COLOR DarkMagenta(flDarkMagenta);
	static const CONSOLE_COLOR DarkYellow(flDarkYellow);
	static const CONSOLE_COLOR Gray(flGray);
	static const CONSOLE_COLOR DarkGray(flDarkGray);
	static const CONSOLE_COLOR White(flWhite);
	static const CONSOLE_COLOR Blue(flBlue);
	static const CONSOLE_COLOR Green(flGreen);
	static const CONSOLE_COLOR Red(flRed);
	static const CONSOLE_COLOR Cyan(flCyan);
	static const CONSOLE_COLOR Magenta(flMagenta);
	static const CONSOLE_COLOR Yellow(flYellow);

	void SetHandle(HANDLE handle) {
		console.hCon = handle;
	}

	inline std::ostream& clr(std::ostream& os)
	{
		os.flush();
		console.Clear();
		return os;
	};

	template <CONSOLE_COLOR color>
	inline std::ostream& fgCol(std::ostream& os)
	{
		os.flush();
		console.SetColor(color, bgMask);
		return os;
	}

	template <CONSOLE_COLOR color>
	inline std::wostream& fgCol(std::wostream& os)
	{
		os.flush();
		console.SetColor(color, bgMask);
		return os;
	}

	template <CONSOLE_COLOR color>
	inline std::ostream& bgCol(std::ostream& os)
	{
		os.flush();
		console.SetColor(color << 4, FgMask);
		return os;
	}

	template <CONSOLE_COLOR color>
	inline std::wostream& bgCol(std::wostream& os)
	{
		os.flush();
		console.SetColor(color << 4, FgMask);
		return os;
	}
}