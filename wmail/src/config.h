///////////////////////////////////////////////////////////////////////////////
// config.h
// configuration file parser, part of wmail
//
// Copyright 2000-2002, Sven Geisenhainer <sveng@informatik.uni-jena.de>.
// Copyright 2016-2017, Doug Torrance <dtorrance@piedmont.edu>.
// Copyright 2019, Jeremy Sowden <jeremy@azazel.net>.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions, and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions, and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#ifndef _CONFIG_H_a6ebf0e22e2f5e21fc1657a8d9975e5c_
#define _CONFIG_H_a6ebf0e22e2f5e21fc1657a8d9975e5c_

typedef enum {
    TICKER_ADDRESS = 0,
    TICKER_FAMILYNAME = 1,
    TICKER_NICKNAME = 2
} ticker_mode_t;

// re-allocated color-names (to enable disposing of them on reconfiguration)
enum {
    SYM_COLOR = 1<<0,
    FNT_COLOR = 1<<1,
    BCK_COLOR = 1<<2,
    OFF_COLOR = 1<<3,
    BGR_COLOR = 1<<4
};

// flags to mark used cmdline options
enum {
    CL_DISPLAY             = 1<<0,
    CL_MAILBOX             = 1<<1,
    CL_RUNCMD              = 1<<2,
    CL_SYMBOLCOLOR         = 1<<3,
    CL_FONTCOLOR           = 1<<4,
    CL_BACKCOLOR           = 1<<5,
    CL_OFFLIGHTCOLOR       = 1<<6,
    CL_BACKGROUNDCOLOR     = 1<<7,
    CL_CHECKSUMFILENAME    = 1<<8,
    CL_CHECKINTERVAL       = 1<<9,
    CL_FPS                 = 1<<10,
    CL_NEWMAILONLY         = 1<<11,
    CL_NOSHAPE             = 1<<12,
    CL_TICKERMODE          = 1<<13,
    CL_SKIPNAMES           = 1<<14,
    CL_CMDONMAIL           = 1<<15,
    CL_CONSIDERSTATUSFIELD = 1<<16,
    CL_READSTATUS          = 1<<17,
    CL_USEX11FONT          = 1<<18
};

typedef struct _config_t {
    char *display;            // display-name
    char *mailBox;            // mailbox-name
    char *runCmd;             // command to run when the run-btn was pressed
    char *symbolColor;        // colorname of the upper-symbols while they are active
    char *fontColor;          // colorname of the ticker- and counter-digits-font
    char *backColor;          // colorname of the background
    char *offlightColor;      // colorname of inactive symbols and counter-digits
    char *backgroundColor;    // colorname of the frame when in noshape-mode
    char *checksumFileName;   // name of the checksum-file
    int checkInterval;        // sesonds between mailbox-checks
    int fps;                  // ticker-frames per second
    bool newMailsOnly;        // true means only ticker and count new (unread) mail
    bool noshape;             // true means don't use shape the dockapp
    ticker_mode_t tickerMode; // ticker senders adress, family-name or nick-name
    char **skipNames;         // sender-names that wmail has to skip
    char *cmdOnMail;          // command to execute when a new mail has received
    bool considerStatusField;  // use the status-field of the mail-header to determine its read-status
    char *readStatus;         // status field content that indicates read mails ("O" for netscape, "ro" for pine etc.)
    char *useX11Font;         // X11 font to render the ticker (NULL -> fallback to buildin font)
    // ---------------------- //
    int colorsUsed;           // used (malloced) color-names
    int givenOptions;         // bitfield flags all options specified on the cmd-line
} config_t;


// app configuration data (declared in config.c)
extern config_t config;

// config manipulation functions
void ReadConfigFile( bool resetConfigStrings );

void ResetConfigStrings( void );

#endif
