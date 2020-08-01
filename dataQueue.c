/*
 * @Description: 
 * @Author: Huang
 * @Email: huangmailbox@126.com
 * @Version: 
 * @Date: 2020-08-01 14:59:38
 * @LastEditors: Huang
 * @LastEditTime: 2020-08-01 19:24:42
 */
#ifdef __cplusplus
extern "C"
{
#endif /* #ifdef __cplusplus */

#include "DataQueue.h"

    /* 数据列表控制块，管理数据队列 */
    struct DQHandleInfo DQHandle = {.DQBuf = {0}, .DQBufFree = __DATA_QUEUE_BUF_SIZE, .DQCnt = 0, .DQList = {0}, .Idle = DQOk, .Stat = DQOk};
    /**
     * @description: 创建一个数据队列，失败操作结果通过 DQGetStatus 获取
     * @author: Huang
     * @param Size：创建的数据队列大小
     * @return: 0:失败，结果通过 DQGetStatu 获取
     */
    DQNum_t DQCreat(unsigned int Size)
    {
        int ListCnt = 0;
        struct DQUnit *Unit = NULL;
        // 状态获取
        if (DQBusy == DQHandle.Idle)
        {
            DQHandle.Stat = DQBusy;
            return 0;
        }
        else
        {
            DQHandle.Idle = DQBusy;
            DQHandle.Stat = DQBusy;
        }
        // 参数校验
        if (0 == Size)
        {
            DQHandle.Idle = DQOk;
            DQHandle.Stat = DQErr;
            return 0;
        }
        if ((__DATA_QUEUE_LIST_SIZE <= DQHandle.DQCnt) || (DQHandle.DQBufFree < Size))
        {
            DQHandle.Idle = DQOk;
            DQHandle.Stat = DQFull;
            return 0;
        }
        // 创建
        for (ListCnt = 0; ListCnt < __DATA_QUEUE_LIST_SIZE; ListCnt++)
        {
            Unit = &DQHandle.DQList[ListCnt];
            if (NULL == Unit->pData)
            { // 找到空闲数据队列单元并初始化成功后返回
                Unit->pData = DQHandle.DQBuf[__DATA_QUEUE_BUF_SIZE - DQHandle.DQBufFree];
                Unit->pHead = Unit->pData;
                Unit->DataSize = Size;
                Unit->pTail = Unit->pHead[Unit->DataSize - 1];
                Unit->DataCnt = 0;
                DQHandle.DQBufFree -= Size;
                DQHandle.Stat = DQOk;
                DQHandle.Idle = DQOk;
                return ListCnt + 1;
            }
        }
        DQHandle.Stat = DQErr;
        DQHandle.Idle = DQOk;
        return 0;
    }
    /**
    * @description: 向数据数据队列中插入数据，为原子操作
    * @author: Huang
    * @param num:数据队列序号
    *        size:数据数量
    *        dat:数据缓存
    * @return:  0:成功
    *          -1:失败，结果保存在handle的status中
    */
    int DQPushDate(DQNum_t num, char *dat, unsigned int size)
    {
        struct DQUnit *Unit = NULL;
        int pushCnt;
        // 检查状态
        if (DQBusy == DQHandle.Idle)
        {
            DQHandle.Stat = DQBusy;
            return -1;
        }
        // 校验参数，参数正确则开始插入内容
        if ((0 == num) || (__DATA_QUEUE_LIST_SIZE <= num) || (NULL == dat) || (0 == size))
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return -1;
        }
        else
        {
            DQHandle.Idle = DQBusy;
            DQHandle.Stat = DQOk;
        }
        // 判断数据队列是否存在，存在则判断容量是否足够，足够则开始插入
        if (NULL == DQHandle.DQList[num].pData)
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return -1;
        }
        else
        {
            Unit = &DQHandle.DQList[num];
            if ((size <= Unit->DataSize) && (size <= (Unit->DataSize - Unit->DataCnt)))
            {
                for (pushCnt = 0; pushCnt < size; pushCnt++)
                {
                    if (Unit->pTail == Unit->pData[Unit->DataSize - 1])
                    {
                        Unit->pTail = Unit->pData;
                    }
                    else
                    {
                        Unit->pTail++;
                    }
                    *Unit->pTail = dat[pushCnt];
                    Unit->DataCnt++;
                }
                DQHandle.Stat = DQOk;
                DQHandle.Idle = DQOk;
                return 0;
            }
            else
            {
                DQHandle.Stat = DQFull;
                DQHandle.Idle = DQOk;
                return -1;
            }
        }
    }

    /**
    * @description: 向数据数据队列中插入数据，为原子操作
    * @author: Huang
    * @param num:数据队列序号
    *        size:数据数量
    *        dat:数据缓存
    * @return:  0:取出的数据量
    */
    int DQPopData(DQNum_t num, char *dat, unsigned int size)
    {
        struct DQUnit *Unit = NULL;
        int popCnt;
        // 检查状态
        if (DQBusy == DQHandle.Idle)
        {
            DQHandle.Stat = DQBusy;
            return 0;
        }
        // 校验参数，参数正确则开始插入内容
        if ((0 == num) || (__DATA_QUEUE_LIST_SIZE <= num) || (NULL == dat) || (0 == size))
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return 0;
        }
        else
        {
            DQHandle.Idle = DQBusy;
            DQHandle.Stat = DQOk;
        }
        // 判断数据队列是否存在，开始取出
        if (NULL == DQHandle.DQList[num].pData)
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return 0;
        }
        else
        {
            Unit = &DQHandle.DQList[num];
            if (0 == Unit->DataCnt)
            {
                DQHandle.Stat = DQEmpty;
                DQHandle.Idle = DQOk;
                return 0;
            }
            else
            {
                for (popCnt = 0; popCnt < (size > Unit->DataSize ? Unit->DataSize : size); popCnt++)
                {
                    if (Unit->pHead == Unit->pData[Unit->DataSize - 1])
                    {
                        Unit->pHead = Unit->pData;
                    }
                    else
                    {
                        Unit->pTail++;
                    }
                    *Unit->pTail = dat[popCnt];
                    if (--Unit->DataCnt == 0)
                    {
                        DQHandle.Stat = DQOk;
                        DQHandle.Idle = DQOk;
                        return popCnt + 1;
                    }
                }
                DQHandle.Stat = DQOk;
                DQHandle.Idle = DQOk;
                return popCnt;
            }
        }
    }

    int DPIsEmpty(DQNum_t num)
    {
        struct DQUnit *Unit = NULL;
        // 判断数据队列是否存在
        Unit = &DQHandle.DQList[num];
        if (NULL == Unit->pData)
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return -1;
        }
        return (Unit->DataCnt == 0 ? 1 : 0);
    }
    int DQIsFull(DQNum_t num)
    {
        struct DQUnit *Unit = NULL;
        // 判断数据队列是否存在
        Unit = &DQHandle.DQList[num];
        if (NULL == Unit->pData)
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return -1;
        }
        return (Unit->DataCnt == Unit->DataSize ? 1 : 0);
    }
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
