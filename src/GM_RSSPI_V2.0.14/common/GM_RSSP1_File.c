/**
* @file GM_RSSP1_Ini_File.c
* @brief initialization file read and write API implementation
* @authro Hao Liming
* @date 2009-9-18 16:22:00
* @version 0.3
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/

#include "GM_RSSP1_File.h"

#ifdef FILE_SYSTEM
static void GM_RSSP1_Ini_File_Parse_Sub1(int * key_s, int newline_start, int * key_e, unsigned int j, unsigned int *valid, const char p[], int * value_s, int * value_e, unsigned int i);
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub2(const char p[], unsigned int j, int newline_start, const char * key, int * key_s, int * key_e, int valid, int * value_s, int * value_e, unsigned int i);
static void GM_RSSP1_Ini_File_Parse_Sub3(unsigned int *j, const char p[], int *valid);
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub4(unsigned int j, unsigned int i, const char p[], int valid, int newline_start, const char * key, int * key_s, int * key_e, int * value_s, int * value_e);
static unsigned int GM_RSSP1_Ini_File_Parse_Sub5(const char p[], unsigned int i);
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub6(const char p[], unsigned int j, unsigned int i, int valid, int newline_start, const char * key, int * key_s, int * key_e, int * value_s, int * value_e);
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub7(const char p[], unsigned int *i, int *newline_start, const char * key, int * key_s, int * key_e, int * value_s, int * value_e);
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub8(unsigned int *i, int section_start, const char * section, const char p[], int *newline_start, int * sec_s, int * sec_e, const char * key, int * key_s, int * key_e, int * value_s, int * value_e);
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse(const char *section, const char *key, const char *buf, int *sec_s, int *sec_e,
                                 int *key_s, int *key_e, int *value_s, int *value_e);




GM_RSSP1_BOOL GM_RSSP1_Ini_File_Load(const char* filePathAndName, char fileContent[], int fileContentSize)
{
    FILE* in = NULL;
    int i    = 0;
    int rt   = 0;
	
    /*fileContentSize判断小于等于0, 1也应该排除掉 0930CR 需要 修改*/
    if ((NULL == filePathAndName) || (NULL == (char*)fileContent) || (fileContentSize <= 1))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_LOAD_PAR_ERR, fileContentSize, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    in = fopen(filePathAndName, "r");

    if (NULL == in)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_LOAD_OPEN_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    fileContent[i] = (char)fgetc(in);

    while (fileContent[i] != (char)EOF)
    {
        ++i;

        if (i < fileContentSize)
        {
            fileContent[i] = (char)fgetc(in);
        }
        else
        {
            fclose(in);
            GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_LOAD_CONTENT_ERR, 0, 0, 0, 0, 0, 0);
            return GM_RSSP1_FALSE;
        }
    }

    fileContent[i] = (char)'\0';

    rt = fclose(in);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_LOAD_CLOSE_ERR, rt, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }
}

/*得到key和value的起止*/
static void GM_RSSP1_Ini_File_Parse_Sub1(int * key_s, int newline_start, int * key_e, unsigned int j, unsigned int *valid, const char p[], int * value_s, int * value_e, unsigned int i)
{
    if ((NULL == key_s) || (NULL == key_e) || (NULL == valid) || (NULL == p) || (NULL == value_s) || (NULL == value_e))
    {
        return;
    }

    /*find the key ok*/
    *key_s = newline_start;

    *key_e = (int)j - 1;

    *valid = j + 1U;

    while ((' ' == p[*valid]) || ('\t' == p[*valid]))
    {
        ++(*valid);
    }

    *value_s = (int)(*valid);

    *value_e = (int)i;

    return;
}

/*判断当前key是否有效 调用sub1得到key和value的 起止*/
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub2(const char p[], unsigned int j, int newline_start, const char * key, int * key_s, int * key_e, int valid, int * value_s, int * value_e, unsigned int i)
{
    size_t temp = 0U;

    if ((NULL == p) || (NULL == key) || (NULL == key_s) || (NULL == key_e) || (NULL == value_s) || (NULL == value_e))
    {
        return GM_RSSP1_FALSE;
    }

    if ('=' == p[j])
    {
        temp = (size_t)j - (size_t)newline_start;

        if ((temp == strlen(key)) && (0 == strncmp(key, p + newline_start, strlen(key))))
        {
            GM_RSSP1_Ini_File_Parse_Sub1(key_s, newline_start, key_e, j, (unsigned int*)(&valid), p, value_s, value_e, i);
            return GM_RSSP1_TRUE;
        }
    }

    return GM_RSSP1_FALSE;
}

