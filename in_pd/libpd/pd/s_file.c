/* Copyright (c) 1997-2004 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*
 * this file implements a mechanism for storing and retrieving preferences.
 * Should later be renamed "preferences.c" or something.
 *
 * In unix this is handled by the "~/.pdsettings" file, in windows by
 * the registry, and in MacOS by the Preferences system.
 */

#include "m_pd.h"
#include "s_stuff.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#ifdef HAVE_UNISTD_H
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#include <io.h>
#endif
#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
#define snprintf _snprintf
#endif

void sys_doflags( void);

static PERTHREAD char *sys_prefbuf;
static PERTHREAD int sys_prefbufsize;
static PERTHREAD FILE *sys_prefsavefp;

static void sys_initloadpreferences_file(const char *filename)
{
    int fd;
    long length;
    if ((fd = open(filename, 0)) < 0)
    {
        if (sys_verbose)
            perror(filename);
        return;
    }
    length = lseek(fd, 0, 2);
    if (length < 0)
    {
        if (sys_verbose)
            perror(filename);
        close(fd);
        return;
    }
    lseek(fd, 0, 0);
    if (!(sys_prefbuf = malloc(length + 2)))
    {
        error("couldn't allocate memory for preferences buffer");
        close(fd);
        return;
    }
    sys_prefbuf[0] = '\n';
    if (read(fd, sys_prefbuf+1, length) < length)
    {
        perror(filename);
        sys_prefbuf[0] = 0;
        close(fd);
        return;
    }
    sys_prefbuf[length+1] = 0;
    close(fd);
    if (sys_verbose)
        post("success reading preferences from: %s", filename);
}

static int sys_getpreference_file(const char *key, char *value, int size)
{
    char searchfor[80], *where, *whereend;
    if (!sys_prefbuf)
        return (0);
    sprintf(searchfor, "\n%s:", key);
    where = strstr(sys_prefbuf, searchfor);
    if (!where)
        return (0);
    where += strlen(searchfor);
    while (*where == ' ' || *where == '\t')
        where++;
    for (whereend = where; *whereend && *whereend != '\n'; whereend++)
        ;
    if (*whereend == '\n')
        whereend--;
    if (whereend > where + size - 1)
        whereend = where + size - 1;
    strncpy(value, where, whereend+1-where);
    value[whereend+1-where] = 0;
    return (1);
}

static void sys_doneloadpreferences_file( void)
{
    if (sys_prefbuf)
        free(sys_prefbuf);
}

static void sys_initsavepreferences_file(const char *filename)
{
    if ((sys_prefsavefp = fopen(filename, "w")) == NULL)
        pd_error(0, "%s: %s", filename, strerror(errno));
}

static void sys_putpreference_file(const char *key, const char *value)
{
    if (sys_prefsavefp)
        fprintf(sys_prefsavefp, "%s: %s\n",
            key, value);
}

static void sys_donesavepreferences_file( void)
{
    if (sys_prefsavefp)
    {
        fclose(sys_prefsavefp);
        sys_prefsavefp = 0;
    }
}


/*****  linux/android/BSD etc: read and write to ~/.pdsettings file ******/
#if !defined(_WIN32) && !defined(__APPLE__)

static void sys_initloadpreferences( void)
{
    char filenamebuf[MAXPDSTRING], *homedir = getenv("HOME");
    int fd, length;
    char user_prefs_file[MAXPDSTRING]; /* user prefs file */
        /* default prefs embedded in the package */
    char default_prefs_file[MAXPDSTRING];
    struct stat statbuf;

    snprintf(default_prefs_file, MAXPDSTRING, "%s/default.pdsettings",
        sys_libdir->s_name);
    snprintf(user_prefs_file, MAXPDSTRING, "%s/.pdsettings",
        (homedir ? homedir : "."));
    if (stat(user_prefs_file, &statbuf) == 0)
        strncpy(filenamebuf, user_prefs_file, MAXPDSTRING);
    else if (stat(default_prefs_file, &statbuf) == 0)
        strncpy(filenamebuf, default_prefs_file, MAXPDSTRING);
    else return;
    filenamebuf[MAXPDSTRING-1] = 0;
    sys_initloadpreferences_file(filenamebuf);
}

static int sys_getpreference(const char *key, char *value, int size)
{
    return (sys_getpreference_file(key, value, size));
}

static void sys_doneloadpreferences( void)
{
    sys_doneloadpreferences_file();
}

