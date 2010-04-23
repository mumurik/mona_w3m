// This file's encoding is UTF-8.

#define MAINPROGRAM
extern "C" {
#include "fm.h"
#undef init

#define DSTR_LEN	256

Hist *LoadHist;
Hist *SaveHist;
Hist *URLHist;
Hist *ShellHist;
Hist *TextHist;

typedef struct _Event {
    int cmd;
    void *data;
    struct _Event *next;
} Event;
static Event *CurrentEvent = NULL;
static Event *LastEvent = NULL;
int on_target = 1;
static int display_ok = FALSE;
int prec_num = 0;

/* yyy */


void
pushEvent(int cmd, void *data)
{
    Event *event;

    event = New(Event);
    event->cmd = cmd;
    event->data = data;
    event->next = NULL;
    if (CurrentEvent)
	LastEvent->next = event;
    else
	CurrentEvent = event;
    LastEvent = event;
}

void
chkURLBuffer(Buffer *buf)
{
}

void
calcTabPos(void)
{
}

void
follow_map(struct parsed_tagarg *arg)
{
}

void
download_action(struct parsed_tagarg *arg)
{
}

static int check_target = TRUE;

char *
searchKeyData(void)
{
    char *data = NULL;

    if (CurrentKeyData != NULL && *CurrentKeyData != '\0')
	data = CurrentKeyData;
    else if (CurrentCmdData != NULL && *CurrentCmdData != '\0')
	data = CurrentCmdData;
    else if (CurrentKey >= 0)
	data = getKeyData(CurrentKey);
    CurrentKeyData = NULL;
    CurrentCmdData = NULL;
    if (data == NULL || *data == '\0')
	return NULL;
    return allocStr(data, -1);
}

void
tmpClearBuffer(Buffer *buf)
{
    if (buf->pagerSource == NULL && writeBufferCache(buf) == 0) {
	buf->firstLine = NULL;
	buf->topLine = NULL;
	buf->currentLine = NULL;
	buf->lastLine = NULL;
    }
}


TabBuffer *
deleteTab(TabBuffer * tab)
{
  return FirstTab;
}

static void
_followForm(int submit)
{
     MONA_TRACE("follow form, NYI\n");
}

static void
pushBuffer(Buffer *buf)
{
    Buffer *b;

#ifdef USE_IMAGE
    deleteImage(Currentbuf);
#endif
    if (clear_buffer)
	tmpClearBuffer(Currentbuf);
    if (Firstbuf == Currentbuf) {
	buf->nextBuffer = Firstbuf;
	Firstbuf = Currentbuf = buf;
    }
    else if ((b = prevBuffer(Firstbuf, Currentbuf)) != NULL) {
	b->nextBuffer = buf;
	buf->nextBuffer = Currentbuf;
	Currentbuf = buf;
    }
#ifdef USE_BUFINFO
    saveBufferInfo();
#endif

}

static void
gotoLabel(char *label)
{
    Buffer *buf;
    Anchor *al;
    int i;

    al = searchURLLabel(Currentbuf, label);
    if (al == NULL) {
	/* FIXME: gettextize? */
	disp_message(Sprintf("%s is not found", label)->ptr, TRUE);
	return;
    }
    buf = newBuffer(Currentbuf->width);
    copyBuffer(buf, Currentbuf);
    for (i = 0; i < MAX_LB; i++)
	buf->linkBuffer[i] = NULL;
    buf->currentURL.label = allocStr(label, -1);
    pushHashHist(URLHist, parsedURL2Str(&buf->currentURL)->ptr);
    (*buf->clone)++;
    pushBuffer(buf);
    gotoLine(Currentbuf, al->start.line);
    if (label_topline)
	Currentbuf->topLine = lineSkip(Currentbuf, Currentbuf->topLine,
				       Currentbuf->currentLine->linenumber
				       - Currentbuf->topLine->linenumber,
				       FALSE);
    Currentbuf->pos = al->start.pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
    return;
}

static Buffer *
loadNormalBuf(Buffer *buf, int renderframe)
{
    pushBuffer(buf);
    if (renderframe && RenderFrame && Currentbuf->frameset != NULL)
	rFrame();
    return buf;
}

static void
delBuffer(Buffer *buf)
{
    if (buf == NULL)
	return;
    if (Currentbuf == buf)
	Currentbuf = buf->nextBuffer;
    Firstbuf = deleteBuffer(Firstbuf, buf);
    if (!Currentbuf)
	Currentbuf = Firstbuf;
}



static Buffer *
loadLink(char *url, char *target, char *referer, FormList *request)
{
    Buffer *buf, *nfbuf;
    union frameset_element *f_element = NULL;
    int flag = 0;
    ParsedURL *base, pu;

    message(Sprintf("loading %s", url)->ptr, 0, 0);
    refresh();

    base = baseURL(Currentbuf);
    if (base == NULL ||
	base->scheme == SCM_LOCAL || base->scheme == SCM_LOCAL_CGI)
	referer = NO_REFERER;
    if (referer == NULL)
	referer = parsedURL2Str(&Currentbuf->currentURL)->ptr;
    buf = loadGeneralFile(url, baseURL(Currentbuf), referer, flag, request);
    if (buf == NULL) {
	char *emsg = Sprintf("Can't load %s", url)->ptr;
	disp_err_message(emsg, FALSE);
	return NULL;
    }

    parseURL2(url, &pu, base);
    pushHashHist(URLHist, parsedURL2Str(&pu)->ptr);

    if (buf == NO_BUFFER) {
	return NULL;
    }
    if (!on_target)		/* open link as an indivisual page */
	return loadNormalBuf(buf, TRUE);

    if (do_download)		/* download (thus no need to render frame) */
	return loadNormalBuf(buf, FALSE);

    if (target == NULL ||	/* no target specified (that means this page is not a frame page) */
	!strcmp(target, "_top") ||	/* this link is specified to be opened as an indivisual * page */
	!(Currentbuf->bufferprop & BP_FRAME)	/* This page is not a frame page */
	) {
	return loadNormalBuf(buf, TRUE);
    }
    nfbuf = Currentbuf->linkBuffer[LB_N_FRAME];
    if (nfbuf == NULL) {
	/* original page (that contains <frameset> tag) doesn't exist */
	return loadNormalBuf(buf, TRUE);
    }

    f_element = search_frame(nfbuf->frameset, target);
    if (f_element == NULL) {
	/* specified target doesn't exist in this frameset */
	return loadNormalBuf(buf, TRUE);
    }

    /* frame page */

    /* stack current frameset */
    pushFrameTree(&(nfbuf->frameQ), copyFrameSet(nfbuf->frameset), Currentbuf);
    /* delete frame view buffer */
    delBuffer(Currentbuf);
    Currentbuf = nfbuf;
    /* nfbuf->frameset = copyFrameSet(nfbuf->frameset); */
    resetFrameElement(f_element, buf, referer, request);
    discardBuffer(buf);
    rFrame();
    {
	Anchor *al = NULL;
	char *label = pu.label;

	if (label && f_element->element->attr == F_BODY) {
	    al = searchAnchor(f_element->body->nameList, label);
	}
	if (!al) {
	    label = Strnew_m_charp("_", target, NULL)->ptr;
	    al = searchURLLabel(Currentbuf, label);
	}
	if (al) {
	    gotoLine(Currentbuf, al->start.line);
	    if (label_topline)
		Currentbuf->topLine = lineSkip(Currentbuf, Currentbuf->topLine,
					       Currentbuf->currentLine->
					       linenumber -
					       Currentbuf->topLine->linenumber,
					       FALSE);
	    Currentbuf->pos = al->start.pos;
	    arrangeCursor(Currentbuf);
	}
    }
    displayBuffer(Currentbuf, B_NORMAL);
    return buf;
}

static void
cmd_loadBuffer(Buffer *buf, int prop, int linkid)
{
    if (buf == NULL) {
	disp_err_message("Can't load string", FALSE);
    }
    else if (buf != NO_BUFFER) {
	buf->bufferprop |= (BP_INTERNAL | prop);
	if (!(buf->bufferprop & BP_NO_URL))
	    copyParsedURL(&buf->currentURL, &Currentbuf->currentURL);
	if (linkid != LB_NOLINK) {
	    buf->linkBuffer[REV_LB[linkid]] = Currentbuf;
	    Currentbuf->linkBuffer[linkid] = buf;
	}
	pushBuffer(buf);
    }
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

static void
cmd_loadURL(char *url, ParsedURL *current, char *referer, FormList *request)
{
    Buffer *buf;

    if (!strncasecmp(url, "mailto:", 7)
#ifdef USE_W3MMAILER
	&& non_null(Mailer) && strchr(url, '?') == NULL
#endif
	) {
#ifdef MONA
         MONA_TRACE("mailto: not supported\n");
	return;
#else /* not MONA */
	/* invoke external mailer */
	Str to = Strnew_charp(url + 7);
#ifndef USE_W3MMAILER
	char *pos;
	if (!non_null(Mailer)) {
	    /* FIXME: gettextize? */
	    disp_err_message("no mailer is specified", TRUE);
	    return;
	}
	if ((pos = strchr(to->ptr, '?')) != NULL)
	    Strtruncate(to, pos - to->ptr);
#endif
	fmTerm();
	system(myExtCommand(Mailer, shell_quote(file_unquote(to->ptr)),
			    FALSE)->ptr);
	fmInit();
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	pushHashHist(URLHist, url);
	return;
#endif /* not MONA */
    }
#if 0
    if (!strncasecmp(url, "news:", 5) && strchr(url, '@') == NULL) {
	/* news:newsgroup is not supported */
	/* FIXME: gettextize? */
	disp_err_message("news:newsgroup_name is not supported", TRUE);
	return;
    }
#endif				/* USE_NNTP */

    refresh();
    buf = loadGeneralFile(url, current, referer, 0, request);
    if (buf == NULL) {
	/* FIXME: gettextize? */
	char *emsg = Sprintf("Can't load %s", conv_from_system(url))->ptr;
	disp_err_message(emsg, FALSE);
    }
    else if (buf != NO_BUFFER) {
	pushBuffer(buf);
	if (RenderFrame && Currentbuf->frameset != NULL)
	    rFrame();
    }
    displayBuffer(Currentbuf, B_NORMAL);
}

static void
_quitfm(int confirm)
{
    MONA_TRACE("quit NYI\n");
}

static void
anchorMn(Anchor *(*menu_func) (Buffer *), int go)
{
    Anchor *a;
    BufferPoint *po;

    if (!Currentbuf->href || !Currentbuf->hmarklist)
	return;
    a = menu_func(Currentbuf);
    if (!a || a->hseq < 0)
	return;
    po = &Currentbuf->hmarklist->marks[a->hseq];
    gotoLine(Currentbuf, po->line);
    Currentbuf->pos = po->pos;
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_NORMAL);
    if (go)
	followA();
}

/* Go to specified line */
static void
_goLine(char *l)
{
    if (l == NULL || *l == '\0' || Currentbuf->currentLine == NULL) {
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
    }
    Currentbuf->pos = 0;
    if (((*l == '^') || (*l == '$')) && prec_num) {
	gotoRealLine(Currentbuf, prec_num);
    }
    else if (*l == '^') {
	Currentbuf->topLine = Currentbuf->currentLine = Currentbuf->firstLine;
    }
    else if (*l == '$') {
	Currentbuf->topLine =
	    lineSkip(Currentbuf, Currentbuf->lastLine,
		     -(Currentbuf->LINES + 1) / 2, TRUE);
	Currentbuf->currentLine = Currentbuf->lastLine;
    }
    else
	gotoRealLine(Currentbuf, atoi(l));
    arrangeCursor(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}


void
w3m_exit(int i)
{
    MONA_TRACE("w3m_exit, do nothing now\n");
}


/* zzz */
#ifdef USE_MARK

/* Set / unset mark */
DEFUN(_mark, MARK, "Set/unset mark")
{
}

#endif

DEFUN(wrapToggle, WRAP_TOGGLE, "Toggle wrap search mode")
{
    MONA_TRACE("Toggle wrap search mode, NYI\n");
}

DEFUN(dispVer, VERSION, "Display version of w3m")
{
    MONA_TRACE("Display version of w3m, NYI\n");
}

DEFUN(lup1, UP, "Scroll up one line")
{
    MONA_TRACE("Scroll up one line, NYI\n");
}

DEFUN(undoPos, UNDO, "Cancel the last cursor movement")
{
    MONA_TRACE("Cancel the last cursor movement, NYI\n");
}

DEFUN(tabR, TAB_RIGHT, "Move current tab right")
{
    MONA_TRACE("Move current tab right, NYI\n");
}

DEFUN(tabMs, TAB_MOUSE, "Move to tab on mouse cursor (for mouse action)")
{
    MONA_TRACE("Move to tab on mouse cursor (for mouse action), NYI\n");
}

DEFUN(tabL, TAB_LEFT, "Move current tab left")
{
    MONA_TRACE("Move current tab left, NYI\n");
}

DEFUN(tabrURL, TAB_GOTO_RELATIVE, "Open relative URL on new tab")
{
    MONA_TRACE("Open relative URL on new tab, NYI\n");
}

DEFUN(tabURL, TAB_GOTO, "Open URL on new tab")
{
    MONA_TRACE("Open URL on new tab, NYI\n");
}

DEFUN(submitForm, SUBMIT, "Submit form")
{
    MONA_TRACE("Submit form, NYI\n");
}

DEFUN(shiftr, SHIFT_RIGHT, "Shift screen right")
{
    MONA_TRACE("Shift screen right, NYI\n");
}

DEFUN(shiftl, SHIFT_LEFT, "Shift screen left")
{
    MONA_TRACE("Shift screen left, NYI\n");
}


DEFUN(setOpt, SET_OPTION, "Set option")
{
    MONA_TRACE("Set option, NYI\n");
}

DEFUN(setEnv, SETENV, "Set environment variable")
{
    MONA_TRACE("Set environment variable, NYI\n");
}

DEFUN(selBuf, SELECT, "Go to buffer selection panel")
{
    MONA_TRACE("Go to buffer selection panel, NYI\n");
}

DEFUN(srchprv, SEARCH_PREV, "Search previous regexp")
{
    MONA_TRACE("Search previous regexp, NYI\n");
}

DEFUN(srchnxt, SEARCH_NEXT, "Search next regexp")
{
    MONA_TRACE("Search next regexp, NYI\n");
}

DEFUN(srchbak, SEARCH_BACK, "Search backward")
{
    MONA_TRACE("Search backward, NYI\n");
}

DEFUN(srchfor, SEARCH SEARCH_FORE WHEREIS, "Search forward")
{
    MONA_TRACE("Search forward, NYI\n");
}

DEFUN(col1R, RIGHT, "Shift screen one column right")
{
    MONA_TRACE("Shift screen one column right, NYI\n");
}

DEFUN(reinit, REINIT, "Reload configuration files")
{
    MONA_TRACE("Reload configuration files, NYI\n");
}

DEFUN(reMark, REG_MARK, "Set mark using regexp")
{
    MONA_TRACE("Set mark using regexp, NYI\n");
}

DEFUN(rdrwSc, REDRAW, "Redraw screen")
{
    MONA_TRACE("Redraw screen, NYI\n");
}

DEFUN(redoPos, REDO, "Cancel the last undo")
{
    MONA_TRACE("Cancel the last undo, NYI\n");
}

DEFUN(readsh, READ_SHELL, "Execute shell command and load")
{
    MONA_TRACE("Execute shell command and load, NYI\n");
}

DEFUN(svBuf, PRINT SAVE_SCREEN, "Save rendered document to file")
{
    MONA_TRACE("Save rendered document to file, NYI\n");
}

DEFUN(movLW, PREV_WORD, "Move to previous word")
{
    MONA_TRACE("Move to previous word, NYI\n");
}

DEFUN(prevVA, PREV_VISITED, "Move to previous visited link")
{
    MONA_TRACE("Move to previous visited link, NYI\n");
}

DEFUN(prevT, PREV_TAB, "Move to previous tab")
{
    MONA_TRACE("Move to previous tab, NYI\n");
}


DEFUN(pgBack, PREV_PAGE, "Move to previous page")
{
    MONA_TRACE("Move to previous page, NYI\n");
}

DEFUN(prevMk, PREV_MARK, "Move to previous mark")
{
    MONA_TRACE("Move to previous mark, NYI\n");
}

DEFUN(prevA, PREV_LINK, "Move to previous link")
{
    MONA_TRACE("Move to previous link, NYI\n");
}

DEFUN(prevBf, PREV, "Move to previous buffer")
{
    MONA_TRACE("Move to previous buffer, NYI\n");
}

DEFUN(pipesh, PIPE_SHELL, "Execute shell command and browse")
{
    MONA_TRACE("Execute shell command and browse, NYI\n");
}

DEFUN(pipeBuf, PIPE_BUF, "Send rendered document to pipe")
{
    MONA_TRACE("Send rendered document to pipe, NYI\n");
}

DEFUN(peekURL, PEEK_LINK, "Peek link URL")
{
    MONA_TRACE("Peek link URL, NYI\n");
}

DEFUN(peekIMG, PEEK_IMG, "Peek image URL")
{
    MONA_TRACE("Peek image URL, NYI\n");
}

DEFUN(curURL, PEEK, "Peek current URL")
{
    MONA_TRACE("Peek current URL, NYI\n");
}

DEFUN(pcmap, PCMAP, "pcmap")
{
    MONA_TRACE("pcmap, NYI\n");
}

DEFUN(movRW, NEXT_WORD, "Move to next word")
{
    MONA_TRACE("Move to next word, NYI\n");
}

DEFUN(nextVA, NEXT_VISITED, "Move to next visited link")
{
    MONA_TRACE("Move to next visited link, NYI\n");
}

DEFUN(nextU, NEXT_UP, "Move to next upward link")
{
    MONA_TRACE("Move to next upward link, NYI\n");
}

DEFUN(nextT, NEXT_TAB, "Move to next tab")
{
    MONA_TRACE("Move to next tab, NYI\n");
}

DEFUN(nextRD, NEXT_RIGHT_DOWN, "Move to next right (or downward) link")
{
    MONA_TRACE("Move to next right (or downward) link, NYI\n");
}

DEFUN(nextR, NEXT_RIGHT, "Move to next right link")
{
    MONA_TRACE("Move to next right link, NYI\n");
}

DEFUN(pgFore, NEXT_PAGE, "Move to next page")
{
    MONA_TRACE("Move to next page, NYI\n");
}

DEFUN(nextMk, NEXT_MARK, "Move to next word")
{
    MONA_TRACE("Move to next word, NYI\n");
}

DEFUN(nextA, NEXT_LINK, "Move to next link")
{
    MONA_TRACE("Move to next link, NYI\n");
}

DEFUN(nextLU, NEXT_LEFT_UP, "Move to next left (or upward) link")
{
    MONA_TRACE("Move to next left (or upward) link, NYI\n");
}

DEFUN(nextL, NEXT_LEFT, "Move to next left link")
{
    MONA_TRACE("Move to next left link, NYI\n");
}

DEFUN(nextD, NEXT_DOWN, "Move to next downward link")
{
    MONA_TRACE("Move to next downward link, NYI\n");
}

DEFUN(nextBf, NEXT, "Move to next buffer")
{
    MONA_TRACE("Move to next buffer, NYI\n");
}

DEFUN(newT, NEW_TAB, "Open new tab")
{
    MONA_TRACE("Open new tab, NYI\n");
}

DEFUN(multimap, MULTIMAP, "multimap")
{
    MONA_TRACE("multimap, NYI\n");
}

DEFUN(msgs, MSGS, "Display error messages")
{
    MONA_TRACE("Display error messages, NYI\n");
}

DEFUN(movU1, MOVE_UP1, "Move cursor up (1 line scrol at the top of screen)")
{
    MONA_TRACE("Move cursor up (1 line scrol at the top of screen), NYI\n");
}


DEFUN(movU, MOVE_UP,
      "Move cursor up (a half screen scroll at the top of screen)")
{
  MONA_TRACE("Move cursor up (a half screen scroll at the top of screen), NYI\n");
}

DEFUN(movR1, MOVE_RIGHT1,
      "Move cursor right (1 columns shift at the right edge)")
{
  MONA_TRACE("Move cursor right (1 columns shift at the right edge), NYI\n");
}


DEFUN(movR, MOVE_RIGHT,
      "Move cursor right (a half screen shift at the right edge)")
{
  MONA_TRACE("Move cursor right (a half screen shift at the right edge), NYI\n");
}
DEFUN(movMs, MOVE_MOUSE, "Move cursor to mouse cursor (for mouse action)")
{
    MONA_TRACE("Move cursor to mouse cursor (for mouse action), NYI\n");
}


DEFUN(movlistMn, MOVE_LIST_MENU,
      "Popup link list menu and move cursor to selected link")
{
  MONA_TRACE("Popup link list menu and move cursor to selected link, NYI\n");
}
DEFUN(movL1, MOVE_LEFT1, "Move cursor left (1 columns shift at the left edge)")
{
    MONA_TRACE("Move cursor left (1 columns shift at the left edge), NYI\n");
}


DEFUN(movL, MOVE_LEFT,
      "Move cursor left (a half screen shift at the left edge)")
{
  MONA_TRACE("Move cursor left (a half screen shift at the left edge), NYI\n");
}

DEFUN(movD1, MOVE_DOWN1,
      "Move cursor down (1 line scroll at the end of screen)")
{
  MONA_TRACE("Move cursor down (1 line scroll at the end of screen), NYI\n");
}

DEFUN(movD, MOVE_DOWN,
      "Move cursor down (a half screen scroll at the end of screen)")
{
  MONA_TRACE("Move cursor down (a half screen scroll at the end of screen), NYI\n");
}

DEFUN(msToggle, MOUSE_TOGGLE, "Toggle activity of mouse")
{
    MONA_TRACE("Toggle activity of mouse, NYI\n");
}

DEFUN(mouse, MOUSE, "mouse operation")
{
    MONA_TRACE("mouse operation, NYI\n");
}

DEFUN(menuMs, MENU_MOUSE, "Popup menu at mouse cursor (for mouse action)")
{
    MONA_TRACE("Popup menu at mouse cursor (for mouse action), NYI\n");
}

DEFUN(chkWORD, MARK_WORD, "Mark current word as anchor")
{
    MONA_TRACE("Mark current word as anchor, NYI\n");
}

DEFUN(chkURL, MARK_URL, "Mark URL-like strings as anchors")
{
    MONA_TRACE("Mark URL-like strings as anchors, NYI\n");
}

DEFUN(ldfile, LOAD, "Load local file")
{
    MONA_TRACE("Load local file, NYI\n");
}

DEFUN(listMn, LIST_MENU, "Popup link list menu and go to selected link")
{
    MONA_TRACE("Popup link list menu and go to selected link, NYI\n");
}

DEFUN(linkLst, LIST, "Show all links and images")
{
    MONA_TRACE("Show all links and images, NYI\n");
}

#ifdef USE_MENU
/* link menu */
DEFUN(linkMn, LINK_MENU, "Popup link element menu")
{
    MONA_TRACE("Popup link element menu, NYI\n");
}
#endif

DEFUN(lastA, LINK_END, "Go to the last link")
{
    MONA_TRACE("Go to the last link, NYI\n");
}

DEFUN(topA, LINK_BEGIN, "Go to the first link")
{
    MONA_TRACE("Go to the first link, NYI\n");
}

DEFUN(curlno, LINE_INFO, "Show current line number")
{
    MONA_TRACE("Show current line number, NYI\n");
}

DEFUN(linend, LINE_END, "Go to the end of line")
{
    MONA_TRACE("Go to the end of line, NYI\n");
}

DEFUN(linbeg, LINE_BEGIN, "Go to the beginning of line")
{
    MONA_TRACE("Go to the beginning of line, NYI\n");
}

DEFUN(col1L, LEFT, "Shift screen one column")
{
    MONA_TRACE("Shift screen one column, NYI\n");
}

DEFUN(isrchbak, ISEARCH_BACK, "Incremental search backward")
{
    MONA_TRACE("Incremental search backward, NYI\n");
}

DEFUN(isrchfor, ISEARCH, "Incremental search forward")
{
    MONA_TRACE("Incremental search forward, NYI\n");
}

DEFUN(pginfo, INFO, "View info of current document")
{
    MONA_TRACE("View info of current document, NYI\n");
}

DEFUN(gorURL, GOTO_RELATIVE, "Go to relative URL")
{
    MONA_TRACE("Go to relative URL, NYI\n");
}

DEFUN(goLine, GOTO_LINE, "Go to specified line")
{
    MONA_TRACE("Go to specified line, NYI\n");
}

DEFUN(goURL, GOTO, "Go to URL")
{
    MONA_TRACE("Go to URL, NYI\n");
}

DEFUN(linkbrz, EXTERN_LINK, "View current link using external browser")
{
    MONA_TRACE("View current link using external browser, NYI\n");
}

DEFUN(extbrz, EXTERN, "Execute external browser")
{
    MONA_TRACE("Execute external browser, NYI\n");
}


/* Execute shell command */
DEFUN(execsh, EXEC_SHELL SHELL, "Execute shell command")
{
    MONA_TRACE("exec shell command, not supported");
}


DEFUN(escmap, ESCMAP, "ESC map")
{
    MONA_TRACE("ESC map, NYI\n");
}


DEFUN(escbmap, ESCBMAP, "ESC [ map")
{
    MONA_TRACE("ESC [ map, NYI\n");
}

DEFUN(goLineL, END, "Go to the last line")
{
    _goLine("$");
}


/* Run editor on the current screen */
DEFUN(editScr, EDIT_SCREEN, "Edit currently rendered document")
{
    MONA_TRACE("run editor on the current screen, NYI\n");
}

/* download panel */
DEFUN(ldDL, DOWNLOAD_LIST, "Display download list panel")
{
    MONA_TRACE("download panel, NYI\n");
}

/* 1 line down */
DEFUN(ldown1, DOWN, "Scroll down one line")
{
    MONA_TRACE("scroll down one line, NYI\n");
}

DEFUN(deletePrevBuf, DELETE_PREVBUF,
      "Delete previous buffer (mainly for local-CGI)")
{
    Buffer *buf = Currentbuf->nextBuffer;
    if (buf)
	delBuffer(buf);
}



DEFUN(defKey, DEFINE_KEY,
      "Define a binding between a key stroke and a user command")
{
    MONA_TRACE("define key binding, NYI\n");
}

#ifdef USE_COOKIE
/* cookie list */
DEFUN(cooLst, COOKIE, "View cookie list")
{
    Buffer *buf;

    buf = cookie_list_panel();
    if (buf != NULL)
	cmd_loadBuffer(buf, BP_NO_URL, LB_NOLINK);
}
#endif				/* USE_COOKIE */

DEFUN(execCmd, COMMAND, "Execute w3m command(s)")
{
    MONA_TRACE("exec cmd, NYI\n");
}

DEFUN(closeTMs, CLOSE_TAB_MOUSE,
      "Close tab on mouse cursor (for mouse action)")
{
    MONA_TRACE("close tab on mouse, NYI\n");
}

DEFUN(closeT, CLOSE_TAB, "Close current tab")
{
    MONA_TRACE("close tab, NYI\n");
}

/* move cursor position to the center of screen */
DEFUN(ctrCsrV, CENTER_V, "Move to the center column")
{
    int offsety;
    if (Currentbuf->firstLine == NULL)
	return;
    offsety = Currentbuf->LINES / 2 - Currentbuf->cursorY;
    if (offsety != 0) {
#if 0
	Currentbuf->currentLine = lineSkip(Currentbuf,
					   Currentbuf->currentLine, offsety,
					   FALSE);
#endif
	Currentbuf->topLine =
	    lineSkip(Currentbuf, Currentbuf->topLine, -offsety, FALSE);
	arrangeLine(Currentbuf);
	displayBuffer(Currentbuf, B_NORMAL);
    }
}

DEFUN(ctrCsrH, CENTER_H, "Move to the center line")
{
    int offsetx;
    if (Currentbuf->firstLine == NULL)
	return;
    offsetx = Currentbuf->cursorX - Currentbuf->COLS / 2;
    if (offsetx != 0) {
	columnSkip(Currentbuf, offsetx);
	arrangeCursor(Currentbuf);
	displayBuffer(Currentbuf, B_NORMAL);
    }
}


DEFUN(goLineF, BEGIN, "Go to the first line")
{
    _goLine("^");
}

/* Add current to bookmark */
DEFUN(adBmark, ADD_BOOKMARK, "Add current page to bookmark")
{
    MONA_TRACE("add bookmark, NYI\n");
}

/* accesskey */
DEFUN(accessKey, ACCESSKEY, "Popup acceskey menu")
{
    anchorMn(accesskey_menu, TRUE);
}

/* Quit */
DEFUN(quitfm, ABORT EXIT, "Quit w3m without confirmation")
{
    _quitfm(FALSE);
}

/* Question and Quit */
DEFUN(qquitfm, QUIT, "Quit w3m")
{
    _quitfm(confirm_on_quit);
}

DEFUN(tabA, TAB_LINK, "Open current link on new tab")
{
    MONA_TRACE("open link on tab, NYI\n"); 
/*    followTab(prec_num ? numTab(PREC_NUM) : NULL); */
}

/* download HREF link */
DEFUN(svA, SAVE_LINK, "Save link to file")
{
    MONA_TRACE("save link, NYI\n");
}

/* view inline image */
DEFUN(followI, VIEW_IMAGE, "View image")
{
    MONA_TRACE("View inline image, NYI\n");
}
/* download IMG link */
DEFUN(svI, SAVE_IMAGE, "Save image to file")
{
    MONA_TRACE("save image, NYI\n");
}

/* load bookmark */
DEFUN(ldBmark, BOOKMARK VIEW_BOOKMARK, "Read bookmark")
{
    cmd_loadURL(BookmarkFile, NULL, NO_REFERER, NULL);
}

/* Load help file */
DEFUN(ldhelp, HELP, "View help")
{
    cmd_loadURL(helpFile(HELP_FILE), NULL, NO_REFERER, NULL);
}

/* option setting */
DEFUN(ldOpt, OPTIONS, "Option setting panel")
{
    cmd_loadBuffer(load_option_panel(), BP_NO_URL, LB_NOLINK);
}

/* render frame */
DEFUN(rFrame, FRAME, "Render frame")
{
    Buffer *buf;

    if ((buf = Currentbuf->linkBuffer[LB_FRAME]) != NULL) {
	Currentbuf = buf;
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    if (Currentbuf->frameset == NULL) {
	if ((buf = Currentbuf->linkBuffer[LB_N_FRAME]) != NULL) {
	    Currentbuf = buf;
	    displayBuffer(Currentbuf, B_NORMAL);
	}
	return;
    }
    if (fmInitialized) {
	message("Rendering frame", 0, 0);
	refresh();
    }
    buf = renderFrame(Currentbuf, 0);
    if (buf == NULL) {
	displayBuffer(Currentbuf, B_NORMAL);
	return;
    }
    buf->linkBuffer[LB_N_FRAME] = Currentbuf;
    Currentbuf->linkBuffer[LB_FRAME] = buf;
    pushBuffer(buf);
    if (fmInitialized && display_ok)
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
}



DEFUN(backBf, BACK, "Back to previous buffer")
{
}

DEFUN(susp, INTERRUPT SUSPEND, "Stop loading document")
{
}

DEFUN(vwSrc, SOURCE VIEW, "View HTML source")
{
   MONA_TRACE("view html NYI\n");
}

DEFUN(reload, RELOAD, "Reload buffer")
{
   MONA_TRACE("reload NYI\n");
}

DEFUN(reshape, RESHAPE, "Re-render buffer")
{
    Currentbuf->need_reshape = TRUE;
    reshapeBuffer(Currentbuf);
    displayBuffer(Currentbuf, B_FORCE_REDRAW);
}

DEFUN(editBf, EDIT, "Edit current document")
{
    MONA_TRACE("edit document, NYI\n");
}

DEFUN(svSrc, DOWNLOAD SAVE, "Save document source to file")
{
    MONA_TRACE("save document source, NYI\n");
}

DEFUN(nulcmd, NOTHING NULL @@@, "Do nothing")
{				/* do nothing */
}



DEFUN(followA, GOTO_LINK, "Go to current link")
{
    Line *l;
    Anchor *a;
    ParsedURL u;
#ifdef USE_IMAGE
    int x = 0, y = 0, map = 0;
#endif
    char *url;

    if (Currentbuf->firstLine == NULL)
	return;
    l = Currentbuf->currentLine;

#ifdef USE_IMAGE
    a = retrieveCurrentImg(Currentbuf);
    if (a && a->image && a->image->map) {
	_followForm(FALSE);
	return;
    }
    if (a && a->image && a->image->ismap) {
	getMapXY(Currentbuf, a, &x, &y);
	map = 1;
    }
#else
    a = retrieveCurrentMap(Currentbuf);
    if (a) {
	_followForm(FALSE);
	return;
    }
#endif
    a = retrieveCurrentAnchor(Currentbuf);
    if (a == NULL) {
	_followForm(FALSE);
	return;
    }
    if (*a->url == '#') {	/* index within this buffer */
	gotoLabel(a->url + 1);
	return;
    }
    parseURL2(a->url, &u, baseURL(Currentbuf));
    if (Strcmp(parsedURL2Str(&u), parsedURL2Str(&Currentbuf->currentURL)) == 0) {
	/* index within this buffer */
	if (u.label) {
	    gotoLabel(u.label);
	    return;
	}
    }
    if (!strncasecmp(a->url, "mailto:", 7)
#ifdef USE_W3MMAILER
	&& non_null(Mailer) && strchr(a->url, '?') == NULL
#endif
	) {
#ifdef MONA
        MONA_TRACE("mailto: not supported\n");
#else /* not MONA */
	/* invoke external mailer */
	Str to = Strnew_charp(a->url + 7);
#ifndef USE_W3MMAILER
	char *pos;
	if (!non_null(Mailer)) {
	    /* FIXME: gettextize? */
	    disp_err_message("no mailer is specified", TRUE);
	    return;
	}
	if ((pos = strchr(to->ptr, '?')) != NULL)
	    Strtruncate(to, pos - to->ptr);
#endif
	fmTerm();
	system(myExtCommand(Mailer, shell_quote(file_unquote(to->ptr)),
			    FALSE)->ptr);
	fmInit();
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	pushHashHist(URLHist, a->url);
#endif
	return;
    }
#if 0
    else if (!strncasecmp(a->url, "news:", 5) && strchr(a->url, '@') == NULL) {
	/* news:newsgroup is not supported */
	/* FIXME: gettextize? */
	disp_err_message("news:newsgroup_name is not supported", TRUE);
	return;
    }
#endif				/* USE_NNTP */
    url = a->url;
#ifdef USE_IMAGE
    if (map)
	url = Sprintf("%s?%d,%d", a->url, x, y)->ptr;
#endif

    if (check_target && open_tab_blank && a->target &&
	(!strcasecmp(a->target, "_new") || !strcasecmp(a->target, "_blank"))) {
#ifdef MONA
      MONA_TRACE("ignore target now\n");
      /* fall through */
#else
	Buffer *buf;

	_newT();
	buf = Currentbuf;
	loadLink(url, a->target, a->referer, NULL);
	if (buf != Currentbuf)
	    delBuffer(buf);
	else
	    deleteTab(CurrentTab);
	displayBuffer(Currentbuf, B_FORCE_REDRAW);
	return;
#endif
    }
    loadLink(url, a->target, a->referer, NULL);
    displayBuffer(Currentbuf, B_NORMAL);
}



/* extern "C" */ }

#include <baygui.h>



class W3MFrame: public Frame {
public:
  W3MFrame() {
     setBounds((800 - 212) / 2, (600 - 50) / 2, 212, 50);
     setTitle("w3m");
     m_label = new Label(m_time, Label::CENTER);
     m_label->setBounds(0, 4, 200, 16);
     add(m_label);
  }

  ~W3MFrame(){
      delete(m_label);
  }

private: 
    char m_time[128];
    Label *m_label;
};

int main(int argc, char* argv[]) {
    W3MFrame *frame = new W3MFrame();
    frame->run();
    delete(frame);
    return 0;
}