/*处理等号之后的空格 tab 多余的=号*/
static void GM_RSSP1_Ini_File_Parse_Sub3(unsigned int *j, const char p[], int *valid)
{
    if ((NULL == j) || (NULL == p) || (NULL == valid))
    {
        return;
    }

    ++*j;

    if ((' ' != p[*j]) && ('\t' != p[*j]) && ('=' != p[*j]))
    {
        *valid = (int)(*j);
    }

    return;
}

/*判断=符号的存在和位置*/
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub4(unsigned int j, unsigned int i, const char p[], int valid, int newline_start, const char * key, int * key_s, int * key_e, int * value_s, int * value_e)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

    if ((NULL == p) || (NULL == key) || (NULL == key_s) || (NULL == key_e) || (NULL == value_s) || (NULL == value_e))
    {
        return GM_RSSP1_FALSE;
    }

    while ((j < i) && (p[j] != '='))
    {
        GM_RSSP1_Ini_File_Parse_Sub3(&j, p, &valid);

        rt = GM_RSSP1_Ini_File_Parse_Sub2(p, j, newline_start, key, key_s, key_e, valid, value_s, value_e, i);

        if (GM_RSSP1_TRUE == rt)
        {
            return rt;
        }
    }

    return GM_RSSP1_FALSE;
}

/*找下一行的起位置*/
static unsigned int GM_RSSP1_Ini_File_Parse_Sub5(const char p[], unsigned int i)
{
    if (NULL == p)
    {
        return i;
    }

    while (('\n' != p[i]) && ('\r' != p[i]) && ('\0' != p[i]))
    {
        ++i;
    }

    return i;
}

/*找key，判断行首的注释符号#，如果存在注释符号，本行不找了*/
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub6(const char p[], unsigned int j, unsigned int i, int valid, int newline_start, const char * key, int * key_s, int * key_e, int * value_s, int * value_e)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

    if ((NULL == p) || (NULL == key) || (NULL == key_s) || (NULL == key_e) || (NULL == value_s) || (NULL == value_e))
    {
        return GM_RSSP1_FALSE;
    }

    if (';' != p[j]) /*skip over comment*/
    {
        rt = GM_RSSP1_Ini_File_Parse_Sub4(j, i, p, valid, newline_start, key, key_s, key_e, value_s, value_e);

        if (GM_RSSP1_TRUE == rt)
        {
            return rt;
        }
    }

    return GM_RSSP1_FALSE;
}

/*sub5确定每行的起位置，sub6找key， 本函数确定 本section下面，key是否存在*/
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub7(const char p[], unsigned int *i, int *newline_start, const char * key, int * key_s, int * key_e, int * value_s, int * value_e)
{
    GM_RSSP1_BOOL rt  = GM_RSSP1_FALSE;
    unsigned int j = 0u;
    int valid      = 0;

    if ((NULL == p) || (NULL == i) || (NULL == newline_start) || (NULL == key) || (NULL == key_s) || (NULL == key_e) || (NULL == value_s) || (NULL == value_e))
    {
        return GM_RSSP1_FALSE;
    }

    while (!((('\n' == p[*i -1u]) || ('\r' == p[*i -1u])) && ('[' == p[*i])))
    {
        /*get a new line*/
        *newline_start = (int)( * i);

        *i = GM_RSSP1_Ini_File_Parse_Sub5(p, *i);

        /*now *i  is equal to end of the line*/
        j = (unsigned int)( * newline_start);
        valid = (int)j;

        rt = GM_RSSP1_Ini_File_Parse_Sub6(p, j, *i, valid, *newline_start, key, key_s, key_e, value_s, value_e);

        if (GM_RSSP1_TRUE == rt)
        {
            return rt;
        }

        if ('\0' == p[*i])
        {
            return GM_RSSP1_FALSE;
        }

        ++*i;
    }

    return GM_RSSP1_FALSE;
}

