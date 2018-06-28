/*============================================================================
| colors
| 
|  Constants for terminal colors.
| 
|  Used by MessLogger
|  
|  Author: Jody Weissmann
\===========================================================================*/ 

#ifndef __COLORS_H__
#define __COLORS_H__

// text colors
// regular
#define BLACK        "\e[0;30m"   // Black
#define RED          "\e[0;31m"   // Red
#define GREEN        "\e[0;32m"   // Green
#define YELLOW       "\e[0;33m"   // Yellow
#define BLUE         "\e[0;34m"   // Blue
#define PURPLE       "\e[0;35m"   // Purple
#define CYAN         "\e[0;36m"   // Cyan
#define WHITE        "\e[0;37m"   // White

// bold
#define BOLDBLACK    "\e[1;30m"   // Black
#define BOLDRED      "\e[1;31m"   // Red
#define BOLDGREEN    "\e[1;32m"   // Green
#define BOLDYELLOW   "\e[1;33m"   // Yellow
#define BOLDBLUE     "\e[1;34m"   // Blue
#define BOLDPURPLE   "\e[1;35m"   // Purple
#define BOLDCYAN     "\e[1;36m"   // Cyan
#define BOLDWHITE    "\e[1;37m"   // White

// underline
#define UNDBLACK     "\e[4;30m"   // Black
#define UNDRED       "\e[4;31m"   // Red
#define UNDGREEN     "\e[4;32m"   // Green
#define UNDYELLOW    "\e[4;33m"   // Yellow
#define UNDBLUE      "\e[4;34m"   // Blue
#define UNDPURPLE    "\e[4;35m"   // Purple
#define UNDCYAN      "\e[4;36m"   // Cyan
#define UNDWHITE     "\e[4;37m"   // White

// background
#define BGBLACK      "\e[0;40m"   // Black
#define BGRED        "\e[0;41m"   // Red
#define BGGREEN      "\e[0;42m"   // Green
#define BGYELLOW     "\e[0;43m"   // Yellow
#define BGBLUE       "\e[0;44m"   // Blue
#define BGPURPLE     "\e[0;45m"   // Purple
#define BGCYAN       "\e[0;46m"   // Cyan
#define BGWHITE      "\e[0;47m"   // White

// high intensity
#define HIBLACK      "\e[0;90m"   // Black
#define HIRED        "\e[0;91m"   // Red
#define HIGREEN      "\e[0;92m"   // Green
#define HIYELLOW     "\e[0;93m"   // Yellow
#define HIBLUE       "\e[0;94m"   // Blue
#define HIPURPLE     "\e[0;95m"   // Purple
#define HICYAN       "\e[0;96m"   // Cyan
#define HIWHITE      "\e[0;97m"   // White

// bold high intensity
#define BOLDHIBLACK  "\e[1;90m"   // Black
#define BOLDHIRED    "\e[1;91m"   // Red
#define BOLDHIGREEN  "\e[1;92m"   // Green
#define BOLDHIYELLOW "\e[1;93m"   // Yellow
#define BOLDHIBLUE   "\e[1;94m"   // Blue
#define BOLDHIPURPLE "\e[1;95m"   // Purple
#define BOLDHICYAN   "\e[1;96m"   // Cyan
#define BOLDHIWHITE  "\e[1;97m"   // White

// high intensity backgrounds
#define BGHIBLACK    "\e[0;100m"  // Black
#define BGHIRED      "\e[0;101m"  // Red
#define BGHIGREEN    "\e[0;102m"  // Green
#define BGHISYELLOW  "\e[0;103m"  // Yellow
#define BGHIBLUE     "\e[0;104m"  // Blue
#define BGHIPURPLE   "\e[0;105m"  // Purple
#define BGHICYAN     "\e[0;106m"  // Cyan
#define BGHIWHITE    "\e[0;107m"  // White

#define BOLD         "\e[1m"
#define BLINK        "\e[5m"
#define OFF          "\e[0m"      // Text Reset


#endif
