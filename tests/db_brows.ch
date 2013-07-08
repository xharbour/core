/*
 * $Id: db_brows.ch 9279 2011-02-14 18:06:32Z druzus $
 */
// Header file for db_brows.prg
#define LI_LEN    42
#define LI_NSTR      mslist[1]
#define LI_CLR       mslist[2]   // Color of a window
#define LI_CLRV      mslist[3]   // Color of a current line
#define LI_BSKIP     mslist[4]   // Codeblock for a 'skip' operation
#define LI_BGTOP     mslist[5]   // Codeblock for a 'go top'
#define LI_BGBOT     mslist[6]   // Codeblock for a 'go bottom'
#define LI_BEOF      mslist[7]   // Codeblock for a 'eof' checking
#define LI_BBOF      mslist[8]   // Codeblock for a 'bof' checking
#define LI_B1        mslist[9]
#define LI_MSF       mslist[10]  // Array of codeblocks for columns
#define LI_NAMES     mslist[11]  // Array of the fields names
#define LI_NMCLR     mslist[12]  // Color of field names line
#define LI_FREEZE    mslist[13]  // Number of fields to 'freeze' from left
#define LI_RCOU      mslist[14]
#define LI_MSREC     mslist[15]
#define LI_PRFLT     mslist[16]
#define LI_TEKZP     mslist[17]
#define LI_KOLZ      mslist[18]
#define LI_VALID     mslist[19]  // Array of codeblocks for postvalidation while changing a field
#define LI_WHEN      mslist[20]  // Array of codeblocks for prevalidation while changing a field
#define LI_MSNAME    mslist[21]
#define LI_MSTYP     mslist[22]
#define LI_MSLEN     mslist[23]
#define LI_MSDEC     mslist[24]
#define LI_EXPFI     mslist[25]
#define LI_BDESHIN   mslist[26]
#define LI_BDESHOUT  mslist[27]
#define LI_RECNO     mslist[28]
#define LI_BGOTO     mslist[29]
#define LI_Y1        mslist[30]
#define LI_X1        mslist[31]
#define LI_Y2        mslist[32]
#define LI_X2        mslist[33]
#define LI_LSOHR     mslist[34]
#define LI_LVIEW     mslist[35]
#define LI_NCOLUMNS  mslist[36]
#define LI_LEFTVISIBLE  mslist[37]
#define LI_NLEFT     mslist[38]
#define LI_COLPOS    mslist[39]
#define LI_XPOS      mslist[40]
#define LI_MSED      mslist[41]
#define LI_COLCOUNT  mslist[42]