/*确定section是否匹配 得到section起止*/
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse_Sub8(unsigned int *i, int section_start, const char * section, const char p[], int *newline_start, int * sec_s, int * sec_e, const char * key, int * key_s, int * key_e, int * value_s, int * value_e)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
    size_t temp = 0U;

    if ((NULL == i) || (NULL == section) || (NULL == p) || (NULL == newline_start) || (NULL == sec_s) || (NULL == sec_e) || (NULL == key) || (NULL == key_s) || (NULL == key_e) || (NULL == value_s) || (NULL == value_e))
    {
        return GM_RSSP1_FALSE;
    }

    temp = (size_t)(*i) - (size_t)section_start;

    if ((temp == strlen(section)) && (0 == strncmp(p + section_start, section, strlen(section))))
    {
        *newline_start = 0;
        ++*i;

        /*Skip over space char after ']'*/

        while (0 != isspace(p[*i]))
        {
            ++*i;
        }

        /*find the section*/
        *sec_s = section_start;

        *sec_e = (int)(*i);

        rt = GM_RSSP1_Ini_File_Parse_Sub7(p, &*i, &*newline_start, key, key_s, key_e, value_s, value_e);

        if (GM_RSSP1_TRUE == rt)
        {
            return rt;
        }
    }

    return GM_RSSP1_FALSE;
}

/*最外层查找，返回section起止位置 key起止位置 value起止位置*/
static GM_RSSP1_BOOL GM_RSSP1_Ini_File_Parse(const char *section, const char *key, const char *buf, int *sec_s, int *sec_e,
                                 int *key_s, int *key_e, int *value_s, int *value_e)
{
    const char *p     = buf;
    unsigned int i    = 0u;
    int section_start = 0;
    int newline_start = 0;
    GM_RSSP1_BOOL rt     = GM_RSSP1_FALSE;

    if ((NULL == section) || (NULL == key) || (NULL == buf) || (NULL == sec_s)
        || (NULL == sec_e) || (NULL == key_s) || (NULL == key_e) || (NULL == value_s) || (NULL == value_e)
        || (0U == strlen(section)) || (0U == strlen(key)))
    {
        return GM_RSSP1_FALSE;
    }

    *sec_e   = -1;
    *sec_s   = -1;
    *key_e   = -1;
    *key_s   = -1;
    *value_s = -1;
    *value_e = -1;

    while ('\0' != *(p+i))
    {
        /*find the section*/
        if ((((unsigned int)0U == i) || (('\n' == *(p+i-1U)) || ('\r' == *(p+i-1U)))) && ('[' == *(p+i)))
        {
            section_start = (int)(i + 1U);

            /*find the ']'*/

            do
            {
                ++i;
            }
            while ((']' != *(p+i)) && ('\0' != *(p+i)));

            rt = GM_RSSP1_Ini_File_Parse_Sub8(&i, section_start, section, p, &newline_start, sec_s, sec_e, key, key_s, key_e, value_s, value_e);

            if (GM_RSSP1_TRUE == rt)
            {
                return rt;
            }
        }
        else
        {
            ++i;
        }
    }

    return GM_RSSP1_FALSE;
}

GM_RSSP1_BOOL GM_RSSP1_Ini_File_Read_String(const char *section, const char *key, char value[], int size, GM_RSSP1_INT8 *iniFileContent)
{
    int sec_s     = 0;
    int sec_e     = 0;
    int key_s     = 0;
    int key_e     = 0;
    int value_s   = 0;
    int value_e   = 0;
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;

    /*check parameters*/

    if ((NULL == section) || (NULL == key) || (NULL == (char*)value) || (NULL == iniFileContent)
        || (0U == strlen(section)) || (0U == strlen(key)) || (size <= 0))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_STRING_PAR_ERR, size, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    rt = GM_RSSP1_Ini_File_Parse(section, key, (char *)iniFileContent, &sec_s, &sec_e, &key_s, &key_e, &value_s, &value_e);

    if (GM_RSSP1_FALSE == rt)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_STRING_NOT_FIND_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE; /*not find the key*/
    }
    else
    {
        int cpcount = value_e - value_s;

        if ((size - 1) < cpcount)
        {
            GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_STRING_SIZE_ERR, size, cpcount, 0, 0, 0, 0);
            return GM_RSSP1_FALSE;/*size必须要大于cpcount+1,因为地址空间的最后还要加上'\0'*/
        }

        GM_RSSP1_memset((void*)value, 0U, (size_t)size);

        GM_RSSP1_memcpy((void*)value, (const void*)(iniFileContent + value_s), (size_t)cpcount);
        value[cpcount] = (char)'\0';

        return GM_RSSP1_TRUE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Ini_File_Read_Int(const char *section, const char *key, int* value, GM_RSSP1_INT8 *iniFileContent)
{
    GM_RSSP1_BOOL rt  = GM_RSSP1_FALSE;
    char valueStr[64U] = {'\0'};

    /*check parameters*/

    if ((NULL == section) || (NULL == key) || (NULL == value) || (NULL == iniFileContent))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_INT_PAR_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    rt = GM_RSSP1_Ini_File_Read_String((const char *)section, (const char *)key, (char *)valueStr, (int)sizeof(valueStr), (GM_RSSP1_INT8 *)iniFileContent);

    if (GM_RSSP1_FALSE == rt)
    {
        return GM_RSSP1_FALSE;
    }
    else
    {
        rt = GM_RSSP1_atoi(valueStr, (GM_RSSP1_UINT32)sizeof(valueStr), value);
        if (GM_RSSP1_FALSE == rt)
        {
            GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_INT_ATOI_ERR, 0, 0, 0, 0, 0, 0);
            return GM_RSSP1_FALSE;
        }
        return GM_RSSP1_TRUE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Ini_File_Read_BOOL(const char *section, const char *key, GM_RSSP1_BOOL* value, GM_RSSP1_INT8 *iniFileContent)
{
    GM_RSSP1_BOOL rt = GM_RSSP1_FALSE;
    int valueInt  = -1;

    /*check parameters*/

    if ((NULL == section) || (NULL == key) || (NULL == value) || (NULL == iniFileContent))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_BOOL_PAR_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    rt = GM_RSSP1_Ini_File_Read_Int((const char *)section, (const char *)key, &valueInt, (GM_RSSP1_INT8 *)iniFileContent);

    if (GM_RSSP1_TRUE == rt)
    {
        if (0 == valueInt)
        {
            *value = GM_RSSP1_FALSE;
            return GM_RSSP1_TRUE;
        }
        else if (1 == valueInt)
        {
            *value = GM_RSSP1_TRUE;
            return GM_RSSP1_TRUE;
        }
        else
        {
            GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_BOOL_VALUE_ERR, valueInt, 0, 0, 0, 0, 0);
            return GM_RSSP1_FALSE;
        }
    }
    else
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_INI_FILE_READ_BOOL_READ_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }
}

