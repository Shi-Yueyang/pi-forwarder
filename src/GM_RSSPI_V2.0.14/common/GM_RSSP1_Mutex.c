/**
* @file GM_RSSP1_Mutex.c
* @brief
* @author Hao Liming
* @date 2010-11-26 22:25:57
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#include "GM_RSSP1_Mutex.h"

#ifdef GM_RSSP1_DISABLE_LOCK

GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex)
{
    return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex)
{
    return GM_RSSP1_TRUE;
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex)
{
    return GM_RSSP1_TRUE;
}

#else

#ifdef WIN32

GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex)
{
    if (NULL == pMutex)
    {
        return GM_RSSP1_FALSE;
    }

    *pMutex = CreateMutex(NULL, GM_RSSP1_FALSE, NULL);

    if (0 == (*pMutex))
    {
        return GM_RSSP1_FALSE;
    }
    else
    {
        return GM_RSSP1_TRUE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex)
{
    DWORD rt = 0;

    rt = WaitForSingleObject(mutex, INFINITE);

    if (WAIT_OBJECT_0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;

    rt = ReleaseMutex(mutex);

    if (0 == rt)
    {
        return GM_RSSP1_FALSE;
    }
    else
    {
        return GM_RSSP1_TRUE;
    }
}
#endif

#if (defined QNX || defined REWORKS)
GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex)
{
    int rt = 0;

    if (NULL == pMutex)
    {
        return GM_RSSP1_FALSE;
    }
    *pMutex = (pthread_mutex_t*)GM_RSSP1_malloc(sizeof(pthread_mutex_t));
    if (NULL == *pMutex)
    {
        return GM_RSSP1_FALSE;
    }

	GM_RSSP1_memset((void *)*pMutex,0,sizeof(pthread_mutex_t));
    rt = pthread_mutex_init(*pMutex, NULL);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;
    rt = pthread_mutex_lock(mutex);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;

    rt = pthread_mutex_unlock(mutex);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}
#endif

#ifdef VXWORKS
GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex)
{
    if (NULL == pMutex)
    {
        return GM_RSSP1_FALSE;
    }

    *pMutex = semBCreate(SEM_Q_FIFO, SEM_FULL);

    if (NULL == (*pMutex))
    {
        return GM_RSSP1_FALSE;
    }
    else
    {
        return GM_RSSP1_TRUE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex)
{
    STATUS rt = OK;

    rt = semTake(mutex, WAIT_FOREVER);

    if (OK == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex)
{
    STATUS rt = OK;

    rt = semGive(mutex);

    if (OK == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

#endif

#ifdef LINUX
GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex)
{
    int rt = 0;

    if (NULL == pMutex)
    {
        return GM_RSSP1_FALSE;
    }
    *pMutex = (pthread_mutex_t*)GM_RSSP1_malloc(sizeof(pthread_mutex_t));
    if (NULL == *pMutex)
    {
        return GM_RSSP1_FALSE;
    }

    GM_RSSP1_memset((void *)*pMutex,0,sizeof(pthread_mutex_t));
    rt = pthread_mutex_init(*pMutex, NULL);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;
    rt = pthread_mutex_lock(mutex);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;

    rt = pthread_mutex_unlock(mutex);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}
#endif

#ifdef REWORKS_CERTV2
GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex)
{
	struct mutex_attr mta;
    int rt = 0;

    if (NULL == pMutex)
    {
        return GM_RSSP1_FALSE;
    }
    
    *pMutex = (struct mutex *)GM_RSSP1_malloc(sizeof(struct mutex));
    if (NULL == *pMutex)
    {
        return GM_RSSP1_FALSE;
    }
    
    mta.ceiling = 255;
    mta.flags = MUTEX_PRIO_INHERIT;
    
    rt = mutex_create(pMutex, &mta);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;
    rt = mutex_lock(mutex, 0);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}

GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex)
{
    int rt = 0;

    rt = mutex_unlock(mutex);

    if (0 == rt)
    {
        return GM_RSSP1_TRUE;
    }
    else
    {
        return GM_RSSP1_FALSE;
    }
}
#endif

#endif