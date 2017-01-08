#include "lua_if.h"

/* mos_ -----------------------------------------------*/

const unsigned char mos_v_stdin  = 5;
const unsigned char mos_v_stdout = 6;
const unsigned char mos_v_stderr = 8;
FIL * mos_stdin  = (FIL*)&mos_v_stdin;
FIL * mos_stdout = (FIL*)&mos_v_stdout;
FIL * mos_stderr = (FIL*)&mos_v_stderr;

#define IS_STD_STREAM(fp) if(fp == mos_stdin ||  fp == mos_stdout || fp == mos_stderr)

int mos_getc(FIL * fp)
{
    MadUint n;
    unsigned char c;
    IS_STD_STREAM(fp) {
        return fgetc(stdin);
    } else {
        if(FR_OK == f_read(fp, &c, 1, &n)) {
            return c;
        } else {
            return EOF;
        }
    }
}

int mos_ungetc(int c, FIL * fp)
{
    MadUint n;
    unsigned char tmp = (unsigned char)c;
    IS_STD_STREAM(fp) {
        return std_ungetc(c);
    } else {
        if(FR_OK == f_write(fp, &tmp, 1, &n)) {
            return c;
        } else {
            return EOF;
        }
    }
}

char* mos_fgets(char * buf, int bufsize, FIL * fp)
{
    IS_STD_STREAM(fp) {
        char * p = buf;
        int cnt = bufsize - 1;
        while(cnt--) {
            *p = fgetc(stdin);
            if(*p == '\r')
                break;
            p++;
        }
        *p = 0;
        return buf;
    } else {
        return f_gets(buf, bufsize, fp);
    }
}

int mos_fputs(const char * s, FIL * fp)
{
    int res;
    IS_STD_STREAM(fp) {
        res = printf("%s", s);
    } else {
        res = f_printf(fp, s);
    }
    return res;
}

int mos_fprintf(FIL * fp, const char * format, ...)
{
    int res;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    res = EOF;
    IS_STD_STREAM(fp) {
        res = printf(format, arg_ptr);
    } else {
        res = f_printf(fp, format, arg_ptr);
    }
    va_end(arg_ptr);
    return res;
}

int mos_setvbuf(FIL * fp, char * buf, int type, unsigned size)
{
    IS_STD_STREAM(fp) {
        return EOF;
    }
    
    if(_MAX_SS < size) {
        return EOF;
    } else {
        return 0;
    }
}

void mos_clearerr(FIL * fp)
{
    IS_STD_STREAM(fp) {
        return;
    }
    
    fp->err = FR_OK;
}

int mos_ferror(FIL * fp)
{
    IS_STD_STREAM(fp) {
        return 0;
    }
    
    if(FR_OK == f_error(fp)) {
        return 0;
    } else {
        return EOF;
    }
}

FIL* mos_tmpfile(void)
{
    // Tag : How auto delete tmpfile ?
    return 0;
}

FIL* mos_fopen(const char * path, const char * mode)
{
    FIL     *f;
    MadU8   bMode;
    MadBool isAdd;
    
    bMode = 0;
    isAdd = MFALSE;
    
    if(strchr(mode, '+')) {
        bMode |= FA_READ | FA_WRITE;
        isAdd = MTRUE;
        if(strchr(mode, 'r')) {
            bMode |= FA_OPEN_EXISTING;
        } else if(strchr(mode, 'w') || strchr(mode, 'a')) {
            bMode |= FA_OPEN_ALWAYS;
        } else {
            bMode = 0;
        }
    } else {
        if(strchr(mode, 'r')) {
            bMode |= FA_READ | FA_OPEN_EXISTING;
        } else if(strchr(mode, 'w') || strchr(mode, 'a')) {
            bMode |= FA_WRITE | FA_OPEN_ALWAYS;
        }
    }
    
    if(bMode && (FR_OK == f_open(f, path, bMode))) {
        if(isAdd) {
            if(FR_OK == f_lseek(f, f_size(f)))
                return f;
        } else {
            return f;
        }
    }
    return 0;
}

FIL* mos_freopen(const char * filename, const char * mode, FIL * fp)
{
    IS_STD_STREAM(fp) {
        // Tag : Std-x is NOT a fp.
        return 0;
    } else {
        f_close(fp);
        return mos_fopen(filename, mode);
    }
}

int mos_fclose(FIL * fp)
{
    IS_STD_STREAM(fp) {
        return 0;
    }
    
    if(FR_OK == f_close(fp)) {
        return 0;
    } else {
        return EOF;
    }
}

int mos_remove(const char * filename)
{
    if(FR_OK == f_unlink(filename)) {
        return 0;
    } else {
        return EOF;
    }
}

int mos_rename(const char * oldn, const char * newn)
{
    if(FR_OK == f_rename(oldn, newn)) {
        return 0;
    } else {
        return EOF;
    }
}

size_t mos_fread(void * buffer, size_t size, size_t count, FIL * fp)
{
    MadSize_t br;
    
    IS_STD_STREAM(fp) {
        // Tag ...
        return 0;
    }
    
    if(FR_OK == f_read(fp, buffer, size * count, &br)) {
        return br / size;
    } else {
        return 0;
    }
}

size_t mos_fwrite(const void * buffer, size_t size, size_t count, FIL * fp)
{
    MadSize_t bw;
    
    IS_STD_STREAM(fp) {
        // Tag ...
        return 0;
    }
    
    if(FR_OK == f_write(fp, buffer, size * count, &bw)) {
        return bw / size;
    } else {
        return 0;
    }
}

int mos_fflush(FIL * fp)
{
    IS_STD_STREAM(fp) {
        return 0;
    }
    
    if(FR_OK == f_sync(fp)) {
        return 0;
    } else {
        return EOF;
    }
}

int mos_fseek(FIL *fp, long o, int w)
{
    FRESULT res;
    
    IS_STD_STREAM(fp) {
        return EOF;
    }
    
    switch (w)
    {
        case SEEK_SET:
            res = f_lseek(fp, o);
            break;
        case SEEK_CUR:
            res = f_lseek(fp, f_tell(fp) + o);
            break;
        case SEEK_END:
            res = f_lseek(fp, f_size(fp) - o);
            break;
        default:
            res = FR_INVALID_OBJECT;
            break;
    }
    
    if(res == FR_OK) {
        return 0;
    } else {
        return EOF;
    }
}

long mos_ftell(FIL * fp)
{
    IS_STD_STREAM(fp) {
        return 0;
    }
    return f_tell(fp);
}

int mos_feof(FIL * fp)
{
    IS_STD_STREAM(fp) {
        return MFALSE;
    }
    return f_eof(fp);
}

/* lua_ -----------------------------------------------*/
size_t lua_writestring(const void * s, size_t l)
{
    MadU8 *buf;
    MadSize_t len = strlen(s);
    if(len <= l) {
        buf = (MadU8*)s;
    } else {
        buf = madMemMalloc(l + 1);
        if(buf) {
            madMemCopy(buf, s, l);
            buf[l] = 0;
        } else {
            return 0;
        }
    }
    return printf("%s", buf);
}