long GM_RSSP1_Get_File_Size(const char *filePathAndName)
{
    long cur_pos = 0;
    long len     = 0;
    FILE *fp     = NULL;
    int rt       = 0;

    if (NULL == filePathAndName)
    {
        return -1;
    }

    fp = fopen(filePathAndName, "r");

    if (NULL == fp)
    {
        return -1;
    }

    /* 取得当前文件流的读取位置 */
    cur_pos = ftell(fp);

	if (-1 == (int)cur_pos)
	{
		fclose(fp);
		return -1;
	}

    /* 将文件流的读取位置设为文件末尾 */
    rt = fseek(fp, 0, SEEK_END);

    if (0 != rt)
    {
        fclose(fp);

        return -1;
    }

    /* 获取文件末尾的读取位置, 即文件大小 */
    len = ftell(fp);

    /* 将文件流的读取位置还原为原先的值 */
    rt = fseek(fp, cur_pos, SEEK_SET);

	if (0 != rt)
	{
		fclose(fp);
		return -1;
	}

    rt = fclose(fp);

	if (0 != rt)
	{
		return -1;
	}
    /*增加 EOF的计数*/
    return (len + 1);
}


GM_RSSP1_BOOL GM_RSSP1_Bin_File_Load(const char* fileName, GM_RSSP1_UINT8* pBuf, GM_RSSP1_UINT32 bufSize)
{
    FILE *pFile = NULL;
    int ret     = 0;
    int len     = 0;

    if ((NULL == fileName) || (NULL == pBuf))
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_BIN_FILE_LOAD_PAR_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    pFile = fopen(fileName, "rb");

    if (NULL == pFile)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_BIN_FILE_LOAD_OPEN_ERR, 0, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    len = (int)GM_RSSP1_Get_File_Size(fileName);
    
    if (len <= 0)
    {
        fclose(pFile);
        GM_RSSP1_Log_Usr(GM_RSSP1_BIN_FILE_LOAD_LEN_ERR, len, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    ret = (int)fread((void *)pBuf, (size_t)1, (size_t)bufSize, pFile);

    /*sjz: 如果文件只读了一部分，返回*/
    if (ret < (len-1) )
    {
        fclose(pFile);
        GM_RSSP1_Log_Usr(GM_RSSP1_BIN_FILE_LOAD_READ_ERR, ret, len, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }

    ret = fclose(pFile);

    if (0 != ret)
    {
        GM_RSSP1_Log_Usr(GM_RSSP1_BIN_FILE_LOAD_CLOSE_ERR, ret, 0, 0, 0, 0, 0);
        return GM_RSSP1_FALSE;
    }
    return GM_RSSP1_TRUE;
}
#endif
