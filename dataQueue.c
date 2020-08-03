/*
 * @Description: 
 * @Author: Huang
 * @Email: huangmailbox@126.com
 * @Version: 
 * @Date: 2020-08-01 14:59:38
 * @LastEditors: Huang
 * @LastEditTime: 2020-08-03 22:52:10
 */
#ifdef __cplusplus
extern "C"
{
#endif /* #ifdef __cplusplus */

#include "DataQueue.h"

    /* 数据列表控制块，管理数据队列 */
    struct DQHandleInfo DQHandle = {.DQBuf = {0}, .DQBufFree = __DATA_QUEUE_BUF_SIZE, .DQCnt = 0, .DQList = {0}, .Idle = DQOk, .Stat = DQOk};

    /**
     * @description: 获取数据列表控制块互斥量
     * @param
     * @return 0：成功
     *         1：失败
     */
    int _DQGetDQIdle(void)
    {
        if (DQBusy == DQHandle.Idle)
        {
            return 1;
        }
        else
        {
            DQHandle.Idle = DQBusy;
            return 0;
        }
    }
    /**
     * @description: 释放数据列表控制块互斥量，无论是否占用都释放
     * @param
     * @return 
     */
    void _DQFreeDQIdle(void)
    {
        DQHandle.Idle = DQBusy;
    }
    /**
     * @description: 判断数据列表序号是否有效
     * @param num 数据列表序号
     * @return 0：有效
     *         1：无效
     */
    int _DQJudgeDQNum(DQNum_t num)
    {
        // 校验参数，参数正确则开始插入内容
        if ((0 == num) || (__DATA_QUEUE_LIST_SIZE <= num))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    /**
     * @description: 获取数据列表互斥量
     * @param 数据列表序号
     * @return 0：成功
     *         1：失败
     */
    int _DQGetQueueIdle(DQNum_t num)
    {
        if (1 == _DQJudgeDQNum(num))
        {
            return 1;
        }
        else
        {
            if (DQBusy == DQHandle.DQList[num].Idle)
            {
                DQHandle.Stat = DQBusy;
                return 1;
            }
            else
            {
                DQHandle.DQList[num].Idle = DQBusy;
                return 0;
            }
        }
    }
    /**
     * @description: 释放数据列表互斥量
     * @param num 数据列表序号
     * @return 0：成功
     *         1：失败
     */
    int _DQFreeQueueIdle(DQNum_t num)
    {
        if (1 == _DQJudgeDQNum(num))
        {
            return 1;
        }
        else
        {
            DQHandle.DQList[num].Idle = DQOk;
            return 0;
        }
    }
    /**
     * @description: 判断数据列表状态
     * @param num 数据列表序号
     * @return 0:无效
     *         1：有效
     *        -1：错误
     */
    int _DQJudgeQueueIsNULL(DQNum_t num)
    {
        if (1 == _DQJudgeDQNum(num))
        {
            return -1;
        }
        else
        {
            if (0 == _DQGetDQIdle())
            {
                if (NULL == DQHandle.DQList[num].pData)
                {
                    _DQFreeDQIdle();
                    return 0;
                }
                else
                {
                    _DQFreeDQIdle();
                    return 1;
                }
            }
        }
    }
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
            if (DQBusy == Unit->Idle)
            {
                Unit->Stat = DQBusy;
                DQHandle.Stat = DQBusy;
                DQHandle.Idle = DQOk;
                return 0;
            }
            else
            {
                Unit->Idle = DQBusy;
            }

            if (NULL == Unit->pData)
            { // 找到空闲数据队列单元并初始化成功后返回
                Unit->pData = DQHandle.DQBuf[__DATA_QUEUE_BUF_SIZE - DQHandle.DQBufFree];
                Unit->pHead = Unit->pData;
                Unit->DataSize = Size;
                Unit->pTail = Unit->pHead[Unit->DataSize - 1];
                Unit->DataCnt = 0;
                Unit->Idle = DQOk;
                Unit->Stat = DQOk;
                DQHandle.DQBufFree -= Size;
                DQHandle.Stat = DQOk;
                DQHandle.Idle = DQOk;
                return ListCnt + 1;
            }
            Unit->Idle = DQOk;
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
        else
        {
            DQHandle.Idle = DQBusy;
        }
        // 校验参数，参数正确则开始插入内容
        if ((0 == num) || (__DATA_QUEUE_LIST_SIZE <= num) || (NULL == dat) || (0 == size))
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return -1;
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
            DQHandle.Stat = DQOk;
            DQHandle.Idle = DQOk;

            Unit = &DQHandle.DQList[num];
            if (DQBusy != Unit->Idle)
            {
                Unit->Idle = DQBusy;
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
                }
                else
                {
                    Unit->Stat = DQFull;
                    Unit->Idle = DQOk;
                    return -1;
                }
                Unit->Stat = DQOk;
                Unit->Idle = DQOk;
                return 0;
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
        else
        {
            DQHandle.Idle = DQBusy;
        }
        // 校验参数，参数正确则开始插入内容
        if ((0 == num) || (__DATA_QUEUE_LIST_SIZE <= num) || (NULL == dat) || (0 == size))
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return 0;
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
            DQHandle.Stat = DQOk;
            DQHandle.Idle = DQOk;

            Unit = &DQHandle.DQList[num];
            if (DQBusy != Unit->Idle)
            {
                Unit->Idle = DQBusy;
                if (0 == Unit->DataCnt)
                {
                    Unit->Stat = DQEmpty;
                    Unit->Idle = DQOk;
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
                            Unit->Stat = DQOk;
                            Unit->Idle = DQOk;
                            return popCnt + 1;
                        }
                    }
                    Unit->Stat = DQOk;
                    Unit->Idle = DQOk;
                    return popCnt;
                }
            }
        }
    }
    enum DQStatus DQGetDQSta(void)
    {
        if (DQOk == DQHandle.Idle)
        {
            return DQHandle.Stat;
        }
        else
        {
            return DQBusy;
        }
    }
    enum DQStatus DQGetQueueSta(DQNum_t num)
    {
        struct DQUnit *Unit = NULL;
        // 检查状态
        if (DQBusy == DQHandle.Idle)
        {
            DQHandle.Stat = DQBusy;
            return DQBusy;
        }
        else
        {
            DQHandle.Idle = DQBusy;
        }
        // 校验参数，参数正确则开始插入内容
        if ((0 == num) || (__DATA_QUEUE_LIST_SIZE <= num))
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return DQErr;
        }
        // 判断数据队列是否存在
        Unit = &DQHandle.DQList[num];
        if (NULL == Unit->pData)
        {
            DQHandle.Stat = DQErr;
            DQHandle.Idle = DQOk;
            return -1;
        }
        if (DQOk == DQHandle.Idle)
        {
            return DQHandle.Stat;
        }
        else
        {
            return DQBusy;
        }
        /**
     * @description: 
     * @param num 数据列表序号
     * @return -1：错误
     *          0：数据列表空
     *          1：数据列表满
     */
    }
    int DQIsFull(DQNum_t num)
    {
        enum DQStatus staTemp;
        int ret;
        staTemp = DQGetQueueSta(num);
        if (DQErr == staTemp)
        {
            return -1;
        }
        if (DQBusy != staTemp)
        {
            DQHandle.DQList[num].Idle = DQBusy;
            ret = DQHandle.DQList[num].DataCnt >= DQHandle.DQList[num].DataSize ? 1 : 0;
            DQHandle.DQList[num].Stat = DQOk;
            DQHandle.DQList[num].Idle = DQOk;
            return ret;
        }
        else
        {
            return -1;
        }
    }
    int DQIsEmpty(DQNum_t num)
    {

        enum DQStatus staTemp;
        int ret;
        staTemp = DQGetQueueSta(num);
        if (DQErr == staTemp)
        {
            return -1;
        }
        if (DQBusy != staTemp)
        {
            DQHandle.DQList[num].Idle = DQBusy;
            ret = DQHandle.DQList[num].DataCnt == 0 ? 1 : 0;
            DQHandle.DQList[num].Stat = DQOk;
            DQHandle.DQList[num].Idle = DQOk;
            return ret;
        }
        else
        {
            return -1;
        }
    }
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
