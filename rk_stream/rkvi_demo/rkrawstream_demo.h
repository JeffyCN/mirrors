#ifndef _RKRAWSTREAM_DEMO_H
#define _RKRAWSTREAM_DEMO_H


/* fmts specified */
#define FmtWidth        (1L<<0)
#define FmtHeight       (1L<<1)
#define FmtPixelFormat  (1L<<4)

/* Available options.

   Please keep the first part (options < 128) in alphabetical order.
   That makes it easier to see which short options are still free.

   In general the lower case is used to set something and the upper
   case is used to retrieve a setting. */
enum Option {
    OptUseRkaiq = 'a',
    OptSetFrameDir = 'd',
    OptSetEntityName = 'e',
    OptSetIqfileDir = 'f',
    OptHelp = 'h',
    OptUseRockit= 'k',
    OptSetMode = 'm',
    OptSetFrameNum = 'n',
    OptSetReadbackFmt = 'r',
    OptSetFrameSkip = 's',
    OptSetVicapFmt = 'v',
    OptSetRawFile = 'w',
    OptLast = 512
};

#endif