static void sys_initsavepreferences( void)
{
    char filenamebuf[MAXPDSTRING],
        *homedir = getenv("HOME");
    FILE *fp;

    if (!homedir)
        return;
    snprintf(filenamebuf, MAXPDSTRING, "%s/.pdsettings", homedir);
    filenamebuf[MAXPDSTRING-1] = 0;
    sys_initsavepreferences_file(filenamebuf);
}

static void sys_putpreference(const char *key, const char *value)
{
    sys_putpreference_file(key, value);
}

static void sys_donesavepreferences( void)
{
    sys_donesavepreferences_file();
}

#else  /* !defined(_WIN32) && !defined(__APPLE__) */

static void sys_initloadpreferences( void)
{
    if (sys_prefbuf)
        bug("sys_initloadpreferences");
}
static void sys_doneloadpreferences( void)
{
    if (sys_prefbuf)
        sys_doneloadpreferences_file();
}
static void sys_initsavepreferences( void)
{
    if (sys_prefsavefp)
        bug("sys_initsavepreferences");
}
static void sys_donesavepreferences( void)
{
    if (sys_prefsavefp)
        sys_donesavepreferences_file();
}

static int sys_getpreference(const char *key, char *value, int size)
{
    if (sys_prefbuf)
        return (sys_getpreference_file(key, value, size));
    else
    {
#ifdef _WIN32
        HKEY hkey;
        DWORD bigsize = size;
        LONG err = RegOpenKeyEx(HKEY_CURRENT_USER,
            "Software\\Pure-Data", 0,  KEY_QUERY_VALUE, &hkey);
        if (err != ERROR_SUCCESS)
            return (0);
        err = RegQueryValueEx(hkey, key, 0, 0, value, &bigsize);
        if (err != ERROR_SUCCESS)
        {
            RegCloseKey(hkey);
            return (0);
        }
        RegCloseKey(hkey);
        return (1);
#endif /* _WIN32 */
#ifdef __APPLE__
        char cmdbuf[256];
        int nread = 0, nleft = size;
        char embedded_prefs[MAXPDSTRING];
        char user_prefs[MAXPDSTRING];
        char *homedir = getenv("HOME");
        struct stat statbuf;
       /* the 'defaults' command expects the filename without .plist at the
            end */
        snprintf(embedded_prefs, MAXPDSTRING, "%s/../org.puredata.pd",
            sys_libdir->s_name);
        snprintf(user_prefs, MAXPDSTRING,
            "%s/Library/Preferences/org.puredata.pd.plist", homedir);
        if (stat(user_prefs, &statbuf) == 0)
            snprintf(cmdbuf, 256, "defaults read org.puredata.pd %s 2> /dev/null\n",
                key);
        else snprintf(cmdbuf, 256, "defaults read %s %s 2> /dev/null\n",
                embedded_prefs, key);
        FILE *fp = popen(cmdbuf, "r");
        while (nread < size)
        {
            int newread = fread(value+nread, 1, size-nread, fp);
            if (newread <= 0)
                break;
            nread += newread;
        }
        pclose(fp);
        if (nread < 1)
            return (0);
        if (nread >= size)
            nread = size-1;
        value[nread] = 0;
        if (value[nread-1] == '\n')     /* remove newline character at end */
            value[nread-1] = 0;
        return(1);
#endif /* __APPLE__ */
    }
}

static void sys_putpreference(const char *key, const char *value)
{
    if (sys_prefsavefp)
        sys_putpreference_file(key, value);
    else
    {
#ifdef _WIN32
        HKEY hkey;
        LONG err = RegCreateKeyEx(HKEY_CURRENT_USER,
            "Software\\Pure-Data", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
            NULL, &hkey, NULL);
        if (err != ERROR_SUCCESS)
        {
            error("unable to create registry entry: %s\n", key);
            return;
        }
        err = RegSetValueEx(hkey, key, 0, REG_EXPAND_SZ, value, strlen(value)+1);
        if (err != ERROR_SUCCESS)
            error("unable to set registry entry: %s\n", key);
        RegCloseKey(hkey);
#endif /* _WIN32 */
#ifdef __APPLE__
        char cmdbuf[MAXPDSTRING];
        snprintf(cmdbuf, MAXPDSTRING,
            "defaults write org.puredata.pd %s \"%s\" 2> /dev/null\n", key, value);
        system(cmdbuf);
#endif /* __APPLE__ */
    }
}
#endif  /* !defined(_WIN32) && !defined(__APPLE__) */
