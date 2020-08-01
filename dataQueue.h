/*
 * @Description: DQ(DataQueue)数据队列，多个数据队列并发访问时可能会出现被占用无法访问状态可尝试将数据队列的标志保存到数据队列自己的恶状态中
 * @Author: Huang
 * @Email: huangmailbox@126.com
 * @Version: 
 * @Date: 2020-08-01 14:59:23
 * @LastEditors: Huang
 * @LastEditTime: 2020-08-01 19:14:01
 */
#ifndef __DATA_QUEUE_H
#define __DATA_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif /* #ifdef __cplusplus */

#ifndef NULL
#define NULL ((void *)0)
#endif /* #ifdef NULL */

/* 
 * 数据队列模块的缓存空间大小
 * 作为简易 malloc 用于为队列分配数据空间
 * 决定了所有数据队列能用多少内存 
 */
#define __DATA_QUEUE_BUF_SIZE (3000)
/* 
 * 数据队列列表最大数量 
 * 决定了能创建多少数据队列
 */
#define __DATA_QUEUE_LIST_SIZE (10)

#define DQ_HANDLE_STATUS DQHandle.Stat
#define DQ_STATUS [num] DQHandle.DQList[num].Idle

    typedef unsigned int DQNum_t; // 数据队列编号, 0无效

    enum DQStatus
    {
        DQOk,
        DQBusy,
        DQErr,
        DQEmpty,
        DQFull
    };
    struct DQUnit
    {
        char *pHead;           // 数据读取点
        char *pTail;           // 数据插入点
        char *pData;           // 数据空间
        unsigned int DataSize; // 缓存大小
        unsigned int DataCnt;  // 缓存的数据数量
        char Idle;             // 忙标志
        enum DQStatus Stat;    // 保存操作结果
    };
    struct DQHandleInfo
    {
        char DQBuf[__DATA_QUEUE_BUF_SIZE];            // 数据队列模块缓存
        unsigned int DQBufFree;                       // 数据队列剩余缓存
        unsigned int DQCnt;                           // 数据队列单元
        struct DQUnit DQList[__DATA_QUEUE_LIST_SIZE]; // 数据队列单元列表
        char Idle;                                    // 数据队列控制块忙标志
        enum DQStatus Stat;                           // 保存操作结果
    };

    DQNum_t DQCreat(unsigned int Size);

    // void DQDelete(DQNum_t num);

    int DQPushDate(DQNum_t num, char *dat, unsigned int size);

    int DQPopData(DQNum_t num, char *dat, unsigned int size);

    int DPIsEmpty(DQNum_t num);
    int DQIsFull(DQNum_t num);

#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */

#endif /* __DATA_QUEUE_H */
